// Builds the generated parts of the JOCKY website from the repository.
//
//   docs/**/*.html            one page per Markdown file in the repo's docs/ folder
//   data/docs.json            the documentation index (sections, titles, links)
//   data/search-index.json    headings and text used by the docs search box
//   data/releases.json        GitHub releases, newest first
//
// Runs in GitHub Actions (see .github/workflows/website.yml) on every push to
// docs/ or website/ and on every release, so the site never needs editing by hand.
//
// Local use (from the website folder):
//   npm install
//   DOCS_DIR=../docs SITE_DIR=. node scripts/build-data.mjs

import { readdir, readFile, mkdir, writeFile, cp, rm } from "node:fs/promises";
import { execFileSync } from "node:child_process";
import path from "node:path";
import { Marked } from "marked";

const repo = process.env.GITHUB_REPOSITORY || "CyberKnight-cmd/jocky";
const branch = process.env.DOCS_BRANCH || "main";
const docsDir = process.env.DOCS_DIR || "docs";              // where the Markdown docs are on disk
const docsRepoPath = process.env.DOCS_REPO_PATH || "docs";   // where they live in the repo (for GitHub links)
const siteDir = process.env.SITE_DIR || "website";           // the website folder
const token = process.env.GITHUB_TOKEN;

const dataDir = path.join(siteDir, "data");
const pagesDir = path.join(siteDir, "docs");
const posix = path.posix;
const MD_RE = /\.(md|markdown|mdx)$/i;

const esc = s => String(s).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;");

/* ======================================================================
   Titles from file names
   ====================================================================== */

const UPPER = new Set(["api", "abi", "ast", "cli", "ffi", "faq", "ir", "jit", "llvm", "os", "ui", "io", "jocky"]);
const SMALL = new Set(["a", "an", "and", "as", "for", "in", "of", "on", "or", "the", "to", "vs"]);
const WORDS = { stdlib: "Standard Library", ref: "Reference", jockyshield: "JOCKYShield" };

function titleCase(slug) {
  return slug
    .split(/[_\-\s]+/)
    .filter(Boolean)
    .map((w, i) => {
      const lw = w.toLowerCase();
      if (WORDS[lw]) return WORDS[lw];
      if (UPPER.has(lw)) return lw.toUpperCase();
      if (i > 0 && SMALL.has(lw)) return lw;
      return lw[0].toUpperCase() + lw.slice(1);
    })
    .join(" ");
}

// Turn a file name into { label, title, section, order }.
function describe(file) {
  const base = file.replace(MD_RE, "");
  let m;
  if (/^(readme|index)$/i.test(base)) return { label: "", title: "Overview", section: "Start here", order: -1 };
  if ((m = base.match(/^ch(?:apter)?[_\-]?(\d+)[_\-]+(.+)$/i)))
    return { label: `Chapter ${Number(m[1])}`, title: titleCase(m[2]), section: "Chapters", order: Number(m[1]) };
  if ((m = base.match(/^appendix[_\-]?([a-z0-9]+)[_\-]+(.+)$/i)))
    return { label: `Appendix ${m[1].toUpperCase()}`, title: titleCase(m[2]), section: "Appendices", order: parseInt(m[1], 36) };
  if ((m = base.match(/^(\d+)[_\-]+(.+)$/)))
    return { label: String(Number(m[1])), title: titleCase(m[2]), section: "", order: Number(m[1]) };
  return { label: "", title: titleCase(base), section: "", order: 1e6 };
}

// docs/ch05_types.md -> ch05_types.html, docs/README.md -> index.html
const pageFor = rel => rel.replace(/(^|\/)(readme|index)\.(md|markdown|mdx)$/i, "$1index.html").replace(MD_RE, ".html");

async function walk(dir, rel = "") {
  let entries;
  try { entries = await readdir(path.join(dir, rel), { withFileTypes: true }); }
  catch { return []; }
  const out = [];
  for (const e of entries) {
    if (e.name.startsWith(".")) continue;
    const p = rel ? `${rel}/${e.name}` : e.name;
    if (e.isDirectory()) out.push(...(await walk(dir, p)));
    else if (MD_RE.test(e.name)) out.push(p);
  }
  return out;
}

function lastUpdated(rel) {
  try {
    const out = execFileSync("git", ["log", "-1", "--format=%cI", "--", path.join(docsDir, rel)], { encoding: "utf8", stdio: ["ignore", "pipe", "ignore"] }).trim();
    return out || null;
  } catch { return null; }
}

/* ======================================================================
   Markdown rendering
   ====================================================================== */

