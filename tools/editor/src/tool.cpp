#include <precompiled_headers.hpp>
#include <tool.hpp>


namespace editor {

GXVec3 Tool::GetCenter ( Selection::Items const &items ) noexcept
{
    GXAABB bounds{};

    for ( Actor const* actor : items )
        bounds.AddVertex ( actor->GetLocation () );

    GXVec3 result;
    bounds.GetCenter ( result );
    return result;
}

} // namespace editor
