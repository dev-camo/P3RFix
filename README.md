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
./create_release.ps1 -Version 1.2.5
```

The script builds Windows x64 release binaries and creates `build/P3RFix_1.2.5.zip` and `build/P3RFix_Reloaded-II.zip`. The version is embedded in the binary and Reloaded-II metadata. Dependencies use the revisions recorded in Git; the standalone package bundles a pinned Ultimate ASI Loader download.

The [Build packages workflow](https://github.com/dev-camo/P3RFix/actions/workflows/build.yml) also builds and packages pushes and pull requests to `main`, and supports manual runs. Review its build result and downloadable artifacts before tagging a release.

## Releasing

Commit the release changes, then push the branch and a version tag pointing at that commit:

```sh
git push origin HEAD
git tag -a 1.2.5 -m "Release 1.2.5"
git push origin 1.2.5
```

Use a new version number for each release. Tags may use `1.2.5` or `v1.2.5`. The [Publish release workflow](https://github.com/dev-camo/P3RFix/actions/workflows/release.yml) checks out the tagged commit and its submodules, builds that version and publishes both ZIP files on the matching [GitHub Release](https://github.com/dev-camo/P3RFix/releases).

To retry a release for an existing tag, open **Publish release → Run workflow** in GitHub Actions and enter the tag in the `tag` field, or use GitHub CLI:

```sh
gh workflow run release.yml -f tag=1.2.5
```

A retry rebuilds the commit referenced by that tag and replaces the release's ZIP attachments. The workflow checks the tag's commit before building and publishing.

## License and Credits

P3RFix is distributed under the [MIT License](LICENSE.md). Original copyrights and third-party attributions are preserved. See [third-party notices](THIRD_PARTY_NOTICES.md) and [license texts](licenses/) for dependency licensing and SDK provenance.

- Lyall for the original P3RFix implementation.
- [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) for ASI loading.
- [inipp](https://github.com/mcmtroffaes/inipp) for INI reading.
- [spdlog](https://github.com/gabime/spdlog) and [{fmt}](https://github.com/fmtlib/fmt) for logging and formatting.
- [SafetyHook](https://github.com/cursey/safetyhook), [Zydis](https://github.com/zyantific/zydis) and [Zycore](https://github.com/zyantific/zycore-c) for hooking and instruction decoding.
- [Dumper-7](https://github.com/Encryqed/Dumper-7), [UnrealContainers](https://github.com/Fischsalat/UnrealContainers) and [UTF-N](https://github.com/Fischsalat/UTF-N) for the generated SDK and support code.
