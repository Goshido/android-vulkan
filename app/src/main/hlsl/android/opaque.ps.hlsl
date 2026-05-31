#include "android/gbuffer_render_targets.hlsl"
#include "android/geometry_pass_attributes.hlsl"
#include "android/geometry_pass_material.hlsl"
#include "android/object_data.hlsl"
#include "geometry_pass.hlsl"


//----------------------------------------------------------------------------------------------------------------------

OutputData PS ( in Attributes attributes )
{
    GBufferResult const r = FillGBuffer ( attributes._uv,
        attributes._tangentView,
        attributes._bitangentView,
        attributes._normalView,
        g_sampler,
        g_emissionTexture,
        g_maskTexture,
        g_normalTexture,
        g_paramTexture,
        g_colorData[ attributes._instanceIndex ],
        (float16_t3)g_diffuseTexture.Sample ( g_sampler, attributes._uv ).xyz
    );

    OutputData result;
    result._albedo = r._albedo;
    result._emission = r._emission;
    result._normal = r._normal;
    result._param = r._param;

    return result;
}
