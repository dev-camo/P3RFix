# Resume maintenance and publish P3RFix 1.2.5

This ExecPlan is a living document. Keep Progress, Surprises & Discoveries, Decision Log, and Outcomes & Retrospective current as work proceeds.

## Purpose / Big Picture


Players will download maintained P3RFix builds from dev-camo/P3RFix. Pushing a version tag will build the source recorded by that tag and publish both the standalone and Reloaded-II installation archives. Release 1.2.5 must include commit cc2582b36b8c3fe37a47781ea5fc98638c3f75b9, which corrects aspect ratio tracking when the game's viewport changes size.

## Progress


- [x] (2026-09-30 22:28Z) Inspected clean main branch, origin, existing metadata, build scripts, and required fix; initialized pinned dependency submodules.
- [x] (2026-09-30 22:30Z) Enabled GitHub Issues, previously disabled on this fork, so support links have a working destination.
- [ ] Remove fundraising assets and update documentation, Reloaded-II update source, attribution, and dependency notices.
- [ ] Implement Windows builds, version injection, deterministic loader download, archives, and tag-triggered publication.
- [ ] Commit focused changes, push main, and observe the Windows build complete.
- [ ] Tag and push 1.2.5, observe successful publication, and inspect attached archives.
- [ ] Record final evidence and sync the completed plan.

## Surprises & Discoveries


The source already reports version 1.2.5, but no tags or releases exist on origin. Origin's main branch and local HEAD both began at cc2582b. The existing create_release.ps1 publishes to Forgejo and Codeberg using the original author's account; it has no GitHub publishing path and downloads an unpinned loader. README.md installation paths also disagree with release_body.md.

GitHub Actions is enabled, and the authenticated dev-camo account has repository administration and workflow access. The fork inherited disabled Issues. Its four external dependencies are Git submodules: directories whose exact revisions are recorded by the parent repository. Recursive initialization also retrieves Zydis's pinned Zycore dependency.

The original MIT copyright notice must remain. Third-party license terms differ, including SafetyHook's Boost Software License. Existing generated SDK provenance must remain visible.

## Decision Log


Decision: Continue the MIT license, retain original copyright, and add dev-camo's 2026 maintenance notice. Include dependency notices in distributed archives. Rationale: maintenance changes do not remove existing attribution or dependency requirements. Date/Author: 2026-09-30, Codex.

Decision: Use the bare tag 1.2.5 for the requested release; accept both numeric and v-prefixed future tags. Inject the normalized tag version into the native build and Reloaded-II metadata. Rationale: a released binary should identify the same version as its source tag, including future versions. Date/Author: 2026-09-30, Codex.

Decision: Build main without publishing before creating the release tag, and use the same build job for tagged releases. Rationale: the Linux development machine cannot perform the intended Windows MSVC build, and the first release tag should point to a source revision with a successful native build. Date/Author: 2026-09-30, Codex.

Decision: Separate archive creation from GitHub publication. Rationale: local builds should not create external releases, while GitHub's publication job needs write permission only after a successful build. Date/Author: 2026-09-30, Codex.

## Outcomes & Retrospective


Repository inspection and access checks are complete. Metadata cleanup and automation implementation are in progress. Publication remains outstanding; no release success is claimed until the remote workflow succeeds and its attachments are inspected.

## Context and Orientation


Work from /home/camo/Projects/P3RFix. src/dllmain.cpp contains the fix and its logged version. xmake.lua builds a Windows x64 ASI module, a DLL loaded into the game by Ultimate ASI Loader. create_release.ps1 currently builds and packages that module and an INI configuration. assets/r2-package/ModConfig.json describes the alternative Reloaded-II mod and its update source. README.md and release_body.md describe installation. LICENSE.md records project licensing. .github/workflows will contain GitHub Actions instructions, and CHANGELOG.md will describe release changes.

## Plan of Work


### Milestone 1: Maintained project metadata


