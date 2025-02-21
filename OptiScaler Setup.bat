REM Setup OptiScaler for your game
@echo off
cls
echo  ::::::::  :::::::::  ::::::::::: :::::::::::  ::::::::   ::::::::      :::     :::        :::::::::: :::::::::  
echo :+:    :+: :+:    :+:     :+:         :+:     :+:    :+: :+:    :+:   :+: :+:   :+:        :+:        :+:    :+: 
echo +:+    +:+ +:+    +:+     +:+         +:+     +:+        +:+         +:+   +:+  +:+        +:+        +:+    +:+ 
echo +#+    +:+ +#++:++#+      +#+         +#+     +#++:++#++ +#+        +#++:++#++: +#+        +#++:++#   +#++:++#:  
echo +#+    +#+ +#+            +#+         +#+            +#+ +#+        +#+     +#+ +#+        +#+        +#+    +#+ 
echo #+#    #+# #+#            #+#         #+#     #+#    #+# #+#    #+# #+#     #+# #+#        #+#        #+#    #+# 
echo  ########  ###            ###     ###########  ########   ########  ###     ### ########## ########## ###    ### 
echo.
echo Coping is strong with this one...
echo.

del "!! EXTRACT ALL FILES TO GAME FOLDER !!" 2>nul

setlocal enabledelayedexpansion

if not exist nvngx.dll (
    echo OptiScaler "nvngx.dll" file is not found!
    goto end
)

REM Set paths based on current directory
set "gamePath=%~dp0"
set "optiScalerFile=%gamePath%\nvngx.dll"
set setupSuccess=false

REM Check if the Engine folder exists
if exist "%gamePath%\Engine" (
    echo Found Engine folder, if this is an Unreal Engine game then please extract Optiscaler to #CODENAME#\Binaries\Win64
    echo.
    
    set /p continueChoice="Continue installation to current folder? [y/n]: "
    set continueChoice=!continueChoice: =!

    if "!continueChoice!"=="y" (
        goto selectFilename
    )

    goto end
)

REM Prompt user to select a filename for OptiScaler
:selectFilename
echo.
echo Choose a filename for OptiScaler (default is dxgi.dll):
echo  [1] dxgi.dll
echo  [2] winmm.dll
echo  [3] version.dll
echo  [4] wininet.dll
echo  [5] winhttp.dll
echo  [6] OptiScaler.asi
set /p filenameChoice="Enter 1-6 (or press Enter for default): "

if "%filenameChoice%"=="" (
    set selectedFilename="dxgi.dll"
) else if "%filenameChoice%"=="1" (
    set selectedFilename="dxgi.dll"
) else if "%filenameChoice%"=="2" (
    set selectedFilename="winmm.dll"
) else if "%filenameChoice%"=="3" (
    set selectedFilename="version.dll"
) else if "%filenameChoice%"=="4" (
    set selectedFilename="wininet.dll"
) else if "%filenameChoice%"=="5" (
    set selectedFilename="winhttp.dll"
) else if "%filenameChoice%"=="6" (
    set selectedFilename="OptiScaler.asi"
) else (
    echo Invalid choice. Please select a valid option.
    echo.
    goto selectFilename
)

if exist %selectedFilename% (
    echo.
    echo WARNING: %selectedFilename% already exists in the current folder.
    echo.
    set /p overwriteChoice="Do you want to overwrite %selectedFilename%? [y/n]: "
    set overwriteChoice=!overwriteChoice: =!
    
    echo.
    if "!overwriteChoice!"=="y" (
        goto queryGPU
    )

    goto selectFilename
)

:queryGPU
if exist %windir%\system32\nvapi64.dll (
    echo.
    echo Nvidia driver files detected.
    set isNvidia=true
) else (
    set isNvidia=false
)

REM Query user for GPU type
echo.
echo Are you using an Nvidia GPU or AMD/Intel GPU?
echo [1] AMD/Intel
echo [2] Nvidia
if "isNvidia"=="true" (
    set /p gpuChoice="Enter 1 or 2 (or press Enter for Nvidia): "
) else (
    set /p gpuChoice="Enter 1 or 2 (or press Enter for AMD/Intel): "
)

if "%gpuChoice%"=="2" (
    REM Nvidia
    echo Nvidia GPU selected. Skipping "nvngx_dlss.dll" handling step.
    goto completeSetup
) else if "%gpuChoice%"=="1" (
    REM AMD/Intel
    echo AMD/Intel GPU selected. Proceeding with "nvngx_dlss.dll" handling.
    goto checkFile
) else (
    if "%gpuChoice%"=="" (
        if "isNvidia"=="true" (
            REM Nvidia
            echo Nvidia GPU selected. Skipping "nvngx_dlss.dll" handling step.
            set gpuChoice=2
            goto completeSetup
        )
        
        REM AMD/Intel
        echo AMD/Intel GPU selected. Proceeding with "nvngx_dlss.dll" handling.
        set gpuChoice=1
        goto checkFile
    )
    
    echo.
    echo Invalid choice. Please enter 1 or 2.
    echo.
    goto queryGPU
)