function slugify(text) {
  return text.toLowerCase().replace(/<[^>]+>/g, "").replace(/[^\p{L}\p{N}\s-]/gu, "").trim().replace(/\s+/g, "-") || "section";
}

const plain = s => s
  .replace(/```[\s\S]*?```/g, " ")
  .replace(/`([^`]*)`/g, "$1")
  .replace(/!\[[^\]]*\]\([^)]*\)/g, " ")
  .replace(/\[([^\]]+)\]\([^)]*\)/g, "$1")
  .replace(/<[^>]+>/g, " ")
  .replace(/[#>*_~|]+/g, " ")
  .replace(/\s+/g, " ")
  .trim();

// Resolve a link found in docs/<fromRel> to where it should point on the site.
function resolveLink(href, fromRel, kind) {
  if (!href || /^([a-z][a-z0-9+.-]*:|#|\/\/)/i.test(href)) return href;           // absolute URL, anchor, mailto…
  const [p, hash = ""] = href.split("#");
  const h = hash ? "#" + hash : "";
  if (!p) return h;
  const fromDir = posix.dirname(fromRel);
  const target = posix.normalize(posix.join(fromDir, decodeURI(p)));
  if (target.startsWith("..")) {                                                   // outside docs/ -> GitHub
    const repoPath = posix.normalize(posix.join(docsRepoPath, target));
    return kind === "image"
      ? `https://raw.githubusercontent.com/${repo}/${branch}/${repoPath}`
      : `https://github.com/${repo}/blob/${branch}/${repoPath}${h}`;
  }
  if (MD_RE.test(target)) return posix.relative(fromDir, pageFor(target)) + h;     // another doc -> its page
  return p + h;                                                                    // image or file copied alongside
}

function renderMarkdown(src, rel) {
  const md = new Marked({ gfm: true });
  md.use({
    renderer: {
      heading(t) {
        const inner = this.parser.parseInline(t.tokens);
        if (t.depth === 1) return `<h1 id="${t.anchorId}">${inner}</h1>\n`;
        return `<h${t.depth} id="${t.anchorId}"><a class="anchor" href="#${t.anchorId}" aria-hidden="true">#</a>${inner}</h${t.depth}>\n`;
      },
      link(t) {
        const href = resolveLink(t.href, rel, "link");
        const external = /^https?:/i.test(href);
        return `<a href="${esc(href)}"${t.title ? ` title="${esc(t.title)}"` : ""}${external ? ' rel="noopener"' : ""}>${this.parser.parseInline(t.tokens)}</a>`;
      },
      image(t) {
        return `<img src="${esc(resolveLink(t.href, rel, "image"))}" alt="${esc(t.text)}"${t.title ? ` title="${esc(t.title)}"` : ""} loading="lazy">`;
      },
      code(t) {
        const lang = (t.lang || "").trim().split(/\s+/)[0];
        const label = lang ? `<span class="code-label">${esc(lang)}</span>` : "";
        const shell = /^(console|shell-session)$/i.test(lang) ? ' data-lang="sh"' : "";
        return `<div class="code">${label}<pre${shell}><code>${esc(t.text)}</code></pre></div>\n`;
      },
      table(t) {
        const cell = (c, tag) => `<${tag}${c.align ? ` style="text-align:${c.align}"` : ""}>${this.parser.parseInline(c.tokens)}</${tag}>`;
        const head = `<tr>${t.header.map(c => cell(c, "th")).join("")}</tr>`;
        const body = t.rows.map(r => `<tr>${r.map(c => cell(c, "td")).join("")}</tr>`).join("");
        return `<div class="table-wrap"><table><thead>${head}</thead><tbody>${body}</tbody></table></div>\n`;
      },
    },
  });

  const tokens = md.lexer(src);
  const used = new Map();
  const toc = [];
  const sections = [];
  let h1 = null;
  let current = { heading: "", anchor: "", text: "" };
  sections.push(current);

  // Give every heading a unique id, and collect the on-page TOC and search text.
  for (const t of tokens) {
    if (t.type === "heading") {
      const text = plain(t.text);
      let id = slugify(text);
      const n = used.get(id) || 0;
      used.set(id, n + 1);
      if (n) id += "-" + n;
      t.anchorId = id;
      if (t.depth === 1 && !h1) h1 = text;
      if (t.depth === 2 || t.depth === 3) toc.push({ depth: t.depth, text, id });
      current = { heading: text, anchor: id, text: "" };
      sections.push(current);
    } else if (t.type !== "code" && t.type !== "space" && t.type !== "hr") {
      if (current.text.length < 600) current.text += " " + plain(t.raw);
    }
  }
  md.walkTokens(tokens, t => {
    if (t.type === "heading" && !t.anchorId) {                // headings nested in lists/quotes
      t.anchorId = slugify(plain(t.text)) + "-" + Math.random().toString(36).slice(2, 6);
    }
  });

  const html = md.parser(tokens);
  return {
    html,
    h1,
    toc,
    sections: sections
      .map(s => ({ ...s, text: s.text.trim().slice(0, 600) }))
      .filter(s => s.heading || s.text),
  };
}

