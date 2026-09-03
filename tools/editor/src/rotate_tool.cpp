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

// FUCK - this values depend from DPI
constexpr float RING_SENSITIVITY = 1.5e-2F;
constexpr float BALL_SENSITIVITY = 1.74532925e-2F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void RotateTool::Activate () noexcept
{
    _x.Show ( _location, _rotation );
    _y.Show ( _location, _rotation );
    _z.Show ( _location, _rotation );
    _ring.Show ( _location, _rotation );
    _body.OnParentUpdated ( _location, _rotation );
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

void RotateTool::Update ( GXVec3 const &rayDirection,
    GXVec3 const &cameraLocation,
    GXMat3 const &cameraBasis,
    GXVec3 const &vi,
    VkOffset2D const &mouse,
    bool leftMouseButtonPressed
) noexcept
{
    bool const prevRotation = ( _rotateAxis != eAxis::None ) | _rotateBall;
    bool const lmbPressed = leftMouseButtonPressed & !_lastLMBPressed;
    bool const lmbReleased = !leftMouseButtonPressed & std::exchange ( _lastLMBPressed, leftMouseButtonPressed );

    eAxis const cases[] = { _rotateAxis, eAxis::None };
    _rotateAxis = cases[ static_cast<size_t> ( lmbReleased ) ];

    if ( _rotateAxis != eAxis::None )
    {
        HandleRingRotate ( mouse );
        return;
    }

    _rotateBall &= !lmbReleased;

    if ( _rotateBall )
    {
        HandleBallRotate ( mouse, cameraBasis );
        return;
    }

    if ( prevRotation )
        ResetVisuals ();

    GXVec3 d {};
    d.Subtract ( _location, cameraLocation );

    GXVec3 k ( cameraBasis.Forward () );
    k.Reverse ();

    float const s = vi.DotProduct ( d );
    Closest closest {};

    CheckRing ( closest,
        _x,
        _xCollider,
        rayDirection,
        cameraLocation,
        cameraBasis,
        k,
        vi,
        mouse,
        s,
        false,
        lmbPressed,
        eAxis::X
    );

    CheckRing ( closest,
        _y,
        _yCollider,
        rayDirection,
        cameraLocation,
        cameraBasis,
        k,
        vi,
        mouse,
        s,
        false,
        lmbPressed,
        eAxis::Y
    );

    CheckRing ( closest,
        _z,
        _zCollider,
        rayDirection,
        cameraLocation,
        cameraBasis,
        k,
        vi,
        mouse,
        s,
        false,
        lmbPressed,
        eAxis::Z
    );

    CheckRing ( closest,
        _ring,
        _ringCollider,
        rayDirection,
        cameraLocation,
        cameraBasis,
        k,
        vi,
        mouse,
        s,
        true,
        lmbPressed,
        eAxis::ToCamera
    );

    CheckBody ( closest, rayDirection, cameraLocation, vi, mouse, lmbPressed );

    if ( LockAxis () || LockBall () )
        return;

    if ( !closest._control )
    {
        DeactivateSDF ();
        return;
    }

    ActivateSDF ( *closest._control );
}

void RotateTool::ActivateSDF ( SDF &sdf ) noexcept
{
    if ( &sdf == _control ) [[likely]]
        return;

    DeactivateSDF ();
    _control = &sdf;

    if ( &sdf == &_body )
    {
        sdf.Show ( _location, _rotation );
        return;
    }

    GXVec3 const &s = _inactiveSize.at ( &sdf );
    sdf.SetScale ( GXVec3 ( s._data[ 0UZ ], s._data[ 1UZ ], AXIS_ACTIVE_SIZE ) );
}

void RotateTool::DeactivateSDF () noexcept
{
    if ( !_control ) [[likely]]
        return;

    if ( _control == &_body )
        _control->Hide ();
    else
        _control->SetScale ( _inactiveSize.at ( _control ) );

    _control = nullptr;
}

void RotateTool::HandleRingRotate ( VkOffset2D const &mouse ) noexcept
{
    // See <repo>/docs/gizmo-rendering.md#inter-ring
    float const f = _tangentProjection.DotProduct (
        GXVec2 (
            static_cast<float> ( mouse.x - _lastMouse.x ),
            static_cast<float> ( mouse.y - _lastMouse.y )
        )
    );

    GXQuat alpha {};
    alpha.FromAxisAngle ( _rotateAxisVector, RING_SENSITIVITY * ( f - _initialScalarDistance ) );
    _rotation.Multiply ( alpha, _initialRotation );

    GXVec3 beta {};
    beta.Subtract ( _tangentRenderPosition, _location );
    _tangentLine.SetLocationAndRotation ( beta, _tangentRenderRotation );
    _tangentLine.OnParentUpdated ( _location, GXQuat::IDENTITY );

    _tangentDirectionB.SetLocationAndRotation ( beta, _tangentDirectionBRenderRotation );
    _tangentDirectionB.OnParentUpdated ( _location, GXQuat::IDENTITY );

    beta.Subtract ( _tangentDirectionARenderPosition, _location );
    _tangentDirectionA.SetLocationAndRotation ( beta, _tangentRenderRotation );
    _tangentDirectionA.OnParentUpdated ( _location, GXQuat::IDENTITY );

    _x.OnParentUpdated ( _location, _rotation );
    _y.OnParentUpdated ( _location, _rotation );
    _z.OnParentUpdated ( _location, _rotation );
}

void RotateTool::HandleBallRotate ( VkOffset2D const &mouse, GXMat3 const &cameraBasis ) noexcept
{
    int32_t const dx = _lastMouse.x - mouse.x;
    int32_t const dy = _lastMouse.y - mouse.y;

    GXVec2 delta {};
    delta.Multiply ( GXVec2 ( static_cast<float> ( dx ), static_cast<float> ( dy ) ), BALL_SENSITIVITY );

    GXQuat alpha {};
    alpha.FromAxisAngle ( cameraBasis.Right (), delta._data[ 1UZ ] );

    GXQuat beta {};
    beta.FromAxisAngle ( cameraBasis.Up (), delta._data[ 0UZ ] );

    GXQuat zeta {};
    zeta.Multiply ( alpha, beta );

    alpha.Multiply ( zeta, _rotation );
    _rotation = alpha;

    _x.OnParentUpdated ( _location, _rotation );
    _y.OnParentUpdated ( _location, _rotation );
    _z.OnParentUpdated ( _location, _rotation );
    _body.OnParentUpdated ( _location, _rotation );

    _lastMouse = mouse;
}

void RotateTool::CheckBody ( Closest &closest,
    GXVec3 const &rayDirection,
    GXVec3 const &cameraLocation,
    GXVec3 const &vi,
    VkOffset2D const &mouse,
    bool lmbPressed
) noexcept
{
    float const d = _bodyCollider.Raycast ( rayDirection,
        _body.GetLocationWorld (),
        cameraLocation,
        vi
    );

    if ( d >= closest._distance )
        return;

    closest._control = &_body;

    if ( !lmbPressed )
        return;

    _rotateBall = true;
    _rotateAxis = eAxis::None;
    _lastMouse = mouse;
}

void RotateTool::CheckRing ( Closest &closest,
    SDF &sdf,
    GizmoRingCollider const &collider,
    GXVec3 const &rayDirection,
    GXVec3 const &cameraLocation,
    GXMat3 const &cameraBasis,
    GXVec3 const &k,
    GXVec3 const &vi,
    VkOffset2D const &mouse,
    float s,
    bool billboard,
    bool lmbPressed,
    eAxis axis
) noexcept
{
    GXQuat const &rotation = sdf.GetRotationWorld ();
    GXVec3 const &location = sdf.GetLocationWorld ();

    float const d = collider.Raycast ( cameraLocation,
        rayDirection,
        location,
        rotation,
        cameraLocation,
        cameraBasis,
        vi,
        billboard
    );

    if ( d >= closest._distance )
        return;

    closest =
    {
        ._control = &sdf,
        ._distance = d
    };

    if ( !lmbPressed )
        return;

    _rotateBall = false;
    _rotateAxis = axis;

    GXVec3 forward {};
    rotation.GetForward ( forward );
    GXVec3 const cases[] = { forward, k };
    _rotateAxisVector = cases[ static_cast<size_t> ( billboard ) ];
    _initialRotation = _rotation;

    TangentLine const info = ResolveTangentLine ( location,
        _rotateAxisVector,
        cameraLocation,
        rayDirection,
        k,
        s * collider.GetRadius ()
    );

    _lastMouse = mouse;
    _tangentLocation = info._tangentLocation;

    GXVec3 const &dir = info._tangentDirection;
    _tangentDirection = dir;

    // See <repo>/docs/gizmo-rendering.md#inter-ring
    GXVec2 m ( dir.DotProduct ( cameraBasis.Right () ), dir.DotProduct ( cameraBasis.Up () ) );
    m._data[ 1U ] = -m._data[ 1UZ ];
    _tangentProjection = m;
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

    GXMat3 m {};
    GXVec3 &up = m.Up ();
    up.CrossProduct ( _rotateAxisVector, _tangentDirection );

    GXVec3 &right = m.Right ();
    right = _tangentDirection;

    m.Forward () = _rotateAxisVector;
    _tangentDirectionBRenderRotation.FromFast ( m );

    up.Reverse ();
    right.Reverse ();
    _tangentRenderRotation.From ( m );

    GXVec3 a {};
    a.Subtract ( _tangentLocation, _location );
    a.Normalize ();

    GXVec3 pivot {};
    pivot.Sum ( _location, offset, a );

    GXVec3 s {};
    s.Multiply ( _tangentDirection, TANGENT_OFFSET_X );
    _tangentRenderPosition.Subtract ( pivot, s );
    _tangentDirectionARenderPosition.Sum ( pivot, s );

    GXVec3 beta {};
    beta.Subtract ( _tangentRenderPosition, _location );
    _tangentLine.SetLocationAndRotation ( beta, _tangentRenderRotation );
    _tangentLine.Show ( _location, GXQuat::IDENTITY );

    _tangentDirectionB.SetLocationAndRotation ( beta, _tangentDirectionBRenderRotation );
    _tangentDirectionB.Show ( _location, GXQuat::IDENTITY );

    beta.Subtract ( _tangentDirectionARenderPosition, _location );
    _tangentDirectionA.SetLocationAndRotation ( beta, _tangentRenderRotation );
    _tangentDirectionA.Show ( _location, GXQuat::IDENTITY );

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
        result._tangentLocation.Sum ( ringPosition, radius, oppositeCameraDirection );
        return result;
    }

    GXVec3 lambda {};
    lambda.Subtract ( ringPosition, rayOrigin );

    GXVec3 g {};
    g.Sum ( rayOrigin, lambda.DotProduct ( ringDirection ) / alpha, rayDirection );

    lambda.Subtract ( g, ringPosition );
    lambda.Normalize ();

    result._tangentDirection.CrossProduct ( ringDirection, lambda );
    result._tangentLocation.Sum ( ringPosition, radius, lambda );

    return result;
}

} // namespace editor
