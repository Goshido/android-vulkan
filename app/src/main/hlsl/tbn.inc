#ifndef TBN_INC
#define TBN_INC


void GetNormalAndTangent ( out float16_t3 normalView, out float16_t3 tangentView, in float16_t4 tbn )
{
    float16_t3 const abc2 = tbn.yzw + tbn.yzw;

    // Note 'T' is just notation for variable separation. Nothing more.
    float16_t4 const rXrTabc2 = tbn.x * float16_t4 ( tbn.x, abc2 );
    float16_t4 const caaaXcaTbc2 = float16_t4 ( tbn.wyyy ) * float16_t4 ( tbn.wy, abc2.yz );
    float16_t2 const bXbTc2 = tbn.z * float16_t2 ( tbn.z, abc2.z );

    float16_t4 const left = float16_t4 ( rXrTabc2.w, caaaXcaTbc2.w, rXrTabc2.z, bXbTc2.y );
    float16_t4 const right = float16_t4 ( caaaXcaTbc2.z, -rXrTabc2.z, caaaXcaTbc2.w, -rXrTabc2.y );
    float16_t4 const tmp = left + right;

    // Note quaternion unpacks to matrix with column-major like behaviour.
    normalView = float16_t3 ( tmp.zw, rXrTabc2.x - caaaXcaTbc2.y - bXbTc2.x + caaaXcaTbc2.x );
    tangentView = float16_t3 ( rXrTabc2.x + caaaXcaTbc2.y - bXbTc2.x - caaaXcaTbc2.x, tmp.xy );
}

float16_t4 RecoverTBN ( in float16_t3 imaginaryTBN )
{
    // By convention xyz contains imaginary part of quaternion. The w component contains mirroring information.
    // Real part of quaternion must be restored. By convention it's used unit quaternions to represent TBN.
    // So the real component will be restored using this property.
    // Note dot product could be a little bit bigger than 1.0H due to float16_t inaccurency. Fixing it with abs.
    return float16_t4 ( sqrt ( abs ( 1.0H - dot ( imaginaryTBN, imaginaryTBN ) ) ), imaginaryTBN );
}

float16_t4 DecompressTBN ( in uint32_t compressedTBN )
{
    uint32_t3 comp = (uint32_t3)compressedTBN & uint32_t3 ( 0x3FF00000U, 0x000FFC00U, 0x000003FFU );
    comp.xy >>= uint32_t2 ( 20U, 10U );

    // 2 / ( ( 2 ^ 10 ) - 1 ) = 1.955034213098729227761485826e-3
    return RecoverTBN ( mad ( (float16_t3)comp, 1.955e-3H, -1.0H ) );
}

uint32_t CompressTBN ( in float16_t4 tbn, in uint32_t oldCompressedTBN )
{
    if ( tbn.x < 0.0H )
        tbn.yzw = -tbn.yzw;

    uint32_t3 unormData = uint32_t3 ( mad ( tbn.yzw, 511.0H, 512.0H ) );
    unormData.xy <<= uint32_t2 ( 20U, 10U );
    return ( oldCompressedTBN & 0xC0000000U ) | unormData.x | unormData.y | unormData.z;
}

float16_t4 RotateTBN ( in float16_t4 tbn, in float16_t4 quaternion )
{
    return float16_t4 (
        dot ( quaternion, float16_t4 ( tbn.x, -tbn.yzw ) ),
        dot ( quaternion, float16_t4 ( tbn.yxw, -tbn.z ) ),
        dot ( quaternion, float16_t4 ( tbn.z, -tbn.w, tbn.xy ) ),
        dot ( quaternion, float16_t4 ( tbn.wz, -tbn.y, tbn.x ) )
    );
}

float16_t4 ToTBN ( in float16_t3x3 m )
{
    float16_t3 const d = float16_t3 ( m[ 0U ][ 0U ], m[ 1U ][ 1U ], m[ 2U ][ 2U ] );

    float16_t4 solutions = float16_t4 ( d.x, d.x, -d.x, -d.x );
    solutions += float16_t4 ( d.y, -d.y, d.y, -d.y );
    solutions += float16_t4 ( d.z, -d.z, -d.z, d.z ) + 1.0H;

#define SOLUTION_X      0U
#define SOLUTION_Y      1U
#define SOLUTION_Z      2U
#define SOLUTION_W      3U

    uint16_t selector = SOLUTION_X;
    float16_t solution = solutions.x;

    if ( solution < solutions.y )
    {
        selector = SOLUTION_Y;
        solution = solutions.y;
    }

    if ( solution < solutions.z )
    {
        selector = SOLUTION_Z;
        solution = solutions.z;
    }

    if ( solution < solutions.w )
    {
        selector = SOLUTION_W;
        solution = solutions.w;
    }

    float16_t const phi = 0.5H * sqrt ( solution );
    float16_t const omega = 0.25H / phi;

    switch ( selector )
    {
        case SOLUTION_X:
        {
            float16_t3 const a = float16_t3 ( m[ 1U ][ 2U ], m[ 2U ][ 0U ], m[ 0U ][ 1U ] );
            float16_t3 const b = a - float16_t3 ( m[ 2U ][ 1U ], m[ 0U ][ 2U ], m[ 1U ][ 0U ] );
            return float16_t4 ( phi, b * omega );
        }

        case SOLUTION_Y:
        {
            float16_t3 const a = float16_t3 ( m[ 1U ][ 2U ], m[ 0U ][ 1U ], m[ 0U ][ 2U ] );
            float16_t3 const b = ( a + float16_t3 ( -m[ 2U ][ 1U ], m[ 1U ][ 0U ], m[ 2U ][ 0U ] ) ) * omega;
            return float16_t4 ( b.x, phi, b.yz );
        }

        case SOLUTION_Z:
        {
            float16_t3 const a = float16_t3 ( m[ 2U ][ 0U ], m[ 0U ][ 1U ], m[ 1U ][ 2U ] );
            float16_t3 const b = ( a + float16_t3 ( -m[ 0U ][ 2U ], m[ 1U ][ 0U ], m[ 2U ][ 1U ] ) ) * omega;
            return float16_t4 ( b.xy, phi, b.z );
        }
    }

    // SOLUTION_W:
    float16_t3 const a = float16_t3 ( m[ 0U ][ 1U ], m[ 0U ][ 2U ], m[ 1U ][ 2U ] );
    float16_t3 const b = a + float16_t3 ( -m[ 1U ][ 0U ], m[ 2U ][ 0U ], m[ 2U ][ 1U ] );
    return float16_t4 ( b * omega, phi );

#undef SOLUTION_X
#undef SOLUTION_Y
#undef SOLUTION_Z
#undef SOLUTION_W

}


#endif // TBN_INC
