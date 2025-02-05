![opti-logo](https://github.com/user-attachments/assets/c7dad5da-0b29-4710-8a57-b58e4e407abd)

**OptiScaler** is a middleware that enables manipulation with various upscaling tech in temporal upscaler-enabled games. 

While previously only DLSS2+ inputs were supported, newer versions also added support for XeSS and FSR2+ inputs (_with some caveats-*_). For example, if a game has DLSS only, the user can hook over DLSS and replace it with XeSS or FSR 3.1 (same goes for an FSR or XeSS-only game). It also offers extensive customization options for all users, including those with Nvidia GPUs using DLSS.

**Key aspects of OptiScaler:**
- Enables usage of XeSS, FSR2, FSR3 and DLSS in upscaler-enabled games
- Allows users to fine-tune their upscaling experience
- Offers a wide range of tweaks and enhancements (RCAS & MAS, Output Scaling, DLSS Presets, Ratio & DRS Overrides etc.)
- With version 0.7.0 and above, added experimental frame generation support with possible HUDfix solution (OptiFG by FSR3)
- Supports integration with [**Fakenvapi**](https://github.com/FakeMichau/fakenvapi) which enables Reflex hooking and injecting Anti-Lag 2 or LatencyFlex (LFX)  
- Since version 0.7.7, support for Nukem's FSR FG mod [**dlssg-to-fsr3**](https://github.com/Nukem9/dlssg-to-fsr3) has also been added

_[*] Regarding XeSS, Unreal Engine plugin does not provide depth, and as such renders XeSS hooking in UE pointless (recommended to set `XeSS=false` in OptiScaler.ini for such games). Regarding FSR inputs, FSR 3.1 is the first version with a fully standardised and forward-looking API and as such removed custom interfaces. Since FSR2 and FSR3 support custom interfaces, game support will depend on the developers' implementation. Always check the [Wiki](https://github.com/cdozdil/OptiScaler/wiki) Compatibility list for known issues._

## How it works?
OptiScaler implements the necessary API methods of DLSS2+ & NVAPI, XeSS and FSR2+ to act as a middleware. It interprets calls from the game and redirects them to the chosen upscaling backend, allowing games using one technology to use another of your choice.

## Official Discord Server: [DLSS2FSR](https://discord.gg/2JDHx6kcXB)

*This project is based on [PotatoOfDoom](https://github.com/PotatoOfDoom)'s excellent [CyberFSR2](https://github.com/PotatoOfDoom/CyberFSR2).*

## Which APIs and Upscalers are Supported?
Currently OptiScaler can be used with DirectX 11, DirectX 12 and Vulkan, but each API has different sets of upscaler options.

#### For DirectX 12
- XeSS (Default)
- FSR2 2.1.2, 2.2.1
- FSR3 3.1 (and FSR2 2.3.2)
- DLSS


#### For DirectX 11
- FSR2 2.2.1 (Default, native DX11)
- FSR3 3.1.2 (unofficial port to native DX11)
- XeSS 1.x.x, FSR2 2.1.2, 2.2.1, FSR3 3.1 & FSR2 2.3.2 (via background DX12 processing) [*]
- DLSS (native DX11)
- XeSS 2.x (_soon™, but Intel ARC only_)

_[*] These implementations use a background DirectX12 device to be able to use Dirext12-only upscalers. There is a 10-15% performance penalty for this method, but allows much more upscaler options. Also native DirectX11 implementation of FSR 2.2.1 is a backport from Unity renderer and has its own problems of which some were fixed by OptiScaler. **These implementations do not support Linux** and will result in a black screen._

#### For Vulkan
- FSR2 2.1.2 (Default), 2.2.1
- FSR3 3.1 (and FSR2 2.3.2)
- DLSS
- XeSS 2.x (_soon™_)


## Installation
> **Warning**: **Do not use this mod with online games.** It may trigger anti-cheat software and cause bans!

### Install as `non-nvngx` (For enabling all features like Frame Gen)
To overcome DLSS 3.7's signature check requirements, I implemented a method developed by **Artur** (creator of [DLSS Enabler](https://www.nexusmods.com/site/mods/757?tab=description)). Later, this method increased the compatibility of `OverlayMenu`, allowed OptiScaler to **spoof DXGI and Vulkan**, let users override the `nvapi64.dll` and even let users to force Anisotropic Filtering and Mipmap Lod Bias. In short, this installation method allowed OptiScaler to provide more features to users.

**`Step-by-step installation`** (**Nvidia users, please skip to step 3**):
1. We need an Nvidia signed dll file to bypass signature checks. All games that support DLSS come with `nvngx_dlss.dll`. Most of the time it's in the games exe folder (e.g. for Unreal Engine games it's `<path-to-game>\Binaries\Win64\`). Some games and engines keep these third party dll's in different folders (like `plugins`). So we need to find the `nvngx_dlss.dll` file and copy it to the games exe folder. If it's already in the games exe folder, make a copy of it. (_You can also just download one from the internet if you don't want to search for it_)
2. Rename the copy of `nvngx_dlss.dll` in the games exe folder to `nvngx.dll`.
3. Rename OptiScaler's `nvngx.dll` to one of the [supported filenames](#optiscaler-supports-these-filenames) (I prefer `dxgi.dll`) [1].
4. Copy the renamed OptiScaler file along with the rest of the archive files to your game's executable folder.
5. If your GPU is not an Nvidia one, check [GPU spoofing options](Spoofing.md).

**Alternatively**, you can also extract all of the Optiscaler files by the main game exe and try the new `OptiScaler Setup.bat` file which should help you automate the renaming process.

**_Example of correct installation (with additional Fakenvapi and Nukem mod)_**
 
![Installation](https://github.com/user-attachments/assets/d2ef6d7b-59a2-45b5-96b0-38e61429cf6b)

#### OptiScaler supports these filenames
* dxgi.dll 
* winmm.dll
* version.dll
* wininet.dll
* winhttp.dll
* OptiScaler.asi (with an ASI loader)

*[1] Linux users should add renamed dll to overrides:*
```
WINEDLLOVERRIDES=dxgi=n,b %COMMAND% 
```

If there is another mod (e.g. Reshade etc.) that uses the same filename (e.g. `dxgi.dll`), if you rename it with the `-original` suffix (e.g. `dxgi-original.dll`), OptiScaler will load this file instead of the original library.   

Alternatively, you can create a new folder called `plugins` and put other mod files in this folder. OptiScaler will check this folder and if it finds the same dll file (for example `dxgi.dll`), it will load this file instead of the original library. 

![image](https://github.com/cdozdil/OptiScaler/assets/35529761/c4bf2a85-107b-49ac-b002-59d00fd06982)

**Please don't rename the ini file, it should stay as `OptiScaler.ini`**.

#### OptiFG (powered by FSR3 FG) + HUDfix (experimental HUD ghosting fix) 
OptiFG was added with 0.7 builds and is only supported in DX12. It uses FSR3 FG to enable Frame Generation in every DX12 upscaler-enabled games, however it's not so simple. Since FSR3 FG doesn't support HUD interpolation itself, it requires a HUDless resource provided by the game to avoid HUD ghosting. In games without native FG, Optiscaler tries to find the HUDless resource when the user enables HUDfix. Depending on how the game draws its UI/HUD, Optiscaler may or may not be successful in fixing these issues. There are several options for tuning the search. A more detailed guide will be available in the [Wiki](https://github.com/cdozdil/OptiScaler/wiki), along with a list of HUDfix incompatible games.

### Install as `nvngx.dll` (deprecated, limited features, FG and Overlay Menu will be disabled)
`Step-by-step installation:`
1. Download the latest relase from [releases](https://github.com/cdozdil/OptiScaler/releases).
2. Extract the contents of the archive next to the game executable file in your games folder. (e.g. for Unreal Engine games it's `<path-to-game>\Binaries\Win64\`) [1]
3. Run `EnableSignatureOverride.reg` from `DlssOverrides` folder and confirm merge. [2][3]
4. If your GPU is not an Nvidia one, check [GPU spoofing options](Spoofing.md).

*[1] This package contains latest version of `libxess.dll` and if the game folder contains any older version of the same library, it will be overwritten. Consider backing up or renaming existing files.*

*[2] Normally Streamline and games check if nvngx.dll is signed, by merging this `.reg` file we are overriding this signature check.*

*[3] Adding signature override on Linux - There are many possible setups, this one will focus on Steam games:*
* *Make sure you have protontricks installed*
* *Run in a terminal protontricks <steam-appid> regedit, replace <steam-appid> with an id for your game*
* *Press "registry" in the top left of the new window -> `Import Registry File` -> navigate to and select `EnableSignatureOverride.reg`*
* *You should see a message saying that you successfully added the entries to the registry*

*If your game is not on Steam, it all boils down to opening regedit inside your game's prefix and importing the file.*

## Update OptiScaler version when using DLSS Enabler  
1. Delete/rename `dlss-enabler-upscaler.dll` in game folder
2. Extract `nvngx.dll` file from OptiScaler 7zip file to a temp folder
3. Rename `nvngx.dll` to `dlss-enabler-upscaler.dll`
4. Copy `dlss-enabler-upscaler.dll` from temp folder to the game folder

## Uninstallation
* Run `DisableSignatureOverride.reg` file 
* Delete `EnableSignatureOverride.reg`, `DisableSignatureOverride.reg`, `nvngx.dll`, `OptiScaler.ini` files (if you used Fakenvapi and/or Nukem mod, then also delete `fakenvapi.ini`, `nvapi64.dll` and `dlssg_to_fsr3` files)
* If there was a `libxess.dll` file and you have backed it up, delete the new file and restore the backed up file. If you overwrote/replaced the old file, **DO NOT** delete `libxess.dll` file. If there was no `libxess.dll` before, it's safe to delete. Same goes for FSR files (`amd_fidelityfx`).

## Features
* Supports multiple upscaling backends (XeSS, FSR 2.1.2, FSR 2.2.1, FSR 3.1 and DLSS)
* Experimental support for frame generation (OptiFG by FSR) with version 0.7.0 and above
* Supports DLSS 3.7 and above (check [installation instructions](#install-as-non-nvngx))
* Supports DLSS-D (Ray Reconstruction) on Nvidia cards (Supports changing presets and using OptiScaler enhancements)
* Ability to modify DLSS/DLSS-D presets on the fly
* Supports XeSS v1.3.x's Ultra Performance, NativeAA modes (**Not using default XeSS 1.3.x scaling ratios, rather the old ones**) 
* An [in-game menu](https://github.com/cdozdil/OptiScaler/blob/master/Config.md) for tuning and saving settings on the fly (Shortcut key is **INSERT**)
* Full integration with [DLSS Enabler](https://www.nexusmods.com/site/mods/757) for DLSS-FG support
* **RCAS** support with **MAS** (Motion Adaptive Sharpening) for all Dx12 & Dx11 upscalers
* **Output Scaling** option (0.5x to 3.0x) for backends running on Dx12 & Dx11
* Supports DXGI spoofing (when running as `dxgi.dll`) as Nvidia GPUs (with XeSS detection to enable XMX on Intel Arc cards)
* Supports Vulkan spoofing (needs to be enabled from `nvngi.ini`) as Nvidia GPUs (not working for Doom Eternal)
* Supports loading specific `nvapi64.dll` file (when running in non-nvngx mode)
* Supports loading specific `nvngx_dlss.dll` file (when running in non-nvngx mode)
* Supports overriding scaling ratios
* Supports overriding DRS range
* Autofixes for [colored lights](https://github.com/cdozdil/OptiScaler/blob/master/Config.md#resource-barriers-dx12-only) on Unreal Engine & AMD graphics cards 
* Autofixes for [missing exposure texture](https://github.com/cdozdil/OptiScaler/blob/master/Config.md#init-flags) information
* Ability to modify [Mipmap Lod Bias](https://github.com/cdozdil/OptiScaler/blob/master/Config.md#mipmap-lod-bias-override-dx12-only) game value
* Supports [Fakenvapi](https://github.com/FakeMichau/fakenvapi) integration which enables Reflex hooking and injecting Anti-Lag 2 or LatencyFlex (LFX)
* Supports Nukem's FSR FG mod [dlssg-to-fsr3](https://github.com/Nukem9/dlssg-to-fsr3) (since version 0.7.7)  
 
## Configuration
Please check [this](Config.md) document for configuration parameters and explanations. *(Will be updated)*

## Known Issues
If you can't open in-game menu:
1. Please check that you have enabled DLSS, XeSS or FSR from game options
2. Please try opening menu while you are in-game (while 3D rendering is happening)
3. If you are using RTSS (MSI Afterburner, CapFrameX), please enable this setting in RTSS and/or try updating RTSS.
 
 ![image](https://github.com/cdozdil/OptiScaler/assets/35529761/8afb24ac-662a-40ae-a97c-837369e03fc7)

Please check [this](Issues.md) document for the rest of the known issues and possible solutions for them. Also check the community [Wiki](https://github.com/cdozdil/OptiScaler/wiki) for possible game issues and HUDfix incompatible games.


## Compilation

### Requirements
* Visual Studio 2022

### Instructions
* Clone this repo with all of its submodules.
* Open the OptiScaler.sln with Visual Studio 2022.
* Build the project

## Thanks
* @PotatoOfDoom for CyberFSR2
* @Artur for DLSS Enabler and helping me implement NVAPI correctly
* @LukeFZ & @Nukem for their great mods and sharing their knowledge 
* @FakeMichau for continous support, testing and feature creep
* @QM for continous testing efforts and helping me to reach games
* @TheRazerMD for continous testing and support
* @Cryio, @krispy, @krisshietala, @Lordubuntu, @scz, @Veeqo for their hard work on [compatibility matrix](https://docs.google.com/spreadsheets/d/1qsvM0uRW-RgAYsOVprDWK2sjCqHnd_1teYAx00_TwUY)
* And the whole DLSS2FSR community for all their support

## Credit
This project uses [FreeType](https://gitlab.freedesktop.org/freetype/freetype) licensed under the [FTL](https://gitlab.freedesktop.org/freetype/freetype/-/blob/master/docs/FTL.TXT)
