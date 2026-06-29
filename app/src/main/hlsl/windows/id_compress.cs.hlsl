#include "platform/windows/pbr/id_compress.inc"
#include "platform/windows/pbr/resource_heap.inc"


#define WINDOW                  8U


struct PushConstants
{
    uint32_t                    _idSet;
    uint32_t                    _uniqueIDs;
    uint32_t                    _uniqueCount;
    uint32_t                    _capacity;
};

[[vk::push_constant]]
PushConstants                   g_pushConstants;

// [2026/09/21] DXC has no syntax with atomics and 'ResourceDescriptorHeap'.
// So the workaround is used.

[[vk::binding ( BIND_RESOURCES, SET_RESOURCE_HEAP )]]
RWStructuredBuffer<uint64_t>    g_idBuffers[]:          register ( u0 );
RWStructuredBuffer<uint32_t>    g_countBuffers[]:       register ( u1 );

static uint64_t                 g_ids[ WINDOW ];

//----------------------------------------------------------------------------------------------------------------------

uint16_t Fill ( in uint32_t idx )
{
    RWStructuredBuffer<uint64_t> idSet = g_idBuffers[ g_pushConstants._idSet ];
    uint16_t stored = 0U;
    uint64_t last = 0ULL;
    idx *= WINDOW;

    for ( uint32_t i = 0U; i < WINDOW; ++i )
    {
        uint32_t target = idx + i;

        if ( target >= g_pushConstants._capacity )
            continue;

        uint64_t const id = idSet[ target ];

        if ( ( id == 0ULL ) | ( last == id ) )
            continue;

        g_ids[ stored++ ] = id;
        last = id;
    }

    return stored;
}

void BubbleSort ( in uint16_t stored )
{
    uint16_t const outerLimit = stored - (uint16_t)1U;

    for ( uint16_t i = 0U; i < outerLimit; ++i )
    {
        uint16_t const innerLimit = outerLimit - (uint16_t)1U;

        for ( uint16_t prevIdx = 0U; prevIdx < innerLimit; ++prevIdx )
        {
            uint32_t const nextIdx = prevIdx + 1U;
            uint64_t const prev = g_ids[ prevIdx ];
            uint64_t const next = g_ids[ nextIdx ];

            if ( prev >= next )
                continue;

            g_ids[ prevIdx ] = next;
            g_ids[ nextIdx ] = prev;
        }
    }
}

void Commit ( in uint16_t stored )
{
    RWStructuredBuffer<uint64_t> uniqueIDSet = g_idBuffers[ g_pushConstants._uniqueIDs ];
    RWStructuredBuffer<uint32_t> uniqueCount = g_countBuffers[ g_pushConstants._uniqueCount ];
    uint64_t last = 0ULL;

    for ( uint16_t i = 0U; i < stored; ++i )
    {
        uint64_t const id = g_ids[ i ];

        if ( last == id )
            continue;

        uint32_t target;
        InterlockedAdd ( uniqueCount[ 0U ], 1U, target );
        uniqueIDSet[ target ] = g_ids[ i ];
        last = id;
    }
}

//----------------------------------------------------------------------------------------------------------------------

[numthreads ( THREADS, 1U, 1U )]
void CS ( in uint32_t3 idx : SV_DispatchThreadID )
{
    uint16_t const stored = Fill ( idx.x );

    if ( stored > 0U )
    {
        BubbleSort ( stored );
        Commit ( stored );
    }
}