Remove Patreon and Ko-fi links and their unused images. Point release, issue, project, and Reloaded-II update destinations at dev-camo/P3RFix. Document installation alongside P3R.exe: Steam uses P3R/Binaries/Win64, and Xbox uses Content/P3R/Binaries/WinGDK. Retain original and dependency credits. Add the maintenance copyright and exact dependency license copies under licenses/, indexed by THIRD_PARTY_NOTICES.md. Describe the viewport fix in CHANGELOG.md. Inspect git diff before committing each coherent group.

### Milestone 2: Build and release automation


Refactor create_release.ps1 into a noninteractive, fail-fast archive builder accepting -Version. Download an explicit Ultimate ASI Loader release and check its SHA256, a digest that detects changes to the downloaded bytes. Stage files under build/ and generate P3RFix_<version>.zip and P3RFix_Reloaded-II.zip, including notices. Pass the version through an xmake option into a C++ macro. Preserve a local default version. Create a reusable Windows build workflow and main-branch build workflow with read access. A separate release workflow runs on version-tag pushes, checks out the exact source commit with recursive dependencies, and publishes the successful build's archives using the GitHub CLI. Publication receives contents:write permission. Manual recovery accepts an existing tag and validates its target.

### Milestone 3: Publish and observe 1.2.5


Push focused commits to origin/main and observe the main build. Resolve any actual Windows build failures in additional focused commits. Confirm the required fix is an ancestor of the release commit. Create an annotated 1.2.5 tag at that commit, push it, and watch the release workflow. Download both release archives and inspect their contents, configuration version, and bundled notices. Record the public release URL and workflow result here.

## Concrete Steps


From the repository root, inspect changes and stage only the intended files for each small commit. Push the branch before tagging:

    git diff --check
    git push origin main
    gh run list --repo dev-camo/P3RFix --limit 5
    gh run view <run-id> --repo dev-camo/P3RFix
    git merge-base --is-ancestor cc2582b36b8c3fe37a47781ea5fc98638c3f75b9 HEAD
    git tag -a 1.2.5 -m "P3RFix 1.2.5"
    git push origin refs/tags/1.2.5
    gh release view 1.2.5 --repo dev-camo/P3RFix --json url,tagName,assets

The ancestry command must exit successfully. Expect the release to list P3RFix_1.2.5.zip and P3RFix_Reloaded-II.zip and to be public rather than a draft.

## Validation and Acceptance


Inspect diffs, JSON metadata, workflow permissions, tag checkout, version propagation, and license packaging. The end-to-end acceptance is an actual successful GitHub Windows build followed by a tag-triggered release with both attached ZIP files. Inspect downloaded ZIP contents and ensure Reloaded-II ModVersion is 1.2.5 and its update owner is dev-camo. Confirm standalone includes P3RFix.asi, P3RFix.ini, dsound.dll, and notices, and Reloaded-II includes P3RFix.asi, P3RFix.ini, ModConfig.json, and notices. In-game execution requires Persona 3 Reload and is outside this environment; do not claim it has been exercised.

## Idempotence and Recovery


Archive staging may be rebuilt under ignored build/. Re-running publication for an existing tag should update its attachments after checking that the tag still resolves to the intended commit. Do not force-push main or move a published tag. Inspect a failed workflow's logs before changing code. If a tag build needs correction, document the failure and recover without silently changing the source associated with the released version.

## Artifacts and Notes


Initial source evidence:

    origin: https://github.com/dev-camo/P3RFix.git
    branch: main
    HEAD: cc2582b36b8c3fe37a47781ea5fc98638c3f75b9
    subject: Fix aspect ratio tracking from viewport resize dimensions

## Interfaces and Dependencies


Use GitHub-hosted Windows MSVC, xmake, CMake, the repository's pinned inipp/spdlog/SafetyHook/Zydis/Zycore revisions, and a pinned Ultimate ASI Loader x64 release. create_release.ps1 -Version 1.2.5 is the packaging interface. GitHub CLI publishes an existing tag with both archives and release notes, using the workflow token. Reloaded-II retains ModId p3rpc.p3rfix and asset filename P3RFix_Reloaded-II.zip so the mod's identity remains stable.

Revision note (2026-09-30): Initial plan records repository findings, authorized scope, and observable release acceptance.
