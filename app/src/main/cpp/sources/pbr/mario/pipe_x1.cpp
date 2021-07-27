#include <pbr/mario/pipe_x1.h>


namespace pbr::mario {

constexpr static char const MESH[] = "pbr/assets/Props/experimental/world-1-1/pipe/pipe-x1.mesh2";
constexpr static GXVec3 const OFFSET ( -0.8F, 1.2F, 0.8F );
constexpr static GXVec3 const SIZE ( 1.6F, 2.4F, 1.6F );

GXVec3 const& PipeX1::GetColliderOffset () const noexcept
{
    return OFFSET;
}

GXVec3 const& PipeX1::GetColliderSize () const noexcept
{
    return SIZE;
}

char const* PipeX1::GetMesh () const noexcept
{
    return MESH;
}

} // namespace pbr::mario
