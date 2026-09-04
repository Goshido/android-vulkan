#include <precompiled_headers.hpp>
#include <gizmo_box_collider.hpp>
#include <gizmo_cylinder_collider.hpp>
#include <scale_tool.hpp>
#include <sdf_size.hpp>

// FUCK - remove
#include <actor.hpp>
#include <logger.hpp>


namespace editor {

// FUCK
extern Actor* fuck_actor;

namespace {

constexpr float             AXIS_SDF_ACTIVE_SIZE = 0.08F;
constexpr float             AXIS_SDF_STANDBY_SIZE = 0.02F;
constexpr float             AXIS_SDF_FULL_LENGTH = 6.08F;
constexpr float             AXIS_SDF_OFFSET = 0.48F;

constexpr float             AXIS_CLIP_OFFSET = 2.4F;
constexpr float             AXIS_FULL_LENGTH = 8.8F;
constexpr float             AXIS_RADIUS = 0.5F;

constexpr float             BOX_ACTIVE_SIZE = 0.5F;
constexpr float             BOX_STANDBY_SIZE = 0.45F;

constexpr float             UNIFORM_SCALE_FACTOR = 0.01F;
constexpr float             GIZMO_RADIUS = 8.3F;
constexpr float             SCALE_LINE_OFFSET_Y = -3.5F;

constexpr eSDFPalette       ORIGIN_COLOR = eSDFPalette::White;


} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ScaleTool::ScaleTool () noexcept
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

void ScaleTool::Activate () noexcept
{
    _origin.Show ( _location, _rotation );
    _xLine.Show ( _location, _rotation );
    _xPlane.Show ( _location, _rotation );
    _xBox.Show ( _location, _rotation );
    _yLine.Show ( _location, _rotation );
    _yPlane.Show ( _location, _rotation );
    _yBox.Show ( _location, _rotation );
    _zLine.Show ( _location, _rotation );
    _zPlane.Show ( _location, _rotation );
    _zBox.Show ( _location, _rotation );
    _xPlaneY.Show ( _location, _rotation );
    _xPlaneZ.Show ( _location, _rotation );
    _yPlaneZ.Show ( _location, _rotation );
    _yPlaneX.Show ( _location, _rotation );
    _zPlaneX.Show ( _location, _rotation );
    _zPlaneY.Show ( _location, _rotation );
    android_vulkan::LogInfo ( ">>> Scale tool activated" );
}

void ScaleTool::Deactivate () noexcept
{
    _origin.Hide ();
    _xLine.Hide ();
    _xPlane.Hide ();
    _xBox.Hide ();
    _yLine.Hide ();
    _yPlane.Hide ();
    _yBox.Hide ();
    _zLine.Hide ();
    _zPlane.Hide ();
    _zBox.Hide ();
    _xPlaneY.Hide ();
    _xPlaneZ.Hide ();
    _yPlaneZ.Hide ();
    _yPlaneX.Hide ();
    _zPlaneX.Hide ();
    _zPlaneY.Hide ();
    android_vulkan::LogInfo ( "<<< Scale tool deactivated" );
}

void ScaleTool::Hover () noexcept
{
    // FUCK
}

void ScaleTool::Click () noexcept
{
    // FUCK
}

void ScaleTool::Begin () noexcept
{
    // FUCK
}

void ScaleTool::Move () noexcept
{
    // FUCK
}

void ScaleTool::End () noexcept
{
    // FUCK
}

void ScaleTool::Cancel () noexcept
{
    // FUCK
}

void ScaleTool::Update ( GXVec3 const &rayDirection,
    GXVec3 const &cameraLocation,
    GXMat3 const &cameraBasis,
    GXVec3 const &vi,
    int32_t mouseY,
    bool leftMouseButtonPressed
) noexcept
{
    bool const prevMoving = ( _workAxis != eAxis::None ) | ( _workPlane != eAxis::None ) | _scaleAll;
    bool const lmbPressed = leftMouseButtonPressed & !_lastLMBPressed;
    bool const lmbReleased = !leftMouseButtonPressed & std::exchange ( _lastLMBPressed, leftMouseButtonPressed );

    eAxis cases[] = { _workAxis, eAxis::None };
    _workAxis = cases[ static_cast<size_t> ( lmbReleased ) ];

    if ( _workAxis != eAxis::None )
    {
        HandleAxisScale ( cameraLocation, rayDirection );
        return;
    }

    cases[ 0UZ ] = _workPlane;
    _workPlane = cases[ static_cast<size_t> ( lmbReleased ) ];

    if ( _workPlane != eAxis::None )
    {
        HandlePlaneScale ( cameraLocation, rayDirection );
        return;
    }

    _scaleAll &= !lmbReleased;

    if ( _scaleAll )
    {
        HandleScaleAll ( mouseY );
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

    float axisRadius = AXIS_RADIUS * pixelSize;
    Closest closest {};

    AxisCheck ( closest,
        _xLine,
        _xBox,
        xTest,
        eAxis::X,
        GXVec3::RIGHT,
        axisRadius,
        cameraLocation,
        rayDirection,
        pixelSize,
        lmbPressed
    );

    AxisCheck ( closest,
        _yLine,
        _yBox,
        yTest,
        eAxis::Y,
        GXVec3::UP,
        axisRadius,
        cameraLocation,
        rayDirection,
        pixelSize,
        lmbPressed
    );

    AxisCheck ( closest,
        _zLine,
        _zBox,
        zTest,
        eAxis::Z,
        GXVec3::FORWARD,
        axisRadius,
        cameraLocation,
        rayDirection,
        pixelSize,
        lmbPressed
    );

    constexpr GXVec3 xOffset ( PLANE_OFFSET._data[ 0UZ ], PLANE_OFFSET._data[ 1UZ ], PLANE_OFFSET._data[ 1UZ ] );

    PlaneCheck ( closest,
        _xPlane,
        xOffset,
        yTest,
        zTest,
        eAxis::X,
        GXVec3::UP,
        GXVec3::FORWARD,
        cameraLocation,
        rayDirection,
        pixelSize,
        lmbPressed
    );

    constexpr GXVec3 yOffset ( PLANE_OFFSET._data[ 1UZ ], PLANE_OFFSET._data[ 0UZ ], PLANE_OFFSET._data[ 1UZ ] );

    PlaneCheck ( closest,
        _yPlane,
        yOffset,
        zTest,
        xTest,
        eAxis::Y,
        GXVec3::FORWARD,
        GXVec3::RIGHT,
        cameraLocation,
        rayDirection,
        pixelSize,
        lmbPressed
    );

    constexpr GXVec3 zOffset ( PLANE_OFFSET._data[ 1UZ ], PLANE_OFFSET._data[ 1UZ ], PLANE_OFFSET._data[ 0UZ ] );

    PlaneCheck ( closest,
        _zPlane,
        zOffset,
        xTest,
        yTest,
        eAxis::Z,
        GXVec3::RIGHT,
        GXVec3::UP,
        cameraLocation,
        rayDirection,
        pixelSize,
        lmbPressed
    );

    AllAxesCheck ( closest, rayDirection, cameraLocation, vi, mouseY, lmbPressed );

    if ( LockAxis () || LockPlane () || LockAllAxes ( cameraBasis ) )
        return;

    if ( !closest._control )
    {
        DeactivateSDF ();
        return;
    }

    ActivateSDF ( *closest._control, closest._cap );
}

std::optional<ScaleTool::ColorSet> ScaleTool::AcquirePlaneColorSet ( SDF &plane ) noexcept
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

void ScaleTool::ActivateSDF ( SDF &sdf, SDF* cap ) noexcept
{
    if ( &sdf == _control )
        return;

    DeactivateSDF ();
    _control = &sdf;
    _cap = cap;

    constexpr GXVec3 cubeSize ( BOX_ACTIVE_SIZE, BOX_ACTIVE_SIZE, BOX_ACTIVE_SIZE );

    if ( &sdf == &_origin )
    {
        sdf.SetScale ( cubeSize );
        return;
    }

    if ( auto const colorSet = AcquirePlaneColorSet ( sdf ); colorSet )
    {
        sdf.SetColor ( colorSet->_active );
        return;
    }

    constexpr GXVec3 axisSize ( AXIS_SDF_FULL_LENGTH - AXIS_SDF_OFFSET,
        AXIS_SDF_ACTIVE_SIZE,
        AXIS_SDF_ACTIVE_SIZE
    );

    sdf.SetScale ( axisSize );
    cap->SetScale ( cubeSize );
}

void ScaleTool::DeactivateSDF () noexcept
{
    if ( !_control )
        return;

    SDF* c = std::exchange ( _control, nullptr );
    constexpr GXVec3 cubeSize ( BOX_STANDBY_SIZE, BOX_STANDBY_SIZE, BOX_STANDBY_SIZE );

    if ( c == &_origin )
    {
        c->SetScale ( cubeSize );
        return;
    }

    if ( auto const colorSet = AcquirePlaneColorSet ( *c ); colorSet )
    {
        c->SetColor ( colorSet->_standby );
        return;
    }

    constexpr GXVec3 axisSize ( AXIS_SDF_FULL_LENGTH - AXIS_SDF_OFFSET,
        AXIS_SDF_STANDBY_SIZE,
        AXIS_SDF_STANDBY_SIZE
    );

    c->SetScale ( axisSize );
    _cap->SetScale ( cubeSize );
}

void ScaleTool::HandleAxisScale ( GXVec3 const &rayOrigin, GXVec3 const &rayDirection ) noexcept
{
    // See <repo>/docs/gizmo-rendering.md#inter-scale-axis
    auto const distance = ResolveAxisScalarDistance ( _controlLocation, _workDirection, rayOrigin, rayDirection );

    if ( distance )
    {
        _target.Sum ( _initialState, *distance + _initialNegativeScalarDistance, _localAxisA );
        fuck_actor->SetScale ( _target );
    }
}

void ScaleTool::HandlePlaneScale ( GXVec3 const &rayOrigin, GXVec3 const &rayDirection ) noexcept
{
    // See <repo>/docs/gizmo-rendering.md#inter-scale-plane
    auto const point = ResolvePlaneIntersection ( _controlLocation, _workDirection, rayOrigin, rayDirection );

    if ( !point )
        return;

    GXVec3 d {};
    d.Subtract ( _controlLocation, *point );
    _target.Sum ( _initialState, d.DotProduct ( _globalAxisA ), _localAxisA );
    _target.Sum ( _target, d.DotProduct ( _globalAxisB ), _localAxisB );
    fuck_actor->SetScale ( _target );
}

void ScaleTool::HandleScaleAll ( int32_t mouseY ) noexcept
{
    // See <repo>/docs/gizmo-rendering.md#inter-uniform-scale
    _target.Multiply ( _initialState,
        std::exp ( UNIFORM_SCALE_FACTOR * ( _controlLocation._data[ 1UZ ] - static_cast<float> ( mouseY ) ) )
    );

    fuck_actor->SetScale ( _target );
}

void ScaleTool::ResetVisuals () noexcept
{
    auto const reset = [ this ] ( SDFLineSegment &axis,
        SDFLineSegmentWithFlip &planeA,
        SDFLineSegmentWithFlip &planeB,
        SDFBoxWithFlip &plane,
        SDFBox &cube,
        size_t axisIdx,
        eSDFPalette color,
        eSDFPalette planeColor
    ) noexcept {
        constexpr GXVec3 axisScale ( AXIS_SDF_FULL_LENGTH - AXIS_SDF_OFFSET,
            AXIS_SDF_STANDBY_SIZE,
            AXIS_SDF_STANDBY_SIZE
        );

        GXVec3 p = GXVec3::ZERO;
        p._data[ axisIdx ] = AXIS_SDF_OFFSET;
        axis.SetLocationAndScale ( p, axisScale );
        axis.SetColor ( color );
        axis.OnParentUpdated ( _location, _rotation );

        planeA.SetColor ( color );
        planeB.SetColor ( color );
        plane.SetColor ( planeColor );

        constexpr GXVec3 cubeScale ( BOX_STANDBY_SIZE, BOX_STANDBY_SIZE, BOX_STANDBY_SIZE );
        cube.SetScale ( cubeScale );
        cube.SetColor ( color );
        cube.OnParentUpdated ( _location, _rotation );
    };

    reset ( _xLine, _xPlaneY, _xPlaneZ, _xPlane, _xBox, 0UZ, X_COLOR, X_PLANE_STANDBY_COLOR );
    reset ( _yLine, _yPlaneZ, _yPlaneX, _yPlane, _yBox, 1UZ, Y_COLOR, Y_PLANE_STANDBY_COLOR );
    reset ( _zLine, _zPlaneX, _zPlaneY, _zPlane, _zBox, 2UZ, Z_COLOR, Z_PLANE_STANDBY_COLOR );

    _origin.SetColor ( ORIGIN_COLOR );

    _origin.Show ( _location, _rotation );
    _xPlaneY.Show ( _location, _rotation );
    _xPlaneZ.Show ( _location, _rotation );
    _xPlane.Show ( _location, _rotation );
    _yPlaneZ.Show ( _location, _rotation );
    _yPlaneX.Show ( _location, _rotation );
    _yPlane.Show ( _location, _rotation );
    _zPlaneX.Show ( _location, _rotation );
    _zPlaneY.Show ( _location, _rotation );
    _zPlane.Show ( _location, _rotation );

    _scaleLine.Hide ();
    _scaleDirectionA.Hide ();
    _scaleDirectionB.Hide ();
}

bool ScaleTool::LockPlane () noexcept
{
    auto const setup = [ this ] ( SDFBoxWithFlip &plane,
        SDFLineSegmentWithFlip &lineA,
        SDFLineSegmentWithFlip &lineB,
        SDFLineSegment &axisA,
        SDFLineSegment &axisB
    ) noexcept {
        plane.SetColor ( WORK_TRANSPARENT_COLOR );
        lineA.SetColor ( WORK_OPAQUE_COLOR );
        lineB.SetColor ( WORK_OPAQUE_COLOR );

        constexpr GXVec3 s ( AXIS_SDF_FULL_LENGTH, AXIS_SDF_STANDBY_SIZE, AXIS_SDF_STANDBY_SIZE );
        axisA.SetLocationAndScale ( GXVec3::ZERO, s );
        axisA.OnParentUpdated ( _location, _rotation );

        axisB.SetLocationAndScale ( GXVec3::ZERO, s );
        axisB.OnParentUpdated ( _location, _rotation );
    };

    switch ( _workPlane )
    {
        case eAxis::X:
            setup ( _xPlane, _xPlaneY, _xPlaneZ, _yLine, _zLine );
            HidePlane( _yPlane, _yPlaneZ, _yPlaneX );
            HidePlane( _zPlane, _zPlaneX, _zPlaneY );
            SetupAxis( _xLine, _xBox, WORK_INACTIVE_COLOR );
            SetupAxis( _yLine, _yBox, WORK_OPAQUE_COLOR );
            SetupAxis( _zLine, _zBox, WORK_OPAQUE_COLOR );
        break;

        case eAxis::Y:
            setup ( _yPlane, _yPlaneZ, _yPlaneX, _zLine, _xLine );
            HidePlane ( _zPlane, _zPlaneX, _zPlaneY );
            HidePlane ( _xPlane, _xPlaneY, _xPlaneZ );
            SetupAxis ( _xLine, _xBox, WORK_OPAQUE_COLOR );
            SetupAxis ( _yLine, _yBox, WORK_INACTIVE_COLOR );
            SetupAxis ( _zLine, _zBox, WORK_OPAQUE_COLOR );
        break;

        case eAxis::Z:
            setup ( _zPlane, _zPlaneX, _zPlaneY, _xLine, _yLine );
            HidePlane( _xPlane, _xPlaneY, _xPlaneZ );
            HidePlane( _yPlane, _yPlaneZ, _yPlaneX );
            SetupAxis( _xLine, _xBox, WORK_OPAQUE_COLOR );
            SetupAxis( _yLine, _yBox, WORK_OPAQUE_COLOR );
            SetupAxis( _zLine, _zBox, WORK_INACTIVE_COLOR );
        break;

        case eAxis::None:
            [[fallthrough]];
        default:
            // NOTHING
        return false;
    }

    _origin.Hide ();
    return true;
}

bool ScaleTool::LockAxis () noexcept
{
    switch ( _workAxis )
    {
        case eAxis::X:
            SetupAxis ( _xLine, _xBox, WORK_OPAQUE_COLOR );
            SetupAxis ( _yLine, _yBox, WORK_INACTIVE_COLOR );
            SetupAxis ( _zLine, _zBox, WORK_INACTIVE_COLOR );
        break;

        case eAxis::Y:
            SetupAxis ( _xLine, _xBox, WORK_INACTIVE_COLOR );
            SetupAxis ( _yLine, _yBox, WORK_OPAQUE_COLOR );
            SetupAxis ( _zLine, _zBox, WORK_INACTIVE_COLOR );
        break;

        case eAxis::Z:
            SetupAxis ( _xLine, _xBox, WORK_INACTIVE_COLOR );
            SetupAxis ( _yLine, _yBox, WORK_INACTIVE_COLOR );
            SetupAxis ( _zLine, _zBox, WORK_OPAQUE_COLOR );
        break;

        case eAxis::None:
            [[fallthrough]];
        default:
            // NOTHING
        return false;
    }

    _origin.Hide ();
    HidePlane ( _xPlane, _xPlaneY, _xPlaneZ );
    HidePlane ( _yPlane, _yPlaneZ, _yPlaneX );
    HidePlane ( _zPlane, _zPlaneX, _zPlaneY );
    return true;
}

bool ScaleTool::LockAllAxes ( GXMat3 const &cameraBasis ) noexcept
{
    if ( !_scaleAll )
        return false;

    _origin.SetColor ( WORK_OPAQUE_COLOR );
    _xLine.SetColor ( WORK_OPAQUE_COLOR );
    _xBox.SetColor ( WORK_OPAQUE_COLOR );
    _yLine.SetColor ( WORK_OPAQUE_COLOR );
    _yBox.SetColor ( WORK_OPAQUE_COLOR );
    _zLine.SetColor ( WORK_OPAQUE_COLOR );
    _zBox.SetColor ( WORK_OPAQUE_COLOR );

    _origin.Hide ();

    _xPlaneY.Hide ();
    _xPlaneZ.Hide ();
    _xPlane.Hide ();
    _yPlaneZ.Hide ();
    _yPlaneX.Hide ();
    _yPlane.Hide ();
    _zPlaneX.Hide ();
    _zPlaneY.Hide ();
    _zPlane.Hide ();

    GXVec3 s {};
    s.Multiply ( cameraBasis.Up (), SCALE_LINE_OFFSET_Y );

    GXVec3 pivot {};
    GXVec3 const &right = cameraBasis.Right ();
    pivot.Sum ( _location, GIZMO_RADIUS, right );

    GXVec3 anchorLocation {};
    anchorLocation.Subtract ( pivot, s );

    GXQuat anchorRotation {};
    GXVec3 const &forward = cameraBasis.Forward ();
    GXVec3 alpha ( forward );
    alpha.Reverse ();
    anchorRotation.From ( right, alpha );

    alpha.Subtract ( anchorLocation, _location );

    GXQuat toParent {};
    toParent.InverseFast ( _rotation );

    GXVec3 beta {};
    toParent.TransformFast ( beta, alpha );

    GXQuat eta {};
    eta.Multiply ( toParent, anchorRotation );
    _scaleLine.SetLocationAndRotation ( beta, eta );
    _scaleLine.Show ( _location, _rotation );

    anchorRotation.From ( right, forward );
    GXQuat omega {};
    omega.Multiply ( toParent, anchorRotation );
    _scaleDirectionB.SetLocationAndRotation ( beta, omega );
    _scaleDirectionB.Show ( _location, _rotation );

    alpha.Sum ( pivot, s );
    alpha.Subtract ( alpha, _location );
    toParent.TransformFast ( beta, alpha );
    _scaleDirectionA.SetLocationAndRotation ( beta, eta );
    _scaleDirectionA.Show ( _location, _rotation );

    return true;
}

void ScaleTool::AxisCheck ( Closest &closest,
    SDFLineSegment &sdf,
    SDFBox &sdfBox,
    bool test,
    eAxis axis,
    GXVec3 const &scaleAxis,
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
    float const offset = cases[ static_cast<size_t> ( test ) ];
    GXVec2 const s ( axisRadius, ( AXIS_FULL_LENGTH - offset ) * pixelSize );
    GizmoCylinderCollider const cylinder ( axisRadius, pixelSize * ( AXIS_FULL_LENGTH - offset ) );

    GXVec3 alpha {};
    alpha.Sum ( _location, offset * pixelSize, a );
    float const d = cylinder.Raycast ( rayOrigin, rayDirection, alpha, a );

    if ( d >= closest._distance )
        return;

    closest =
    {
        ._control = &sdf,
        ._cap = &sdfBox,
        ._distance = d
    };

    if ( !lmbPressed )
        return;

    _workPlane = eAxis::None;
    _scaleAll = false;
    auto const distance = ResolveAxisScalarDistance ( _location, a, rayOrigin, rayDirection );

    if ( !distance )
        return;

    _controlLocation = _location;
    _workDirection = a;
    _localAxisA = scaleAxis;
    _initialNegativeScalarDistance = -distance.value ();

    _workAxis = axis;
    _initialState = _target;
}

void ScaleTool::PlaneCheck ( Closest &closest,
    SDFBoxWithFlip &sdf,
    GXVec3 const &offset,
    bool aTest,
    bool bTest,
    eAxis planeAxis,
    GXVec3 const &aScaleAxis,
    GXVec3 const &bScaleAxis,
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
    GXVec3 const &up = m.Up ();
    GXVec3 const &forward = m.Forward ();

    cases[ 1UZ ].Sum ( realP, PLANE_FLIP_OFFSET, up );
    auto const aTestIdx = static_cast<size_t> ( aTest );
    realP = cases[ aTestIdx ];

    cases[ 1UZ ].Sum ( realP, PLANE_FLIP_OFFSET, forward );
    auto const bTestIdx = static_cast<size_t> ( bTest );
    realP = cases[ bTestIdx ];

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
    _scaleAll = false;

    GXVec3 const &right = m.Right ();
    auto const point = ResolvePlaneIntersection ( _location, right, rayOrigin, rayDirection );

    if ( !point )
        return;

    _controlLocation = *point;
    _workDirection = right;
    _localAxisA = aScaleAxis;
    _localAxisB = bScaleAxis;

    cases[ 0UZ ] = up;
    cases[ 1UZ ] = up;
    cases[ 1UZ ].Reverse ();
    _globalAxisA = cases[ aTestIdx ];

    cases[ 0UZ ] = forward;
    cases[ 1UZ ] = forward;
    cases[ 1UZ ].Reverse ();
    _globalAxisB = cases[ bTestIdx ];

    _workPlane = planeAxis;
    _initialState = _target;
}

void ScaleTool::AllAxesCheck ( Closest &closest,
    GXVec3 const &rayDirection,
    GXVec3 const &cameraLocation,
    GXVec3 const &vi,
    int32_t mouseY,
    bool lmbPressed
) noexcept
{
    if ( _originCollider.Raycast ( rayDirection, _location, cameraLocation, vi ) >= closest._distance )
        return;

    closest._control = &_origin;

    if ( !lmbPressed )
        return;

    _workAxis = eAxis::None;
    _workPlane = eAxis::None;
    _scaleAll = true;
    _initialState = _target;
    _controlLocation._data[ 1U ] = static_cast<float> ( mouseY );
}

void ScaleTool::SetupAxis ( SDFLineSegment &axis, SDFBox &box, eSDFPalette color ) noexcept
{
    axis.SetColor ( color );
    axis.SetScale ( GXVec3 ( axis.GetScale ()._data[ 0UZ ], AXIS_SDF_STANDBY_SIZE, AXIS_SDF_STANDBY_SIZE ) );

    constexpr GXVec3 boxSize ( BOX_STANDBY_SIZE, BOX_STANDBY_SIZE, BOX_STANDBY_SIZE );
    box.SetScale ( boxSize );
    box.SetColor ( color );
}

} // namespace editor
