#ifndef COLOR_SPACE_HLSL
#define COLOR_SPACE_HLSL


float16_t3 LinearToSRGB ( in float16_t3 linearRGB )
{
    // See <repo>/docs/srgb#linear-to-srgb
    float16_t3 const srgb = linearRGB * 12.92H;
    float16_t3 const ex = mad ( 1.055H, pow ( linearRGB, 4.1667e-1H ), -5.5e-2H );
    return lerp ( srgb, ex, (float16_t3)( linearRGB > 3.1308e-3H ) );
}

float16_t3 SRGBToLinear ( in float16_t3 sRGB )
{
    // See <repo>/docs/srgb#srgb-to-linear
    float16_t3 const lin = sRGB * 7.74e-2H;
    float16_t3 const ex = pow ( mad ( sRGB, (float16_t3)9.479e-1H, (float16_t3)5.213e-2H ), 2.4H );
    return lerp ( lin, ex, (float16_t3)( sRGB > 4.045e-2H ) );
}


#endif // COLOR_SPACE_HLSL
