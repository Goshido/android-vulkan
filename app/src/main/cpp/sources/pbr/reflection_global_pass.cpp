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

void ReflectionGlobalPass::Reset ()
{
    _prefilters.clear ();
}

} // namespace pbr
