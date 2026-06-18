#include "platform/windows/pbr/id.inc"
#include "platform/windows/pbr/resource_heap.inc"


struct PushConstants
{
    uint32_t                _idImage;
    uint32_t                _idSet;
    uint32_t                _capacity;
};

[[vk::push_constant]]
PushConstants               g_pushConstants;

// [2025/09/11] DXC has issue with 'globallycoherent' and 'ResourceDescriptorHeap'.
// So the workaround is used.
// See https://github.com/microsoft/DirectXShaderCompiler/issues/7740

[[vk::binding ( BIND_RESOURCES, SET_RESOURCE_HEAP )]]
Texture2D<uint32_t4>        g_images[]:     register ( t0 );

struct Pair
{
    uint64_t                _key : 63;
    uint64_t                _hasValue: 1;
    uint64_t                _id;
};

[[vk::binding ( BIND_RESOURCES, SET_RESOURCE_HEAP )]]
RWStructuredBuffer<Pair>    g_buffers[]:    register ( u0 );

//----------------------------------------------------------------------------------------------------------------------

uint64_t UnpackID ( in Texture2D<uint32_t4> ids, in uint32_t2 pix )
{
    uint64_t4 alpha = (uint64_t4)ids[ pix ];
    alpha.yzw <<= uint64_t3 ( 16U, 32U, 48U );
    return alpha.x | alpha.y | alpha.z | alpha.w;
}

uint64_t Hash ( in uint64_t id )
{
    // Taken from MSVC std::hash<void const*>.
    uint64_t4 alpha = (uint64_t4)id;
    uint64_t4 beta = (uint64_t4)id;

    alpha.yzw &= uint64_t3 ( 0x00FF000000000000ULL, 0x0000FF0000000000ULL, 0x000000FF00000000ULL );
    beta &= uint64_t4 ( 0x00000000FF000000ULL, 0x0000000000FF0000ULL, 0x000000000000FF00ULL, 0x00000000000000FFULL );

    alpha >>= uint32_t4 ( 56U, 48U, 40U, 32U );
    beta.xyz >>= uint32_t3 ( 24U, 16U, 8U );

    uint64_t const fnvPrime = 1099511628211ULL;
    uint64_t hash = 14695981039346656037ULL ^ beta.w;
    hash *= fnvPrime;

    hash ^= beta.z;
    hash *= fnvPrime;

    hash ^= beta.y;
    hash *= fnvPrime;

    hash ^= beta.x;
    hash *= fnvPrime;

    hash ^= alpha.w;
    hash *= fnvPrime;

    hash ^= alpha.z;
    hash *= fnvPrime;

    hash ^= alpha.y;
    hash *= fnvPrime;

    hash ^= alpha.x;
    return hash * fnvPrime;
}

void InsertID ( uint64_t id )
{
    if ( id == 0U )
        return;

    RWStructuredBuffer<Pair> idSet = ResourceDescriptorHeap[ g_pushConstants._idSet ];
    uint64_t const hash = Hash ( id );
}

//----------------------------------------------------------------------------------------------------------------------

[numthreads ( THREADS_X, THREADS_Y, 1U )]
void CS ( in uint32_t3 pix : SV_DispatchThreadID )
{
    Texture2D<uint32_t4> ids = ResourceDescriptorHeap[ g_pushConstants._idImage ];
    uint32_t2 resolution;
    ids.GetDimensions ( resolution.x, resolution.y );

    if ( all ( pix.xy < resolution ) )
    {
        InsertID ( UnpackID ( ids, pix.xy ) );
    }
}
