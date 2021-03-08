#include <pbr/global_reflection_pass.h>


namespace pbr {

GlobalReflectionPass::GlobalReflectionPass () noexcept:
    _prefilters {}
{
    // NOTHING
}

void GlobalReflectionPass::Append ( TextureCubeRef &prefilter )
{
    _prefilters.push_back ( prefilter );
}

void GlobalReflectionPass::Reset ()
{
    _prefilters.clear ();
}

} // namespace pbr
