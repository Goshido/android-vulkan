#include <pbr/reflection_local_pass.h>


namespace pbr {

ReflectionLocalPass::Call::Call ( GXVec3 const &location, TextureCubeRef &prefilter, float size ):
    _location ( location ),
    _prefilter ( prefilter ),
    _size ( size )
{
    // NOTHING
}

ReflectionLocalPass::ReflectionLocalPass () noexcept:
    _calls {},
    _program {}
{
    // NOTHING
}

void ReflectionLocalPass::Append ( TextureCubeRef &prefilter, GXVec3 const &location, float size )
{
    _calls.emplace_back ( Call ( location, prefilter, size ) );
}

size_t ReflectionLocalPass::GetReflectionLocalCount () const
{
    return _calls.size ();
}

bool ReflectionLocalPass::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &viewport
)
{
    if ( _program.Init ( renderer, renderPass, subpass, viewport ) )
        return true;

    Destroy ( renderer.GetDevice () );
    return false;
}

void ReflectionLocalPass::Destroy ( VkDevice device )
{
    _program.Destroy ( device );
}

void ReflectionLocalPass::Reset ()
{
    _calls.clear ();
}

} // namespace pbr
