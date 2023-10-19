#include "skin.inc"


#define BONES_PER_VERTEX    4U

struct Mesh2Vertex
{
    float32_t3          _vertex;
    float32_t2          _uv;
    float32_t3          _normal;
    float32_t3          _tangent;
    float32_t3          _bitangent;
};

struct SkinInfluence
{
    uint32_t            _boneIndex;
    float32_t           _boneWeight;
};

struct SkinVertex
{
    SkinInfluence       _influences[ BONES_PER_VERTEX ];
};

struct Pose
{
    float32_t3          _location;
    float32_t4          _orientation;
};

[[vk::constant_id ( PUSH_VERTEX_COUNT )]]
uint32_t const                      g_vertexCount = 777U;

[[vk::binding ( BIND_POSE, SET_RESOURCE )]]
StructuredBuffer<Pose>              g_pose:             register ( t0 );

[[vk::binding ( BIND_REFERENCE_MESH, SET_RESOURCE )]]
StructuredBuffer<Mesh2Vertex>       g_referenceMesh:    register ( t1 );

[[vk::binding ( BIND_SKIN, SET_RESOURCE )]]
StructuredBuffer<SkinVertex>        g_skin:             register ( t2 );

[[vk::binding ( BIND_SKIN_MESH, SET_RESOURCE )]]
RWStructuredBuffer<Mesh2Vertex>     g_skinMesh:         register ( u0 );

//----------------------------------------------------------------------------------------------------------------------

float32_t4x3 ExtractBoneTransform ( in uint32_t boneIdx )
{
    Pose const joint = g_pose[ boneIdx ];

    float32_t4 const rabc = joint._orientation;
    float32_t3 const abc2 = rabc.yzw + rabc.yzw;

    // Note 'T' is just notation for variable separation. Nothing more.
    float32_t4 const rXrTabc2 = rabc.x * float32_t4 ( rabc.x, abc2 );
    float32_t3 const aXaTbc2 = rabc.y * float32_t3 ( rabc.y, abc2.yz );
    float32_t2 const bXbTc2 = rabc.z * float32_t2 ( rabc.z, abc2.z );
    float32_t const cXc = rabc.w * rabc.w;

    return float32_t4x3
    (
        // First row.
        rXrTabc2.x + aXaTbc2.x - bXbTc2.x - cXc,
        rXrTabc2.w + aXaTbc2.y,
        aXaTbc2.z - rXrTabc2.z,

        // Second row.
        aXaTbc2.y - rXrTabc2.w,
        rXrTabc2.x - aXaTbc2.x + bXbTc2.x - cXc,
        rXrTabc2.y + bXbTc2.y,

        // Third row.
        rXrTabc2.z + aXaTbc2.z,
        bXbTc2.y - rXrTabc2.y,
        rXrTabc2.x - aXaTbc2.x - bXbTc2.x + cXc,

        // Forth row.
        joint._location.x,
        joint._location.y,
        joint._location.z
    );
}

float32_t4x4 ComputeSkinTransform ( in uint32_t vertexIndex )
{
    SkinVertex const skin = g_skin[ vertexIndex ];

    float32_t4x3 const t0 = ExtractBoneTransform ( skin._influences[ 0U ]._boneIndex );
    float32_t4x3 const t1 = ExtractBoneTransform ( skin._influences[ 1U ]._boneIndex );

    float32_t4x3 const w0 = t0 * skin._influences[ 0U ]._boneWeight;
    float32_t4x3 const t2 = ExtractBoneTransform ( skin._influences[ 2U ]._boneIndex );

    float32_t4x3 const w1 = t1 * skin._influences[ 1U ]._boneWeight;
    float32_t4x3 const t3 = ExtractBoneTransform ( skin._influences[ 3U ]._boneIndex );

    float32_t4x3 const w01 = w0 + w1;
    float32_t4x3 const w2 = t2 * skin._influences[ 2U ]._boneWeight;
    float32_t4x3 const w3 = t3 * skin._influences[ 3U ]._boneWeight;
    float32_t4x3 const w012 = w01 + w2;
    float32_t4x3 const w0123 = w012 + w3;

    return float32_t4x4
    (
        float32_t4 ( w0123[ 0U ], 0.0F ),
        float32_t4 ( w0123[ 1U ], 0.0F ),
        float32_t4 ( w0123[ 2U ], 0.0F ),
        float32_t4 ( w0123[ 3U ], 1.0F )
    );
}

//----------------------------------------------------------------------------------------------------------------------

[numthreads ( THREADS_PER_GROUP, 1U, 1U )]
void CS ( in uint32_t localThreadIndex: SV_GroupIndex, in uint32_t3 globalThreadIndex: SV_DispatchThreadID )
{
    uint32_t2 const flatten = globalThreadIndex.yz * uint32_t2 ( THREADS_PER_GROUP, THREADS_PER_Z );
    uint32_t const idx = localThreadIndex + flatten.x + flatten.y;

    if ( idx >= g_vertexCount )
        return;

    Mesh2Vertex const srcVertex = g_referenceMesh[ idx ];
    float32_t4x4 const skinTransform = ComputeSkinTransform ( idx );

    // Note UV data is already baked into skin mesh data and it's constant. So no any actions are needed for that data.
    Mesh2Vertex skinVertex;
    skinVertex._vertex = mul ( skinTransform, float32_t4 ( srcVertex._vertex, 1.0F ) ).xyz;

    float32_t3x3 const orientation = (float32_t3x3)skinTransform;
    skinVertex._normal = mul ( orientation, srcVertex._normal );
    skinVertex._tangent = mul ( orientation, srcVertex._tangent );
    skinVertex._bitangent = mul ( orientation, srcVertex._bitangent );

    g_skinMesh[ idx ] = skinVertex;
}
