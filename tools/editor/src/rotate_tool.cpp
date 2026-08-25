#include <precompiled_headers.hpp>
#include <rotate_tool.hpp>

// FUCK - remove
#include <logger.hpp>


namespace editor {

namespace {

constexpr float AXIS_ACTIVE_SIZE = 6.0e-2F;
constexpr float ORTHOGONAL_THRESHOLD = 1.0e-4F;

constexpr eSDFPalette ACTIVE_COLOR = eSDFPalette::Yellow;
constexpr eSDFPalette INACTIVE_COLOR = eSDFPalette::Grey;

constexpr eSDFPalette X_COLOR = eSDFPalette::Red;
constexpr eSDFPalette Y_COLOR = eSDFPalette::Green;
constexpr eSDFPalette Z_COLOR = eSDFPalette::Blue;
constexpr eSDFPalette RING_COLOR = eSDFPalette::Grey;

constexpr float TANGENT_OFFSET_X = -3.5F;

// FUCK - this value depends from DPI
constexpr float BALL_SENSITIVITY = 1.74532925e-2F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void RotateTool::Activate () noexcept
{
    _x.Show ( _location, _rotation );
    _y.Show ( _location, _rotation );
    _z.Show ( _location, _rotation );
    _ring.Show ( _location, _rotation );
    android_vulkan::LogInfo ( ">>> Rotate tool activated" );
}

void RotateTool::Deactivate () noexcept
{
    _x.Hide ();
    _y.Hide ();
    _z.Hide ();
    _ring.Hide ();
    _body.Hide ();
    _tangentLine.Hide ();
    _tangentDirectionA.Hide ();
    _tangentDirectionB.Hide ();
    android_vulkan::LogInfo ( "<<< Rotate tool deactivated" );
}

void RotateTool::Hover () noexcept
{
    // FUCK
}

void RotateTool::Click () noexcept
{
    // FUCK
}

void RotateTool::Begin () noexcept
{
    // FUCK
}

void RotateTool::Move () noexcept
{
    // FUCK
}

void RotateTool::End () noexcept
{
    // FUCK
}

void RotateTool::Cancel () noexcept
{
    // FUCK
}

void RotateTool::Update () noexcept
{
    _x.OnParentUpdated ( _location, _rotation );
    _y.OnParentUpdated ( _location, _rotation );
    _z.OnParentUpdated ( _location, _rotation );
    _ring.OnParentUpdated ( _location, _rotation );
}

void RotateTool::ActivateSDF ( SDF &sdf ) noexcept
{
    if ( &sdf == _controlSDF ) [[likely]]
        return;

    DeactivateSDF ();
    _controlSDF = &sdf;

    if ( &sdf == &_body )
    {
        sdf.Show ( _location, _rotation );
        return;
    }

    GXVec3 const &s = _inactiveSize.at ( &sdf );
    sdf.SetScale ( GXVec3 ( s._data[ 0U ], s._data[ 1U ], AXIS_ACTIVE_SIZE ) );
}

void RotateTool::DeactivateSDF () noexcept
{
    if ( !_controlSDF ) [[likely]]
        return;

    if ( _controlSDF == &_body )
        _controlSDF->Hide ();
    else
        _controlSDF->SetScale ( _inactiveSize.at ( _controlSDF ) );

    _controlSDF = nullptr;
}

void RotateTool::HandleRingRotate ( GXVec3 const &rayOrigin, GXVec3 const &rayDirection ) noexcept
{
    // See <repo>/docs/gizmo-rendering.md#inter-ring
    float const f = ResolveSkewLines ( rayOrigin, rayDirection, _tangentPosition, _tangentDirection );

    GXQuat alpha {};
    alpha.FromAxisAngle ( _rotateAxisVector, _rotateSpeed * ( f - _initialScalarDistance ) );

    _rotation.Multiply ( alpha, _initialRotation );
    _tangentLine.SetLocationAndRotation ( _tangentRenderPosition, _tangentRenderRotation );
    _tangentDirectionA.SetLocationAndRotation ( _tangentDirectionARenderPosition, _tangentRenderRotation );
    _tangentDirectionB.SetLocationAndRotation ( _tangentRenderPosition, _tangentDirectionBRenderRotation );
}

void RotateTool::HandleBallRotate ( GXVec2 const &mouse, GXMat3 const &cameraBasis ) noexcept
{
    GXVec2 delta {};
    delta.Subtract ( mouse, _lastMouse );
    delta.Multiply ( delta, BALL_SENSITIVITY );

    GXQuat alpha {};
    alpha.FromAxisAngle ( *reinterpret_cast<GXVec3 const*> ( cameraBasis._data ), delta._data[ 1U ] );

    GXQuat beta {};
    alpha.FromAxisAngle ( *reinterpret_cast<GXVec3 const*> ( cameraBasis._data[ 1U ] ), -delta._data[ 0U ] );

    GXQuat zeta {};
    zeta.Multiply ( alpha, beta );

    alpha.Multiply ( zeta, _rotation );
    _rotation = alpha;

    _lastMouse = mouse;
}

float RotateTool::SetupRing ( SDFRingBase &ring, eSDFPalette color ) const noexcept
{
    ring.SetColor ( color );
    GXVec3 const &s = _inactiveSize.find ( &ring )->second;
    ring.SetScale ( s );
    return s._data[ 0U ];
}

void RotateTool::ResetVisuals () noexcept
{
    std::ignore = SetupRing ( _x, X_COLOR );
    std::ignore = SetupRing ( _y, Y_COLOR );
    std::ignore = SetupRing ( _z, Z_COLOR );

    _ring.Show ( _location, _rotation );
    std::ignore = SetupRing ( _ring, RING_COLOR );

    _tangentLine.Hide ();
    _tangentDirectionA.Hide ();
    _tangentDirectionB.Hide ();
}

bool RotateTool::LockAxis () noexcept
{
    float offset;

    switch ( _rotateAxis )
    {
        case eAxis::X:
            offset = SetupRing ( _x, ACTIVE_COLOR );
            std::ignore = SetupRing ( _y, INACTIVE_COLOR );
            std::ignore = SetupRing ( _z, INACTIVE_COLOR );
            std::ignore = SetupRing ( _ring, INACTIVE_COLOR );
        break;

        case eAxis::Y:
            std::ignore = SetupRing ( _x, INACTIVE_COLOR );
            offset = SetupRing ( _y, ACTIVE_COLOR );
            std::ignore = SetupRing ( _z, INACTIVE_COLOR );
            std::ignore = SetupRing ( _ring, INACTIVE_COLOR );
        break;

        case eAxis::Z:
            std::ignore = SetupRing ( _x, INACTIVE_COLOR );
            std::ignore = SetupRing ( _y, INACTIVE_COLOR );
            offset = SetupRing ( _z, ACTIVE_COLOR );
            std::ignore = SetupRing ( _ring, INACTIVE_COLOR );
        break;

        case eAxis::ToCamera:
            std::ignore = SetupRing ( _x, INACTIVE_COLOR );
            std::ignore = SetupRing ( _y, INACTIVE_COLOR );
            std::ignore = SetupRing ( _z, INACTIVE_COLOR );
            offset = SetupRing ( _ring, ACTIVE_COLOR );
        break;

        case eAxis::None:
            [[fallthrough]];
        default:
            // NOTHING
        return false;
    }

    _body.Hide ();

    // FUCK - try to construct 3x3 matrix, convert to quaternion, reverse up and right, convert to quaternion.
    GXVec3 up {};
    up.CrossProduct ( _rotateAxisVector, _tangentDirection );
    _tangentDirectionBRenderRotation.From ( _rotateAxisVector, up );

    up.Reverse ();
    _tangentRenderRotation.From ( _rotateAxisVector, up );

    GXVec3 a {};
    a.Subtract ( _tangentPosition, _location );
    a.Normalize ();

    GXVec3 pivot {};
    pivot.Sum ( _location, offset, a );

    GXVec3 s {};
    s.Multiply ( _tangentDirection, TANGENT_OFFSET_X );
    _tangentRenderPosition.Subtract ( pivot, s );
    _tangentDirectionARenderPosition.Sum ( pivot, s );

    _tangentLine.Show ( GXVec3::ZERO, GXQuat::IDENTITY );
    _tangentDirectionA.Show ( GXVec3::ZERO, GXQuat::IDENTITY );
    _tangentDirectionB.Show ( GXVec3::ZERO, GXQuat::IDENTITY );

    return true;
}

bool RotateTool::LockBall () noexcept
{
    if ( !_rotateBall )
        return false;

    std::ignore = SetupRing ( _x, ACTIVE_COLOR );
    std::ignore = SetupRing ( _y, ACTIVE_COLOR );
    std::ignore = SetupRing ( _z, ACTIVE_COLOR );

    _body.Show ( _location, _rotation );
    _ring.Hide ();
    _tangentLine.Hide ();
    _tangentDirectionA.Hide ();
    _tangentDirectionB.Hide ();
    return true;
}

RotateTool::TangentLine RotateTool::ResolveTangentLine ( GXVec3 const &ringPosition,
    GXVec3 const &ringDirection,
    GXVec3 const &rayOrigin,
    GXVec3 const &rayDirection,
    GXVec3 const &oppositeCameraDirection,
    float radius
) noexcept
{
    // See <repo>/docs/gizmo-rendering.md#inter-ring
    TangentLine result {};
    float const alpha = ringDirection.DotProduct ( rayDirection );

    if ( std::abs ( alpha ) < ORTHOGONAL_THRESHOLD )
    {
        result._tangentDirection.CrossProduct ( ringDirection, rayDirection );
        result._tangentPosition.Sum ( ringPosition, radius, oppositeCameraDirection );
    }
    else
    {
        GXVec3 lambda {};
        lambda.Subtract ( ringPosition, rayOrigin );

        GXVec3 g {};
        g.Sum ( rayOrigin, lambda.DotProduct ( ringDirection ) / alpha, rayDirection );

        lambda.Subtract ( g, ringPosition );
        lambda.Normalize ();

        result._tangentDirection.CrossProduct ( ringDirection, lambda );
        result._tangentPosition.Sum ( ringPosition, radius, lambda );
    }

    result._distance = ResolveSkewLines ( rayOrigin, rayDirection, result._tangentPosition, result._tangentDirection );
    return result;
}

float RotateTool::ResolveSkewLines ( GXVec3 const &aPosition,
    GXVec3 const &aDirection,
    GXVec3 const &bPosition,
    GXVec3 const &bDirection
) noexcept
{
    // See <repo>/docs/gizmo-rendering.md#inter-ring
    GXVec3 eta {};
    eta.CrossProduct ( bDirection, aDirection );

    GXVec3 alpha {};
    alpha.Subtract ( aPosition, bPosition );

    return alpha.DotProduct ( eta ) / bDirection.DotProduct ( eta );
}

} // namespace editor
