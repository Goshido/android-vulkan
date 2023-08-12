// It's modified AMD SPD algorithm.
// See https://gpuopen.com/fidelityfx-spd/

// Note: The most detailed mip is skipped.
// Luma values are calculated inplace from HDR image during calculation of first mip level.
#define MIP_COUNT       10U

#include "spd_common.cs"


[numthreads ( THREAD_X, THREAD_Y, THREAD_Z )]
void CS ( in uint32_t threadID: SV_GroupIndex, in uint32_t3 workGroupID: SV_GroupID )
{
    const uint32_t2 base = GetThreadTarget ( threadID );
    DownsampleMips01 ( base.x, base.y, workGroupID.xy, threadID );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip2 ( base.x, base.y, workGroupID.xy, threadID, 2U );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip3 ( base.x, base.y, workGroupID.xy, threadID, 3U );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip4 ( base.x, base.y, workGroupID.xy, threadID, 4U );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip5 ( workGroupID.xy, threadID, 5U );

    // increase the global atomic counter for the given slice and check if it's the last remaining thread group:
    // terminate if not, continue if yes.
    if ( ExitWorkgroup ( threadID ) )
        return;

    // reset the global atomic counter back to 0 for the next spd dispatch
    g_GlobalAtomic[ 0U ]._counter = 0U;

    DownsampleMips67 ( base.x, base.y );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip2 ( base.x, base.y, workGroupID.xy, threadID, 8U );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip3 ( base.x, base.y, workGroupID.xy, threadID, 9U );
}