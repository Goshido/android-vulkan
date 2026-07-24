#ifndef COLOR_PACKING_HLSL
#define COLOR_PACKING_HLSL


// 1.0 / 255.0
#define UNORM_FACTOR_F16    3.9216e-3H
#define UNORM_FACTOR_F32    3.921569e-3F

struct ColorUNORM
{
    uint32_t        _r: 8;
    uint32_t        _g: 8;
    uint32_t        _b: 8;
    uint32_t        _a: 8;
};

struct ColorData
{
    uint32_t        _emiR: 8;

    uint32_t        _col0R: 8;
    uint32_t        _col0G: 8;
    uint32_t        _col0B: 8;

    uint32_t        _emiG: 8;

    uint32_t        _col1R: 8;
    uint32_t        _col1G: 8;
    uint32_t        _col1B: 8;

    uint32_t        _emiB: 8;

    uint32_t        _col2R: 8;
    uint32_t        _col2G: 8;
    uint32_t        _col2B: 8;

    uint32_t        _col0A: 8;

    uint32_t        _emiIntens: 24;
};

//----------------------------------------------------------------------------------------------------------------------

float16_t3 UnpackColorF16x3 ( in uint32_t r, in uint32_t g, in uint32_t b )
{
    return float16_t3 ( uint32_t3 ( r, g, b ) ) * UNORM_FACTOR_F16;
}

float32_t4 UnpackColorF32x4 ( in ColorUNORM color )
{
    return float32_t4 ( uint32_t4 ( color._r, color._g, color._b, color._a ) ) * UNORM_FACTOR_F32;
}


#endif // COLOR_PACKING_HLSL
