/* ==========================================================================
   JOCKY website — shared script
   Releases, downloads and news are loaded live from GitHub Releases, so
   publishing a release on GitHub updates the site with no edits here.
   ========================================================================== */

const SITE = {
  name: "JOCKY",
  fullName: "Just-in-time Optimized Compiler Kernel for sYstems",
  owner: "CyberKnight-cmd",
  repoName: "jocky",
  year: new Date().getFullYear(),
};
SITE.repo = `https://github.com/${SITE.owner}/${SITE.repoName}`;
SITE.discussions = `${SITE.repo}/discussions`;
SITE.welcome = `${SITE.repo}/discussions/1`;
SITE.issues = `${SITE.repo}/issues`;
SITE.newIssue = `${SITE.repo}/issues/new`;
SITE.contributing = `${SITE.repo}/blob/main/CONTRIBUTING.md`;
SITE.docs = `${SITE.repo}/tree/main/docs`;
SITE.security = `${SITE.repo}/security/advisories/new`;
SITE.releases = `${SITE.repo}/releases`;
SITE.releasesFeed = `${SITE.repo}/releases.atom`;
SITE.api = `https://api.github.com/repos/${SITE.owner}/${SITE.repoName}`;

const LOGO = `<svg viewBox="0 0 48 48" aria-hidden="true"><rect width="48" height="48" rx="11" fill="var(--accent)"/><rect x="12" y="12" width="24" height="5.5" rx="2.75" fill="#fff"/><rect x="12" y="21.25" width="17" height="5.5" rx="2.75" fill="#fff" opacity=".85"/><rect x="12" y="30.5" width="9" height="5.5" rx="2.75" fill="#fff" opacity=".7"/></svg>`;
const ICON_SUN = `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" aria-hidden="true"><circle cx="12" cy="12" r="4"/><path d="M12 2v2M12 20v2M4.9 4.9l1.4 1.4M17.7 17.7l1.4 1.4M2 12h2M20 12h2M4.9 19.1l1.4-1.4M17.7 6.3l1.4-1.4"/></svg>`;
const ICON_MOON = `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" aria-hidden="true"><path d="M21 12.8A9 9 0 1 1 11.2 3a7 7 0 0 0 9.8 9.8z"/></svg>`;
const ICON_GH = `<svg viewBox="0 0 16 16" fill="currentColor" aria-hidden="true"><path d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0 0 16 8c0-4.42-3.58-8-8-8z"/></svg>`;

const esc = s => String(s).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;");