/* ======================================================================
   Page template
   ====================================================================== */

function docPage({ doc, rendered, groups, prev, next, updated }) {
  const depth = doc.page.split("/").length;              // docs/<page> -> ../ ; docs/a/<page> -> ../../
  const root = "../".repeat(depth);
  const toDoc = other => posix.relative(posix.dirname(doc.page), other.page) || posix.basename(other.page);
  const title = rendered.h1 || doc.title;

  const nav = groups.map(g => `
      <h4>${esc(g.name)}</h4>
      <ul>${g.items.map(i => `<li><a href="${esc(toDoc(i))}"${i.page === doc.page ? ' aria-current="page"' : ""}>${i.label ? `<span class="doc-nav-label">${esc(i.label.replace(/^(Chapter|Appendix) /, ""))}</span>` : ""}${esc(i.title)}</a></li>`).join("")}</ul>`).join("");

  const toc = rendered.toc.length > 1
    ? `<aside class="doc-toc" aria-label="On this page"><h4>On this page</h4><ul>${rendered.toc.map(t => `<li class="lvl-${t.depth}"><a href="#${t.id}">${esc(t.text)}</a></li>`).join("")}</ul></aside>`
    : `<aside class="doc-toc" aria-hidden="true"></aside>`;

  const pager = (d, dir) => d
    ? `<a class="doc-pager-${dir}" href="${esc(toDoc(d))}"><span>${dir === "prev" ? "← Previous" : "Next →"}</span><b>${esc(d.label ? `${d.label} · ${d.title}` : d.title)}</b></a>`
    : "<span></span>";

  return `<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<title>${esc(title)} · JOCKY Documentation</title>
<meta name="description" content="${esc((rendered.sections.find(s => s.text)?.text || title).slice(0, 155))}">
<link rel="icon" href="${root}assets/img/favicon.svg" type="image/svg+xml">
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Archivo:wght@500;700;800&family=IBM+Plex+Sans:ital,wght@0,400;0,500;0,600;1,400&family=JetBrains+Mono:wght@400;500;600&display=swap">
<link rel="stylesheet" href="${root}assets/css/site.css">
</head>
<body data-page="docs" data-root="${root}">
<div id="site-header"></div>

<main id="main">
  <div class="wrap doc-page">
    <aside class="doc-side">
      <div class="doc-search" data-doc-search>
        <label class="skip-link" for="doc-search-input">Search the documentation</label>
        <input id="doc-search-input" type="search" placeholder="Search docs" autocomplete="off">
        <div class="doc-search-results" hidden></div>
      </div>
      <details class="doc-nav" data-doc-nav>
        <summary>Documentation menu</summary>
        <nav aria-label="Documentation">${nav}
        </nav>
      </details>
    </aside>

    <article class="doc-content">
      <nav class="breadcrumbs" aria-label="Breadcrumb"><a href="${root}index.html">Home</a><span>/</span><a href="${root}docs.html">Documentation</a><span>/</span><span>${esc(doc.label || title)}</span></nav>
      ${doc.label ? `<span class="eyebrow">${esc(doc.label)}</span>` : ""}
      ${rendered.h1 ? "" : `<h1>${esc(doc.title)}</h1>`}
      ${rendered.html}
      <footer class="doc-foot">
        <span>${updated ? `Last updated ${new Date(updated).toLocaleDateString("en-US", { year: "numeric", month: "long", day: "numeric" })}` : ""}</span>
        <span><a href="${esc(doc.editUrl)}">Edit this page on GitHub</a> · <a href="https://github.com/${repo}/issues/new?title=${encodeURIComponent("Docs: " + title)}">Report a problem</a></span>
      </footer>
      <nav class="doc-pager" aria-label="Previous and next pages">${pager(prev, "prev")}${pager(next, "next")}</nav>
    </article>

    ${toc}
  </div>
</main>

<div id="site-footer"></div>
<script src="${root}assets/js/site.js"></script>
</body>
</html>
`;
}

/* ======================================================================
   Build docs
   ====================================================================== */

