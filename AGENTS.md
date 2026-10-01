# Repository Guidelines

## Project Structure & Module Organization

P3RFix is a Windows x64 C++ ASI plugin for Persona 3 Reload.

- `src/dllmain.cpp` handles initialization, configuration, and game hooks; `src/helper.hpp` provides shared utilities.
- `src/unreal/Integration.hpp` exposes the focused Unreal API. Keep memory layouts and runtime internals in `src/unreal/detail/`.
- `src/render/Scaling.*` defines render-target scaling; `src/input/RawMousePacket.*` validates Windows raw-input reads and applies the existing mouse deltas.
- `tests/unreal_integration_tests.cpp` contains synthetic memory tests; `tools/unreal_inventory.py` audits integration sources.
- `tests/fix_behavior_tests.cpp` runs scaling and packet-reader cases from the adjacent test headers.
- `external/` contains pinned dependency submodules. `assets/r2-package/` holds Reloaded-II metadata; `.github/` contains workflows, issue templates, and screenshots.

## Documentation & Repository Layout

Keep `README.md` focused on end users: what the fix does, installation, basic configuration, known issues, and support. Put detailed user guides in `docs/`, maintainer documentation and release tooling in appropriate `.github/` subfolders, and agent plans or handoffs in `.agents/`. Link to those documents instead of expanding the README with build commands, release procedures, or runtime internals.

Keep package inputs in `assets/`. Root files should be limited to the README, agent instructions and the `CLAUDE.md` symlink, project license, build entry point, and required repository configuration. See [.github/maintainers/README.md](.github/maintainers/README.md) for development and release procedures.

## Build, Test, and Development Commands

Run from the repository root on Windows with Visual Studio 2022 C++ tools, Windows SDK, CMake, PowerShell 7.2+, and xmake available.

```powershell
git submodule update --init --recursive
xmake f -y -p windows -a x64 -m debug --fix_version=1.4.0
xmake build -y P3RFix
xmake build -y unreal-integration-tests
xmake run unreal-integration-tests
xmake build -y fix-behavior-tests
xmake run fix-behavior-tests
python tools/unreal_inventory.py --phase minimal
./create_release.ps1 -Version 1.4.0
```

These commands initialize dependencies, configure a debug build, build the plugin, build/run tests, audit source boundaries, and package a release. Packaging creates standalone and Reloaded-II ZIPs under `build/`, validates all thirteen license/notice inputs, and verifies each archive's combined `LICENSES` and binary paths. The maintained source license files remain in the repository. Load the packaged fix with the game using the installation instructions in `README.md`.

## Coding Style & Naming Conventions

Use four-space indentation and match the surrounding brace style. The build selects `cxxlatest`. Use PascalCase for functions/types and lowerCamelCase for new Unreal fields; preserve existing prefixes such as `b`, `i`, and `f` in hook code. No repository-wide formatter or linter is configured.

Keep Unreal layout additions minimal, record original Dumper-7 symbols, and add boundary tests for new fields.

## Testing Guidelines

Tests use a custom C++ runner with `Require` assertions and descriptive case strings. Fixtures must use independently reviewed literal offsets. Cover invalid inputs, failure paths, and memory-write boundaries. Run the test target in debug and release modes; reconfigure with `-m release` before repeating build/run commands. No numeric coverage threshold is configured. Validate console, hook, and rendering changes in the actual game against the baseline DLL.

## Commit & Pull Request Guidelines

Follow the history's short imperative subjects, such as `Fix aspect ratio tracking` or `Add minimal Unreal runtime`. Keep commits focused. PRs should describe behavior changes, link relevant issues, and report automated/manual validation. Include screenshots for visual changes and relevant `P3RFix.log` excerpts for runtime fixes. Review the Windows build workflow and package artifacts. Preserve third-party notices when changing dependencies.

Use `[skip ci]` on documentation-only and other maintenance commits that do not change code or automated workflows. Allow CI when code, build scripts, or workflows change. Do not publish a new release for repository housekeeping alone; release only when behavior or another meaningful shipped change warrants it.
