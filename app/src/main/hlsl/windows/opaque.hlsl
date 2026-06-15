#ifndef OPAQUE_HLSL
#define OPAQUE_HLSL


#include "geometry_pass.hlsl"
#include "platform/windows/pbr/samplers.inc"
#include "windows/gbuffer_attributes.hlsl"
#include "windows/gbuffer_push_constants.hlsl"
#include "windows/gbuffer_render_targets.hlsl"
#include "windows/gbuffer_streams.hlsl"


OutputData Compute ( in Attributes attributes, in uint64_t shadingStream )
{
    Shading const shading = vk::RawBufferLoad<Shading> ( shadingStream + attributes._instanceID * sizeof ( Shading ),
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


#endif // OPAQUE_HLSL
