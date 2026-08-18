#include <precompiled_headers.hpp>
#include <rotate_tool.hpp>

// FUCK - remove
#include <logger.hpp>


namespace editor {

void RotateTool::Activate () noexcept
{
    GXQuat const rotation = GXQuat::IDENTITY;
    GXVec3 const location ( 0.0F, 0.0F, 12.0F );
    _x.Show ( location, rotation );
    _y.Show ( location, rotation );
    _z.Show ( location, rotation );
    _ring.Show ( location, rotation );
    android_vulkan::LogInfo ( ">>> Rotate tool activated" );
}

void RotateTool::Deactivate () noexcept
{
    // FUCK
    android_vulkan::LogInfo ( "<<< Rotate tool deactivated" );
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
    GXQuat const rotation = GXQuat::IDENTITY;
    GXVec3 const location ( 0.0F, 0.0F, 12.0F );
    _x.OnParentUpdated ( location, rotation );
    _y.OnParentUpdated ( location, rotation );
    _z.OnParentUpdated ( location, rotation );
    _ring.OnParentUpdated ( location, rotation );
}

} // namespace editor