:checkFile
set dlssFile = 
goto check_nvngx_dlss
:resume_nvngx_dlss

REM Copy nvngx_dlss.dll file
echo.
echo Copying "nvngx_dlss.dll" to game folder...
copy /y "%dlssFile%" .\nvngx_dlss_copy.dll 
if errorlevel 1 (
    echo.
    echo ERROR: Failed to copy "nvngx_dlss.dll".
    goto end
)

:completeSetup
REM Rename OptiScaler file
echo.
if "!overwriteChoice!"=="y" (
    echo Removing previous %selectedFilename%...
    del /F %selectedFilename% 
)

echo Renaming OptiScaler file to %selectedFilename%...
rename "%optiScalerFile%" "%selectedFilename%"
if errorlevel 1 (
    echo.
    echo ERROR: Failed to rename OptiScaler file to %selectedFilename%.
    goto end
)

REM Rename nvngx_dlss.dll file if AMD/Intel is selected
if "%gpuChoice%"=="1" (
    echo Renaming "nvngx_dlss_copy.dll" file to "nvngx.dll"...
    rename nvngx_dlss_copy.dll nvngx.dll
    if errorlevel 1 (
        echo.
        echo ERROR: Failed to rename "nvngx_dlss.dll" to "nvngx.dll".
        goto end
    )
)

goto create_uninstaller
:create_uninstaller_return

cls
echo  OptiScaler setup completed successfully...
echo.
echo   ___                 
echo  (_         '        
echo  /__  /)   /  () (/  
echo          _/      /    
echo.

set setupSuccess=true

:end
pause

if "%setupSuccess%"=="true" (
    REM Remove "OptiScaler Setup.bat"
    del %0
)

REM Remove leftover "nvngx_dlss_copy.dll"
if exist "nvngx_dlss_copy.dll" (
    del /y "nvngx_dlss_copy.dll"
)

exit /b

REM check for nvngx_dlss.dll
:check_nvngx_dlss
if exist "nvngx_dlss.dll" (
    echo Found "nvngx_dlss.dll" in the current folder.
    set dlssFile=nvngx_dlss.dll
    goto resume_nvngx_dlss
)

REM Search for nvngx_dlss.dll in the current folder and subfolders
set fileToSearch=nvngx_dlss.dll
set foundFile=

for /f "delims=" %%F in ('dir /s /b %fileToSearch% 2^>nul') do (
    set dlssFile=%%F
    goto fileFound
)

REM Check for UE Win64 folder
cd ..
if exist Win64 (
    echo.
    echo Going to root folder of Unreal Engine game and searching again
    cd ..
    cd ..

    for /f "delims=" %%F in ('dir /s /b %fileToSearch% 2^>nul') do (
        set dlssFile=%%F
        goto fileFound
    )
)

:fileNotFound
cd "%gamePath%"
echo ERROR: "nvngx_dlss.dll" not found in expected locations. Please manually copy it and run setup again.
goto end

:fileFound
cd "%gamePath%"
echo File found at %dlssFile%
goto resume_nvngx_dlss

:create_uninstaller
copy /y NUL "Remove OptiScaler.bat"

(
echo @echo off
echo cls
echo echo  ::::::::  :::::::::  ::::::::::: :::::::::::  ::::::::   ::::::::      :::     :::        :::::::::: :::::::::  
echo echo :+:    :+: :+:    :+:     :+:         :+:     :+:    :+: :+:    :+:   :+: :+:   :+:        :+:        :+:    :+: 
echo echo +:+    +:+ +:+    +:+     +:+         +:+     +:+        +:+         +:+   +:+  +:+        +:+        +:+    +:+ 
echo echo +#+    +:+ +#++:++#+      +#+         +#+     +#++:++#++ +#+        +#++:++#++: +#+        +#++:++#   +#++:++#:  
echo echo +#+    +#+ +#+            +#+         +#+            +#+ +#+        +#+     +#+ +#+        +#+        +#+    +#+ 
echo echo #+#    #+# #+#            #+#         #+#     #+#    #+# #+#    #+# #+#     #+# #+#        #+#        #+#    #+# 
echo echo  ########  ###            ###     ###########  ########   ########  ###     ### ########## ########## ###    ### 
echo echo.
echo echo Coping is strong with this one...
echo echo.
echo.
echo set /p removeChoice="Do you want to remove OptiScaler? [y/n]: "
echo.
echo if "%%removeChoice%%"=="y" ^(
if "%gpuChoice%"=="1" (
    echo    del nvngx.dll
)
echo    del OptiScaler.log
echo    del OptiScaler.ini
echo    del %selectedFilename%
echo    del /Q DlssOverrides\*
echo    rd DlssOverrides
echo    del /Q Licenses\*
echo    rd Licenses
echo    echo.
echo    echo OptiScaler removed!
echo    echo.
echo ^) else ^(
echo    echo.
echo    echo Operation cancelled.
echo    echo.
echo ^)
echo.
echo pause
echo if "%%removeChoice%%"=="y" ^(
echo 	del %%0
echo ^)
) >> "Remove OptiScaler.bat"

echo.
echo Uninstaller created.
echo.

goto create_uninstaller_return


