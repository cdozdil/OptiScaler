# Configuration
This document will try explain `nvngx.ini` / in-game menu settings as much as possible.

![in-game menu](images/ingame_menu.png)

### Upscalers
OmniScaler supports DirectX 11, DirectX 12 and Vulkan APIs with multiple upscaler backends. You can select which upscaler you would like to use from `[Upscalers]` section in `nvngx.ini` file.

```ini
[Upscalers]
; Select upscaler for Dx11 games
; fsr22 (native dx11), xess (with dx12), fsr21_12 (dx11 with dx12) or fsr22_12 (dx11 with dx12)
; Default (auto) is fsr22
Dx11Upscaler=auto

; Select upscaler for Dx12 games
; xess, fsr21 or fsr22
; Default (auto) is xess
Dx12Upscaler=auto

; Select upscaler for Vulkan games
; fsr21 or fsr22
; Default (auto) is fsr21
VulkanUpscaler=auto
```

* `fsr21` means FSR 2.1.2
* `fsr22` means FSR 2.2.1
* `xess` means XeSS

*For DirectX11 `fsr21_12`, `fsr22_12` and `xess` uses a background DirectX12 device to be able to use Dirext12 only upscalers. There is %10-15 performance penalty for this method but allows much more upscaler options. Also native DirectX11 implementation of FSR 2.2.1 is a backport from Unity renderer and has it's own problems which some of them avoided by OmniScaler.*

For selecting upscalers from in-game menus `Upscalers` section could be used.

![upscalers](images/Upscalers.png)

### Pseudo SuperSampling
With OmniScaler 0.4 there are new options for pseudo supersampling under `[Upscalers]`

```ini
[Upscalers]
; Enables supersampling option for Dx12 and Dx11 with Dx12 backends
; true or false - Default (auto) is false
SuperSamplingEnabled=auto

; Supersampling ratio 
; 0.0 - 5.0 - Default (auto) is 2.5
SuperSamplingMultiplier=auto
```
To explain it clearly for example normally when your game is running at 1080p and `Quality` selected as DLSS preset it would render a 720p image and sends it to upscaler with other necessary input information and generates a 1080p image as output.

When supersampling is enabled it uses `SuperSamplingMultiplier` to calculate target render size of upscaler. For 720p with default multiplier (2.5) it would be 1800p. So now upscaler will upscale image to 1800p instead of 1080p then OmniSaler will downsample output image to 1080p.

![pseudo superSampling](images/pss.png)

Because of higher upscale target resolution there will be a performance loss compared to just upscaling. But subjectively it could create close to DLAA quality images with higher performance levels. 

From in-game menu it can be changed with realtime results.

![pss config](images/pss_config.png)

### Dx11withDx12 Sync Settings
For DirectX11 with `fsr21_12`, `fsr22_12` and `xess` upscaler options OmniScaler uses a background DirectX12 device to be able to use these Dirext12 only upscalers. This is a really niche feature and might cause insctability with GPU drivers (Specially on Intel). To mitigate and prevent crashes or graphical issues this option could be used.

```ini
[Dx11withDx12]
; Safe syncing measures for Dx11 with Dx12
; Might be needed for Intel Arc cards or different Dx11 drivers
;
; Valid values are;
;	0 - Safe syncing is off                         (fastest, most prone to errors)
;	1 - Only Fences
;	2 - Fences + Flush after Dx11 texture copies
;	3 - Sync after output copy				
;	4 - No fences, all sync done with queries       (slowest)
;
; 0 is fastest, 4 is slowest
;
; Default (auto) is 1
UseSafeSyncQueries=auto
```

From in-game menu it can be changed with realtime results.

![dx11 sync setings](images/dx11sync.png)

### XeSS Settings

```ini
[XeSS]
; Building pipeline for XeSS before init
; true or false - Default (auto) is true
BuildPipelines=auto 

;Select XeSS network model
; 0 = KPSS
; 1 = Splat
; 2 = Model 3
; 3 = Model 4
; 4 = Model 5
; 5 = Model 6
;
; Default (auto) is 0
NetworkModel=auto

[CAS]
; Enables CAS shapening for XeSS
; true or false - Default (auto) is true
Enabled=auto

; Color space conversion for input and output
; Possible values are at the end of the file - Default (auto) is 0
ColorSpaceConversion=auto
```

`BuildPipelines` parameter allows building of XeSS pipelines during context creation to prevent stutter later on.

`NetworkModel` is for selecting network model to be used with XeSS upscaling. **(Currently have no visible effect on upscaled image)**

#### CAS
Normally XeSS tend to create softer final image compared to other upscalers and have no sharpening option to mitigate it. So OmniScaler allows you to use AMD's CAS sharpening filter on final image to balance upscaled images soft look. CAS is not perfect tho, on some games it causes some artifacts/issues like dissapering bloom effects, shifting color tone of image or causing black screen with no image at all.

`ColorSpaceConversion` for fixing color space conversion issues but **almost always** default setting would work ok.

From in-game menu it can be changed with realtime results.

![xess](images/xess.png)

`Dump` option is for debugging purposes, which would dump input and output parameters and textures for XeSS to game folder.

### FSR Settings

```ini
[FSR]
; 0.0 to 180.0 - Default (auto) is 60.0
VerticalFov=auto

; If vertical fov is not defined will be used to calculate vertical fov
; 0.0 to 180.0 - Default (auto) is off
HorizontalFov=auto
```

For improving image quality you might try to match vertical or horzontal FOV of your game with these settings. Default is 60° vertical FOV and most of the time it works ok.

From in-game menu it can be changed with realtime results.

![fsr](images/fsr.png)

### Shapness
DLSS used to have shaprness option but later on it's removed. So some games have sharpness sliders and some not. With this option you can override or enable sharpness of final image. FSR has build it sharpness but for XeSS CAS option must be enabled.

```ini
[Sharpness]
; Override DLSS sharpness paramater with fixed shapness value
; true or false - Default (auto) is false
OverrideSharpness=auto

; Strength of sharpening, 
; value range between 0.0 and 1.0 - Default (auto) is 0.3
Sharpness=auto
```

From in-game menu it can be changed with realtime results.

![sharpness](images/sharpness.png)




