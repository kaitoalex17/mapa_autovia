@echo off
chcp 65001 > nul
echo ===============================================================================
echo   AUTOPISTAS DE ESPAÑA - COMPILACIÓN Y LANZAMIENTO DEL JUEGO (UE 5.8)
echo ===============================================================================
echo.

set ENGINE_PATH=E:\APP\ENGINE\UE_5.8
set PROJECT_PATH=%~dp0AutopistasEspana.uproject
set DOTNET_EXE=%ENGINE_PATH%\Engine\Binaries\ThirdParty\DotNet\10.0\win-x64\dotnet.exe
set UBT_DLL=%ENGINE_PATH%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll
set EDITOR_EXE=%ENGINE_PATH%\Engine\Binaries\Win64\UnrealEditor.exe

echo [1/3] Verificando motor Unreal Engine 5.8...
if not exist "%EDITOR_EXE%" (
    echo [ERROR] No se encuentra UnrealEditor.exe en %ENGINE_PATH%
    pause
    exit /b 1
)
echo [OK] Motor localizado en %ENGINE_PATH%

echo.
echo [2/3] Compilando modulos C++ (Development Win64)...
"%DOTNET_EXE%" "%UBT_DLL%" AutopistasEspanaEditor Win64 Development -Project="%PROJECT_PATH%" -WaitMutex -NoHotReload
if %errorlevel% neq 0 (
    echo.
    echo [AVISO] La compilacion C++ directa ha requerido herramientas de compilacion adicionales.
    echo Intentando abrir el proyecto directamente en Unreal Editor para recompilacion guiada...
)

echo.
echo [3/3] Abriendo Autopistas de Espana en Unreal Editor...
start "" "%EDITOR_EXE%" "%PROJECT_PATH%"

echo.
echo ===============================================================================
echo   Proyecto enviado al Editor. Disfruta del desarrollo!
echo ===============================================================================
