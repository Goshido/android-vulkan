#include "pbr/skin.inc"
#include "tbn.inc"


#define BONES_PER_VERTEX    4U

struct Mesh2Vertex
{
    float32_t2                      _uv;
    uint32_t                        _tbn;
};

struct SkinInfluence
{
    uint32_t                        _boneIndex;
    float32_t                       _boneWeight;
};

struct SkinVertex
{
    SkinInfluence                   _influences[ BONES_PER_VERTEX ];
};

struct Pose
{
    float32_t3                      _location;
    float32_t4                      _orientation;
};

struct PushConstants
{
    uint32_t                        _count;
};

[[vk::push_constant]]
PushConstants                       g_vertexCount;

[[vk::binding ( BIND_POSE, SET_RESOURCE )]]
StructuredBuffer<Pose>              g_pose:                 register ( t0 );

[[vk::binding ( BIND_REFERENCE_POSITIONS, SET_RESOURCE )]]
StructuredBuffer<float32_t3>        g_referencePositons:    register ( t1 );

[[vk::binding ( BIND_REFERENCE_REST, SET_RESOURCE )]]
StructuredBuffer<Mesh2Vertex>       g_referenceRest:        register ( t2 );

[[vk::binding ( BIND_SKIN_VERTICES, SET_RESOURCE )]]
StructuredBuffer<SkinVertex>        g_skinVertices:         register ( t3 );

[[vk::binding ( BIND_SKIN_POSITIONS, SET_RESOURCE )]]
RWStructuredBuffer<float32_t3>      g_skinPositions:        register ( u0 );

[[vk::binding ( BIND_SKIN_REST, SET_RESOURCE )]]
RWStructuredBuffer<Mesh2Vertex>     g_skinRest:             register ( u1 );

//----------------------------------------------------------------------------------------------------------------------

float32_t4x3 ExtractBoneTransform ( in uint32_t boneIdx )
{
    Pose const joint = g_pose[ boneIdx ];

    float32_t4 const rabc = joint._orientation;
    float32_t3 const abc2 = rabc.yzw + rabc.yzw;

    // Note 'T' is just notation for variable separation. Nothing more.
    float32_t4 const rXrTabc2 = rabc.x * float32_t4 ( rabc.x, abc2 );
    float32_t4 const caaaXcaTbc2 = float32_t4 ( rabc.wyyy ) * float32_t4 ( rabc.wy, abc2.yz );
    float32_t2 const bXbTc2 = rabc.z * float32_t2 ( rabc.z, abc2.z );

    float32_t4 const left0 = float32_t4 ( rXrTabc2.w, caaaXcaTbc2.w, caaaXcaTbc2.z, rXrTabc2.y );
    float32_t4 const right0 = float32_t4 ( caaaXcaTbc2.z, -rXrTabc2.z, -rXrTabc2.w, bXbTc2.y );

    float32_t2 const tmp1 = float32_t2 ( rXrTabc2.z, bXbTc2.y ) + float32_t2 ( caaaXcaTbc2.w, -rXrTabc2.y );
    float32_t4 const tmp0 = left0 + right0;

    // Note quaternion unpacks to matrix with column-major like behaviour.
    return float32_t4x3
    (
        // First row.
        rXrTabc2.x + caaaXcaTbc2.y - bXbTc2.x - caaaXcaTbc2.x,
        tmp0.x,
        tmp0.y,

        // Second row.
        tmp0.z,
        rXrTabc2.x - caaaXcaTbc2.y + bXbTc2.x - caaaXcaTbc2.x,
        tmp0.w,

        // Third row.
        tmp1.x,
        tmp1.y,
        rXrTabc2.x - caaaXcaTbc2.y - bXbTc2.x + caaaXcaTbc2.x,

        // Forth row.
        joint._location.x,
        joint._location.y,
        joint._location.z
    );
}

float32_t4x3 ComputeSkinTransform ( in uint32_t vertexIndex )
{
    // [2024/11/16] Note it could be assumed that it's correct that weighted sum trick could work for quaternions.
    // Unfortunatelly it does not. The math breaks because of quaternion duality. Summing quaternions like that
    // will produce incorrect result. That's why converting quaternions to 3x3 matrix is required to
    // perform weighted sum later.
    SkinVertex const skin = g_skinVertices[ vertexIndex ];

    SkinInfluence const influence0 = skin._influences[ 0U ];
    SkinInfluence const influence1 = skin._influences[ 1U ];
    SkinInfluence const influence2 = skin._influences[ 2U ];
    SkinInfluence const influence3 = skin._influences[ 3U ];

    float32_t4x3 const t0 = ExtractBoneTransform ( influence0._boneIndex );
    float32_t4x3 const t1 = ExtractBoneTransform ( influence1._boneIndex );

    float32_t4x3 const w0 = t0 * influence0._boneWeight;
    float32_t4x3 const t2 = ExtractBoneTransform ( influence2._boneIndex );

    float32_t4x3 const w1 = t1 * influence1._boneWeight;
    float32_t4x3 const t3 = ExtractBoneTransform ( influence3._boneIndex );

    float32_t4x3 const w01 = w0 + w1;
    float32_t4x3 const w2 = t2 * influence2._boneWeight;
    float32_t4x3 const w3 = t3 * influence3._boneWeight;
    float32_t4x3 const w012 = w01 + w2;

    return w012 + w3;
}

//----------------------------------------------------------------------------------------------------------------------

[numthreads ( THREADS_PER_GROUP, 1U, 1U )]
void CS ( in uint32_t localThreadIndex: SV_GroupIndex, in uint32_t3 dispatch: SV_GroupID )
{
    uint32_t const baseIdx = dot ( dispatch, uint32_t3 ( THREADS_PER_GROUP, THREADS_PER_PLANE, THREADS_PER_SLICE ) );
    uint32_t const idx = localThreadIndex + baseIdx;

    if ( idx >= g_vertexCount._count )
        return;

    Mesh2Vertex const referenceRest = g_referenceRest[ idx ];
    uint32_t const tbn = referenceRest._tbn;

    float32_t4x3 const skinTransform = ComputeSkinTransform ( idx );
    g_skinPositions[ idx ] = mul ( float32_t4 ( g_referencePositons[ idx ], 1.0F ), skinTransform );

    Mesh2Vertex skinRest;
    skinRest._uv = referenceRest._uv;
    skinRest._tbn = CompressTBN ( RotateTBN ( DecompressTBN ( tbn ), ToTBN ( (float16_t3x3)skinTransform ) ), tbn );

    g_skinRest[ idx ] = skinRest;
}
