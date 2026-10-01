# Persona 3 Reload Fix

This fix adds custom resolutions, ultrawide support and more to Persona 3 Reload.

Maintained by [dev-camo](https://github.com/dev-camo) and contributors, continuing the original work by Lyall. Downloads, updates and bug reports are hosted at [dev-camo/P3RFix](https://github.com/dev-camo/P3RFix).

## Features

- Custom resolution support.
- Ultrawide/narrow aspect ratio support.
- Correct FOV at any aspect ratio.
- Configurable intro skip.
- Enable developer console.
- Render scale override.
- Control over pausing when alt+tabbed.
- The ability to increase/decrease resolution of render texture targets (i.e menus, persona model previews).

## Installation

- Download `P3RFix_<version>.zip` from the latest [release](https://github.com/dev-camo/P3RFix/releases/latest).
- Extract its contents into the folder containing `P3R.exe`:
  - Steam: `steamapps\common\P3R\P3R\Binaries\Win64`
  - Xbox/MS Store: `XboxGames\Persona 3 Reload\Content\P3R\Binaries\WinGDK`
- `P3RFix.asi`, `P3RFix.ini` and `dsound.dll` should be alongside `P3R.exe`.

Both ZIPs include one `LICENSES` document with the project license and complete third-party notices. Overwriting an older installation can leave its previous notice files in place; `LICENSES` contains the complete notices for the new package.

### Steam Deck/Linux Additional Instructions

- Open up the game properties in Steam and add `WINEDLLOVERRIDES="dsound=n,b" %command%` to the launch options.

<details>
<summary>Installing P3RFix as a Reloaded II Mod</summary>
  
*This applies to both Windows and Steam Deck/Linux.*

If you previously installed the standalone package, back up your `P3RFix.ini` settings and remove its `P3RFix.asi`, `P3RFix.ini`, `dsound.ini` (if present) and `dsound.dll` from the game folder before switching to Reloaded-II.

- Download `P3RFix_Reloaded-II.zip` from the latest [release](https://github.com/dev-camo/P3RFix/releases/latest).
- Drag and drop the ZIP onto the Reloaded-II window. (Alternatively: [manual installation](https://reloaded-project.github.io/Reloaded-II/QuickStart/).)
- Enable P3RFix in the Reloaded-II mod list, then launch the game through Reloaded-II.
- Edit `P3RFix.ini` in the installed mod folder to configure the fix.

</details>

## Configuration

- See **P3RFix.ini** to adjust settings for the fix.
- `[Render Texture Resolution].Multiplier` applies after automatic scaling based on viewport height and screen percentage. Automatic scaling keeps at least the native 1080p target quality; an explicit value below 1 can still reduce it. The combined multiplier is limited to 0.25–4.

## Screenshots

| ![p3r_comparison](.github/images/p3r_comparison.gif) |
|:--------------------------:|
| Gameplay |

## Known Issues

Please [report issues](https://github.com/dev-camo/P3RFix/issues) with your P3RFix version, steps to reproduce and `P3RFix.log`.

- Some screen fades/transitions may not span the screen at non-16:9 resolutions.
- If you skip intro to the load save menu, then back out, you will softlock the game.
- Disabling pause on focus loss while using m/kbd can cause issues with mouse capture. Enter a menu first to release the cursor.

## Building

Use Windows with Visual Studio 2022's C++ build tools and Windows SDK, CMake, PowerShell and xmake available on `PATH`.

```powershell
git clone --recurse-submodules https://github.com/dev-camo/P3RFix.git
cd P3RFix
git submodule update --init --recursive
./create_release.ps1 -Version 1.4.0
```

The script builds Windows x64 release binaries and creates `build/P3RFix_1.4.0.zip` and `build/P3RFix_Reloaded-II.zip`. The version is embedded in the binary and Reloaded-II metadata. Dependencies use the revisions recorded in Git; the standalone package bundles a pinned Ultimate ASI Loader download. Packaging combines all thirteen maintained license/notice documents into `LICENSES` and checks both archives for complete matching notices and the expected binary/configuration paths.

The [Build packages workflow](https://github.com/dev-camo/P3RFix/actions/workflows/build.yml) also builds and packages pushes and pull requests to `main`, and supports manual runs. Review its build result and downloadable artifacts before tagging a release.

### Unreal integration maintenance

Console construction and menu render texture access use the focused interface in `src/unreal/Integration.hpp`. Game memory layouts and dispatch constants live privately in `src/unreal/detail/Layouts.hpp`, with their original Dumper-7 symbols recorded beside them. The full generated SDK is no longer a build or checkout dependency. Add only the fields needed for a new feature, with provenance and a boundary test.

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
python tools/unreal_inventory.py --phase minimal
```

The integration tests check layouts, reflected console calls, failure handling, name conversion, and render writes using independent byte fixtures. The behavior tests check scaling arithmetic, integer bounds, raw-input read failures, signed mouse deltas, and allocation failure using a controlled Windows reader. They do not establish compatibility with a particular game update; compare console operation, input, and menu rendering with the baseline DLL in the actual game. Console lookup assumes the inherited signatures, registry layout, and virtual dispatch entry; independent hook installation runs before the optional console discovery wait.

## Releasing

Validate the packages and game behavior, then have a human review the branch before merging. After approval, push a new version tag pointing at the exact tested release commit:

```sh
git push origin HEAD
git tag -a 1.4.0 -m "Release 1.4.0"
git push origin 1.4.0
```

Use a new version number for each release. Tags may use `1.4.0` or `v1.4.0`. The [Publish release workflow](https://github.com/dev-camo/P3RFix/actions/workflows/release.yml) checks out the tagged commit and its submodules, builds that version and publishes both ZIP files on the matching [GitHub Release](https://github.com/dev-camo/P3RFix/releases).

To retry a release for an existing tag, open **Publish release → Run workflow** in GitHub Actions and enter the tag in the `tag` field, or use GitHub CLI:

```sh
gh workflow run release.yml -f tag=1.4.0
```

A retry rebuilds the commit referenced by that tag and replaces the release's ZIP attachments. The workflow checks the tag's commit before building and publishing.

## License and Credits

P3RFix is distributed under the [MIT License](LICENSE.md). Original copyrights and third-party attributions are preserved. In the source checkout, see [third-party notices](THIRD_PARTY_NOTICES.md) and [license texts](licenses/) for dependency licensing and retained Unreal integration provenance. Release ZIPs ship those complete texts together in one generated `LICENSES` document; individual source notice files remain maintained here.

- Lyall for the original P3RFix implementation.
- [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) for ASI loading.
- [inipp](https://github.com/mcmtroffaes/inipp) for INI reading.
- [spdlog](https://github.com/gabime/spdlog) and [{fmt}](https://github.com/fmtlib/fmt) for logging and formatting.
- [SafetyHook](https://github.com/cursey/safetyhook), [Zydis](https://github.com/zyantific/zydis) and [Zycore](https://github.com/zyantific/zycore-c) for hooking and instruction decoding.
- [Dumper-7](https://github.com/Encryqed/Dumper-7), [UnrealContainers](https://github.com/Fischsalat/UnrealContainers) and [UTF-N](https://github.com/Fischsalat/UTF-N) for the original generated layouts and retained private support code.
