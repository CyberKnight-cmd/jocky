# Publishing releases and docs to the JOCKY website

The website updates itself. You never edit HTML to announce a release or add a
docs page. The **Website** GitHub Action (`.github/workflows/website.yml`)
rebuilds and redeploys the site when:

| Trigger | What updates |
|---|---|
| A release is published, edited or deleted | Download button, downloads page, release history, news |
| A push to `main` changes anything in `docs/` | Documentation index on the Docs page |
| A push to `main` changes anything in `website/` | The whole site |
| Every 6 hours | Everything (catches files uploaded to a release after it was published) |
| Actions → Website → Run workflow | Everything, immediately |

A deploy takes about 1–2 minutes.

---

## Releasing a new version

### Checklist

1. **Draft first.** On GitHub go to Releases → *Draft a new release*.
2. **Tag it** with a version, e.g. `v0.2.0`.
3. **Write the release notes.** The first ~240 characters become the summary on
   the News page, so open with a plain sentence saying what changed.
4. **Attach the binaries** (see naming rules below) and, optionally, a
   checksums file.
5. **Tick "Set as a pre-release"** if it's a beta or release candidate.
6. **Publish.** The site updates within a couple of minutes.

Attach files *before* publishing. GitHub sends no event when files are added
to a release that's already published. If you do upload late, the site catches
up within 6 hours, or immediately if you run the workflow manually.

### Name your files by OS and CPU

The site reads each file name to decide which platform it's for. That decides
which file the **Download** button gives each visitor.

Recommended pattern:

```
jocky-<version>-<os>-<arch>.<ext>
```

| Platform | Example file name |
|---|---|
| Linux, x86-64 | `jocky-v0.2.0-linux-x86_64.tar.gz` |
| Linux, ARM64 | `jocky-v0.2.0-linux-arm64.tar.gz` |
| Windows, x86-64 | `jocky-v0.2.0-windows-x86_64.zip` |
| macOS, Apple silicon | `jocky-v0.2.0-macos-arm64.tar.gz` |
| macOS, Intel | `jocky-v0.2.0-macos-x86_64.tar.gz` |
| macOS, both | `jocky-v0.2.0-macos-universal.tar.gz` |

Words the site recognizes:

| | Recognized in the file name |
|---|---|
| macOS | `mac`, `macos`, `darwin`, `osx`, `apple`, `.pkg`, `.dmg` |
| Windows | `windows`, `win32`, `win64`, `-win-`, `.exe`, `.msi` |
| Linux | `linux`, `.deb`, `.rpm`, `appimage` |
| ARM64 | `arm64`, `aarch64` |
| x86-64 | `x86_64`, `x86-64`, `amd64`, `x64`, `win64` |
| Universal | `universal` |

A file with no OS in its name (e.g. just `jocky`) is still listed on the
downloads page as "Other", but the Download button can't pick it for anyone.

When a release has several files for the same OS, the Download button
prefers a universal build, then ARM64 on macOS and x86-64 on Windows and Linux.
Visitors can always pick another file from the table on the downloads page.

### Checksums and signatures

GitHub records a SHA-256 digest for every uploaded file, and the downloads
page shows it automatically. You don't need to do anything.

If you also attach a checksums or signature file, name it so the site lists it
separately instead of as a download: include `sha256`, `sha512` or `checksum`
in the name, or end it with `.sig`, `.asc` or `.minisig`
(e.g. `SHA256SUMS`, `checksums.txt`, `jocky-v0.2.0-linux-x86_64.tar.gz.sig`).

### Pre-releases

Releases marked **pre-release** appear in the release history and on the News
page with a *pre-release* label. The Download button always points to the
newest **stable** release. It only uses a pre-release if no stable release
exists yet.

### Fixing or removing a release

- **Edit** the notes or files on GitHub. The site rebuilds automatically.
- **Delete** the release on GitHub. It disappears from the site on the next rebuild.
- **Drafts** never appear on the site.

---

## Adding documentation

Put Markdown files in the repository's `docs/` folder and push to `main`.
Each file becomes a page on the website at `docs/<file>.html`, in the site's own
design. Every page gets:

- the chapter menu on the left and an "On this page" list of its `##` and `###` headings,
- Previous / Next links in chapter order,
- full-text search (the search box on the Docs page and on every docs page),
- "Last updated" (from git history), "Edit this page on GitHub" and "Report a problem" links.

The file name decides where a page appears in the menu:

| File name | Shown as | Section |
|---|---|---|
| `README.md` or `index.md` | Overview | Start here |
| `ch07_variables_and_scope.md` | Chapter 7 · Variables and Scope | Chapters (sorted by number) |
| `appendix_b_keywords.md` | Appendix B · Keywords | Appendices |
| `03_setup.md` | 3 · Setup | Guides |
| `faq.md` | FAQ | Guides |
| `guides/cross_compiling.md` | Cross Compiling | a section named after the folder ("Guides") |

Tips:

- Separate words with `_` or `-`. They become spaces and each word is capitalized.
- Common acronyms are capitalized automatically: API, ABI, AST, CLI, FFI, FAQ,
  IR, JIT, LLVM, OS, IO, JOCKY. `stdlib` becomes "Standard Library", `ref`
  becomes "Reference" and `jockyshield` becomes "JOCKYShield".
- Zero-pad chapter numbers (`ch01`, `ch02`, …). Sorting is numeric either way,
  but padded names also sort correctly on GitHub.
- Renaming or deleting a file updates the list on the next push.
- The page title is the file's first `# Heading`. If there isn't one, the title
  comes from the file name.

Writing tips:

- Link to other docs with their Markdown file names, e.g.
  `[Types](ch06_types.md#integers)`. They become links to the right page on the site.
- Images and other files inside `docs/` (e.g. `docs/img/diagram.png`) are published
  alongside the pages, so relative image links work.
- Links to files outside `docs/` (e.g. `../CONTRIBUTING.md`) go to GitHub.
- Use ```` ```console ```` for terminal sessions with `$` prompts. The Copy button
  then copies only the commands.

### Preview docs on your machine

From the `website` folder:

```bash
npm install                     # once
DOCS_DIR=../docs SITE_DIR=. SKIP_RELEASES=1 node scripts/build-data.mjs
python3 -m http.server 8000     # then open http://localhost:8000/docs.html
```

The generated `docs/` and `data/` folders are ignored by git. The Action builds
them fresh on every deploy.

---

## Troubleshooting

| Problem | Fix |
|---|---|
| Site didn't update after a release | Actions tab → check the latest **Website** run. Re-run it with *Run workflow*. |
| Release shows but has no download files | Files were uploaded after publishing. Run the workflow manually. |
| Download button says "See all files" | No file name matches the visitor's OS. Rename files using the table above. |
| A file shows as "Other" | Add the OS to its name. |
| Workflow fails at "deploy" | Settings → Pages → Source must be **GitHub Actions**. |
| Docs page shows "Couldn't load the documentation index" | The site was deployed without the workflow. Deploy through the Action. |
| A docs page is missing | The file must end in `.md` and live inside `docs/`. Check the Website run log for "docs: N pages". |
