# Resume maintenance and publish P3RFix 1.2.5

This ExecPlan is a living document. Keep Progress, Surprises & Discoveries, Decision Log, and Outcomes & Retrospective current as work proceeds.

## Purpose / Big Picture


Players will download maintained P3RFix builds from dev-camo/P3RFix. Pushing a version tag will build the source recorded by that tag and publish both the standalone and Reloaded-II installation archives. Release 1.2.5 must include commit cc2582b36b8c3fe37a47781ea5fc98638c3f75b9, which corrects aspect ratio tracking when the game's viewport changes size.

## Progress


- [x] (2026-09-30 22:28Z) Inspected clean main branch, origin, existing metadata, build scripts, and required fix; initialized pinned dependency submodules.
- [x] (2026-09-30 22:30Z) Enabled GitHub Issues, previously disabled on this fork, so support links have a working destination.
- [x] (2026-09-30 22:34Z) Removed fundraising assets; updated documentation, Reloaded-II update source, attribution, and dependency notices; pushed four focused commits to origin/main.
- [x] (2026-09-30 22:37Z) Implemented Windows builds, version injection, pinned and verified loader download, archives, and tag-triggered publication; reviewed locally.
- [x] (2026-09-30 22:40Z) Committed focused automation changes, pushed main, and observed Windows build 36786715366 succeed at 93815d9; inspected both build archives and generated release notes.
- [x] (2026-09-30 22:43Z) Updated deprecated Action runtimes; Windows preflight build 36787078874 succeeded at 12baef999df058c56bebe68e017396bceaefa7d0.
- [x] (2026-09-30 22:46Z) Tagged and pushed annotated 1.2.5; publication workflow 36787299471 succeeded and attached both archives to a public release.
- [x] (2026-09-30 22:48Z) Downloaded published archives, verified their checksums and contents, and recorded final evidence for the completion commit.

## Surprises & Discoveries


The source already reports version 1.2.5, but no tags or releases exist on origin. Origin's main branch and local HEAD both began at cc2582b. The existing create_release.ps1 publishes to Forgejo and Codeberg using the original author's account; it has no GitHub publishing path and downloads an unpinned loader. README.md installation paths also disagree with release_body.md.

GitHub Actions is enabled, and the authenticated dev-camo account has repository administration and workflow access. The fork inherited disabled Issues. Its four external dependencies are Git submodules: directories whose exact revisions are recorded by the parent repository. Recursive initialization also retrieves Zydis's pinned Zycore dependency.

The original MIT copyright notice must remain. Third-party license terms differ, including SafetyHook's Boost Software License. Existing generated SDK provenance must remain visible.

The pinned Zydis CMake build must use the same static MSVC runtime as P3RFix. Its default differs from xmake's /MT setting, so the build now explicitly sets CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded and skips examples, tools, and tests. Both Zydis and Zycore already enable the CMake policy needed for this setting. Annotated tags may involve both a tag object and its underlying commit; verification compares the underlying commits.

The first remote build passed in 1 minute 27 seconds. GitHub reported that pinned checkout/upload Actions still target deprecated Node.js 20 and were forced onto Node.js 24. Their pins and the corresponding download Action were updated before tagging. Downloaded build archives each contain 16 files, including 11 dependency license files, and Reloaded-II metadata reports version 1.2.5 and update owner dev-camo.

Published notice and configuration text uses Windows checkout line endings. Comparing those files to Linux source requires normalizing CRLF to LF; after normalization their contents match. Both ZIP SHA256 digests match GitHub's published digests, their archive integrity checks pass, and both contain the same Windows x64 ASI with the 1.2.5 version string.

## Decision Log


Decision: Continue the MIT license, retain original copyright, and add dev-camo's 2026 maintenance notice. Include dependency notices in distributed archives. Rationale: maintenance changes do not remove existing attribution or dependency requirements. Date/Author: 2026-09-30, Codex.

Decision: Use the bare tag 1.2.5 for the requested release; accept both numeric and v-prefixed future tags. Inject the normalized tag version into the native build and Reloaded-II metadata. Rationale: a released binary should identify the same version as its source tag, including future versions. Date/Author: 2026-09-30, Codex.

Decision: Build main without publishing before creating the release tag, and use the same build job for tagged releases. Rationale: the Linux development machine cannot perform the intended Windows MSVC build, and the first release tag should point to a source revision with a successful native build. Date/Author: 2026-09-30, Codex.

Decision: Separate archive creation from GitHub publication. Rationale: local builds should not create external releases, while GitHub's publication job needs write permission only after a successful build. Date/Author: 2026-09-30, Codex.

Decision: Pin Ultimate ASI Loader v9.7.4 and xmake 3.1.1, and pin third-party Actions by commit. Rationale: version-tag builds should use recorded dependency versions instead of changing latest downloads. Date/Author: 2026-09-30, Codex.

Decision: Use checkout v7.0.1, upload-artifact v7.0.1, and download-artifact v8.0.1 with verified commit pins. Rationale: these supported Actions use Node.js 24, resolving the runtime deprecation observed during the initial build. Date/Author: 2026-09-30, Codex.

## Outcomes & Retrospective


P3RFix 1.2.5 is publicly available at https://github.com/dev-camo/P3RFix/releases/tag/1.2.5 and is the repository's latest release. Both P3RFix_1.2.5.zip and P3RFix_Reloaded-II.zip are attached. The annotated tag points to 12baef999df058c56bebe68e017396bceaefa7d0, whose history includes the required cc2582b fix and eight focused maintenance commits. The native build and publishing jobs passed; downloaded published artifacts passed checksum, archive integrity, Windows x64 binary, version, configuration, and license-content inspection.

