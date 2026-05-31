#ifndef TBN_HLSL
#define TBN_HLSL


#include "quat.hlsl"


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


#endif // TBN_HLSL
