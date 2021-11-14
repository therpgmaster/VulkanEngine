@echo off
title GLSL to SPIR-V shader compiler
@echo on

G:\VulkanDev\VulkanSDK\1.2.170.0\Bin\glslc.exe shader.vert -o shader.vert.spv
G:\VulkanDev\VulkanSDK\1.2.170.0\Bin\glslc.exe shader.frag -o shader.frag.spv
G:\VulkanDev\VulkanSDK\1.2.170.0\Bin\glslc.exe shader2test.frag -o shader2test.frag.spv


@echo off
echo.
echo GLSLC completed

timeout /t 3 /NOBREAK > nul
exit