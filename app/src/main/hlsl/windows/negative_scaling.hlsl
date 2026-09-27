#ifndef NEGATIVE_SCALING_HLSL
#define NEGATIVE_SCALING_HLSL


uint16_t3 ComputeNegativeScalingInfo ( in float32_t3 scale )
{
    // Idea extract the most significant bit from each coordinate - it's value sign. 0 - positive, 1 - negative.
    return (uint16_t3)( ( asuint ( scale ) & (uint32_t3)0x80000000U ) >> 16U );
}

float16_t3 CorrectUnitVector ( in float16_t3 unit, in uint16_t3 negativeScalingInfo )
{
    // Idea flip the most significant bit of each coordinate if scale is negative.
    return asfloat16 ( negativeScalingInfo ^ asuint16 ( unit ) );
}

bool NeedFaceToggle ( in uint16_t3 negativeScalingInfo )
{
    // Idea: need to count how many axis has negative scale. If this count is odd - need to toggle face. That's it!
    return (bool)( negativeScalingInfo.x ^ negativeScalingInfo.y ^ negativeScalingInfo.z );
}


#endif // NEGATIVE_SCALING_HLSL
