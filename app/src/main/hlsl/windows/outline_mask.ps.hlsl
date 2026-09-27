#include "platform/windows/pbr/outline_mask_binds.inc"


struct OutputData
{
    [[vk::location ( OUT_MASK )]]
    float32_t4      _mask:      SV_Target0;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData PS ()
{
    OutputData result;
    result._mask = float32_t4 ( 1.0F, 0.0F, 0.0F, 0.0F );
    return result;
}
