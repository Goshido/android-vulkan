#ifndef GBUFFER_RENDER_TARGETS_PS_HLSL
#define GBUFFER_RENDER_TARGETS_PS_HLSL


#include "pbr/geometry_pass_binds.inc"


struct OutputData
{
    [[vk::location ( OUT_ALBEDO )]]
    float32_t4      _albedo:        SV_Target0;

    [[vk::location ( OUT_EMISSION )]]
    float32_t4      _emission:      SV_Target1;

    [[vk::location ( OUT_NORMAL )]]
    float32_t4      _normal:        SV_Target2;

    [[vk::location ( OUT_PARAM )]]
    float32_t4      _param:         SV_Target3;
};


#endif // GBUFFER_RENDER_TARGETS_PS_HLSL
