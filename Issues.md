# Known Issues

## In-game Menu
In-game menu is a recent addition have some issues on certain condutions.

* On some system & game combinations opening menu might crash the game or cause graphical corruption (specially in Unreal Engine 5 games).

![Banishers](/images/banishers.png)

* Changing settings mostly tested by might cause crashes (specially changing backends or reinitializing backends).
* Some games does not release mouse control, kayboard & gamepag controls should still work in these situations.

## Exposure Textures
Sometimes games exposure texture format is not recognizable to upscalers. Most of the time manifests itself as crushed colors (specially in dark areas). Most of the time enabling AutoExposure feature should fix these issues.

## Resource Barriers
Unreal Engine DLSS plugin is known for sending DLSS resources in wrong states. Normally OmniScaler check Engine info of NVSDK and enable fixes automatically for Unreal Engine games but some games does not report engine correctly. This problem manifest itself as colored regions at the bottom of the screen most of the time. 

![Banishers](/images/christmas.png)

Solution for this issue is setting `ColorResourceBarrier=4` from `nvngx.ini` or selecting `RENDER_TARGET` for `Color` at `Resource Barriers (Dx12)` from in-game menu.
