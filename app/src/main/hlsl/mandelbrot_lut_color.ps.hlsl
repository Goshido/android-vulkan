#include "mandelbrot.ps.hlsl"


Texture1D<float32_t4>       lutTexture:     register ( t0 );

[[vk::binding ( 0 )]]
SamplerState                lutSampler:     register ( s0 );

//----------------------------------------------------------------------------------------------------------------------

float32_t4 MapColor ( in uint32_t iterations )
{
    return lutTexture.Sample ( lutSampler, iterations * INV_MAX_ITERATIONS );
}

//----------------------------------------------------------------------------------------------------------------------

float32_t4 PS ( [[vk::location ( 0 )]] in linear float32_t2 coordinate: COORDINATE ): SV_Target0
{
    return MapColor ( CountIterations ( coordinate ) );
}
