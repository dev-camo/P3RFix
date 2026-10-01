## Installation

- Download `<RELEASE_ZIP_NAME>` from the release assets below.
- Extract its contents into the folder containing `P3R.exe`: `steamapps\common\P3R\P3R\Binaries\Win64` for Steam or `XboxGames\Persona 3 Reload\Content\P3R\Binaries\WinGDK` for Xbox/MS Store.
- The standalone ZIP contains `P3RFix.asi`, `P3RFix.ini`, `dsound.dll`, and one complete `LICENSES` document. An overwrite upgrade can leave older notice files in place.

### Steam Deck/Linux

Add `WINEDLLOVERRIDES="dsound=n,b" %command%` to the game's Steam launch options.

<details>
<summary>Installing with Reloaded-II</summary>

This option applies to both Windows and Steam Deck/Linux.

- If you previously installed the standalone package, back up your configuration and remove its `P3RFix.ini`, `P3RFix.asi`, `dsound.ini` (if present), and `dsound.dll` from the game folder before switching.
- Download `P3RFix_Reloaded-II.zip` from the release assets below and drag it onto the Reloaded-II window. See the [Reloaded-II installation guide](https://reloaded-project.github.io/Reloaded-II/QuickStart/) for manual setup.
- Enable P3RFix in the Reloaded-II mod list and launch the game through Reloaded-II.
- This ZIP contains the ASI, INI, Reloaded-II metadata, and the same `LICENSES` document. It does not include the standalone loader binary.

</details>

## Configuration

Edit `P3RFix.ini` to adjust settings. Report problems through [GitHub Issues](https://github.com/dev-camo/P3RFix/issues), including your version and `P3RFix.log`.
