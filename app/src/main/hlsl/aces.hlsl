#ifndef ACES_HLSL
#define ACES_HLSL


float16_t3 ApplyACES ( in Texture2D<float32_t4> hdrImage,
    in SamplerState clampToEdgeSampler,
    in float32_t2 uv,
    in float32_t exposure
)
{
    // See https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
    float16_t3 const hdr = (float16_t3)hdrImage.SampleLevel ( clampToEdgeSampler, uv, 0.0F ).xyz;
    float16_t3 const raw = (float16_t)exposure * hdr;

    float16_t3 const alpha = mad ( (float16_t3)2.51H, raw, (float16_t3)0.03H );
    float16_t3 const beta = mad ( (float16_t3)2.43H, raw, (float16_t3)0.59H );
    return saturate ( ( raw * alpha ) / mad ( raw, beta, (float16_t3)0.14H ) );
}


#endif // ACES_HLSL
