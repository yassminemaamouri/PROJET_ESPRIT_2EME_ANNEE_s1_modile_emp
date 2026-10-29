@echo off
setlocal
set "RACINE=%~dp0"
set "EXE=%RACINE%release\smartweather.exe"

if not exist "%EXE%" (
    echo ERREUR : %EXE% est introuvable.
    echo Compilez d'abord le projet avec qmake puis mingw32-make.
    pause
    exit /b 1
)

set "QT_BIN=C:\Qt\6.7.3\mingw_64\bin"
if defined QTDIR if exist "%QTDIR%\bin\windeployqt.exe" set "QT_BIN=%QTDIR%\bin"
if not exist "%QT_BIN%\windeployqt.exe" (
    for /f "delims=" %%I in ('where windeployqt 2^>nul') do set "QT_BIN=%%~dpI"
)
if not exist "%QT_BIN%\windeployqt.exe" (
    echo ERREUR : windeployqt.exe est introuvable.
    echo Ajoutez le dossier bin de Qt au PATH ou definissez QTDIR.
    pause
    exit /b 1
)

echo Deploiement des bibliotheques Qt dans release...
"%QT_BIN%\windeployqt.exe" --release --no-translations --no-opengl-sw --compiler-runtime "%EXE%"
if errorlevel 1 (
    echo.
    echo Echec du deploiement.
    pause
    exit /b 1
)

echo.
echo Deploiement termine. Lancez : release\smartweather.exe
echo Conservez tout le dossier release (l'exe seul ne suffit pas).
pause
