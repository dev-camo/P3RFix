# Changelog

## [1.4.0] - 2026-10-01

### Fixed

- Base automatic menu render texture scaling on effective viewport height, retaining native quality below 1080p while honoring explicit user multipliers and rejecting invalid or overflowing dimensions.
- Ignore failed, incomplete, and inconsistent raw-input reads before parsing mouse movement; preserve the existing sensitivity and accepted mouse flags.
- Attempt independent fix installation before the optional console discovery wait on the initialization worker, including intro skipping and mouse hooks.

### Maintenance

- Replace the release ZIPs' separate license documents and `licenses/` directory with one complete `LICENSES` file, preserving all maintained notices and existing binary destinations. Overwrite upgrades may retain earlier notice files.
- Run focused scaling and Windows packet-reader tests alongside the Unreal integration checks in debug and release builds.

## [1.3.0] - 2026-10-01

### Maintenance

- Replace the generated game SDK with a focused private Unreal integration for console construction and menu render textures, with documented memory layouts and synthetic boundary tests.
- Report unavailable console dependencies without preventing independent fixes; keep console key reporting separate from console creation.
- Validate the integration tests and source inventory in Windows build automation before packaging.

## [1.2.5] - 2026-09-30

### Fixed

- Track the actual viewport dimensions when calculating aspect ratio and HUD placement, including borderless window changes. Initialize custom resolution tracking at startup and retain the last valid dimensions while the viewport is empty or minimized. Includes [cc2582b](https://github.com/dev-camo/P3RFix/commit/cc2582b36b8c3fe37a47781ea5fc98638c3f75b9).

### Maintenance

- Continue project maintenance at [dev-camo/P3RFix](https://github.com/dev-camo/P3RFix), update release and Reloaded-II update links, and remove fundraising links and assets.
- Add Windows build automation and GitHub Releases for version tags, with both standalone and Reloaded-II archives.
- Retain the MIT license and original attribution, add the maintenance copyright, and include third-party license notices in release archives.
