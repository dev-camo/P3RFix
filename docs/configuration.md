# Configuration

[Back to the README](../README.md)

Close the game before editing `P3RFix.ini`, save your changes, then launch the game again. The fix reads this file when it starts.

- **Standalone:** edit the file beside `P3R.exe`.
- **Reloaded-II:** edit the file in the installed P3RFix mod folder.

The [configuration supplied with the fix](../P3RFix.ini) includes the defaults and comments for every option. Use `true` or `false` for switches, and keep the section names and setting names intact.

## Display and performance

| Section | Use |
| --- | --- |
| `[Custom Resolution]` | Enable it and set `Width` and `Height` to override the game's resolution. Leaving both at `0` uses the desktop resolution. Borderless mode always uses the desktop resolution. |
| `[Fix Aspect Ratio]` | Correct the aspect ratio and field of view for ultrawide or narrower displays. Enabled by default. |
| `[Fix HUD]` | Allow the HUD to use the wider screen area. Enabled by default. |
| `[Uncap 60FPS Menus]` | Let menus use the target frame rate instead of the 60 FPS cap. Enabled by default. |
| `[Screen Percentage]` | Override render scale. `100` is native resolution; higher values downsample for a sharper image and lower values reduce rendering resolution. Disabled by default. |
| `[Render Texture Resolution]` | Adjust the resolution of render textures such as character previews in menus. Enabled by default, with `Multiplier = 1`. |

Render textures scale automatically with viewport height and render scale, keeping at least their native 1080p quality. `Multiplier` applies on top of that automatic scaling: leave it at `1` for the automatic result, raise it for higher quality, or lower it to reduce cost. A value below `1` can reduce quality below the native baseline. The combined multiplier is limited to `0.25–4`; increasing it may cause hitching or reduce performance.

## Startup and window behavior

| Section | Use |
| --- | --- |
| `[Intro Skip]` | `SkipLogos = true` skips warning and network screens. `SkipTo` chooses the opening movie (`1`), main menu (`2`, the default), or load-save menu (`3`). Backing out after skipping directly to Load Save can softlock the game. |
| `[Pause on Focus Loss]` | Set `Enabled = false` to keep the game running when you switch windows. With mouse controls, enter a menu first to release the cursor. |
| `[Enable Console]` | Enable the developer console. The usual key is tilde, beneath Esc; see [troubleshooting](troubleshooting.md#console-key). Disabled by default. |

## Experimental options

These options are disabled by default. If problems begin after enabling one, disable it and try again.

- **`[FPS Cap]`:** set `AdjustFPSCap = true` and choose a `Framerate`. This overrides the menu FPS option and can affect voice lines or game timing.
- **`[Mouse Fix]`:** set `Enabled = true` to change mouse camera movement. Adjust `MouseMultiplierX` and `MouseMultiplierY` for sensitivity; negative values invert an axis. Keep `IgnoreGamepad = true` to let the fix stop applying when using a controller. Unusual camera movement is a reason to disable the fix.
