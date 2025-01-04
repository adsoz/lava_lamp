@echo off
set WORKSPACE_FOLDER=%1

echo Cleaning up build files in %WORKSPACE_FOLDER%

del /Q /F "%WORKSPACE_FOLDER%\build\*.spv"
del /Q /F "%WORKSPACE_FOLDER%\build\obj\*"
del /Q /F "%WORKSPACE_FOLDER%\build\lavalamp.*"

echo Cleaned up previous build files.