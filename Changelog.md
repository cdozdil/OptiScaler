## Release and Build Change Log (Newest to Oldest)

## v0.7.7-pre10
* Enabled usage of Dilated MV's without Depth (aka UE XeSS inputs) when using FSR3.1 (and FSR4) (thanks to hereCOMESlappy)
* Fixed pressedkeys getting stuck while opening menu (thanks to peterberbec)
* Added support for loading ASI files from plugin folder (Needs to be tested)
* Added custom spoofing options which gives you control over spoofed device. SpoofedVendorId & SpoofedDeviceId
* Added custom spoofing options for selecting device(s) to be spoofed. TargetVendorId & TargetDeviceId
* Added hex value support for int/uint ini settings.Specially usefull for shortcut keys and device ids.

## v0.7.7-pre9 (Pre-Release)
* Prevented unloading of hooked DLLs (thanks to **WickedZergling**)
* Added path detection for `nvngx_dlss.dll`, `nvngx_dlssd.dll`, and `nvngx_dlssg.dll` for NVNGX initialization
* Revamped init-flag handling and added buttons to reset values to their defaults
* Added game info (executable and product name) to the in-game menu header (thanks to **IncorrectW**)
* Changed XeSS version-checking method to prevent crashes
* Moved FSR FOV and camera values into a collapsible section (thanks to **TheRazzerMD**)
* Added game quirks for No Man's Sky and Minecraft
* Simplified the `FSR4Upgrade` approach
* Fixed a crash related to `SetFullscreenState`
* Fixed an issue with MAS and Sharpness values above 1.0
* Added a contrast option to RCAS (thanks to **Od1sseas**)
* Made some changes to INI naming and sections
* Improved DXGI and D3D12 mode compatibility
* OptiScaler now checks the local folder first for `amdxcffx64.dll`
* Added upscaler-input and active-upscaler info to the overlay
* Improved general overlay compatibility (Steam, Epic, Overwolf, etc.)
  
## v0.7.7-pre8
* Now by default FG is disabled and need to be selected from menu or ini.
* Better store overlay handling (Still OptiFG will block them by default)
* Disabled forced logging for pre builds
* When switching between games upscalers sometimes Display Res MV setting was not updating
* Added input source info for Dx11 too
* Added Contrast option to RCAS (Higher values might cause image corruption)
* Fixed DLSS input issue on Nvidia (thanks to **DarkH2O**)


