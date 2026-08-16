#include "platform/windows/pbr/id_collect.inc"
#include "platform/windows/pbr/resource_heap.inc"


struct PushConstants
{
    uint32_t                    _idImage;
    uint32_t2                   _offset;
    uint32_t2                   _size;
    uint32_t                    _idSet;
    uint32_t                    _capacity;
};

[[vk::push_constant]]
PushConstants                   g_pushConstants;

// [2026/08/16] DXC has no syntax explicit image format and 'ResourceDescriptorHeap'.
// So the workaround is used.
// [2026/08/16] Never mix ResourceDescriptorHeap|SamplerDescriptorHeap and explicit descriptor indexing in same shader.
// The DXC will produce unexpected 'Binding' locations in SPIR-V. Be consistent.

[[vk::binding ( BIND_RESOURCES, SET_RESOURCE_HEAP )]]
[[vk::image_format ( "rg32ui" )]]
RWTexture2D<uint32_t2>          g_images[]:     register ( u0 );

[[vk::binding ( BIND_RESOURCES, SET_RESOURCE_HEAP )]]
RWStructuredBuffer<uint64_t>    g_buffers[]:    register ( u1 );

//----------------------------------------------------------------------------------------------------------------------

uint64_t UnpackID ( in uint32_t2 pix )
{
    RWTexture2D<uint32_t2> ids = g_images[ g_pushConstants._idImage ];
    uint64_t2 const alpha = (uint64_t2)ids[ pix ];
    return alpha.x | ( alpha.y << 32U );
}

uint32_t BucketIndex ( in uint64_t id )
{
    // The implementation is based on ideas from from MSVC std::hash<void const*>.
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
    return (uint32_t)( ( hash * fnvPrime ) % (uint64_t)g_pushConstants._capacity );
}

void InsertID ( in uint64_t id )
{
    if ( id == 0U )
        return;

    // The implementation is based on ideas from
    // https://developer.nvidia.com/blog/maximizing-performance-with-massively-parallel-hash-maps-on-gpus/
    RWStructuredBuffer<uint64_t> idSet = g_buffers[ g_pushConstants._idSet ];

    for ( uint32_t i = BucketIndex ( id ); ; i = ( i + 1U ) % g_pushConstants._capacity )
    {
        uint64_t old;
        InterlockedCompareExchange ( idSet[ i ], 0ULL, id, old );

        if ( ( old == 0ULL ) | ( old == id ) )
        {
            return;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

[numthreads ( THREADS_X, THREADS_Y, 1U )]
void CS ( in uint32_t3 pix : SV_DispatchThreadID )
{
    if ( all ( pix.xy <= g_pushConstants._size ) )
    {
        InsertID ( UnpackID ( pix.xy + g_pushConstants._offset ) );
    }
}
