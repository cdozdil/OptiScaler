# Known Issues

## In-game Menu
In-game menu is a recent addition and might have some issues on certain condutions.

* On some system and game combinations opening in-game menu might crash the game or cause graphical corruption (specially in Unreal Engine 5 games).

![Banishers](/images/banishers.png)

* Changing settings mostly tested but might cause crashes (specially changing backends or reinitializing backends).
* Some games does not release mouse control, kayboard & gamepag controls should still work in these situations.

## Exposure Texture
Sometimes games exposure texture format is not recognizable to upscalers. Most of the time manifests itself as crushed colors (specially in dark areas). 

![exposure](/images/exposure.png)

Most of the time enabling `AutoExposure=true` from `nvngx.ini` or selecting `Auto Exposure` at `Init Parameters` from in-game menu should fix these issues.

## Resource Barriers
Unreal Engine DLSS plugin is known for sending DLSS resources in wrong states. Normally OmniScaler check Engine info of NVSDK and enable fixes automatically for Unreal Engine games but some games does not report engine correctly. This problem manifest itself as colored regions at the bottom of the screen most of the time. 

![christmas lights](/images/christmas.png)

Solution for this issue is setting `ColorResourceBarrier=4` from `nvngx.ini` or selecting `RENDER_TARGET` for `Color` at `Resource Barriers (Dx12)` from in-game menu.