## v0.7.7-pre6
* Updated in-game menu for easier DLSS-G/OptiFG selection (thanks to **Vladzor**)
* Added FSR3 Patten Matching for Red Dead Redemption 1 (thanks to **TheRazzerMD**)
* Fixed OptiFG frame pacing (one frame wasn't rendering)
* Added Quirk for RDR1 & updated Cp77 one to disable OptiFG
* Fixed hangs on game boot when Mutex for Present is active (thanks to Burak)
	
	
## v0.7.7-pre5
* DLSS-D (RR) crash fixed (thanks to **FakeMichau**)
* Fixed internal config settings saved to ini (huge thanks to **FakeMichau**)
* Started to use original FSR dll's again, might need to set UseFGSwapChain=false with RDR1
* Hopefully OptiFG should be more stable
* Prevented FSR3 input double hooks
* Added 4 options to fine tune Hudfix and OptiFG (better leave them at defaults 😊)
* Added option to enable/disable FSR2 pattern matching (thanks to **FakeMichau**)
* Made Inputs menu options visible when no upscaler context available (thanks to **Vladzor**)
* Fixed some issues with installer & uninstaller batch file when Nvidia is selected (thanks to **JoeGreen**)
* Added a check to prevent crashes with FMF2 & FSR inputs (thanks to **TheRazzerMD**)
	
	
## v0.7.7-pre4
* Fixed Hudfix for Space Marine 2 and possibly some other games (thanks to **Vladzor**)
* Fixed DLSSG menu option (thanks to **Vladzor**)
	
	
## v0.7.7-pre3
* Tried to smooth-out frametime estimations a bit
* Tried to improve FG logic and prevent crashes
* Added Fsr2Pattern to nvngx.ini to enable FSR2 pattern search (thanks to **FakeMichau**)
* Added DLSSG menu options for setting ini values, need to enable Advanced Settings (thanks to **Vladzor**)
	
	
## v0.7.7-pre2
* Fixed Hudfix compatibility regression, probably not completely (thanks to **Vladzor** & **BayuPratama**)
* Improved FSR2 input compatibility
* Ingame menu improvements (thanks to **FakeMichau**)
	
	
## v0.7.7-pre1
* Added integration with Nukem's dlssg-to-fsr3 (HUGE thanks to **FakeMichau** 🙏🏻)
* Added patter matching for FSR2 inputs, which should increase compatibility with UE games
* Added auto enable for nvapi override when original nvngx.dll is not found (thanks to **FakeMichau**)
* Fixed TLOU FSR3 inputs (thanks to Od1sseas & **BayuPratama**)
	
	
## v.0.7.0-pre93
* Fixed Vulkan & Linux regression (thanks to **Crowland**, **FakeMichau**, **markanini**)
* Fixed fps overlay cycle key registered more than once (thanks to **FakeMichau**)
* Added API info to fps overlay
	
	
## v0.7.0-pre92
* Added one line horizontal layout for Fps Overlay (thanks to **TheRazzerMD** 😁)
* Fixed gamepad interactions when Fps Overlay is visible, turns out it wasn't a bug but a feature of ImGui (thanks to **DarkH2O**)
* Fixed Hot Wheels Unleashed split screen issue for XeSS, sadly FSR does not support it (thanks to **SuperSamus**)
* Made in-game menu and fps overlay more stable by fixing the float value sizes
	
	
## v0.7.0-pre91.5
* Added fps overlay ShowFps option (default off) or from in-game menu
  - Added 5 overlay types which can be set from FpsOverlayType or from in-game menu
  - Added 4 overlay positions (corners) which can be set from FpsOverlayPos or from in-game menu
  - Added shortcut key to show/hide fps overlay (default PGUP) can be changed from FpsShortcutKey
  - Added shortcut key to cycle between overlay types (default PGDOWN) can be changed from FpsCycleShortcutKey
* Fixed a stack overflow issue when loading nvngx.ini again (thanks to **Vladzor** x2)
	
	
## v0.7.0-pre90
* Fixed black screen/no screen when running under Linux (thanks to **FakeMichau**)
* Fixed no menu when working with DLSS Enabler (thanks to **DarkH2O**)
* Added option to not use camera values from FSR inputs (thanks to **Vladzor**)
* Fixed menu UI scale selection (thanks to Od1sseas & **Vladzor**)
* Added option to enable/disable usage of FSR inputs from menu. This allows live switching between games FSR and Opti upscaler (thanks to **TheRazzerMD**)
* Batch file now tries to remove all files & folders of OptiScaler
* Added passing of reset signal from upscaler inputs to OptiFG
* Added auto disabling of spoofing if there is no nvngx.dll in game folder
	
	
## v0.7.0-pre87
* Added input (DLSS/XeSS/FSR) selection to nvngx.ini
* Added a check for custom FSR2/3 implementations to prevent crashes
* Fixed the blank/no game window issue Nvidia notebooks (thanks to **DarkerThanDA**)
	
	
## v0.7.0-pre86
* PreferDedicatedGpu and PreferFirstDedicatedGpu now default disabled
* Now enabled Camera V.Fov, Near & Far in menu when FG Swapchain is active (or any FSR upscaler)
* Changed default Camera Near & Far values to 10 and 500000
* Now FG is using camera values from FSR inputs when available
* Now Opti auto disabling spoofing on non-Nvidia devices when no nvngx.dll file in game folder
* OptiScaler is now skipping format transfer for FSR-FG supported hudless formats
	
	
## v0.7.0-pre85
* Added high performance GPU selection override (thanks to **Artur**)
* PreferDedicatedGpu can be enabled/disabled from nvngx.ini (default enabled)

	
## v0.7.0-pre83
* Added OptiScaler Setup.bat file for auto installation (thanks to **FakeMichau**)
	
	
## v0.7.0-pre82
* Added Fsr2/Fs3 inputs support (Only for Dx12)
* Fixed a few issues with XeSS and Ffx inputs
* HybrisSpin is now default off
* RestoreComputeSignatureis now default off
* Changed default FSR Camera Near/Far values to 10<->♾
	
	
## v0.7.0-pre81
* Fixed CPU performance regression when FG Swapchain active
* Fixed some Ffx inputs issues
* Fixed an issue with UpscaleRatio Overide which mixes width & height of XeSS inputs
	
	
## v0.7.0-pre75
* Tightened Hybrid Spin values to gain some CPU time
* Added correct FSR & XeSS dll's
	
	
## v0.7.0-pre74
* Added FSR 3.1.3 amd_fidelityfx_dx12.dll & amd_fidelityfx_vk.dll
* Added XeSS 2.0 libxess.dll Mistakenly put 1.3.1 libxess.dll to this one 😅
* Added HybridSpin option to nvngx.ini
* Fixed borderless fullscreen black screen issue (Control)
* Reverted mutex from pre73 to prevent freezes
	

## v0.7.0-pre73
* Added FSR 3.1.3 amd_fidelityfx_dx12.dll & amd_fidelityfx_vk.dll
* Added XeSS 2.0 libxess.dll
* Enabled Hybrid Spin support for FG swapchain (might reduce CPU usage)
* Enabled one mutex for FGPresent (might cause or solve issues, needs checking)
	
	
## v0.7.0-pre715 (Vulkan Inputs 🖖🏻)
* Fixed Enshrouded and possible other crashes when using FSR3.1
* Fixed Enshrouded crash issue with FSR3.1 but IQ will suffer. Game is using an image format FSR3.1 does not support. Used an alternative format identifier but motion vectors looks funky 🙂
* Fixed one stack overflow issue (double hooking) which would cause crash
* Probably some other fixes here and there
	
	
## v0.7.0-pre714 (Vulkan Inputs)
* Fixed more issues with XeSS Vulkan inputs
* Improved Vulkan menu stability and prevented some crashes
	
	
## v0.7.0-pre71 (XeSS Vulkan Test Build)	
	
	
## v0.7.0-pre69 (RazzSpecial)
* Added DisableOverlays and AlwaysTrackHeaps (with menu option) options for FG
* Added a check to prevent crashes when not using AlwaysTrackHeaps and resource is not tracked correctly (not %100 solution but should be enough for now)
* Added a few check to prevent possible menu related crashes
* Changed threaded heap tracking with async one to prevent crashes (when game is heavily threaded might cause slowdowns)
* Fixed another mistake on DXGI VRAM spoofing
	
	
## v0.7.0-pre682 (😁)
* Crashes when exiting game
* Crashes when Alt-tab, res change or windowed -> fullscreen -> borderless transitions
* Fix framepacing issues (tried to keep present in order) which might help crashes with Hudfix etc


## v0.7.0-pre66 (Pre-Release)
This build adds **experimental** Frame Generation support to `v0.6.8-pre4` for DLSS & XeSS supported games. 

**Added**
* Added auto-fallback to classical FG when hudless can't be captured
* Added Reflex FPS framerate limit slider (thanks to @FakeMichau #108) 
* Added freetype font for menu (thanks to @FakeMichau #117)
* Added `Immediate Capture` option for HudFix
* Added `Capture Lists` for HudFix, which would help some games (Star Wars: Outlaws etc.)
* Added option for loading `ReShade64.dll`
* Added crude reimplementation of [EndlesslyFlowering](https://github.com/EndlesslyFlowering)'s [AutoHDR-ReShade](https://github.com/EndlesslyFlowering/AutoHDR-ReShade) `ForceHDR` and `UseHDR10` options, don't forget to use `Inverse ToneMapping` shaders
* Added auto disabling of Epic Overlay when using FSR-FG swapchain
* Added FSR 3.1.2 support
* Added resource barrier fixes to FG inputs too (Velocity & Depth, again fixes Death Stranding)
* Added FG Rectangle option for games with black bars (ultra wide etc)
* Added Source Api info to menu (DLSS/XeSS)

**Changed**
* Changed to hook dxgi/d3d12/d3d11/vulkan-1 when game loads them. This should increase compatibility with Death Stranding and Capcom games (thanks to **Maple**)

**Fixed**
* Fixed issues on loading `libxess.dll` (thanks to **FakeMichau**)
* Fixed more resource tracking issues, HudFix should be much more stable now
* Fixed spoofed GPU name issue (Thanks to @MapleHinata)
* Fixed RDR crash issue (only with bundled `amd_fidelityfx_dx12.dll`)
* Fixed XeSS not selectable on Capcom games (Thanks to **FakeMichau** & **BayuPratama**)
* Fixed ini reload logic to not overwrite user changed values


## v0.6.8-pre4 (Pre-Release)
**Added**
* `UseGenericAppIdWithDlss` for DLSS preset overriding on games like Monster Hunter Rise (thanks to @MapleHinata)  #72
* XeSS inputs support. Now if a game supports XeSS upscaling, OptiScaler can use XeSS inputs instead of DLSS. Sadly, **Unreal XeSS Plugin does not provide a depth buffer which leads degraded visual quality.**  
* FakeNvapi menu support (thanks to @FakeMichau) #81

**Fixed**
* DirectX 11 FSR 3.1.1 backend crash issue with old VC Runtime versions. (thanks to @MapleHinata)  #70 #71 
* Prevented using Ultra Quality of preset with real DLSS. (thanks to @MapleHinata) #80 
* XeSS inputs were not working with Death Stranding Director's Cut (thanks to **BayuPratama**)
* Broken DLSS upscaler backend (thanks to @MapleHinata)
* Broken custom `nvngx_dlss.dll` loading option (thanks to @MapleHinata)
* Broken XeSS 1.2 support (thanks to **mayoringram**)


## v0.6.7 (Release)
**Added**
* **Native DirectX11 FSR 3.1.1** upscaler backend (thanks to new contributor @MapleHinata)
* **FSR 3.1.1 libraries**
* FSR 3.1.1 `Velocity Factor` option to in-game menu and `nvngx.ini`. Which let's users to control the balance between image stability & blurriness vs shimmering & shaprness. Lower values are more stable but prone to ghosting. Default value is 1.0.
* In-game menu is now always available when OptiScaler is loaded
* Simple frame & upscaler time graphics
* Disabled spoofing during Vulkan upscaler creation. This might allow non-Nvidia GPUs to use FP16 codepath and gain a bit more performance.
* DXVK detection, now `Overlay Menu` uses Vulkan API with DXVK
* DirectX 11 support for `Force Anisotropic Filtering` and `Mipmap LOD Bias`
* Support for latest version of Visual Studio (thanks to **FakeMichau**)
* FSR 3.1 option for DLSS Enabler generator selection at in-game menu
* `DxgiVRAM` and `SpoofedGPUName` spoofing options (thanks to **ChemGuy1611**)
* `LoadSpecialK` option to load SpecialK manually  (thanks to **DARKERthanDA**)

**Changed**
* From now on `Overlay Menu` is not available for `nvngx.dll` installations.
* Updated ImGui to ~~v1.91.1~~ v1.91.2

**Fixed**
* RCAS was crashing with `Typeless` textures #46, fixes Space Marine 2 (thanks to **LeidenXaXa**)
* Possible crashes while using `Overlay Menu` with RTSS (thanks to **Artur**)
* Crash when switching to `FSR 3.1` from in-game menu (thanks to **Melinch** & **BayuPratama**)
* DirectX11 performance regression after upscaler time calculation (thanks to **Melinch** & **BayuPratama**)
* A typo at menu #49 (thanks to **donizettilorenzo**)
* Compability issue with GoW:Ragnarok #53  (thanks to **TheRazzerMD** & **rigopoui**)
* Possible crashes during resolution changes and Alt+Tab (thanks to **FakeMichau**)
* Menu was not working with Shadow of the Tomb Raider & Rise of the Tomb Raider (thanks to **Merlinch**)
* Wine detection error (thanks to **Lilith**)

## v0.6.6 (Release)
**Added**
* `RCAS`, `Output Scaling` and `Mask Bias` options to FSR2.2 DirectX 11 backend
* `RCAS` and  `Output Scaling`  options to DLSS DirectX 11 backend (thanks to **Sildur** for testing)
* Option to use FSR1 with `Output Scaling` (thanks to **FakeMichau**)
* Option to `Force Anisotropic Filtering` #26  (thanks to **aufkrawall**)

**Changed**
* Made `Output Scaling`, `Mipmap LOD Bias` & `Force Anisotropic Filtering` menu entries a bit more user friendly (thanks to **FakeMichau** & **aufkrawall**)
* Enabled denoiser option of `RCAS` #32 (thanks to **aufkrawall**)
* Increased maximum `RCAS` sharpness to 1.3 #32 (thanks to **aufkrawall**)
* Changed XeSS & FSR3.1 load order to first check OptiScaler and ini folder

**Fixed**
* For scaling ratios below 1.0 DLSS and XeSS can't handle downscaling, now OptiScaler auto-enable `Output Scaling` with 1.0 ratio when needed (thanks to **Merlinch** & **DARKERthanDA**)
* OptiScaler wasn't saving `Force Anisotropic Filtering` value #26 (thanks to **aufkrawall**)
* A possible race condition crash when exiting from Witcher 3 (thanks to **Artur**)
* Possible crash when exiting DirectX 11 games #30  (thanks to **cek33**)
* Possible `Output Scaling` issues (thanks to **Adéline**)
* Broken `Output Scaling` when using DLAA #34 (thanks to **Nygglatho**)
* Can't select DLSS preset F & G from in-game menu (thanks to **Merlinch**)
* Sending above 1.0 values to FSR's sharpener (thanks to **Rigel.**)
  
## v0.6.5 (Release)
**Added**
* **Added support for FSR 3.1.0 upscaler backends**
* **Added support for selecting FSR 2.3.2 when using FSR 3.1.0**
* Added FSR-FG 3.1 `Overlay Menu` support
* Added `VulkanExtensionSpoofing` option to `nvngx.ini` which would spoof `VK_NVX_BINARY_IMPORT` and `VK_NVX_IMAGE_VIEW_HANDLE`. Now **RTX Remix** and **No Man's Sky** (FSR 3.1 is broken) let's users to select DLSS.
* Added tooltip support to in-game menu (thanks to @FakeMichau) #24 
* Added options to use DLSS Input Bias Masks as FSR Reactive (with bias adjustment) and Transparency masks (thanks to **solewalker** & **FakeMichau**)  
* Added precompiled shaders for RCAS, Output Scaling and Mask Bias with menu and `UsePrecompiledShaders` ini option
* Added `NVNGX_DLSS_Path` option to `nvngx.ini` for loading different `nvngx_dlss.dll` file (thanks to **Sildur**)
* Added ability to extend scaling ratio override range to 0.1-6.0. This allows game (not all games support this) to render higher than display resolution and downsample to display resolution. When using with XeSS set Output Scaling to 1.0 (thanks to **FakeMichau** & **krispy**)
* Added Debug View option for FSR 3.1.0 to in-game menu and `nvngx.ini` (needs `Advanced Settings`)
* Added CameraNear / CameraFar (for FSR 2.2 and above) options to in-game menu and `nvngx.ini` (needs `Advanced Settings`)
* Added selective spoofing for DXGI (check `DxgiBlacklist` option in `nvngx.ini`) (Does not work on Linux)
* Added `SkipFirstFrames` option to `nvngx.ini` which would allow OptiScaler to skip rendering desired amount of frames (Needed for **Dragons Dogma 2**) (thanks to **CAPCOM**)
* Added ability to override Mipmap Bias when running non-nvngx mode without resoluion/preset change (thanks to **Ridianoid**)
* Added/Implemented Driver Vendor, Name & Info spoofing for Vulkan spoofing option (Doom Eternal still does not enable DLSS)
* Added missing `NVSDK_NGX_VULKAN_GetFeatureRequirements`, `NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements` and `NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements` methods
* Added `Advanced Settings` option to in-game menu (thanks to **FakeMichau**)
* Added ability to use negative values for `Motion Adaptive Sharpness`
* Added FidelityFX license 

**Changed**
* **Updated Intel XeSS library to 1.3.1**
* Grouped upscaler Init Flag options together at `nvngx.ini`
* Seperated Output Scaling options at `nvngx.ini`
* Changed default FSR Camera Near & Camera Far values to ~~0.0001 & 0.9999~~ 0.01 & 0.99
* Changed DLSS and XeSS path options at `nvngx.ini` to accept both file path and folders. This should let users use XeSS with Uniscaler. (thanks to **MV** & **Voosh**)
* Reverted back to original FSR 2.1.2 (thanks to **DLSS2FSR community**)
* Improved menu hooking routines which speed up init a bit with DirectX 12
* Disabled Reactive Mask checkbox at menu when game is not providing reactive mask texture (thanks to **FakeMichau**)
* Improved DXGI spoofing when OptiScaler is not `dxgi.dll` (thanks to **Sildur**)

**Fixed** 
* Fixed ghosting issues of FSR 3.1 (thanks to **DLSS2FSR community**)
* Fixed FSR 3.1 Vulkan crashes on Windows (Linux still has issues) (thanks to **Matthew Simm**)
* Remove DLSS option from upscaler list when there is no original nvngx.dll (thanks to **Od1ssseas**)
* Fixed crashes during loading when game path contains unicode characters on unicode systems (thanks to **Rigel.**) 
* Improved working mode checks and stability of mod (thanks to **Sildur** & **krispy**)
* Fixed 16XX GPU spoofing for DLSS on Vulkan & Dx11 (thanks to **Merlinch** & **DARKERthanDA**)
* Possible crash during resolution/quality change [issue #11](https://github.com/cdozdil/OptiScaler/issues/11) (thanks to **donizettilorenzo**)
* Fixed/improved `RestoreComputeSignature` & `RestoreGraphicSignature` options (Needed for **Dragons Dogma 2**) (thanks to **CAPCOM**)
* Prevented casting exceptions during ini loading (thanks to **CAPCOM**)
* Fixed Mipmap Bias calculator Display Resolution value when OutputScaling is active (thanks to **Ridianoid**)
* Prevented possible early unloading of mod when running as non-nvngx (thanks to **Merlinch**)
* Added checks to disable `OverlayMenu` when running on Linux with Nvidia GPUs (thanks to **KomiksPL**)
* ~~Improved implementation of GetCapabilityParamers methods~~ Reverted (thanks to **Neostein**)



## v0.6.1 (Release)
This release aims for a more precise implementation of nvngx and bug fixes.

**Added**
* DLSS-D (Ray Reconstruction) support on Nvidia cards (thanks to **Artur**, **AKBLT-YT** and **QM**)
* Automatically redirect calls for features not supported by OptiScaler to original nvngx on Nvidia GPUs.
* Added auto skipping spoofing for XeSS. **This should activate XMX codepath on Intel ARC series GPUs**. It can be disabled from `nvngx.ini` or in-game menu (**only works in dxgi mode**)
* Automatically select DLSS as upscaler on Nvidia GPUs (if not overridden in ini or DLSS Enabler) (thanks to **AKBLT-YT**)
* Auto disable spoofing on Nvidia GPUs (**only works in dxgi mode**)
* Hide DLSS upscaler option for non-Nvidia GPUs
* Added full DLSS Enabler ini capability and in-game menu support (thanks to **FakeMichau** and **...Merlinch?**)
* Added Vulkan spoofing support. It is disabled by default and can be enabled from `nvngx.ini` (**only works in non-nvngx mode**) (Doom Eternal still doesn't work) (thanks to **FakeMichau**)
* Added option to disable DXGI spoofing (**only works in dxgi mode**) 
* Added missing Vulkan Init methods
* Added option to define path for check orginal file when running in non-nvngx mode (**only works in non-nvngx mode**) (thanks to **Artur**)

**Fixed**
* Prevented capturing mouse and keyboard when OptiScaler is not active
* Fixed crashes when running Unity games on non-nvngx mode (thanks to **Erik Coelho**)
* Reading correct rendering resolution on DLSS and DLSSD (thanks to **...Merlinch?**)
* Fixed issue with Vulkan DLSS settings are not being visible after upscaler change (thanks to **...Merlinch?**)
* Fixed possible menu crashes on DirectX 12 only mode (thanks to **Jo**)
* Fixed possible crashes on `LoadLibrary` hooks
* Wrong scaling ratio reporting when using DLAA on Nvidia GPUs (thanks to **...Merlinch?**)


## v0.6.0 (Release)
This release aims to improve the compatibility of the new in-game menu (especially when used with frame-gen) and add support for DLSS 3.7.

### Added
* Added spoofing support for DLSS 3.7, usage description is below
* Added spoofing support when running as `dxgi.dll` (acts like [d3d12-proxy](https://github.com/cdozdil/d3d12-proxy)).
* Added support for loading custom `nvapi64.dll` file when not working as `nvngx.dll` (from dll folder or user defined folder).

### Fixed
* Tried to cover all frame-gen cases for the new in-game menu, hopefully it will be ok (There is a detection delay of about 170 frames, if the menu does not appear immediately after loading the game, please wait a bit after pressing the menu button)
* Fixed setting wrong internal rendering resolution during create feature (thanks to **Sub** & **DARKERthanDA**)
* Fixed when DLSS selected from ini was not applied when using DirectX 11 (thanks to **...Merlinch** & **wixuan**)
* Fixed `Motion Sharpness` issues (thanks to **Od1sseas**)
* Increased the `Motion Sharpness` scale minimum to 0.1 to prevent division by zero problems (thanks to **fakemichau**).
* Added extra checks for `Output Scaling` and `Display Size MV` (Thanks to **DARKERthanDA**)
* Fixed reverting to initial upscaler on quality or resolution change when working with DLSS Enabler (thanks to **fakemichau**).
* Auto disable `RCAS` and `Output Scaling` on shader compilation issues, for linux check notes are at bottom (thanks to **fakemichau**).
* Fixed `Output Scaling` disabling conditions (Thanks to **DARKERthanDA**)
* Fixed conflicting preset selection & quality ratio when using DLSS (thanks to **...Merlinch?**)
* Added `yield()` to /w Dx12 backend query wait spins, resulting in improved performance on Nvidia cards (thanks to **DARKERthanDA**)

### Changed
* The structure of `nvngx.ini` has changed a bit, if your old settings are not applied, please compare with the new structure.


## v0.5.5 (Release)
**Added**
* Added DLSS upscaling support, when using DLSS with OptiScaler you will have DLSS specific;
  - Control over DLSS presets (DLSS 3.x only)
  - Auto enabling DLSS support for 16XX GPUs
  - Added spoofing for GetFeatureRequirements
  - Override DLSS sharpening (up to DLSS 2.5.1)
* Also with DLSS you can use OptiScaler's;
  - Scaling overrides
  - Override DLSS init flags
  - In-game menu
  - RCAS sharpening (only on Dx12)
  - Output resolution scaling (only on Dx12)
* Added **new in-game menu** which works on all APIs and does not affected by games rendering pipeline. In case of compatibility issues (or for nostalgia 😅 ) old menu still can be enabled by setting `OverlayMenu=false` from `nvngx.ini` (Thanks to **TheRazzerMD**, **regar** **DARKERthanDA**, **Brutale1090**, **iamnotstanley** and **Od1sseas** for all the testing). 
* Added `Motion Sharpening` option, which increases sharpening amount of pixels based on their motion. (RCAS only) (Thanks to **Od1sseas**)
* Added option to select RCAS for FSR upscalers (Dx12 and Dx11 with Dx12 only)
* Added an option to round internal resolution to desired number
* Added DRS (Dynamic Resolution Scaling) now have options to limit minimum or maximum resolution to render resolution. 
* Added more integration options with [DLSS Enabler](https://www.nexusmods.com/site/mods/757)

**Changed**
* XeSS upscaling library `libxess.dll` is not statically linked anymore. This means now even without `libxess.dll` mod works (ofc without XeSS support). With new ini option users can point a folder which contains library or even if game natively supports XeSS mod now uses library loaded by game. Also with limited testing I can confirm 2 different versions of XeSS libraries (for example game loads 1.2.0 and mod loads 1.3.0) coexist and work on same instance.
* CAS sharpening replaced with RCAS, which solves issues with bloom, glow and other effects etc.
* Super Sampling option evoled into Output Scaling, now it's working based on display resolution and in 0.5 - 3.0 scaling range. This allows users to set lower target resolutions than display resolutions which was requested by Deck users. 
* Restructured in-game menu to show only currently available options to prevent confusion.
* DRS range limited to 0.5x - 1.0x to mimic Nvidia aproach

**Fixed**
* Sometimes DLSS upscaler had wrong motion vectors setup, now fixed (Thanks to **DARKERthanDA**)
* Some games were not releasing the mouse, now they should be fixed. (Cyberpunk 2077, Horizon Zero Dawn, etc.) 
* Some games were not registering mouse clicks, now they should be fixed. (Doom Eternal) (Thanks to **TheRazzerMD**)
* Dx11 XeSS libxess.dll not found fallback fixed
* Added controls for junk Vulkan DLSS init data and prevent crashes
* Added SDK version checks for logging options


## v0.4.3 (Release)
New:
* Added an in-game menu scaling option

Changed
* Added new options for Dx11wDx12 creation & syncing. I have tried to explain it [here](https://github.com/cdozdil/OptiScaler/blob/master/Config.md#dx11withdx12-sync-settings). These settings are game & hw dependant so users need to tweak and find best performing configuration for their system. I have set safe and balanced values as default values.


## v0.4.2 (Release)
New:
* XeSS now have a new parameter `CreateHeaps` (default **true**). When it's enabled OptiScaler internally create heap objects and bind them instead of relying XeSS to create them. This fixes Guardians of the Galaxy black screen with XeSS issue. (Thanks to **TheRazzerMD**)

# OptiScaler 

## v0.4.1 (Release)
Mod renamed to **OptiScaler**

Fixed:
* Crysis 3 Remastered showing black screen w/Dx12 backends on non-Arc GPUs fixed. (Thanks to **closesim** & **TheRazzerMD**)

Changed:
* Dx11wDx12 backends had a few seconds worth of delays to ensure stability and compatibility. After the improvements made in v0.4 release looks like these delays aren't necessary anymore. For compatiblity sake I have added a new parameter `UseDelayedInit` (default **false**) for enabling these delays again.

## v0.4.0 (Release)
New:
* Added XeSS 1.3 support and matched DLSS quality modes with XeSS quality (with v1.3 libxess.dll mod uses UltraPerformance & NativeAA modes of XeSS)
* Added ImGui in-game menu with saving ini ability for Dx11 and Dx12 backends (shortcut key is **HOME**)
* Added Pseudo-SuperSampling option for DirectX 12 upscalers
* Added option to change MipmapLodBias
* Added option to restore Compute and Graphic root signatures after upscaling
* Added [this](https://github.com/GPUOpen-Effects/FidelityFX-FSR2/pull/27) patch to FSR 2.1.2 (DLSS depth buffer is always in 0.0-1.0 range it might help on dilated depth)
* Added auto enabling logic for AutoExposure
* Added auto disabling logic for DisplaySizeMotionVectos
* Added option to limit Maximum Dynamic Resolution (DRS)
* Added support for NVNGX logging API (for better integration with [DLSS Enabler](https://www.nexusmods.com/site/mods/757))
* Added documentation for [settings](https://github.com/cdozdil/OmniScaler/blob/master/Config.md) and [issues](https://github.com/cdozdil/OmniScaler/blob/master/Config.md) 

Fixes:
* CAS is now disabled by default for compatiblity reasons, can be enabled from in-game menu or from ini
* CAS errors will not cause black screen or screen not updating but CAS will be auto disabled
* Fixed `UpscaleRatioOverride`, it works correctly now
* Fixed `QualityRatioOverride` min witdh limiting for DRS
* Fixed blackscreen issue with Dx11wDx12 upscalers (Thanks to **Sildur** and **KITE**)
* Added simple logic to check if upscaler is running to enable menu shortcut key (Thanks to **krispy**)
* Fixed Crysis 3 black screen issue
* Fixed default ini & log saving location, now it's next to `nvngx.dll` file
* *More and more* Dx11 with Dx12 stability and performance improvements (Still not enough for Arc tho 😢 )
* ~~Fixed washed out colors with XeSS in Alan Wake 2 & Stranger of Paradise - Final Fantasy Origin (ColorSpaceFix option in ini)~~ Thanks to [Intel](https://github.com/intel/xess/issues/19) not needed anymore

## v0.3.3 (Release)
New:
* Added FSR2 (2.1.2) support for DX11 (w/DX12), DX12 and Vulkan
* Added FSR2 (2.2.1) support for DX11 (native & w/DX12 options), DX12 and Vulkan
* Improved performance of w/DX12 **a lot**, should be around **%85-90** of native performance now (if experiencing crashes, there is a new ini option for compatibility)
* Added black screen fix for native FSR2 DX11 games (God of War, Nioh II, etc.)
* Added auto-fixes for Unreal Engine 4/5 games (Xmas lights, weird colors etc.)
* Added CAS support back for XeSS (default enabled)
* Added DLSS Enabler upscaler selection integration
* Added Vertical Fov calculation from horizonal fov for FSR (not tested)
* And much more minor fixes that I can't remember now

Fixes:
* Fixed a problem with FSR backends, init parameter MaxRenderResolution is now DisplayResolution (Fixed black screen problem with Ratchet & Clank - Rift Apart, Spiderman)


## v0.2.0 (Release)
* Thanks to **Artur** much better implementation of NVNGX API now CyberXeSS is usable with his excellent [DLSS Enabler](https://www.nexusmods.com/site/mods/757)
* Now DLSS Sharpness value is used for CAS (can be forced from ini)
* Now DisableReactiveMask is default true for better compatibility (can be changed from ini)
* Fix for HDR DLSS flag (was inverted in some occasions)
* Removed DelayedInit option which was breaking DLSS 
* Added ResourceBarrier support for all input sources (can be set from ini)
* Added optional quality ratio overrides for fixing flickering in some Unreal Engine game (can be enabled from ini)
* Added libxess.dll to package for easy installation

  
## v0.1.0 (Release)
* Added CAS support (Check ini file for options)
* Added alpha Dx11 support (with possible performance & compatibility problems)
* Added fix (ini option ColorResourceBarrier=true) for rainbow colored texture problems on AMD hardware (thanks to Nukem & LukeFZ)
* Added logging console option (Any logging level below 2 may cause performance issues)
* General stability improvements

# CyberXeSS 
