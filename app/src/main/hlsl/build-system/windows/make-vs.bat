@echo off
set COMPILE_FLAGS=-HV 2021 -spirv -fvk-use-dx-layout -fspv-reduce-load-size -fspv-target-env=vulkan1.1 -enable-16bit-types -WX -O3
set PIVOT_DIRECTORY=.\..\..\..

@echo on
"%ANDROID_VULKAN_DXC_ROOT%\dxc.exe" %COMPILE_FLAGS% -T vs_6_7 -E VS -I %PIVOT_DIRECTORY%\hlsl -I %PIVOT_DIRECTORY%\cpp\include\pbr -Fo %PIVOT_DIRECTORY%\assets\shaders\%1-vs.spv %PIVOT_DIRECTORY%\hlsl\%1.vs

@echo off
echo Done
