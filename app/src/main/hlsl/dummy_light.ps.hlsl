#include "lightup_common.hlsl"


//----------------------------------------------------------------------------------------------------------------------

OutputData PS ()
{
    /*
    [2024/12/07] Note it's needed to actually touch input attachment. VVL 1.3.302 checks SPIR-V blob and tracks
    real usage of the descriptor set resources. Otherwise it will be WAW sync error:
    [ SYNC-HAZARD-WRITE-AFTER-WRITE ]
        name = Render session, type = VK_OBJECT_TYPE_RENDER_PASS;
    vkCmdEndRenderPass(): Hazard WRITE_AFTER_WRITE
        in subpass 1 for attachment 4 depth aspect
        during store with
            storeOp VK_ATTACHMENT_STORE_OP_DONT_CARE.
    Access info
        (usage: SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
    prior_usage:
        SYNC_IMAGE_LAYOUT_TRANSITION,
    write_barriers:
        SYNC_FRAGMENT_SHADER_COLOR_ATTACHMENT_READ|SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ|
        SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ|
        SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE
    command: vkCmdNextSubpass
    */

    OutputData result;
    result._color = (float32_t4)g_depth.SubpassLoad ();
    return result;
}
