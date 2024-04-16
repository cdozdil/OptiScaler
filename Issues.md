# Known Issues

## In-game Menu
In-game menu is a recent addition and might have some issues on certain condutions.

* On some system and game combinations opening in-game menu might crash the game or cause graphical corruption (specially in Unreal Engine 5 games).

![Banishers](/images/banishers.png)<br>*Banishers: Ghosts of New Eden*

* Changing settings mostly tested but might cause crashes (specially changing backends or reinitializing backends).
* Some games does not release mouse control, kayboard & gamepag controls should still work in these situations.

## Exposure Texture
Sometimes games exposure texture format is not recognizable to upscalers. Most of the time manifests itself as crushed colors (specially in dark areas). 

![exposure](/images/exposure.png)<br>*Shadow of the Tomb Raider*

Most of the time enabling `AutoExposure=true` from `nvngx.ini` or selecting `Auto Exposure` at `Init Parameters` from in-game menu should fix these issues.

## Resource Barriers
Unreal Engine DLSS plugin is known for sending DLSS resources in wrong states. Normally OmniScaler check Engine info of NVSDK and enable necessary fixes automatically for Unreal Engine games but some games does not report engine info correctly. This problem manifest itself as colored regions at the bottom of the screen most of the time. 

![christmas lights](/images/christmas.png)<br>*Deep Rock Galactic*

Solution for this issue is setting `ColorResourceBarrier=4` from `nvngx.ini` or selecting `RENDER_TARGET` for `Color` at `Resource Barriers (Dx12)` from in-game menu.

## CAS (Conrast Adaptive Sharpening)
CAS added to XeSS backends to mitigate softness upscaler but CAS might cause some image issues too.

![cas](/images/cas.png)<br>*Deep Rock Galactic*

1. Bloom removed
2. Color tone is changed

## Performance Issues
* In general XeSS is heavier then FSR for GPUs so it's expected to be have lower performance even on Intel Arc GPUs.
* As a result of spoofing Nvidia card to enable DLSS some games would use Nvidia optimized codepath which might lead to lower performance on other GPUs.

## Graphichal Corruption and Crashes
As said earlier spoofing a Nvidia card may lead games to use specific codepaths that may cause

* Graphichal corruptions
  
![talos principle 2](/images/talos.png)<br>*Talos Principle 2*

* And crashes, speacially when Ray Tracing is active.
