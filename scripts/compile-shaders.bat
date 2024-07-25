:: NOTE This script compiles with the most recent version of the VulkanSDK found
@echo off
set ShaderDirectory=..\shaders
set CurrentShader=NULL
set VulkanDirectory=C:/VulkanSDK

call :FindVulkanDirectory
call :FindShaders

echo Compilations finished!
pause
goto :eof



:FindShaders
for %%i in (%ShaderDirectory%/*) do (
   setlocal enabledelayedexpansion
   set CurrentShader=%%~nxi
   :: Vertex shader
   if "%%~xi"==".vert" (
      call :CompileShader
   )
   :: Tessellation Control shader
   if "%%~xi"==".tesc" (
      call :CompileShader
   )
   :: Tessellation Evaluation shader
   if "%%~xi"==".tese" (
      call :CompileShader
   )
   :: Geometry shader
   if "%%~xi"==".geom" (
      call :CompileShader
   )
   :: Fragment shader
   if "%%~xi"==".frag" (
      call :CompileShader
   )
   :: Compute shader
   if "%%~xi"==".comp" (
      call :CompileShader
   )
)
exit /b

:CompileShader
echo Compiling %CurrentShader%
for %%i in (%CurrentShader%) do (
   setlocal enabledelayedexpansion
   set Extension=%%~xi
   set Extension=!Extension:~1!
   %VulkanDirectory%/Bin/glslc.exe %ShaderDirectory%/%%~nxi -o "%ShaderDirectory%/%%~ni_!Extension!.spv"
)
exit /b

:FindVulkanDirectory
if NOT exist %VulkanDirectory% (
   set ERR_MESSAGE="Could not find the VulkanSDK"
   call :LogError
   exit /b
)

for /D %%i in (%VulkanDirectory%/*) do (
   set VulkanDirectory=%VulkanDirectory%/%%~nxi
)
exit /b

:LogError
echo ERR:
echo ERR_MESSAGE
pause
