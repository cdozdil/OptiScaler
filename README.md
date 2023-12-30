# DLSS2XeSS
Drop-in DLSS replacement with XeSS for various games such as Cyberpunk 2077.

Based on [PotatoOfDoom](https://github.com/PotatoOfDoom)'s excellent [CyberFSR2](https://github.com/PotatoOfDoom/CyberFSR2). I have adapted it to use Intel's [XeSS](https://github.com/intel/xess/)

As XeSS officially only supports DirectX12 (Support for DirectX11 added but consider it as alpha and expect performance and other problems).

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

### Instructions
* Clone this repo with all of its submodules.
* Open the CyberXeSS.sln with Visual Studio 2022.
* Copy the compiled `nvngx.dll` from the XeSS Directory to your Cyberpunk 2077 executable directory.
* Copy the [libxess.dll](https://raw.githubusercontent.com/intel/xess/main/bin/libxess.dll) to your Cyberpunk 2077 executable directory.
* Run the `EnableSignatureOverride.reg` to allow Cyberpunks DLSS implementation to load unsigned DLSS versions
* Run the game and set the quality in the DLSS settings
* Play the game with XeSS
