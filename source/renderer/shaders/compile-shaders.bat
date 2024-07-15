@echo off
:: TODO: Automate finding of shaders to compile.
echo Compiling vertex shader
C:/VulkanSDK/1.3.283.0/Bin/glslc.exe shader.vert -o vert.spv
echo Compiling fragment shader
C:/VulkanSDK/1.3.283.0/Bin/glslc.exe shader.frag -o frag.spv

echo Compilations finished!
pause