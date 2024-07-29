# Spoofing
Except from first generation of DLSS2 games they all have some kind of NVidia verifying to enable DLSS option.   
To bypass these checks modders have developed some tools.

## Windows
### DirectX
For spoofing DXGI adapter checks d3d12-proxy can be used. This mod reports your GPU as RTX 4090.   
##### Usage 
Just place dxgi.dll file next to games executable.  
##### Link 
[d3d12-proxy](https://github.com/cdozdil/d3d12-proxy/releases)   

### Vulkan
For spoofing `GetPhysicalDeviceProperties` checks vulkan-spoofer can be used. This mod reports your GPU as RTX 4090.   
Compatiblity is a bit hit and miss, works for No Man's Sky but not working with Doom Eternal.  
##### Usage 
Just place version.dll file next to games executable.  
##### Link 
[vulkan-spoofer](https://github.com/cdozdil/vulkan-spoofer/releases)   

### NVAPI
For spoofing NVAPI calls nvapi-dummy can be used. Mod tries to implement most used calls and respond to almost all calls.
##### Usage 
Just put `nvapi64.dll` next to OptiScaler and set `OverrideNvapiDll=true` from `nvngx.ini`. This only works when OptiScaler is working as non-nvngx (not as `nvngx.dll`).

For using without OptiScaler:  

You need to put `nvapi64.dll` file to your `%WINDIR%\System32` but **be careful!**
* If you are an NVidia user **backup your original file** and restore after mod usage is over.
* Do not use this mod with online games, it might cause anti cheat issues or banning.
##### Link 
[nvapi-dummy](https://github.com/FakeMichau/nvapi-dummy/releases)   

## Linux
On Linux with you can use Wine & DXVK's embedded spoofing mechanisms. 

### DirectX & Vulkan
For DXGI & Vulkan spoofing just create a `dxvk.conf` file next to game's executable with this content or just download it from [here](https://raw.githubusercontent.com/cdozdil/CyberXeSS/imgui-intergration/dxvk.conf).
```ini
dxgi.customVendorId = 10de
dxgi.hideAmdGpu = True
dxgi.hideNvidiaGpu = False
dxgi.customDeviceId = 2684
dxgi.customDeviceDesc = "NVIDIA GeForce RTX 4090"
```
### NVAPI
For spoofing NVAPI with Proton set this envvar `PROTON_FORCE_NVAPI=1`

## Goghor's DLSS Unlocker's
Goghor have created DLSS Enabler mods for a lot of games which can be found on his [Nexus](https://www.nexusmods.com/spidermanmilesmorales/users/12564231?tab=user+files&BH=0) profile.   
For example as far as I know for Doom Eternal still only way to enable DLSS is his mod.