async function buildDocs() {
  const files = await walk(docsDir);
  const sections = new Map();
  for (const p of files) {
    const parts = p.split("/");
    const d = describe(parts.at(-1));
    // Files in sub-folders are grouped under the folder's name.
    const section = parts.length > 1 ? titleCase(parts.slice(0, -1).join(" ")) : d.section || "Guides";
    if (!sections.has(section)) sections.set(section, []);
    sections.get(section).push({
      label: d.label,
      title: d.title,
      source: p,
      page: pageFor(p),
      path: `${docsRepoPath}/${p}`,
      url: `https://github.com/${repo}/blob/${branch}/${docsRepoPath}/${p}`,
      editUrl: `https://github.com/${repo}/edit/${branch}/${docsRepoPath}/${p}`,
      order: d.order,
    });
  }
  const rank = s => ({ "Start here": 0, Chapters: 1, Guides: 2, Appendices: 9 })[s] ?? 5;
  const groups = [...sections.entries()]
    .sort(([a], [b]) => rank(a) - rank(b) || a.localeCompare(b))
    .map(([name, items]) => ({ name, items: items.sort((a, b) => a.order - b.order || a.source.localeCompare(b.source)) }));
  const ordered = groups.flatMap(g => g.items);

  // Fresh output folder; copy images and other non-Markdown files alongside the pages.
  await rm(pagesDir, { recursive: true, force: true });
  await mkdir(pagesDir, { recursive: true });
  if (files.length) await cp(docsDir, pagesDir, { recursive: true, filter: src => !MD_RE.test(src) && !path.basename(src).startsWith(".") });

  const search = [];
  for (let i = 0; i < ordered.length; i++) {
    const doc = ordered[i];
    const src = await readFile(path.join(docsDir, doc.source), "utf8");
    const rendered = renderMarkdown(src, doc.source);
    const out = path.join(pagesDir, doc.page);
    await mkdir(path.dirname(out), { recursive: true });
    await writeFile(out, docPage({ doc, rendered, groups, prev: ordered[i - 1], next: ordered[i + 1], updated: lastUpdated(doc.source) }));
    search.push({
      page: `docs/${doc.page}`,
      title: rendered.h1 || doc.title,
      label: doc.label,
      sections: rendered.sections,
    });
  }

  const index = {
    generatedAt: new Date().toISOString(),
    commit: process.env.GITHUB_SHA || null,
    folderUrl: `https://github.com/${repo}/tree/${branch}/${docsRepoPath}`,
    count: files.length,
    groups: groups.map(g => ({
      name: g.name,
      items: g.items.map(({ label, title, page, path: p, url }) => ({ label, title, page: `docs/${page}`, path: p, url })),
    })),
  };
  return { index, search };
}

/* ======================================================================
   Releases
   ====================================================================== */

async function buildReleases() {
  const headers = { Accept: "application/vnd.github+json", "User-Agent": "jocky-website" };
  if (token) headers.Authorization = `Bearer ${token}`;
  const all = [];
  for (let page = 1; page <= 10; page++) {
    const res = await fetch(`https://api.github.com/repos/${repo}/releases?per_page=100&page=${page}`, { headers });
    if (!res.ok) throw new Error(`GitHub API ${res.status}: ${await res.text()}`);
    const batch = await res.json();
    all.push(...batch);
    if (batch.length < 100) break;
  }
  const releases = all
    .filter(r => !r.draft)
    .sort((a, b) => new Date(b.published_at) - new Date(a.published_at))
    .map(r => ({
      name: r.name,
      tag_name: r.tag_name,
      prerelease: r.prerelease,
      published_at: r.published_at,
      html_url: r.html_url,
      body: (r.body || "").slice(0, 4000),
      zipball_url: r.zipball_url,
      tarball_url: r.tarball_url,
      assets: r.assets.map(a => ({ name: a.name, size: a.size, browser_download_url: a.browser_download_url, digest: a.digest || null })),
    }));
  return { generatedAt: new Date().toISOString(), releases };
}

/* ======================================================================
   Main
   ====================================================================== */

await mkdir(dataDir, { recursive: true });

const { index, search } = await buildDocs();
await writeFile(path.join(dataDir, "docs.json"), JSON.stringify(index, null, 2) + "\n");
await writeFile(path.join(dataDir, "search-index.json"), JSON.stringify(search) + "\n");
console.log(`docs: ${index.count} pages in ${index.groups.length} sections`);

if (process.env.SKIP_RELEASES) {
  console.log("releases: skipped");
} else {
  const rel = await buildReleases();
  await writeFile(path.join(dataDir, "releases.json"), JSON.stringify(rel, null, 2) + "\n");
  console.log(`releases: ${rel.releases.length}`);
}
