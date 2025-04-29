:: NOTE This script compiles with the version of the VulkanSDK set as a system variable
@echo off
set "ShaderDirectory=%~pd0..\shaders"
set "CurrentShader=NULL"

call :FindShaders

echo Compilations finished!
goto :eof



:FindShaders
for %%i in ("%ShaderDirectory%/*") do (
   setlocal enabledelayedexpansion
   set CurrentShader=%%~nxi
   echo !CurrentShader!
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
   "%VULKAN_SDK%/Bin/glslc.exe" "%ShaderDirectory%/%%~nxi" -o "%ShaderDirectory%/%%~ni_!Extension!.spv"
)
exit /b

:LogError
echo ERR:
echo ERR_MESSAGE
