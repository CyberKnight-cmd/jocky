// Generates the website's data files from the repository.
//
//   data/docs.json      index of the docs folder, built from file names only
//   data/releases.json  GitHub releases, newest first
//
// Runs in GitHub Actions (see .github/workflows/website.yml) on every push to
// docs/ and every release, so the site never needs editing by hand.
// Requires Node 18+. No dependencies.

import { readdir, mkdir, writeFile } from "node:fs/promises";
import path from "node:path";

const repo = process.env.GITHUB_REPOSITORY || "CyberKnight-cmd/jocky";
const branch = process.env.DOCS_BRANCH || "main";
const docsDir = process.env.DOCS_DIR || "docs";            // where the docs are on disk
const docsRepoPath = process.env.DOCS_REPO_PATH || docsDir; // where they are in the repo (for links)
const outDir = process.env.OUT_DIR || "website/data";
const token = process.env.GITHUB_TOKEN;

/* ---------- Docs index ---------- */

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

// Turn a file name into { label, title, section, order } without opening the file.
function describe(file) {
  const base = file.replace(/\.(md|markdown|mdx)$/i, "");
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

async function walk(dir, rel = "") {
  let entries;
  try { entries = await readdir(path.join(dir, rel), { withFileTypes: true }); }
  catch { return []; }
  const out = [];
  for (const e of entries) {
    if (e.name.startsWith(".")) continue;
    const p = rel ? `${rel}/${e.name}` : e.name;
    if (e.isDirectory()) out.push(...(await walk(dir, p)));
    else if (/\.(md|markdown|mdx)$/i.test(e.name)) out.push(p);
  }
  return out;
}

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
      path: `${docsRepoPath}/${p}`,
      url: `https://github.com/${repo}/blob/${branch}/${docsRepoPath}/${p}`,
      order: d.order,
    });
  }
  const rank = s => ({ "Start here": 0, Chapters: 1, Guides: 2, Appendices: 9 })[s] ?? 5;
  const groups = [...sections.entries()]
    .sort(([a], [b]) => rank(a) - rank(b) || a.localeCompare(b))
    .map(([name, items]) => ({
      name,
      items: items
        .sort((a, b) => a.order - b.order || a.path.localeCompare(b.path))
        .map(({ order, ...rest }) => rest),
    }));
  return {
    generatedAt: new Date().toISOString(),
    commit: process.env.GITHUB_SHA || null,
    folderUrl: `https://github.com/${repo}/tree/${branch}/${docsRepoPath}`,
    count: files.length,
    groups,
  };
}

/* ---------- Releases ---------- */

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

/* ---------- Main ---------- */

await mkdir(outDir, { recursive: true });

const docs = await buildDocs();
await writeFile(path.join(outDir, "docs.json"), JSON.stringify(docs, null, 2) + "\n");
console.log(`docs.json: ${docs.count} files in ${docs.groups.length} sections`);

if (process.env.SKIP_RELEASES) {
  console.log("releases.json: skipped");
} else {
  const rel = await buildReleases();
  await writeFile(path.join(outDir, "releases.json"), JSON.stringify(rel, null, 2) + "\n");
  console.log(`releases.json: ${rel.releases.length} releases`);
}
