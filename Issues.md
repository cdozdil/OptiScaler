# Known Issues

## In-game Menu

* Some games do not release mouse control, kayboard & gamepad controls should still work in these situations.
* On some system and game combinations, opening the old in-game menu may cause the game to crash or cause graphical corruption (especially in Unreal Engine 5 games).

![Banishers](/images/banishers.png)<br>*Banishers: Ghosts of New Eden*

* Changing settings mostly tested but might cause crashes (especially changing backends or reinitializing backends).
* In games that use Unity Engine old in-game menu will be upside down.

![barrel roll](/images/upsidedown.png)<br>*Sons of Forest*

## DirectX 11 with DirectX 12 Upscalers
This implementations uses a background DirectX12 device to be able to use Dirext12 only upscalers. There is %10-15 performance penalty for this method but allows much more upscaler options. 

## Exposure Texture
Sometimes games exposure texture format is not recognized by upscalers. Most of the time manifests itself as crushed colors (especially in dark areas). 

![exposure](/images/exposure.png)<br>*Shadow of the Tomb Raider*

In most cases, enabling `AutoExposure=true` in `nvngx.ini` or selecting `Auto Exposure` in `Init Parameters` from the in-game menu should fix these issues.

## Resource Barriers
The Unreal Engine DLSS plugin is known to send DLSS resources in the wrong state. Normally OptiScaler checks engine info from NVSDK and automatically enables necessary fixes for Unreal Engine games, but some games do not report engine info correctly. This problem usually manifests itself as colored areas at the bottom of the screen.

![christmas lights](/images/christmas.png)<br>*Deep Rock Galactic*

Workaround is to set `ColorResourceBarrier=4` from `nvngx.ini` or select `RENDER_TARGET` for `Color` at `Resource Barriers (Dx12)` from the in-game menu.

## Black Screen with XeSS
Some users have reported that when using XeSS upscaler backend, the result is a black screen with UI. In some cases downloading the latest version of [DirectX Shader Compiler](https://github.com/microsoft/DirectXShaderCompiler/releases) and extracting `dxcompiler.dll`, `dxil.dll` from `bin\x64\` next to the game exe file resolved this issue.  

## Minecraft RTX
XeSS 1.1 has the best compatibility with Minecraft RTX. But I've seen reports that with [various launchers](https://github.com/MCMrARM/mc-w10-version-launcher/releases) it's possible to use 1.2 and above as well.

## Shader Compilation error on Linux
If you are using OptiScaler with Linux and you have problems with `RCAS`, `Reactive Mask Bias` or `Output Scaling`, you will probably notice this message in your logs.
```
CompileShader error compiling shader : <anonymous>:83:26: E5005: Function "rcp" is not defined.
```
To solve this problem you need to install `d3dcompiler_47` with `WineTricks` or `ProtonTricks`. OptiScaler uses custom shaders for these features and depends on this compiler file to compile these shaders at runtime. 

## Performance Issues
* In general XeSS is heavier than FSR for GPUs so it's expected to be have lower performance even on Intel Arc GPUs.
* As a result of spoofing the Nvidia card to enable DLSS some games would use an Nvidia-optimized codepath which may result in lower performance on other GPUs.

## Display Resolution Motion Vectors
Sometimes games would set the wrong `DisplayResolution` init flag, resulting in excessive motion blur. Setting or resetting `DisplayResolution` would help resolve this issue.

![mv wrong](/images/mv_wrong.png)<br>*Deep Rock Galactic*

## Graphichal Corruption and Crashes
As mentioned above, spoofing an Nvidia card can cause games to use special codepaths that can cause

* Graphichal corruptions
  
![talos principle 2](/images/talos.png)<br>*Talos Principle 2*

* And crashes, especially when raytracing is enabled.

