#include "geometry_pass.hlsl"
#include "platform/windows/pbr/gbuffer_pass_binds.inc"
#include "platform/windows/pbr/samplers.inc"
#include "windows/gbuffer_attributes.hlsl"
#include "windows/gbuffer_push_constants.hlsl"


struct Shading
{
    uint32_t        _albedo;
    uint32_t        _emission;
    uint32_t        _mask;
    uint32_t        _param;
    uint32_t        _normal;
    ColorData       _colors;
};

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

//----------------------------------------------------------------------------------------------------------------------

OutputData PS ( in Attributes attributes )
{
    Shading const shading = vk::RawBufferLoad<Shading> (
        g_pushConstants._shadingStream + attributes._instanceID * sizeof ( Shading ),
        4U
    );

    SamplerState materialSampler = SamplerDescriptorHeap[ MATERIAL_SAMPLER ];

    Texture2D<float32_t4> albedo = ResourceDescriptorHeap[ NonUniformResourceIndex ( shading._albedo ) ];
    Texture2D<float32_t4> emission = ResourceDescriptorHeap[ NonUniformResourceIndex ( shading._emission ) ];
    Texture2D<float32_t4> mask = ResourceDescriptorHeap[ NonUniformResourceIndex ( shading._mask ) ];
    Texture2D<float32_t4> param = ResourceDescriptorHeap[ NonUniformResourceIndex ( shading._param ) ];
    Texture2D<float32_t4> normal = ResourceDescriptorHeap[ NonUniformResourceIndex ( shading._normal ) ];

    GBufferResult const r = FillGBuffer ( attributes._uv,
        attributes._tangentView,
        attributes._bitangentView,
        attributes._normalView,
        materialSampler,
        emission,
        mask,
        normal,
        param,
        shading._colors,
        (float16_t3)albedo.Sample ( materialSampler, attributes._uv ).xyz
    );

    OutputData result;
    result._albedo = r._albedo;
    result._emission = r._emission;
    result._normal = r._normal;
    result._param = r._param;

    return result;
}
