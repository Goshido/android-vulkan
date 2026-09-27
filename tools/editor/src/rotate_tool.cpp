#include <precompiled_headers.hpp>
#include <rotate_tool.hpp>
#include <trace.hpp>
#include <workspace.hpp>


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

//----------------------------------------------------------------------------------------------------------------------

class RotateActorAction final : public Action
{
    public:
        struct State final
        {
            GXQuat      _rotation {};
            GXVec3      _location {};
        };

        struct Item final
        {
            Actor*      _actor = nullptr;
            State       _before {};
            State       _after {};
        };

        using Items = std::vector<Item>;

    private:
        Items           _items {};

    public:
        RotateActorAction () = delete;

        RotateActorAction ( RotateActorAction const & ) = delete;
        RotateActorAction &operator = ( RotateActorAction const & ) = delete;

        RotateActorAction ( RotateActorAction && ) = default;
        RotateActorAction &operator = ( RotateActorAction && ) = default;

        explicit RotateActorAction ( Items &&items ) noexcept;

        ~RotateActorAction () override = default;

    private:
        void Redo () noexcept override;
        void Undo () noexcept override;
};

RotateActorAction::RotateActorAction ( Items &&items ) noexcept:
    _items ( std::move ( items ) )
{
    // NOTHING
}

void RotateActorAction::Redo () noexcept
{
    AV_TRACE ( "Redo rotate" )

    for ( Item &item : _items )
    {
        State const &state = item._after;
        item._actor->SetLocal ( state._rotation, state._location );
    }

    Workspace::Instance ().OnContentUpdated ();
}

void RotateActorAction::Undo () noexcept
{
    AV_TRACE ( "Undo rotate" )

    for ( Item &item : _items )
    {
        State const &state = item._before;
        item._actor->SetLocal ( state._rotation, state._location );
    }

    Workspace::Instance ().OnContentUpdated ();
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void RotateTool::Activate () noexcept
{
    AV_TRACE ( "Rotate tool activate" )
    _x.Show ( _location, _rotation );
    _y.Show ( _location, _rotation );
    _z.Show ( _location, _rotation );
    _ring.Show ( _location, _rotation );
    _body.OnParentUpdated ( _location, _rotation );
}

void RotateTool::Deactivate () noexcept
{
    AV_TRACE ( "Rotate tool deactivate" )
    _x.Hide ();
    _y.Hide ();
    _z.Hide ();
    _ring.Hide ();
    _body.Hide ();
    _tangentLine.Hide ();
    _tangentDirectionA.Hide ();
    _tangentDirectionB.Hide ();
}

void RotateTool::Begin ( Selection::Actors &items, GXQuat const &rotation ) noexcept
{
    AV_TRACE ( "Rotate tool begin" )
    size_t const count = items.size ();

    _items.clear ();
    _items.reserve ( count );

    GXVec3 const c = GetCenter ( items );
    GXQuat toGizmo {};
    toGizmo.Inverse ( rotation );

    for ( Actor const *actor : items )
    {
        Item item
        {
            ._actorRotation = actor->GetRotation (),
            ._actorLocation = actor->GetLocation ()
        };

        item._actorOffset.Subtract ( item._actorLocation, c );
        item._gizmoRotation.Multiply ( toGizmo, item._actorRotation );

        _items.push_back ( std::move ( item ) );
    }

    _location = c;
    _rotation = rotation;

    _x.OnParentUpdated ( c, rotation );
    _y.OnParentUpdated ( c, rotation );
    _z.OnParentUpdated ( c, rotation );
    _ring.OnParentUpdated ( c, rotation );
}

void RotateTool::End () noexcept
{
    AV_TRACE ( "Rotate tool end" )
    Selection::Actors &actors = Workspace::Instance ().GetSelection ().GetActors ();
    RotateActorAction::Items rotateItems {};
    rotateItems.reserve ( actors.size () );
    auto items = _items.cbegin ();

    for ( Actor* actor : actors )
    {
        Item const &backup = *items++;

        rotateItems.push_back (
            RotateActorAction::Item
            {
                ._actor = actor,

                ._before
                {
                    ._rotation = backup._actorRotation,
                    ._location = backup._actorLocation
                },

                ._after
                {
                    ._rotation = actor->GetRotation (),
                    ._location = actor->GetLocation ()
                }
            }
        );
    }

    History &history = History::Instance ();
    history.Begin ();
    history.Append ( std::make_unique<RotateActorAction> ( std::move ( rotateItems ) ) );
    history.End ();
}

void RotateTool::Cancel () noexcept
{
    AV_TRACE ( "Rotate tool cancel" )
    auto items = _items.cbegin ();

    for ( Actor* actor : Workspace::Instance ().GetSelection ().GetActors () )
    {
        Item const &backup = *items++;
        actor->SetLocal ( backup._actorRotation, backup._actorLocation );
    }
}

bool RotateTool::Update ( GXVec3 const &rayDirection,
    GXVec3 const &cameraLocation,
    GXMat3 const &cameraBasis,
    GXVec3 const &vi,
    VkOffset2D const &mouse,
    bool leftMouseButtonPressed
) noexcept
{
    AV_TRACE ( "Rotate tool update" )
    bool const prevRotation = ( _rotateAxis != eAxis::None ) | _rotateBall;
    bool const lmbPressed = leftMouseButtonPressed & !_lastLMBPressed;
    bool const lmbReleased = !leftMouseButtonPressed & std::exchange ( _lastLMBPressed, leftMouseButtonPressed );

    eAxis const cases[] = { _rotateAxis, eAxis::None };
    _rotateAxis = cases[ static_cast<size_t> ( lmbReleased ) ];

    if ( _rotateAxis != eAxis::None )
    {
        HandleRingRotate ( mouse );
        return true;
    }

    _rotateBall &= !lmbReleased;

    if ( _rotateBall )
    {
        HandleBallRotate ( mouse, cameraBasis );
        return true;
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
        return true;

    if ( !closest._control )
    {
        DeactivateSDF ();
        return false;
    }

    ActivateSDF ( *closest._control );
    return true;
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
            static_cast<float> ( _lastMouse.y - mouse.y )
        )
    );

    GXQuat alpha {};
    alpha.FromAxisAngle ( _rotateAxisVector, f * RING_SENSITIVITY );
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

    UpdateChildren ();
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

    _body.OnParentUpdated ( _location, _rotation );
    _lastMouse = mouse;

    UpdateChildren ();
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

    float const d = collider.Raycast ( rayDirection,
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

void RotateTool::UpdateChildren () noexcept
{
    _x.OnParentUpdated ( _location, _rotation );
    _y.OnParentUpdated ( _location, _rotation );
    _z.OnParentUpdated ( _location, _rotation );

    auto items = _items.cbegin ();
    GXVec3 alpha {};
    GXVec3 beta {};
    GXQuat zeta {};

    for ( Actor* actor : Workspace::Instance ().GetSelection ().GetActors () )
    {
        Item const &item = *items++;
        zeta.Multiply ( _rotation, item._gizmoRotation );
        _rotation.TransformFast ( alpha, item._actorOffset );
        beta.Sum ( alpha, _location );
        actor->SetLocal ( zeta, beta );
    }
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
