#include <precompiled_headers.hpp>
#include <move_tool.hpp>

// FUCK - remove
#include <logger.hpp>


namespace editor {

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

    Update ();
    Activate ();
}

void MoveTool::Activate () noexcept
{
    GXQuat const rotation = GXQuat::IDENTITY;
    GXVec3 const location = GXVec3::ZERO;
    _origin.Show ( location, rotation );
    _xLine.Show ( location, rotation );
    _xPlane.Show ( location, rotation );
    _xCone.Show ( location, rotation );
    _yLine.Show ( location, rotation );
    _yPlane.Show ( location, rotation );
    _yCone.Show ( location, rotation );
    _zLine.Show ( location, rotation );
    _zPlane.Show ( location, rotation );
    _zCone.Show ( location, rotation );
    _xPlaneY.Show ( location, rotation );
    _xPlaneZ.Show ( location, rotation );
    _yPlaneZ.Show ( location, rotation );
    _yPlaneX.Show ( location, rotation );
    _zPlaneX.Show ( location, rotation );
    _zPlaneY.Show ( location, rotation );
    android_vulkan::LogInfo ( ">>> Move tool activated" );
}

void MoveTool::Deactivate () noexcept
{
    // FUCK
    android_vulkan::LogInfo ( "<<< Move tool deactivated" );
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

void MoveTool::Update () noexcept
{
    // FUCK
    GXQuat const rotation = GXQuat::IDENTITY;
    GXVec3 const location = GXVec3::ZERO;

    _origin.OnParentUpdated ( location, rotation );
    _xLine.OnParentUpdated ( location, rotation );
    _xPlane.OnParentUpdated ( location, rotation );
    _xCone.OnParentUpdated ( location, rotation );
    _yLine.OnParentUpdated ( location, rotation );
    _yPlane.OnParentUpdated ( location, rotation );
    _yCone.OnParentUpdated ( location, rotation );
    _zLine.OnParentUpdated ( location, rotation );
    _zPlane.OnParentUpdated ( location, rotation );
    _zCone.OnParentUpdated ( location, rotation );
    _xPlaneY.OnParentUpdated ( location, rotation );
    _xPlaneZ.OnParentUpdated ( location, rotation );
    _yPlaneZ.OnParentUpdated ( location, rotation );
    _yPlaneX.OnParentUpdated ( location, rotation );
    _zPlaneX.OnParentUpdated ( location, rotation );
    _zPlaneY.OnParentUpdated ( location, rotation );
}

} // namespace editor
