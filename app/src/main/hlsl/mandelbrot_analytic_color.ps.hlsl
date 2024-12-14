#include "mandelbrot.ps.hlsl"


#define TWO_PI                  6.28318F
#define ITERATION_TO_ANGLE      ( TWO_PI * INV_MAX_ITERATIONS )

#define HUE_OFFSET_RED          0.0F
#define HUE_OFFSET_GREEN        2.09439F
#define HUE_OFFSET_BLUE         4.18879F
#define HUE_OFFSET_RGB          float32_t3 ( HUE_OFFSET_RED, HUE_OFFSET_GREEN, HUE_OFFSET_BLUE )

//----------------------------------------------------------------------------------------------------------------------

float32_t4 MapColor ( in uint32_t iterations )
{
    float32_t4 result;
    result.xyz = sin ( HUE_OFFSET_RGB + ( iterations * ITERATION_TO_ANGLE ) );
    result.xyz = ( result.xyz + 1.0F ) * 0.5F;
    result.w = 1.0F;

    return result;
}

//----------------------------------------------------------------------------------------------------------------------

float32_t4 PS ( [[vk::location ( 0 )]] in linear float32_t2 coordinate: COORDINATE ): SV_Target0
{
    return MapColor ( CountIterations ( coordinate ) );
}
