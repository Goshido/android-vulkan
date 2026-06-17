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
    uint64_t                _empty: 1;
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

void InsertID ( uint64_t id )
{
    if ( id == 0U )
        return;

    RWStructuredBuffer<Pair> idSet = ResourceDescriptorHeap[ g_pushConstants._idSet ];
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
