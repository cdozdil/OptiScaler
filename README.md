![OptiScaler](images/optiscaler.png)

OptiScaler (was CyberXeSS) is drop-in DLSS2 to XeSS/FSR2/FSR3/DLSS replacement for games. 

### Official Discord Server: [DLSS2FSR](https://discord.gg/2JDHx6kcXB)

**Do not use this mod with online games, it might trigger anti-cheat software and cause bans!**

*This project is based on [PotatoOfDoom](https://github.com/PotatoOfDoom)'s excellent [CyberFSR2](https://github.com/PotatoOfDoom/CyberFSR2).*

## Installation
### Install as `non-nvngx`
With DLSS 3.7 Nvidia disabled signature check override, this means `nvngx.dll` or `_nvngx.dll` must be signed by Nvidia. For this override I am using a method developed by **Artur** (developer of [DLSS Enabler](https://www.nexusmods.com/site/mods/757?tab=description)).  

This method increases compatibility of `OverlayMenu`, allows OptiScaler to **spoof DXGI and Vulkan** and overriding `nvapi64.dll`.  
In short this installation method allows OptiScaler to work with all it's features.

Step-by-step installation (**Nvidia users, step 3 is enough for you**):
1. We need an Nvidia signed dll file to bypass signature checks. All games that support DLSS come with `nvngx_dlss.dll`. Most of the time it's in the games exe folder. Some games and engines keep these third party dll's in different folders (like `plugins`). So we need to find the `nvngx_dlss.dll` file and copy it to the games exe folder. If it's already in the games exe folder, make a copy of it.
2. Rename the copy of `nvngx_dlss.dll` in games exe folder to `nvngx.dll`.
3. Rename OptiScaler's `nvngx.dll` to one of the supported filenames (I prefer `dxgi.dll`, it also eliminates the need for d3d12 proxy) and copy it to the games exe folder.
4. If your GPU is not an Nvidia one, check [here](#spoofing-nvidia) for spoofing options.

OptiScaler supports being loaded by these filenames:  
* dxgi.dll (with Nvidia GPU spoofing for non-Nvidia cards)
* winmm.dll
* version.dll
* wininet.dll
* winhttp.dll
* OptiScaler.asi (with an ASI loader)

Linux users should add renamed dll to overrides:
```
WINEDLLOVERRIDES=dxgi=n,b %COMMAND% 
```

If there is another mod (Reshade etc.) that uses the same filename (for example `dxgi.dll`), if you rename it with `-original` suffix (for example `dxgi-original.dll`), OptiScaler will load this file instead of the original library.   

Alternatively you can create a new folder called `plugins` and put other mods files in this folder. OptiScaler will check this folder and if finds same dll file (for example `dxgi.dll`) will load this file instead of the original library. 
![image](https://github.com/cdozdil/OptiScaler/assets/35529761/c4bf2a85-107b-49ac-b002-59d00fd06982)

**Please don't rename the ini file, it should stay as `nvngx.ini`**.

### Install as `nvngx.dll`
Step-by-step installation (**Nvidia users, please skip step 4**):
1. Download the latest relase from releases.
2. Extract the contents of the archive next to the game executable file in your games folder. [1]
3. Run `EnableSignatureOverride.reg` from `DlssOverrides` folder and confirm merge. [2][3]
4. If your GPU is not an Nvidia one, check [here](#spoofing-nvidia) for spoofing options.

*[1] This package contains latest version of `libxess.dll` and if game folder contains any older version of same library it would be overwritten. Consider backing up or renaming existing file.*

*[2] Normally Streamline and games check if nvngx.dll is signed, by merging this `.reg` file we are overriding this signature check.*

*[3] Adding signature override on Linux - There are many possible setups, this one will focus on steam games:*
* *Make sure you have protontricks installed*
* *Run in a terminal protontricks <steam-appid> regedit, replace <steam-appid> with an id for your game*
* *Press "registry" in the top left of the new window -> `Import Registry File` -> navigate to and select `EnableSignatureOverride.reg`*
* *You should see a message saying that you successfully added the entries to the registry*

*If your game is not on Steam, it all boils down to opening regedit inside your game's prefix and importing the file.*

## Spoofing Nvidia 
Most games have checks for enabling DLSS options. To enable DLSS options in these games there are several methods which explained [here](https://github.com/cdozdil/OptiScaler/blob/master/Spoofing.md)

## Update OptiScaler version when using DLSS Enabler  
1. Delete/rename `dlss-enabler-upscaler.dll` in game folder
2. Copy `nvngx.dll` file from OptiScaler 7zip file to game folder
3. Rename `nvngx.dll` to `dlss-enabler-upscaler.dll`

## Uninstallation
* Run `DisableSignatureOverride.reg` file 
* Delete `EnableSignatureOverride.reg`, `DisableSignatureOverride.reg`, `nvngx.dll`, `nvngx.ini` files
* If there were a `libxess.dll` file and you have backed it up delete the new file and restore the backed up file. If you have overwrote old file **DO NOT** delete `libxess.dll` file. If there were no `libxess.dll` file it's safe to delete.

## How it works?
OptiScaler implements all necessary API methods of DLSS2 & NVAPI to act as a man in the middle. So from games perspective it's using DLSS2 but actually using OptiScaler and calls are interpreted/redirected to XeSS & FSR2 or DLSS with OptiScaler's tweaks and enchancements.

## Features
* Supports multiple upscaling backends (XeSS, FSR 2.1.2, FSR 2.2.1, FSR 3.1, DLSS)
* Supports DLSS 3.7 and above (check installation for 3.7 specific instructions)
* Supports DLSS-D (Ray Reconstruction) on Nvidia cards (Supports changing presets and using OptiScaler enchanchements)
* Ability to modify DLSS/DLSS-D presets on the fly
* Supports XeSS v1.3's Ultra Performance, NativeAA modes (**Not using XeSS 1.3 scaling ratios**) 
* An [in-game menu](https://github.com/cdozdil/OptiScaler/blob/master/Config.md) for tuning and saving settings on the fly (Shortcut key is **INSERT**)
* Full integration with [DLSS Enabler](https://www.nexusmods.com/site/mods/757) for DLSS-FG support
* **RCAS** support with **MAS** (Motion Adaptive Sharpening) for all Dx12 & Dx11 with Dx12 upscalers
* **Output Scaling** option (0.5x to 3.0x) for backends running on Dx12 & Dx11 with Dx12
* Supports DXGI spoofing (when running as `dxgi.dll`) as Nvidia GPUs (with XeSS detection to enable XMX on Intel Arc cards)
* Supports Vulkan spoofing (needs to be enabled from `nvngi.ini`) as Nvidia GPUs (not working for Doom Eternal and RTX Remix)
* Supports loading specific `nvapi64.dll` file (when running in non-nvngx mode)
* Supports overriding scaling ratios
* Supports overriding DRS range
* Autofixes for [colored lights](https://github.com/cdozdil/OptiScaler/blob/master/Config.md#resource-barriers-dx12-only) on Unreal Engine & AMD graphics cards 
* Autofixes for [missing exposure texture](https://github.com/cdozdil/OptiScaler/blob/master/Config.md#init-flags) information
* Ability to modify [Mipmap Lod Bias](https://github.com/cdozdil/OptiScaler/blob/master/Config.md#mipmap-lod-bias-override-dx12-only) value of game 
 
## Configuration
Please check [this](Config.md) document for configuration parameters and explanations. *(Will be updated)*

## Known Issues
Please check [this](Issues.md) document for known issues and possible solutions for them.

## Which APIs and Upscalers are Supported?
Currently OptiScaler can be used with DirectX 11, DirectX 12 and Vulkan but each API has different sets of upscaler options.

#### For DirectX 12
* **XeSS 1.x.x** (Default upscaler)
* **FSR2 2.1.2** 
* **FSR2 2.2.1**
* **FSR3 3.1.0 & FSR2 2.3.2**
* **DLSS**

#### For DirectX 11
* **FSR2 2.2.1** native DirectX11 implementation (Default upscaler)
* **XeSS 1.x.x** with background DirectX12 processing [*]
* **FSR2 2.1.2** with background DirectX12 processing [*]
* **FSR2 2.2.1** with background DirectX12 processing [*]
* **FSR3 3.1.0 & FSR2 2.3.2** with background DirectX12 processing [*]
* **DLSS** native DirectX11 implementation

[*] This implementations uses a background DirectX12 device to be able to use Dirext12 only upscalers. There is %10-15 performance penalty for this method but allows much more upscaler options. Also native DirectX11 implementation of FSR 2.2.1 is a backport from Unity renderer and has it's own problems which some of them avoided by OptiScaler. **These implementations does not support Linux** and will result black screen.

#### For Vulkan
* **FSR2 2.1.2** (Default upscaler)
* **FSR2 2.2.1** 
* **FSR3 3.1.0 & FSR2 2.3.2**
* **DLSS**

## Compilation

### Requirements
* Visual Studio 2022

### Instructions
* Clone this repo with all of its submodules.
* Open the OptiScaler.sln with Visual Studio 2022.
* Build the project

## Thanks
* @PotatoOfDoom for CyberFSR2
* @Artur for DLSS Enabler and helping me to implement NVAPI correctly
* @LukeFZ & @Nukem for their great mods and sharing their knowledge 
* @FakeMichau for support, testing and feature creep
* @QM for continous testing efforts and helping me to reach games
* @Cryio, @krispy, @krisshietala, @Lordubuntu, @scz, @Veeqo for their hard work on compatibility matrix
* @RazzerBrazzer and DLSS2FSR community for all their support
