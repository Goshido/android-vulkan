#include <pbr/mario/camera.h>


namespace pbr::mario {

constexpr static float const FIELD_OF_VIEW = 60.0F;
constexpr static float const Z_NEAR = 0.1F;
constexpr static float const Z_FAR = 1.0e+4F;
constexpr static GXVec3 const OFFSET ( 512.0F, 128.0F, 0.0F );
constexpr static float const TOLERANCE = 1.0e-4F;
constexpr static float const SPEED = 2.0e-2F;

Camera::Camera () noexcept:
    _local {},
    _projection {},
    _target ( nullptr )
{
    _local.RotationY ( -GX_MATH_HALF_PI );
    _projection.Identity ();
}

void Camera::Focus () noexcept
{
    if ( !_target )
        return;

    GXVec3 target {};
    _target->GetTransform ().GetW ( target );

    GXVec3 ideal {};
    ideal.Sum ( target, OFFSET );
    _local.SetW ( ideal );
}

GXMat4 const& Camera::GetLocalMatrix () const noexcept
{
    return _local;
}

GXMat4 const& Camera::GetProjectionMatrix () const noexcept
{
    return _projection;
}

void Camera::OnUpdate ( float deltaTime ) noexcept
{
    if ( !_target )
        return;

    GXVec3 target {};
    _target->GetTransform ().GetW ( target );

    GXVec3 ideal {};
    ideal.Sum ( target, OFFSET );

    GXVec3 loc {};
    _local.GetW ( loc );

    GXVec3 delta {};
    delta.Subtract ( ideal, loc );

    float const length = delta.Length ();

    if ( length < TOLERANCE )
        return;

    loc.Sum ( loc, length * SPEED * deltaTime, delta );
    _local.SetW ( loc );
}

void Camera::OnResolutionChanged ( VkExtent2D const &resolution ) noexcept
{
    _projection.Perspective ( GXDegToRad ( FIELD_OF_VIEW ),
        static_cast<float> ( resolution.width ) / static_cast<float> ( resolution.height ),
        Z_NEAR,
        Z_FAR
    );
}

void Camera::SetTarget ( ITarget const &target ) noexcept
{
    _target = &target;
}

} // namespace pbr::mario
