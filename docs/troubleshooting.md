# Troubleshooting

[Back to the README](../README.md)

## The fix does not load

For standalone installations, confirm that `P3RFix.asi`, `P3RFix.ini` and `dsound.dll` are in the same folder as `P3R.exe`, rather than the game's top-level folder. On Steam Deck/Linux, check the [Steam launch option](installation.md#steam-deck-and-linux).

For Reloaded-II, confirm that P3RFix is enabled and that you launch the game through Reloaded-II. Avoid leaving the standalone P3RFix installation active at the same time; see [switching installation methods](installation.md#reloaded-ii).

After launching, look for `P3RFix.log` beside `P3R.exe`. The log records the loaded version, configuration and any failures. The configuration file must be beside `P3RFix.asi`, including when that file is in a Reloaded-II mod folder.

## A setting causes problems

Close the game and undo the last configuration change. In particular:

- If voice lines or timing behave unexpectedly, disable the experimental FPS cap.
- If camera movement behaves unexpectedly, disable the experimental mouse fix.
- If higher render texture resolution causes hitching, restore `Multiplier = 1` or reduce it.
- If the mouse stays captured after switching windows with pause disabled, enter a menu before switching.

## Console key

Enable `[Enable Console]` in `P3RFix.ini`, then check `P3RFix.log` for the console key. The usual key is tilde, beneath Esc.

If the log says no console key is bound, add the following to `%LOCALAPPDATA%\P3R\Saved\Config\Windows\Input.ini`:

```ini
[/Script/Engine.InputSettings]
ConsoleKeys = Tilde
```

You can change `Tilde` to another Unreal Engine key name. If the log reports a different console failure, include it in an issue report.

## Known issues

- Some fades and transitions do not span the screen at non-16:9 resolutions.
- Skipping directly to the load-save menu can softlock the game if you back out. Set `[Intro Skip].SkipTo = 2` to use the main menu instead.
- Disabling pause on focus loss with mouse controls can affect mouse capture. Enter a menu first to release the cursor.

## Report a problem

[Open an issue](https://github.com/dev-camo/P3RFix/issues) with your P3RFix version, installation method, steps to reproduce the problem and `P3RFix.log` from the folder containing `P3R.exe`. Include a screenshot or video when it helps show a display problem.
