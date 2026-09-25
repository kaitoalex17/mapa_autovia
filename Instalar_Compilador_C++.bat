@echo off
chcp 65001 > nul
echo ===============================================================================
echo   AUTOPISTAS DE ESPAÑA - INSTALADOR DE HERRAMIENTAS DE COMPILACIÓN C++ (MSVC)
echo ===============================================================================
echo.
echo Para compilar el código C++ del juego en Unreal Engine 5.8 es necesario el
echo compilador de Microsoft C++ (MSVC v14.38+ / v14.50) y Windows SDK.
echo.
echo Se solicitarán permisos de Administrador para completar la instalación oficial
echo a través de Windows Package Manager (winget).
echo.
pause

powershell -Command "Start-Process winget -ArgumentList 'install --id Microsoft.VisualStudio.2022.BuildTools --override \"\"--passive --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended\"\" --accept-package-agreements --accept-source-agreements' -Verb RunAs -Wait"

echo.
echo ===============================================================================
echo   Instalación completada o en proceso. Ya puedes ejecutar Compilar_Y_Abrir_Juego.bat
echo ===============================================================================
pause
