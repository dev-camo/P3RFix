# Installation, updates and removal

[Back to the README](../README.md)

## Standalone

1. Download `P3RFix_<version>.zip` from the [latest release](https://github.com/dev-camo/P3RFix/releases/latest).
2. Extract all contents into the folder containing `P3R.exe`:

   | Store | Game executable folder |
   | --- | --- |
   | Steam | `steamapps\common\P3R\P3R\Binaries\Win64` |
   | Xbox / Microsoft Store | `XboxGames\Persona 3 Reload\Content\P3R\Binaries\WinGDK` |

3. Check that `P3RFix.asi`, `P3RFix.ini` and `dsound.dll` are alongside `P3R.exe`.
4. Launch the game normally.

### Steam Deck and Linux

Open the game's properties in Steam and add this to **Launch Options**:

```text
WINEDLLOVERRIDES="dsound=n,b" %command%
```

## Reloaded-II

These steps apply to Windows and Steam Deck/Linux.

If you previously installed the standalone package, back up your `P3RFix.ini` settings and remove its `P3RFix.asi`, `P3RFix.ini`, `dsound.ini` (if present) and `dsound.dll` from the game folder before switching. Keep loader files that another mod still needs.

1. Download `P3RFix_Reloaded-II.zip` from the [latest release](https://github.com/dev-camo/P3RFix/releases/latest).
2. Drag the ZIP onto the Reloaded-II window. Alternatively, follow Reloaded-II's [manual installation instructions](https://reloaded-project.github.io/Reloaded-II/QuickStart/).
3. Enable P3RFix in the mod list and launch the game through Reloaded-II.
4. Edit `P3RFix.ini` in the installed P3RFix mod folder to configure it.

## Update

Close the game and back up your existing `P3RFix.ini` before installing an update. Install the new package using the same method, then copy your preferred values into the new configuration file so that any new settings remain available.

Both packages include a `LICENSES` document with the project license and complete third-party notices. Older installations may leave separate notice files behind; the new package's `LICENSES` contains the complete notices for that package.

## Remove

- **Standalone:** close the game and remove the installed `P3RFix.asi` and `P3RFix.ini`. Remove `dsound.dll` and `dsound.ini` only if no other installed mod uses that loader. You can also remove the generated `P3RFix.log` and the package's `LICENSES` document.
- **Reloaded-II:** disable P3RFix in the mod list, or remove it through Reloaded-II. You can also remove the generated `P3RFix.log` beside `P3R.exe`.

On Steam Deck/Linux, remove the `dsound` override from Steam launch options if you no longer use the standalone loader.
