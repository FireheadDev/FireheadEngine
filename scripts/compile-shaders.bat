@echo off
set ShaderDirectory=../shaders

:: TODO: Automate finding of shaders to compile.
echo Compiling vertex shader
C:/VulkanSDK/1.3.283.0/Bin/glslc.exe %ShaderDirectory%/shader.vert -o %ShaderDirectory%/vert.spv
echo Compiling fragment shader
C:/VulkanSDK/1.3.283.0/Bin/glslc.exe %ShaderDirectory%/shader.frag -o %ShaderDirectory%/frag.spv

echo Compilations finished!
pause