# Maintaining P3RFix

P3RFix is a Windows x64 C++ ASI plugin for Persona 3 Reload. User installation and configuration belong in the [README](../../README.md) and [user guides](../../docs/). Repository instructions for coding agents are in [AGENTS.md](../../AGENTS.md); agent plans and handoffs belong in `.agents/`.

## Build and package

Use Windows with Visual Studio 2022 C++ tools, the Windows SDK, CMake, PowerShell 7.2+, Python, and xmake available on `PATH`. The build workflow pins its xmake version; see [build.yml](../workflows/build.yml).

For a new checkout, clone the repository, then package from its root:

```powershell
git clone --recurse-submodules https://github.com/dev-camo/P3RFix.git
cd P3RFix
git submodule update --init --recursive
./.github/scripts/create_release.ps1 -Version 1.4.0
```

The script builds Windows x64 release binaries and creates `build/P3RFix_1.4.0.zip` and `build/P3RFix_Reloaded-II.zip`. It embeds the requested version in the binary and Reloaded-II metadata. Dependencies use the revisions recorded in Git; the standalone package bundles a pinned Ultimate ASI Loader download.

Packaging combines all thirteen maintained license and notice documents into one `LICENSES` file. It checks both archives for complete, matching notices and the expected binary and configuration paths. Keep the [third-party notices](../../docs/THIRD_PARTY_NOTICES.md), [license inputs](../../assets/licenses/), and explicit script manifest consistent when changing dependencies. The standalone ZIP includes `dsound.dll`; the Reloaded-II ZIP includes `ModConfig.json` and omits that loader.

## Automated tests and game validation

Run the synthetic memory and fix behavior tests in Windows x64 debug and release configurations:

```powershell
xmake f -y -p windows -a x64 -m debug --fix_version=1.4.0
xmake build -y unreal-integration-tests
xmake run unreal-integration-tests
xmake build -y fix-behavior-tests
xmake run fix-behavior-tests
xmake f -y -p windows -a x64 -m release --fix_version=1.4.0
xmake build -y unreal-integration-tests
xmake run unreal-integration-tests
xmake build -y fix-behavior-tests
xmake run fix-behavior-tests
python .github/scripts/unreal_inventory.py --phase minimal
```

The integration tests check layouts, reflected console calls, failure handling, name conversion, and render writes using independent byte fixtures. The behavior tests check scaling arithmetic, integer bounds, raw-input read failures, signed mouse deltas, and allocation failure using a controlled Windows reader.

These tests do not establish compatibility with a particular game update. Compare console operation, input, and menu rendering with the baseline DLL in the actual game when changing those behaviors. Include relevant `P3RFix.log` excerpts for runtime fixes and screenshots for visual changes.

## Unreal integration

Console construction and menu render texture access use the focused interface in `src/unreal/Integration.hpp`. Game memory layouts and dispatch constants live privately in `src/unreal/detail/Layouts.hpp`, with their original Dumper-7 symbols recorded beside them. The full generated SDK is no longer a build or checkout dependency. Add only the fields needed for a feature, with provenance and a boundary test.

Console lookup assumes the inherited signatures, registry layout, and virtual dispatch entry. Independent hook installation runs before the optional console discovery wait.

## Workflows and releases

The [Build packages workflow](https://github.com/dev-camo/P3RFix/actions/workflows/build.yml) tests, builds, and packages pushes and pull requests to `main`, and supports manual runs. Use `[skip ci]` on documentation-only or maintenance commits without code or automated workflow changes. Let code, build-script, and workflow changes run CI, and review the result and downloadable artifacts.

Do not publish a release for repository cleanup alone. For a meaningful shipped change, update the [changelog](../release/CHANGELOG.md) and [release notes template](../release/release_body.md), validate the packages and game behavior, and have a human review the release commit. Push a new version tag pointing at that exact tested commit:

```sh
git push origin HEAD
git tag -a 1.4.0 -m "Release 1.4.0"
git push origin 1.4.0
```

The version above is an example; use a new version number for each release. Tags may use `X.Y.Z` or `vX.Y.Z`. The [Publish release workflow](https://github.com/dev-camo/P3RFix/actions/workflows/release.yml) checks out the tagged commit and its submodules, builds that version, and publishes both ZIP files on the matching GitHub Release.

To retry an existing release, open **Publish release → Run workflow** in GitHub Actions and enter the existing tag in the `tag` field, or run:

```sh
gh workflow run release.yml -f tag=1.4.0
```

A retry rebuilds the commit referenced by that tag and replaces the release's ZIP attachments. The workflow checks the tag's commit before building and publishing. A branch push alone does not publish a release.