Maintenance metadata, fundraising cleanup, licensing, and reusable tag release automation are complete and pushed. The final documentation commit records this acceptance evidence. In-game behavior was not exercised because the environment does not contain Persona 3 Reload; the existing viewport fix was preserved and included in the released source. The main lessons were to validate the native build before creating an immutable release tag and to distinguish Windows checkout line endings when comparing packaged text.

## Context and Orientation


Work from /home/camo/Projects/P3RFix. src/dllmain.cpp contains the fix and its logged version. xmake.lua builds a Windows x64 ASI module, a DLL loaded into the game by Ultimate ASI Loader. create_release.ps1 currently builds and packages that module and an INI configuration. assets/r2-package/ModConfig.json describes the alternative Reloaded-II mod and its update source. README.md and release_body.md describe installation. LICENSE.md records project licensing. .github/workflows will contain GitHub Actions instructions, and CHANGELOG.md will describe release changes.

## Plan of Work


### Milestone 1: Maintained project metadata


Remove Patreon and Ko-fi links and their unused images. Point release, issue, project, and Reloaded-II update destinations at dev-camo/P3RFix. Document installation alongside P3R.exe: Steam uses P3R/Binaries/Win64, and Xbox uses Content/P3R/Binaries/WinGDK. Retain original and dependency credits. Add the maintenance copyright and exact dependency license copies under licenses/, indexed by THIRD_PARTY_NOTICES.md. Describe the viewport fix in CHANGELOG.md. Inspect git diff before committing each coherent group.

### Milestone 2: Build and release automation


Refactor create_release.ps1 into a noninteractive, fail-fast archive builder accepting -Version. Download Ultimate ASI Loader v9.7.4 and check its SHA256, a digest that detects changes to the downloaded bytes. Stage files under build/ and generate P3RFix_<version>.zip and P3RFix_Reloaded-II.zip, including notices. Generate build/release_body.md using the matching CHANGELOG.md section and release_body.md installation template. Pass the version through the xmake fix_version option into the C++ P3RFIX_VERSION macro. Preserve a local default version. .github/workflows/build.yml performs reusable Windows builds, main-branch and pull-request builds, and manual development builds with read access. .github/workflows/release.yml runs on version-tag pushes, checks out the exact source commit with recursive dependencies, and publishes the successful build's archives using the GitHub CLI. Publication receives contents:write permission. Manual recovery accepts an existing tag and validates its target.

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

Completed publication used tag 1.2.5 at 12baef999df058c56bebe68e017396bceaefa7d0. Local downloaded acceptance artifacts are under ignored build/release-1.2.5/. The final plan-only commit uses [skip ci] in its message because the tagged native build and publication have already passed and this edit does not change build inputs.

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

First Windows build evidence:

    https://github.com/dev-camo/P3RFix/actions/runs/36786715366
    source: 93815d98dabd03df6bc3478ec3ba7e8cd148ab2b
    conclusion: success
    P3RFix_1.2.5.zip: P3RFix.asi, P3RFix.ini, dsound.dll, notices, 11 license files
    P3RFix_Reloaded-II.zip: P3RFix.asi, P3RFix.ini, ModConfig.json, notices, 11 license files

Final release evidence:

    preflight: https://github.com/dev-camo/P3RFix/actions/runs/36787078874 (success)
    publication: https://github.com/dev-camo/P3RFix/actions/runs/36787299471 (success)
    release: https://github.com/dev-camo/P3RFix/releases/tag/1.2.5
    source: 12baef999df058c56bebe68e017396bceaefa7d0
    annotated tag object: 334d6ddf6cdcde4fa86b96b4a5e7e7c2056cbcd0
    P3RFix_1.2.5.zip: 875532 bytes
    SHA256: 84ab1ed77711689d4d27c7fa057855c6ea45cc3455f3013baa329ceee01787f8
    P3RFix_Reloaded-II.zip: 479968 bytes
    SHA256: 455e3d91e928a2e454b248b9b5fc00631015e7751be0cfd06c6af190f1d20c8e
    Each package: 1194496-byte Windows x64 P3RFix.asi, version 1.2.5
    Remote comparison with cc2582b: ahead 8, behind 0

## Interfaces and Dependencies


Use GitHub-hosted windows-2022 MSVC, xmake 3.1.1, CMake, the repository's pinned inipp/spdlog/SafetyHook/Zydis/Zycore revisions, and Ultimate ASI Loader v9.7.4 x64. create_release.ps1 -Version 1.2.5 is the packaging interface. GitHub CLI publishes an existing tag with both archives and release notes, using the workflow token. Reloaded-II retains ModId p3rpc.p3rfix and asset filename P3RFix_Reloaded-II.zip so the mod's identity remains stable. Manual publication recovery is gh workflow run release.yml -f tag=1.2.5; it rebuilds that tag and replaces its attachments.

Revision note (2026-09-30): Initial plan records repository findings, authorized scope, and observable release acceptance.

Revision note (2026-09-30 22:37Z): Recorded completed cleanup, implemented interfaces, pinned tools, runtime compatibility, and annotated-tag handling before the first remote build.

Revision note (2026-09-30 22:41Z): Recorded successful native build and inspected archives; added a runtime modernization step prompted by actual GitHub annotations.

Revision note (2026-09-30 22:48Z): Recorded successful modernized build, annotated tag publication, public release assets, and downloaded artifact acceptance evidence. No implementation work remains.
