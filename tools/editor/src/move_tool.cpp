#include <precompiled_headers.hpp>
#include <gizmo_box_collider.hpp>
#include <gizmo_cylinder_collider.hpp>
#include <move_tool.hpp>
#include <sdf_size.hpp>

// FUCK - remove
#include <actor.hpp>
#include <logger.hpp>


namespace editor {

// FUCK
extern Actor* fuck_actor;

namespace {

constexpr float     AXIS_SDF_ACTIVE_SIZE = 0.08F;
constexpr float     AXIS_SDF_STANDBY_SIZE = 0.02F;
constexpr float     AXIS_SDF_LENGTH = 6.4F;

constexpr float     AXIS_CLIP_OFFSET = 2.4F;
constexpr float     AXIS_FULL_LENGTH = 8.8F;
constexpr float     AXIS_RADIUS = 0.35F;

constexpr float     CONE_ACTIVE_LENGTH = 1.55F;
constexpr float     CONE_STANDBY_LENGTH = 1.5F;
constexpr float     CONE_ACTIVE_SIZE = 0.5F;
constexpr float     CONE_STANDBY_SIZE = 0.45F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

MoveTool::MoveTool () noexcept
{
    _xPlane.SetFlipSensors ( _yPlane.GetRotationWorld (),
        _yPlane.GetLocationWorld (),
        _zPlane.GetRotationWorld (),
        _zPlane.GetLocationWorld ()
    );

    _yPlane.SetFlipSensors ( _zPlane.GetRotationWorld (),
        _zPlane.GetLocationWorld (),
        _xPlane.GetRotationWorld (),
        _xPlane.GetLocationWorld ()
    );

    _zPlane.SetFlipSensors ( _xPlane.GetRotationWorld (),
        _xPlane.GetLocationWorld (),
        _yPlane.GetRotationWorld (),
        _yPlane.GetLocationWorld ()
    );
}

void MoveTool::Activate () noexcept
{
    _origin.Show ( _location, _rotation );
    _xLine.Show ( _location, _rotation );
    _xPlane.Show ( _location, _rotation );
    _xCone.Show ( _location, _rotation );
    _yLine.Show ( _location, _rotation );
    _yPlane.Show ( _location, _rotation );
    _yCone.Show ( _location, _rotation );
    _zLine.Show ( _location, _rotation );
    _zPlane.Show ( _location, _rotation );
    _zCone.Show ( _location, _rotation );
    _xPlaneY.Show ( _location, _rotation );
    _xPlaneZ.Show ( _location, _rotation );
    _yPlaneZ.Show ( _location, _rotation );
    _yPlaneX.Show ( _location, _rotation );
    _zPlaneX.Show ( _location, _rotation );
    _zPlaneY.Show ( _location, _rotation );
    android_vulkan::LogInfo ( ">>> Move tool activated" );
}

void MoveTool::Deactivate () noexcept
{
    _origin.Hide ();
    _xLine.Hide ();
    _xPlane.Hide ();
    _xCone.Hide ();
    _yLine.Hide ();
    _yPlane.Hide ();
    _yCone.Hide ();
    _zLine.Hide ();
    _zPlane.Hide ();
    _zCone.Hide ();
    _xPlaneY.Hide ();
    _xPlaneZ.Hide ();
    _yPlaneZ.Hide ();
    _yPlaneX.Hide ();
    _zPlaneX.Hide ();
    _zPlaneY.Hide ();
    android_vulkan::LogInfo ( "<<< Move tool deactivated" );
}

void MoveTool::Hover () noexcept
{
    // FUCK
}

void MoveTool::Click () noexcept
{
    // FUCK
}

void MoveTool::Begin () noexcept
{
    // FUCK
}

void MoveTool::Move () noexcept
{
    // FUCK
}

void MoveTool::End () noexcept
{
    // FUCK
}

void MoveTool::Cancel () noexcept
{
    // FUCK
}

void MoveTool::Update ( GXVec3 const &rayDirection,
    GXVec3 const &cameraLocation,
    GXVec3 const &vi,
    bool leftMouseButtonPressed
) noexcept
{
    bool const prevMoving = _workAxis != eAxis::None || _workPlane != eAxis::None;
    bool const lmbPressed = leftMouseButtonPressed & !_lastLMBPressed;
    bool const lmbReleased = !leftMouseButtonPressed & std::exchange ( _lastLMBPressed, leftMouseButtonPressed );

    eAxis cases[] = { _workAxis, eAxis::None };
    _workAxis = cases[ static_cast<size_t> ( lmbReleased ) ];

    if ( _workAxis != eAxis::None )
    {
        HandleAxisMove ( cameraLocation, rayDirection );
        return;
    }

    cases[ 0UZ ] = _workPlane;
    _workPlane = cases[ static_cast<size_t> ( lmbReleased ) ];

    if ( _workPlane != eAxis::None )
    {
        HandlePlaneMove ( cameraLocation, rayDirection );
        return;
    }

    if ( prevMoving )
        ResetVisuals ();

    GXVec3 alpha {};
    alpha.Subtract ( _location, cameraLocation );
    float const pixelSize = SDF_PIXEL_SIZE_SCALE * vi.DotProduct ( alpha );

    bool const xTest = FlipTest ( _xPlane, cameraLocation );
    bool const yTest = FlipTest ( _yPlane, cameraLocation );
    bool const zTest = FlipTest ( _zPlane, cameraLocation );

    float const axisRadius = AXIS_RADIUS * pixelSize;
    Closest closest {};

    AxisCheck ( closest,
        _xLine,
        _xCone,
        xTest,
        eAxis::X,
        axisRadius,
        cameraLocation,
        rayDirection,
        pixelSize,
        lmbPressed
    );

    AxisCheck ( closest,
        _yLine,
        _yCone,
        yTest,
        eAxis::Y,
        axisRadius,
        cameraLocation,
        rayDirection,
        pixelSize,
        lmbPressed
    );

    AxisCheck ( closest,
        _zLine,
        _zCone,
        zTest,
        eAxis::Z,
        axisRadius,
        cameraLocation,
        rayDirection,
        pixelSize,
        lmbPressed
    );

    PlaneCheck ( closest,
        _xPlane,
        GXVec3 ( PLANE_OFFSET._data[ 0UZ ], PLANE_OFFSET._data[ 1UZ ],
        PLANE_OFFSET._data[ 1U ] ),
        yTest,
        zTest,
        eAxis::X,
        cameraLocation,
        rayDirection,
        pixelSize,
        lmbPressed
    );

    PlaneCheck ( closest,
        _yPlane,
        GXVec3 ( PLANE_OFFSET._data[ 1UZ ], PLANE_OFFSET._data[ 0UZ ], PLANE_OFFSET._data[ 1UZ ] ),
        zTest,
        xTest,
        eAxis::Y,
        cameraLocation,
        rayDirection,
        pixelSize,
        lmbPressed
    );

    PlaneCheck ( closest,
        _zPlane,
        GXVec3 ( PLANE_OFFSET._data[ 1UZ ], PLANE_OFFSET._data[ 1UZ ], PLANE_OFFSET._data[ 0UZ ] ),
        xTest,
        yTest,
        eAxis::Z,
        cameraLocation,
        rayDirection,
        pixelSize,
        lmbPressed
    );

    if ( LockAxis () || LockPlane ( cameraLocation ) )
        return;

    if ( !closest._control )
    {
        DeactivateSDF ();
        return;
    }

    ActivateSDF ( *closest._control, closest._cap );
}

std::optional<MoveTool::ColorSet> MoveTool::AcquirePlaneColorSet ( SDF &plane ) noexcept
{
    constexpr ColorSet x
    {
        ._active = X_PLANE_ACTIVE_COLOR,
        ._standby = X_PLANE_STANDBY_COLOR
    };

    constexpr ColorSet y
    {
        ._active = Y_PLANE_ACTIVE_COLOR,
        ._standby = Y_PLANE_STANDBY_COLOR
    };

    constexpr ColorSet z
    {
        ._active = Z_PLANE_ACTIVE_COLOR,
        ._standby = Z_PLANE_STANDBY_COLOR
    };

    if ( &plane == &_xPlane )
        return std::optional<ColorSet> { x };

    if ( &plane == &_yPlane )
        return std::optional<ColorSet> { y };

    if ( &plane == &_zPlane )
        return std::optional<ColorSet> { z };

    return std::nullopt;
}

void MoveTool::ActivateSDF ( SDF &sdf, SDF* cap ) noexcept
{
    if ( &sdf == _control )
        return;

    DeactivateSDF ();
    _control = &sdf;
    _cap = cap;

    if ( auto const colorSet = AcquirePlaneColorSet ( sdf ); colorSet )
    {
        sdf.SetColor ( colorSet->_active );
        return;
    }

    constexpr GXVec3 coneSize ( CONE_ACTIVE_LENGTH, CONE_ACTIVE_SIZE, CONE_ACTIVE_SIZE );
    cap->SetScale ( coneSize );

    constexpr GXVec3 axisSize ( AXIS_SDF_LENGTH, AXIS_SDF_ACTIVE_SIZE, AXIS_SDF_ACTIVE_SIZE );
    sdf.SetScale ( axisSize );
}

void MoveTool::DeactivateSDF () noexcept
{
    if ( !_control )
        return;

    SDF* c = std::exchange ( _control, nullptr );

    if ( auto const colorSet = AcquirePlaneColorSet ( *c ); colorSet )
    {
        c->SetColor ( colorSet->_standby );
        return;
    }

    constexpr GXVec3 coneSize ( CONE_STANDBY_LENGTH, CONE_STANDBY_SIZE, CONE_STANDBY_SIZE );
    _cap->SetScale ( coneSize );

    constexpr GXVec3 axisSize ( AXIS_SDF_LENGTH, AXIS_SDF_STANDBY_SIZE, AXIS_SDF_STANDBY_SIZE );
    c->SetScale ( axisSize );
}

void MoveTool::HandleAxisMove ( GXVec3 const &rayOrigin, GXVec3 const &rayDirection ) noexcept
{
    // See <repo>/docs/gizmo-rendering.md#inter-move-axis
    auto const d = ResolveAxisScalarDistance ( _initialState, _workDirection, rayOrigin, rayDirection );

    if ( !d )
        return;

    _location.Sum ( _initialState, *d + _initialNegativeScalarDistance, _workDirection );
    UpdateChildren ();

    fuck_actor->SetLocation ( _location );
}

void MoveTool::HandlePlaneMove ( GXVec3 const &rayOrigin, GXVec3 const &rayDirection ) noexcept
{
    // See <repo>/docs/gizmo-rendering.md#inter-move-plane
    auto const p = ResolvePlaneIntersection ( _initialState,
        _workDirection,
        rayOrigin,
        rayDirection
    );

    if ( !p )
        return;

    _location.Sum ( *p, _initialNegativePlaneOffset );
    UpdateChildren ();

    fuck_actor->SetLocation ( _location );
}

void MoveTool::ResetVisuals () noexcept
{
    auto const reset = [ this ] ( SDFLineSegment &axis,
        SDFLineSegmentWithFlip &planeA,
        SDFLineSegmentWithFlip &planeB,
        SDFBoxWithFlip &plane,
        SDFCone &cone,
        eSDFPalette color,
        eSDFPalette planeColor
    ) noexcept
    {
        constexpr GXVec3 axisSize ( AXIS_SDF_LENGTH, AXIS_SDF_STANDBY_SIZE, AXIS_SDF_STANDBY_SIZE );
        axis.SetScale ( axisSize );
        axis.SetColor ( color );

        planeA.Show ( _location, _rotation );
        planeA.SetColor ( color );
        planeA.UnlockSensors ();

        planeB.Show ( _location, _rotation );
        planeB.SetColor ( color );
        planeB.UnlockSensors ();

        plane.Show ( _location, _rotation );
        plane.SetColor ( planeColor );
        plane.UnlockSensors ();

        constexpr GXVec3 coneSize ( CONE_STANDBY_LENGTH, CONE_STANDBY_SIZE, CONE_STANDBY_SIZE );
        cone.SetScale ( coneSize );
        cone.SetColor ( color );
    };

    reset ( _xLine, _xPlaneY, _xPlaneZ, _xPlane, _xCone, X_COLOR, X_PLANE_STANDBY_COLOR );
    reset ( _yLine, _yPlaneZ, _yPlaneX, _yPlane, _yCone, Y_COLOR, Y_PLANE_STANDBY_COLOR );
    reset ( _zLine, _zPlaneX, _zPlaneY, _zPlane, _zCone, Z_COLOR, Z_PLANE_STANDBY_COLOR );
}

void MoveTool::UpdateChildren () noexcept
{
    _origin.OnParentUpdated ( _location, _rotation );
    _xLine.OnParentUpdated ( _location, _rotation );
    _xPlane.OnParentUpdated ( _location, _rotation );
    _xCone.OnParentUpdated ( _location, _rotation );
    _yLine.OnParentUpdated ( _location, _rotation );
    _yPlane.OnParentUpdated ( _location, _rotation );
    _yCone.OnParentUpdated ( _location, _rotation );
    _zLine.OnParentUpdated ( _location, _rotation );
    _zPlane.OnParentUpdated ( _location, _rotation );
    _zCone.OnParentUpdated ( _location, _rotation );
    _xPlaneY.OnParentUpdated ( _location, _rotation );
    _xPlaneZ.OnParentUpdated ( _location, _rotation );
    _yPlaneZ.OnParentUpdated ( _location, _rotation );
    _yPlaneX.OnParentUpdated ( _location, _rotation );
    _zPlaneX.OnParentUpdated ( _location, _rotation );
    _zPlaneY.OnParentUpdated ( _location, _rotation );
}

bool MoveTool::LockAxis () noexcept
{
    switch ( _workAxis )
    {
        case eAxis::X:
            SetupAxis ( _xLine, _xCone, WORK_OPAQUE_COLOR );
            SetupAxis ( _yLine, _yCone, WORK_INACTIVE_COLOR );
            SetupAxis ( _zLine, _zCone, WORK_INACTIVE_COLOR );
        break;

        case eAxis::Y:
            SetupAxis ( _xLine, _xCone, WORK_INACTIVE_COLOR );
            SetupAxis ( _yLine, _yCone, WORK_OPAQUE_COLOR );
            SetupAxis ( _zLine, _zCone, WORK_INACTIVE_COLOR );
        break;

        case eAxis::Z:
            SetupAxis ( _xLine, _xCone, WORK_INACTIVE_COLOR );
            SetupAxis ( _yLine, _yCone, WORK_INACTIVE_COLOR );
            SetupAxis ( _zLine, _zCone, WORK_OPAQUE_COLOR );
        break;

        case eAxis::None:
            [[fallthrough]];
        default:
            // NOTHING
        return false;
    }

    HidePlane ( _xPlane, _xPlaneY, _xPlaneZ );
    HidePlane ( _yPlane, _yPlaneZ, _yPlaneX );
    HidePlane ( _zPlane, _zPlaneX, _zPlaneY );
    return true;
}

bool MoveTool::LockPlane ( GXVec3 const &cameraLocation ) noexcept
{
    constexpr auto setup = [] ( SDFBoxWithFlip &plane,
        SDFLineSegmentWithFlip &lineA,
        SDFLineSegmentWithFlip &lineB,
        GXVec3 const &cameraLocation
    ) noexcept {
        plane.SetColor ( WORK_TRANSPARENT_COLOR );
        plane.LockSensors ( cameraLocation );

        lineA.SetColor ( WORK_OPAQUE_COLOR );
        lineA.LockSensors ( cameraLocation );

        lineB.SetColor ( WORK_OPAQUE_COLOR );
        lineB.LockSensors ( cameraLocation );
    };

    switch ( _workPlane )
    {
        case eAxis::X:
            setup ( _xPlane, _xPlaneY, _xPlaneZ, cameraLocation );
            HidePlane ( _yPlane, _yPlaneZ, _yPlaneX );
            HidePlane ( _zPlane, _zPlaneX, _zPlaneY );
            SetupAxis ( _xLine, _xCone, WORK_INACTIVE_COLOR );
            SetupAxis ( _yLine, _yCone, WORK_OPAQUE_COLOR );
            SetupAxis ( _zLine, _zCone, WORK_OPAQUE_COLOR );
        return true;

        case eAxis::Y:
            setup ( _yPlane, _yPlaneZ, _yPlaneX, cameraLocation );
            HidePlane ( _zPlane, _zPlaneX, _zPlaneY );
            HidePlane ( _xPlane, _xPlaneY, _xPlaneZ );
            SetupAxis ( _xLine, _xCone, WORK_OPAQUE_COLOR );
            SetupAxis ( _yLine, _yCone, WORK_INACTIVE_COLOR );
            SetupAxis ( _zLine, _zCone, WORK_OPAQUE_COLOR );
        return true;

        case eAxis::Z:
            setup ( _zPlane, _zPlaneX, _zPlaneY, cameraLocation );
            HidePlane ( _xPlane, _xPlaneY, _xPlaneZ );
            HidePlane ( _yPlane, _yPlaneZ, _yPlaneX );
            SetupAxis ( _xLine, _xCone, WORK_OPAQUE_COLOR );
            SetupAxis ( _yLine, _yCone, WORK_OPAQUE_COLOR );
            SetupAxis ( _zLine, _zCone, WORK_INACTIVE_COLOR );
        return true;

        case eAxis::None:
            [[fallthrough]];
        default:
            // NOTHING
        break;
    }

    return false;
}

void MoveTool::AxisCheck ( Closest &closest,
    SDFLineSegment &sdf,
    SDFCone &sdfCone,
    bool test,
    eAxis axis,
    float axisRadius,
    GXVec3 const &rayOrigin,
    GXVec3 const &rayDirection,
    float pixelSize,
    bool lmbPressed
) noexcept
{
    GXVec3 a {};
    sdf.GetRotationWorld ().GetRight ( a );
    float const cases[] = { AXIS_CLIP_OFFSET, 0.0F };
    float const offset = cases[ static_cast<size_t> ( test  ) ];
    GizmoCylinderCollider const cylinder ( axisRadius, ( AXIS_FULL_LENGTH - offset ) * pixelSize );

    GXVec3 alpha {};
    alpha.Sum ( _location, offset * pixelSize, a );
    float const d = cylinder.Raycast ( rayOrigin, rayDirection, alpha, a );

    if ( d >= closest._distance )
        return;

    closest =
    {
        ._control = &sdf,
        ._cap = &sdfCone,
        ._distance = d
    };

    if ( !lmbPressed )
        return;

    _workPlane = eAxis::None;
    auto const distance = ResolveAxisScalarDistance ( _location, a, rayOrigin, rayDirection );

    if ( !distance )
        return;

    _workAxis = axis;
    _workDirection = a;
    _initialNegativeScalarDistance = -distance.value ();
    _initialState = _location;
}

void MoveTool::PlaneCheck ( Closest &closest,
    SDFBoxWithFlip &sdf,
    GXVec3 const &offset,
    bool aTest,
    bool bTest,
    eAxis axis,
    GXVec3 const &rayOrigin,
    GXVec3 const &rayDirection,
    float pixelSize,
    bool lmbPressed
) noexcept
{
    GXVec3 alpha {};
    _rotation.TransformFast ( alpha, offset );

    GXVec3 cases[ 2UZ ];
    GXVec3 &realP = cases[ 0UZ ];
    realP.Sum ( _location, alpha );

    GXQuat const &tr = sdf.GetRotationWorld ();
    GXMat3 m {};
    m.FromFast ( tr );

    cases[ 1UZ ].Sum ( realP, PLANE_FLIP_OFFSET, m.Up () );
    realP = cases[ static_cast<size_t> ( aTest ) ];

    cases[ 1UZ ].Sum ( realP, PLANE_FLIP_OFFSET, m.Forward () );
    realP = cases[ static_cast<size_t> ( bTest ) ];

    alpha.Multiply ( PLANE_SCALE, pixelSize );
    GizmoBoxCollider const box ( tr, alpha );

    alpha.Subtract ( realP, _location );

    GXVec3 beta {};
    beta.Sum ( _location, pixelSize, alpha );
    float const d = box.Raycast ( rayOrigin, rayDirection, beta );

    if ( d >= closest._distance )
        return;

    closest._distance = d;
    closest._control = &sdf;

    if ( !lmbPressed )
        return;

    _workAxis = eAxis::None;
    GXVec3 const &right = m.Right ();
    auto const point = ResolvePlaneIntersection ( _location, right, rayOrigin, rayDirection );

    if ( !point )
        return;

    _workPlane = axis;
    _workDirection = right;
    _initialNegativePlaneOffset.Subtract ( _location, *point );
    _initialState = _location;
}

void MoveTool::SetupAxis ( SDFLineSegment &axis, SDFCone &cone, eSDFPalette color ) noexcept
{
    axis.SetColor ( color );
    axis.SetScale ( GXVec3 ( axis.GetScale ()._data[ 0UZ ], AXIS_SDF_STANDBY_SIZE, AXIS_SDF_STANDBY_SIZE ) );

    constexpr GXVec3 coneSize ( CONE_STANDBY_LENGTH, CONE_STANDBY_SIZE, CONE_STANDBY_SIZE );
    cone.SetScale ( coneSize );
    cone.SetColor ( color );
}

} // namespace editor
