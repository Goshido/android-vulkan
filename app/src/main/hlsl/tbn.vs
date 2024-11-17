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

float16_t4 RotateTBN ( in float16_t3 imaginaryTBN, in float32_t4 localView )
{
    // By convention xyz contains imaginary part of quaternion. The w component contains mirroring information.
    // Real part of quaternion must be restored. By convention it's used unit quaternions to represent TBN.
    // So the real component will be restored using this property.
    float16_t4 const rotation = (float16_t4)localView;

    // Note dot product could be a little bit bigger than 1.0H due to float16_t inaccurency. Fixing it with abs.
    float16_t4 tbn = float16_t4 ( sqrt ( abs ( 1.0H - dot ( imaginaryTBN, imaginaryTBN ) ) ), imaginaryTBN );

    return float16_t4 (
        dot ( rotation, float16_t4 ( tbn.x, -tbn.yzw ) ),
        dot ( rotation, float16_t4 ( tbn.yxw, -tbn.z ) ),
        dot ( rotation, float16_t4 ( tbn.z, -tbn.w, tbn.xy ) ),
        dot ( rotation, float16_t4 ( tbn.wz, -tbn.y, tbn.x ) )
    );
}
