#ifndef BRIGHTNESS_FACTOR_PS_HLSL
#define BRIGHTNESS_FACTOR_PS_HLSL


#include "pbr/brightness_factor.inc"


[[vk::constant_id ( CONST_BRIGHTNESS_FACTOR )]]
float const     g_brightnessFactor = 1.0F;


#endif // BRIGHTNESS_FACTOR_PS_HLSL
