#ifndef MANDELBROT_PS_HLSL
#define MANDELBROT_PS_HLSL


#define SQUARE_MODULE_STOP_TRAIT    4.0F
#define MAX_ITERATIONS              512U
#define INV_MAX_ITERATIONS          ( 1.0F / (float32_t)MAX_ITERATIONS )

//----------------------------------------------------------------------------------------------------------------------

float32_t2 SquareComplex ( in float32_t2 value )
{
    return float32_t2 ( value.x * value.x - value.y * value.y, 2.0F * value.x * value.y );
}

float32_t SquareModuleComplex ( in float32_t2 value )
{
    return dot ( value, value );
}

uint32_t CountIterations ( in float32_t2 coordinate )
{
    uint32_t iteration = 0U;
    float32_t2 z = (float32_t2)0.0F;

    while ( iteration <= MAX_ITERATIONS && SquareModuleComplex ( z ) <= SQUARE_MODULE_STOP_TRAIT )
    {
        z = SquareComplex ( z ) + coordinate;
        ++iteration;
    }

    return iteration;
}


#endif // MANDELBROT_PS_HLSL
