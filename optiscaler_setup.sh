#!/bin/bash

# OptiScaler Linux/Proton Setup Script

clear
echo "  ::::::::  :::::::::  ::::::::::: :::::::::::  ::::::::   ::::::::      :::     :::        :::::::::: :::::::::  "
echo " :+:    :+: :+:    :+:     :+:         :+:     :+:    :+: :+:    :+:   :+: :+:   :+:        :+:        :+:    :+: "
echo " +:+    +:+ +:+    +:+     +:+         +:+     +:+        +:+         +:+   +:+  +:+        +:+        +:+    +:+ "
echo " +#+    +:+ +#++:++#+      +#+         +#+     +#++:++#++ +#+        +#++:++#++: +#+        +#++:++#   +#++:++#:  "
echo " +#+    +#+ +#+            +#+         +#+            +#+ +#+        +#+     +#+ +#+        +#+        +#+    +#+ "
echo " #+#    #+# #+#            #+#         #+#     #+#    #+# #+#    #+# #+#     #+# #+#        #+#        #+#    #+# "
echo "  ########  ###            ###     ###########  ########   ########  ###     ### ########## ########## ###    ### "
echo ""
echo "Coping is strong with this one..."
echo ""

# !! EXTRACT ALL FILES TO GAME FOLDER !!

if [[ ! -f "OptiScaler.dll" ]]; then
    echo 'OptiScaler "OptiScaler.dll" file is not found!'
    exit 1
fi

gamePath="$(pwd)"
optiScalerFile="$gamePath/OptiScaler.dll"
setupSuccess=false

if [[ -d "$gamePath/Engine" ]]; then
    echo "Found Engine folder. If this is an Unreal Engine game, then please extract OptiScaler to #CODENAME#/Binaries/Win64"
    echo ""
    read -p "Continue installation to current folder? [y/n]: " continueChoice
    if [[ "$continueChoice" != "y" ]]; then
        exit 0
    fi
fi

echo ""
echo "Choose a filename for OptiScaler (default is dxgi.dll):"
echo " [1] dxgi.dll"
echo " [2] winmm.dll"
echo " [3] version.dll"
echo " [4] dbghelp.dll"
echo " [5] wininet.dll"
echo " [6] winhttp.dll"
echo " [7] OptiScaler.asi"
read -p "Enter 1-7 (or press Enter for default): " filenameChoice

case "$filenameChoice" in
    ""|"1") selectedFilename="dxgi.dll" ;;
    "2") selectedFilename="winmm.dll" ;;
    "3") selectedFilename="version.dll" ;;
    "4") selectedFilename="dbghelp.dll" ;;
    "5") selectedFilename="wininet.dll" ;;
    "6") selectedFilename="winhttp.dll" ;;
    "7") selectedFilename="OptiScaler.asi" ;;
    *) echo "Invalid choice."; exit 1 ;;
esac

if [[ -f "$selectedFilename" ]]; then
    echo ""
    echo "WARNING: $selectedFilename already exists in the current folder."
    read -p "Do you want to overwrite $selectedFilename? [y/n]: " overwriteChoice
    if [[ "$overwriteChoice" != "y" ]]; then
        echo "Aborting."
        exit 0
    fi
fi

# Detect GPU (via nvapi64.dll in Wine prefix)
if [[ -z "$WINEPREFIX" ]]; then
    WINEPREFIX="$HOME/.wine"
fi

if [[ -f "$WINEPREFIX/drive_c/windows/system32/nvapi64.dll" ]]; then
    isNvidia=true
else
    isNvidia=false
fi

echo ""
echo "Are you using an Nvidia GPU or AMD/Intel GPU?"
echo " [1] AMD/Intel"
echo " [2] Nvidia"
if [[ "$isNvidia" == "true" ]]; then
    read -p "Enter 1 or 2 (or press Enter for Nvidia): " gpuChoice
    [[ -z "$gpuChoice" ]] && gpuChoice="2"
else
    read -p "Enter 1 or 2 (or press Enter for AMD/Intel): " gpuChoice
    [[ -z "$gpuChoice" ]] && gpuChoice="1"
fi

if [[ "$gpuChoice" == "1" ]]; then
    echo "AMD/Intel GPU selected. Proceeding with nvngx_dlss.dll handling."

    echo ""
    echo "Will you try to use DLSS inputs?"
    echo " [1] Yes"
    echo " [2] No"
    read -p "Enter 1 or 2 (or press Enter for Yes): " copyNvngx
    [[ -z "$copyNvngx" ]] && copyNvngx="1"

    if [[ "$copyNvngx" != "2" ]]; then
        # Search for nvngx_dlss.dll
        dlssFile=""
        if [[ -f "nvngx_dlss.dll" ]]; then
            dlssFile="nvngx_dlss.dll"
        else
            mapfile -t foundFiles < <(find . -iname "nvngx_dlss.dll")
            if [[ ${#foundFiles[@]} -gt 0 ]]; then
                dlssFile="${foundFiles[0]}"
            fi
        fi

        if [[ -z "$dlssFile" ]]; then
            echo "ERROR: nvngx_dlss.dll not found. Please manually copy it and run setup again."
            exit 1
        fi

        echo "Copying $dlssFile to nvngx_dlss_copy.dll..."
        cp "$dlssFile" ./nvngx_dlss_copy.dll || { echo "Failed to copy file."; exit 1; }
    fi
else
    echo "Nvidia GPU selected. Skipping nvngx_dlss.dll handling."
fi

# Final setup
[[ -f "$selectedFilename" && "$overwriteChoice" == "y" ]] && rm -f "$selectedFilename"

echo "Renaming OptiScaler.dll to $selectedFilename..."
mv -f "$optiScalerFile" "$selectedFilename" || { echo "Failed to rename."; exit 1; }

if [[ "$gpuChoice" == "1" && "$copyNvngx" != "2" ]]; then
    echo "Renaming nvngx_dlss_copy.dll to nvngx.dll..."
    mv -f nvngx_dlss_copy.dll nvngx.dll || { echo "Failed to rename nvngx."; exit 1; }
fi

# Create uninstaller
echo "Creating uninstaller..."
cat <<EOF > "Remove OptiScaler.sh"
#!/bin/bash

read -p "Do you want to remove OptiScaler? [y/n]: " removeChoice
if [[ "\$removeChoice" == "y" ]]; then
    rm -f nvngx.dll
    rm -f OptiScaler.log OptiScaler.ini "$selectedFilename"
    rm -rf D3D12_Optiscaler DlssOverrides Licenses
    echo "OptiScaler removed!"
    rm -f "\$0"
else
    echo "Operation cancelled."
fi
EOF

chmod +x "Remove OptiScaler.sh"
echo ""
echo "OptiScaler setup completed successfully."
setupSuccess=true

# Clean up
rm -f "nvngx_dlss_copy.dll"

exit 0
