# DLSS2XeSS
Drop-in DLSS replacement with XeSS for various games such as Cyberpunk 2077.

## Official Discord Channel: https://discord.gg/2JDHx6kcXB

## Installation
*Following instructions were written for Cyberpunk 2077 and may differ for other games.*
### Windows 
* Download the latest relase from releases.
* Extract the contents of the archive next to the `nvngx_dlss.dll` file in your games folder.
* Run `EnableSignatureOverride.reg` and confirm merge.
* That's it. Now DLSS option should appear in settigs.

### Linux
* Download the latest relase from releases.
* Extract the contents of the archive next to the `nvngx_dlss.dll` file in your games folder.
* Run the linuxinstall.sh script
* Or just run the following command with the appropriate file paths to install the tweaks manually:
```
WINEPREFIX=/path/where/the/steam/library/is/steamapps/compatdata/1091500/pfx /bin/wine64 regedit ../../common/Cyberpunk\ 2077/bin/x64/FidelityFx\ Super\ Resolution\ 2.0-3001-0-3-1656426926/EnableSignatureOverride.reg
```
* That's it. Now DLSS option should appear in settigs.

### Uninstallation
* Just run `DisableSignatureOverride.reg`
* Linux users should refer to prior command.

## Compilation

### Requirements
* Visual Studio 2022
* latest Vulkan SDK (1.3.216.0)

### Instructions
* Clone this repo with all of its submodules.
* Open the CyberXeSS.sln with Visual Studio 2022.
* Copy the compiled `libxess.dll` from the XeSS Directory to your Cyberpunk 2077 executable directory.
* Rename the compiled DLL from two steps ago to `nvngx.dll` if it is `CyberXeSS.dll`.
* Copy `nvngx.dll` to your Cyberpunk 2077 executable directory.
* Run the `EnableSignatureOverride.reg` to allow Cyberpunks DLSS implementation to load unsigned DLSS versions
* Run the game and set the quality in the DLSS settings
* Play the game with XeSS
