#include <pbr/reflection_global_pass.h>


namespace pbr {

ReflectionGlobalPass::ReflectionGlobalPass () noexcept:
    _prefilters {}
{
    // NOTHING
}

void ReflectionGlobalPass::Append ( TextureCubeRef &prefilter )
{
    _prefilters.push_back ( prefilter );
}

[[maybe_unused]] void ReflectionGlobalPass::Execute ( GXMat4 const &/*viewToWorld*/ )
{
    // TODO
}

[[maybe_unused]] bool ReflectionGlobalPass::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &viewport
)
{
    return _program.Init ( renderer, renderPass, subpass, viewport );
}

[[maybe_unused]] void ReflectionGlobalPass::Destroy ( VkDevice /*device*/ )
{
    // TODO
}

void ReflectionGlobalPass::Reset ()
{
    _prefilters.clear ();
}

} // namespace pbr
