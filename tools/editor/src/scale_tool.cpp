#include <precompiled_headers.hpp>
#include <scale_tool.hpp>

// FUCK - remove
#include <logger.hpp>


namespace editor {

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
    GXQuat const rotation = GXQuat::IDENTITY;
    GXVec3 const location ( 6.0F, 0.0F, 12.0F );
    _origin.Show ( location, rotation );
    _xLine.Show ( location, rotation );
    _xPlane.Show ( location, rotation );
    _xCube.Show ( location, rotation );
    _yLine.Show ( location, rotation );
    _yPlane.Show ( location, rotation );
    _yCube.Show ( location, rotation );
    _zLine.Show ( location, rotation );
    _zPlane.Show ( location, rotation );
    _zCube.Show ( location, rotation );
    _xPlaneY.Show ( location, rotation );
    _xPlaneZ.Show ( location, rotation );
    _yPlaneZ.Show ( location, rotation );
    _yPlaneX.Show ( location, rotation );
    _zPlaneX.Show ( location, rotation );
    _zPlaneY.Show ( location, rotation );
    android_vulkan::LogInfo ( ">>> Scale tool activated" );
}

void ScaleTool::Deactivate () noexcept
{
    // FUCK
    android_vulkan::LogInfo ( "<<< Scale tool deactivated" );
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

void ScaleTool::Update () noexcept
{
    // FUCK
    GXQuat const rotation = GXQuat::IDENTITY;
    GXVec3 const location ( 6.0F, 0.0F, 12.0F );
    _origin.Show ( location, rotation );
    _xLine.Show ( location, rotation );
    _xPlane.Show ( location, rotation );
    _xCube.Show ( location, rotation );
    _yLine.Show ( location, rotation );
    _yPlane.Show ( location, rotation );
    _yCube.Show ( location, rotation );
    _zLine.Show ( location, rotation );
    _zPlane.Show ( location, rotation );
    _zCube.Show ( location, rotation );
    _xPlaneY.Show ( location, rotation );
    _xPlaneZ.Show ( location, rotation );
    _yPlaneZ.Show ( location, rotation );
    _yPlaneX.Show ( location, rotation );
    _zPlaneX.Show ( location, rotation );
    _zPlaneY.Show ( location, rotation );
}

} // namespace editor
