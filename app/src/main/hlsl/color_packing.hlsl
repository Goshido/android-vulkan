#ifndef COLOR_PACKING_HLSL
#define COLOR_PACKING_HLSL


// 1.0 / 255.0
#define UNORM_FACTOR_F16    3.9216e-3H
#define UNORM_FACTOR_F32    3.921569e-3F


float16_t3 UnpackColorF16x3 ( in uint32_t r, in uint32_t g, in uint32_t b )
{
    return float16_t3 ( uint32_t3 ( r, g, b ) ) * UNORM_FACTOR_F16;
}

float32_t4 UnpackColorF32x4 ( in uint32_t r, in uint32_t g, in uint32_t b, in uint32_t a )
{
    return float32_t4 ( uint32_t4 ( r, g, b, a ) ) * UNORM_FACTOR_F32;
}


#endif // COLOR_PACKING_HLSL
