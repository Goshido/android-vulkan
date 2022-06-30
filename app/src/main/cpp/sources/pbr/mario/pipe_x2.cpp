#include <pbr/mario/pipe_x2.h>


namespace pbr::mario {

constexpr static GXVec3 OFFSET ( -0.8F, 1.625F, 0.8F );
constexpr static GXVec3 SIZE ( 1.6F, 3.25F, 1.6F );

//----------------------------------------------------------------------------------------------------------------------

void PipeX2::Spawn ( Scene &scene, float x, float y, float z ) noexcept
{
    PipeBase::SpawnBase ( scene, x, y, z, OFFSET, SIZE );
}

} // namespace pbr::mario
