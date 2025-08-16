#ifndef COLOR_PACKING_HLSL
#define COLOR_PACKING_HLSL


// 1.0 / 255.0
#define UNORM_FACTOR_F16    3.9216e-3H
#define UNORM_FACTOR_F32    3.921569e-3F


float16_t3 UnpackColorF16x3 ( in uint32_t packedColor )
{
    uint32_t3 const segmented = ( (uint32_t3)packedColor ) & uint32_t3 ( 0x0000FF00U, 0x00FF0000U, 0xFF000000U );
    return (float16_t3)( segmented >> uint32_t3 ( 8U, 16U, 24U ) ) * UNORM_FACTOR_F16;
}

float32_t4 UnpackColorF32x4 ( in uint32_t packedColor )
{
    uint32_t4 const segmented =
        ( (uint32_t4)packedColor ) & uint32_t4 ( 0x000000FFU, 0x0000FF00U, 0x00FF0000U, 0xFF000000U );

    return (float32_t4)( segmented >> uint32_t4 ( 0U, 8U, 16U, 24U ) ) * UNORM_FACTOR_F32;
}


#endif // COLOR_PACKING_HLSL