/* ---------- Header / footer ---------- */
function renderChrome() {
  const page = document.body.dataset.page || "";
  const nav = [
    ["about", "About", "about.html"],
    ["downloads", "Downloads", "downloads.html"],
    ["docs", "Documentation", "docs.html"],
    ["start", "Getting Started", "getting-started.html"],
    ["community", "Community", "community.html"],
    ["news", "News", "news.html"],
  ];
  const header = document.getElementById("site-header");
  if (header) {
    header.outerHTML = `
<a class="skip-link" href="#main">Skip to content</a>
<div class="meta-bar">
  <div class="wrap">
    <ul class="meta-tabs">
      <li><a href="index.html" aria-current="true">${SITE.name}</a></li>
      <li><a href="${SITE.docs}">Docs</a></li>
      <li><a href="${SITE.discussions}">Discussions</a></li>
      <li><a href="${SITE.releases}">Releases</a></li>
    </ul>
    <div class="meta-tools">
      <button class="icon-btn" id="theme-toggle" type="button" aria-label="Toggle dark mode"></button>
    </div>
  </div>
</div>
<header class="site-header">
  <div class="wrap header-row">
    <a class="logo" href="index.html" aria-label="${SITE.name} home">
      ${LOGO}
      <span><span class="logo-word">${SITE.name}</span><span class="logo-sub">${SITE.fullName}</span></span>
    </a>
    <a class="gh-btn" href="${SITE.repo}">${ICON_GH}<span>View on GitHub</span></a>
    <button class="nav-toggle" id="nav-toggle" type="button" aria-expanded="false" aria-controls="main-nav">Menu</button>
  </div>
  <nav class="main-nav" id="main-nav" aria-label="Main">
    <div class="wrap">
      <ul>${nav.map(([k, label, href]) => `<li><a href="${href}"${k === page ? ' aria-current="page"' : ""}>${label}</a></li>`).join("")}</ul>
    </div>
  </nav>
</header>`;
  }

  const footer = document.getElementById("site-footer");
  if (footer) {
    footer.outerHTML = `
<footer class="site-footer">
  <div class="wrap">
    <div class="footer-grid footer-grid-5">
      <div><h4>About</h4><ul>
        <li><a href="about.html">Overview</a></li>
        <li><a href="about.html#name">The name</a></li>
        <li><a href="about.html#focus">Focus areas</a></li>
        <li><a href="about.html#brand">Brand guidelines</a></li></ul></div>
      <div><h4>Downloads</h4><ul>
        <li><a href="downloads.html">Latest release</a></li>
        <li><a href="downloads.html#source">Build from source</a></li>
        <li><a href="${SITE.releases}">All releases on GitHub</a></li></ul></div>
      <div><h4>Documentation</h4><ul>
        <li><a href="getting-started.html">Getting started</a></li>
        <li><a href="${SITE.docs}">Language documentation</a></li>
        <li><a href="docs.html#pipeline">Compiler pipeline</a></li>
        <li><a href="docs.html#contributor-docs">Contributor guides</a></li></ul></div>
      <div><h4>Community</h4><ul>
        <li><a href="${SITE.discussions}">Discussions</a></li>
        <li><a href="${SITE.issues}">Issue tracker</a></li>
        <li><a href="community.html#conduct">Code of conduct</a></li>
        <li><a href="${SITE.security}">Report a vulnerability</a></li></ul></div>
      <div><h4>Contribute</h4><ul>
        <li><a href="${SITE.contributing}">Contributing guide</a></li>
        <li><a href="community.html#contribute">Ways to contribute</a></li>
        <li><a href="${SITE.repo}">Source code</a></li></ul></div>
    </div>
    <div class="footer-bottom">
      <span>© ${SITE.year} The ${SITE.name} contributors.</span>
      <span><a href="${SITE.repo}">GitHub</a> · <a href="${SITE.releasesFeed}">Releases feed</a> · <a href="#top">Back to top ↑</a></span>
    </div>
  </div>
</footer>`;
  }

  // Fill any link marked data-site="key" with the matching SITE URL.
  document.querySelectorAll("[data-site]").forEach(a => { if (SITE[a.dataset.site]) a.href = SITE[a.dataset.site]; });
}

/* ---------- Theme ---------- */
function initTheme() {
  const btn = document.getElementById("theme-toggle");
  if (!btn) return;
  const root = document.documentElement;
  let saved = null;
  try { saved = localStorage.getItem("theme"); } catch (e) {}
  if (saved) root.dataset.theme = saved;
  const isDark = () => root.dataset.theme ? root.dataset.theme === "dark" : matchMedia("(prefers-color-scheme: dark)").matches;
  const paint = () => { btn.innerHTML = isDark() ? ICON_SUN : ICON_MOON; btn.setAttribute("aria-label", isDark() ? "Switch to light mode" : "Switch to dark mode"); };
  paint();
  btn.addEventListener("click", () => {
    root.dataset.theme = isDark() ? "light" : "dark";
    try { localStorage.setItem("theme", root.dataset.theme); } catch (e) {}
    paint();
  });
}

function initNav() {
  const btn = document.getElementById("nav-toggle");
  const nav = document.getElementById("main-nav");
  if (!btn || !nav) return;
  btn.addEventListener("click", () => {
    const open = nav.classList.toggle("open");
    btn.setAttribute("aria-expanded", String(open));
  });
}

