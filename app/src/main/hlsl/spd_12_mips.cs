// See <repo/docs/spd-algorithm.md>

// Note: The most detailed mip is skipped.
// Luma values are calculated inplace from HDR image during calculation of first mip level.
// Mip 5 is using dedicated UAV.
// Result will be stored in buffer instead of mip level.
// That's why it's needed to remove 3 elements.
#define MIP_COUNT       9U

#include "spd_handle_mip_10.cs"


[numthreads ( THREAD_X, THREAD_Y, THREAD_Z )]
void CS ( in uint32_t threadID: SV_GroupIndex, in uint32_t3 workGroupID: SV_GroupID )
{
    const uint32_t2 base = GetThreadTarget ( threadID );
    DownsampleMips01 ( base.x, base.y, workGroupID.xy, threadID );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip2 ( base.x, base.y, workGroupID.xy, threadID );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip3 ( base.x, base.y, workGroupID.xy, threadID );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip4 ( base.x, base.y, workGroupID.xy, threadID );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip5 ( workGroupID.xy, threadID );

    if ( ExitWorkgroup ( threadID ) )
        return;

    // Reset the global atomic counter back to 0 for the next spd dispatch.
    if ( threadID == 0U )
        g_GlobalAtomic[ 0U ]._counter = 0U;

    DownsampleMips67 ( base.x, base.y );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip8 ( base.x, base.y, threadID );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip9 ( base.x, base.y, threadID );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip10Last ( base.x, base.y, threadID );
}
