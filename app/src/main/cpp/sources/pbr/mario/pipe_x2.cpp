#include <pbr/mario/pipe_x2.h>


namespace pbr::mario {

constexpr static char const MESH[] = "pbr/assets/Props/experimental/world-1-1/pipe/pipe-x2.mesh2";
constexpr static GXVec3 OFFSET ( -0.8F, 1.625F, 0.8F );
constexpr static GXVec3 SIZE ( 1.6F, 3.25F, 1.6F );

//----------------------------------------------------------------------------------------------------------------------

GXVec3 const& PipeX2::GetColliderOffset () const noexcept
{
    return OFFSET;
}

GXVec3 const& PipeX2::GetColliderSize () const noexcept
{
    return SIZE;
}

char const* PipeX2::GetMesh () const noexcept
{
    return MESH;
}

} // namespace pbr::mario
