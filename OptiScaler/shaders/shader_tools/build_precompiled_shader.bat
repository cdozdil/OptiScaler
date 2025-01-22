@echo off

if "%~1"=="" (
    echo Usage: %~nx0 ShaderName
    exit /b 1
)

set ShaderName=%1

echo Creating Dx12 CSO
"%~dp0dxc.exe" -T cs_6_0 -E CSMain -Cc -Vi "%ShaderName%.hlsl" -Fo "%ShaderName%_Shader.cso"

echo Creating Dx12 Header
python "%~dp0create_header.py" "%ShaderName%_Shader.cso" "%ShaderName%_Shader.h" %ShaderName%_cso

echo Creating Dx11 CSO
"%~dp0fxc.exe" -T cs_5_0 -E CSMain -Cc -Vi "%ShaderName%.hlsl" -Fo "%ShaderName%_Shader_Dx11.cso"

echo Creating Dx11 Header
python "%~dp0create_header.py" "%ShaderName%_Shader_Dx11.cso" "%ShaderName%_Shader_Dx11.h" %ShaderName%_cso