/* ---------- Shell code blocks ---------- */
function highlightShell(src) {
  return src.split("\n").map(line => {
    if (/^\s*#/.test(line)) return `<span class="tok-com">${esc(line)}</span>`;
    const m = line.match(/^(\$|PS>)\s(.*)$/);
    if (m) return `<span class="tok-prompt">${esc(m[1])} </span>${esc(m[2])}`;
    return `<span class="tok-out">${esc(line)}</span>`;
  }).join("\n");
}

function initCode() {
  document.querySelectorAll("pre[data-lang=sh]").forEach(pre => {
    pre.innerHTML = highlightShell(pre.textContent.replace(/^\n/, "").replace(/\s+$/, ""));
  });
  document.querySelectorAll(".code").forEach(block => {
    const pre = block.querySelector("pre");
    if (!pre) return;
    const btn = document.createElement("button");
    btn.type = "button";
    btn.className = "copy-btn";
    btn.textContent = "Copy";
    btn.addEventListener("click", () => {
      const cmds = pre.textContent.split("\n").filter(l => /^(\$|PS>) /.test(l)).map(l => l.replace(/^(\$|PS>) /, ""));
      const text = cmds.length ? cmds.join("\n") : pre.textContent;
      const done = () => { btn.textContent = "Copied"; setTimeout(() => (btn.textContent = "Copy"), 1600); };
      const fallback = () => { const r = document.createRange(); r.selectNodeContents(pre); const s = getSelection(); s.removeAllRanges(); s.addRange(r); btn.textContent = "Press Ctrl+C"; };
      try { navigator.clipboard.writeText(text).then(done, fallback); } catch (e) { fallback(); }
    });
    block.appendChild(btn);
  });
}

/* ---------- Home: pipeline animation ---------- */
function initPipeline() {
  const steps = [...document.querySelectorAll(".pipeline [data-stage]")];
  if (!steps.length || matchMedia("(prefers-reduced-motion: reduce)").matches) return;
  let i = 0;
  const tick = () => {
    steps.forEach((s, j) => { s.classList.toggle("done", j < i); s.classList.toggle("active", j === i); });
    i = (i + 1) % (steps.length + 2);
  };
  tick();
  setInterval(tick, 850);
}

/* ---------- GitHub Releases ----------
   data/releases.json is written by the GitHub Action on every release.
   If it is missing (e.g. the site is served without the Action), fall back
   to the GitHub API directly. */
let releasesPromise;
function getReleasesFromApi() {
  const KEY = "jocky-releases";
  try {
    const cached = JSON.parse(sessionStorage.getItem(KEY) || "null");
    if (cached && Date.now() - cached.at < 10 * 60 * 1000) return Promise.resolve(cached.data);
  } catch (e) {}
  return fetch(`${SITE.api}/releases?per_page=30`, { headers: { Accept: "application/vnd.github+json" } })
    .then(r => { if (!r.ok) throw new Error("GitHub API " + r.status); return r.json(); })
    .then(list => {
      const data = list.filter(r => !r.draft);
      try { sessionStorage.setItem(KEY, JSON.stringify({ at: Date.now(), data })); } catch (e) {}
      return data;
    });
}
function getReleases() {
  if (!releasesPromise) {
    releasesPromise = fetch("data/releases.json", { cache: "no-cache" })
      .then(r => { if (!r.ok) throw new Error("no releases.json"); return r.json(); })
      .then(d => d.releases)
      .catch(getReleasesFromApi);
  }
  return releasesPromise;
}

/* ---------- Docs index (data/docs.json, generated from the docs/ file tree) ---------- */
function initDocsIndex() {
  const el = document.getElementById("docs-index");
  if (!el) return;
  const side = document.getElementById("docs-side-index");
  fetch("data/docs.json", { cache: "no-cache" })
    .then(r => { if (!r.ok) throw new Error(); return r.json(); })
    .then(d => {
      if (!d.groups.length) { setState(el, `No documentation files found yet. <a href="${SITE.docs}">Browse the docs folder on GitHub</a>.`); return; }
      const slug = s => "docs-" + s.toLowerCase().replace(/[^a-z0-9]+/g, "-");
      el.innerHTML = d.groups.map(g => `
        <h3 id="${slug(g.name)}" class="doc-group">${esc(g.name)} <span class="muted">${g.items.length}</span></h3>
        <ol class="doc-list">${g.items.map(i => `
          <li><a href="${i.url}">${i.label ? `<span class="doc-label">${esc(i.label)}</span>` : ""}<span class="doc-title">${esc(i.title)}</span></a></li>`).join("")}
        </ol>`).join("") +
        `<p class="muted doc-meta">${d.count} documents · index updated ${fmtDate(d.generatedAt)}${d.commit ? ` from commit <a href="${SITE.repo}/commit/${d.commit}"><code>${d.commit.slice(0, 7)}</code></a>` : ""}</p>`;
      if (side) side.innerHTML = d.groups.map(g => `<li><a href="#${slug(g.name)}">${esc(g.name)}</a></li>`).join("");
    })
    .catch(() => setState(el, `Couldn't load the documentation index. <a href="${SITE.docs}">Browse the docs folder on GitHub</a>.`));
}
const latestStable = list => list.find(r => !r.prerelease) || list[0];
const relName = r => r.name || r.tag_name;
const fmtDate = iso => new Date(iso).toLocaleDateString("en-US", { year: "numeric", month: "short", day: "numeric" });
const fmtSize = b => b >= 1048576 ? (b / 1048576).toFixed(1) + " MB" : Math.max(1, Math.round(b / 1024)) + " KB";

function detectOS() {
  const p = (navigator.userAgentData && navigator.userAgentData.platform) || navigator.platform || navigator.userAgent;
  if (/win/i.test(p)) return "windows";
  if (/mac|iphone|ipad/i.test(p)) return "macos";
  if (/linux|x11|cros/i.test(p)) return "linux";
  return "";
}
const OS_LABEL = { windows: "Windows", macos: "macOS", linux: "Linux", "": "Other" };
// macOS is checked first: "darwin" contains "win".
function assetOS(name) {
  if (/mac|darwin|osx|apple|\.pkg$|\.dmg$/i.test(name)) return "macos";
  if (/windows|win32|win64|(^|[-_.])win([-_.]|$)|\.exe$|\.msi$/i.test(name)) return "windows";
  if (/linux|\.deb$|\.rpm$|appimage/i.test(name)) return "linux";
  return "";
}
function assetArch(name) {
  if (/universal/i.test(name)) return "Universal";
  if (/aarch64|arm64/i.test(name)) return "ARM64";
  if (/x86[_-]?64|amd64|x64|win64/i.test(name)) return "x86-64";
  if (/i[3-6]86|x86|win32/i.test(name)) return "x86";
  return "";
}
const isChecksum = name => /sha256|sha512|checksum|\.sig$|\.asc$|\.minisig$|\.sbom|\.intoto/i.test(name);
const platformLabel = name => [OS_LABEL[assetOS(name)], assetArch(name)].filter(Boolean).join(" · ") || "Other";

// Best file for the visitor's OS. Browsers don't reliably report CPU type, so
// prefer universal builds, then ARM64 on macOS (Apple silicon) and x86-64 elsewhere.
function pickAsset(assets, os) {
  const pref = os === "macos" ? ["Universal", "ARM64", "x86-64", ""] : ["x86-64", "Universal", "", "ARM64", "x86"];
  const rank = a => { const i = pref.indexOf(assetArch(a.name)); return i < 0 ? 99 : i; };
  return assets.filter(a => !isChecksum(a.name) && os && assetOS(a.name) === os).sort((a, b) => rank(a) - rank(b))[0];
}

function releaseExcerpt(body, n = 240) {
  const text = (body || "")
    .replace(/```[\s\S]*?```/g, " ")
    .replace(/!\[[^\]]*\]\([^)]*\)/g, " ")
    .replace(/\[([^\]]+)\]\([^)]*\)/g, "$1")
    .replace(/[#>*_`~|-]+/g, " ")
    .replace(/\s+/g, " ")
    .trim();
  return text.length > n ? text.slice(0, n).replace(/\s\S*$/, "") + "…" : text;
}

function setState(el, html) { if (el) el.innerHTML = `<p class="state">${html}</p>`; }
const NO_RELEASES = `No releases have been published yet. You can <a href="downloads.html#source">build ${SITE.name} from source</a> today, and follow progress in <a href="${SITE.discussions}">Discussions</a>.`;
const OFFLINE = `Couldn't load releases from GitHub right now. <a href="${SITE.releases}">View releases on GitHub</a>.`;

/* Download buttons and version labels (home + downloads) */
function initDownloadButtons() {
  const buttons = document.querySelectorAll("[data-download-primary]");
  const metas = document.querySelectorAll("[data-release-meta]");
  if (!buttons.length && !metas.length) return;
  getReleases().then(list => {
    const rel = latestStable(list);
    if (!rel) {
      buttons.forEach(a => { a.href = "downloads.html#source"; a.innerHTML = `<span>Build ${SITE.name} from source</span><small>No binary release yet</small>`; });
      metas.forEach(m => (m.innerHTML = `<span>No releases yet</span><span><a href="${SITE.repo}">Follow on GitHub →</a></span>`));
      return;
    }
    const os = detectOS();
    const asset = pickAsset(rel.assets, os);
    buttons.forEach(a => {
      a.href = asset ? asset.browser_download_url : "downloads.html#dl-title";
      a.innerHTML = `<span>Download ${SITE.name} ${esc(rel.tag_name)}</span><small>${asset ? `for ${platformLabel(asset.name)} · ${fmtSize(asset.size)}` : "See all files"}</small>`;
    });
    metas.forEach(m => (m.innerHTML = `<span>Latest <b>${esc(rel.tag_name)}</b></span><span>Released <b>${fmtDate(rel.published_at)}</b></span><span><a href="downloads.html#all-releases">Other versions →</a></span>`));
  }).catch(() => {
    buttons.forEach(a => { a.href = SITE.releases; });
    metas.forEach(m => (m.innerHTML = `<span><a href="${SITE.releases}">Releases on GitHub →</a></span>`));
  });
}

/* Downloads page tables */
function initDownloadsPage() {
  const filesEl = document.getElementById("dl-files");
  if (!filesEl) return;
  const allEl = document.getElementById("dl-all");
  const titleEl = document.getElementById("dl-title");
  const os = detectOS();
  getReleases().then(list => {
    const rel = latestStable(list);
    if (!rel) { setState(filesEl, NO_RELEASES); setState(allEl, "Once releases are published on GitHub they will be listed here automatically."); return; }
    if (titleEl) titleEl.textContent = `Files for ${SITE.name} ${rel.tag_name}`;
    const best = pickAsset(rel.assets, os);
    const files = rel.assets.filter(a => !isChecksum(a.name))
      .sort((a, b) => (b === best) - (a === best) || (assetOS(b.name) === os) - (assetOS(a.name) === os) || a.name.localeCompare(b.name));
    const checks = rel.assets.filter(a => isChecksum(a.name));
    const row = (label, href, platform, size, digest, mine) => `
      <tr>
        <td><a href="${href}">${esc(label)}</a>${mine ? ' <span class="pill pill-ok">your system</span>' : ""}</td>
        <td>${platform}</td>
        <td class="num">${size}</td>
        <td class="sums">${digest ? esc(digest.replace(/^sha256:/, "")) : "—"}</td>
      </tr>`;
    const rows = files.map(a => row(a.name, a.browser_download_url, platformLabel(a.name), fmtSize(a.size), a.digest, a === best)).join("")
      + row("Source code (zip)", rel.zipball_url, "Any", "—", "", false)
      + row("Source code (tar.gz)", rel.tarball_url, "Any", "—", "", false);
    filesEl.innerHTML = `
      <div class="table-wrap"><table>
        <thead><tr><th>File</th><th>Platform</th><th class="num">Size</th><th>SHA-256</th></tr></thead>
        <tbody>${rows}</tbody>
      </table></div>
      ${checks.length ? `<p class="muted">Checksums and signatures: ${checks.map(c => `<a href="${c.browser_download_url}">${esc(c.name)}</a>`).join(", ")}</p>` : ""}
      <p class="muted">Released ${fmtDate(rel.published_at)} · <a href="${rel.html_url}">Release notes on GitHub</a></p>`;

    allEl.innerHTML = `
      <div class="table-wrap"><table>
        <thead><tr><th>Release</th><th class="num">Date</th><th>Type</th><th>Links</th></tr></thead>
        <tbody>${list.map(r => `
          <tr>
            <td><b>${esc(relName(r))}</b></td>
            <td class="num">${fmtDate(r.published_at)}</td>
            <td>${r.prerelease ? '<span class="pill pill-accent">pre-release</span>' : r === rel ? '<span class="pill pill-ok">latest</span>' : '<span class="pill pill-muted">stable</span>'}</td>
            <td><a href="${r.html_url}">Notes and files</a></td>
          </tr>`).join("")}</tbody>
      </table></div>`;
  }).catch(() => { setState(filesEl, OFFLINE); setState(allEl, OFFLINE); });
}

/* News lists (home feed + news page) */
function initNews() {
  const feed = document.getElementById("news-feed");
  const posts = document.getElementById("news-posts");
  if (!feed && !posts) return;
  getReleases().then(list => {
    if (!list.length) { setState(feed, NO_RELEASES); setState(posts, NO_RELEASES); return; }
    if (feed) feed.innerHTML = `<ul class="feed">${list.slice(0, 5).map(r => `
      <li><time datetime="${r.published_at}">${r.published_at.slice(0, 10)}</time><a href="${r.html_url}">${SITE.name} ${esc(relName(r))} ${r.prerelease ? "pre-release is available for testing" : "released"}</a></li>`).join("")}</ul>`;
    if (posts) posts.innerHTML = list.map(r => `
      <article class="post">
        <time datetime="${r.published_at}">${fmtDate(r.published_at)}</time>
        <div>
          <h2><a href="${r.html_url}">${SITE.name} ${esc(relName(r))}</a></h2>
          ${r.body ? `<p>${esc(releaseExcerpt(r.body))}</p>` : ""}
          <div class="tags">${r.prerelease ? '<span class="pill pill-accent">pre-release</span>' : '<span class="pill pill-ok">release</span>'}<a href="${r.html_url}">Read the release notes →</a></div>
        </div>
      </article>`).join("");
  }).catch(() => { setState(feed, OFFLINE); setState(posts, OFFLINE); });
}

document.documentElement.id = "top";
renderChrome();
initTheme();
initNav();
initCode();
initPipeline();
initDownloadButtons();
initDownloadsPage();
initNews();
initDocsIndex();
