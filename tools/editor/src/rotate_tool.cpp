#include <precompiled_headers.hpp>
#include <rotate_tool.hpp>

// FUCK - remove
#include <logger.hpp>


namespace editor {

namespace {

//constexpr float AXIS_ACTIVE_SIZE = 6.0e-2F;
constexpr float ORTHOGONAL_THRESHOLD = 1.0e-4F;
//
//constexpr eSDFPalette ACTIVE_COLOR = eSDFPalette::Yellow;
//constexpr eSDFPalette INACTIVE_COLOR = eSDFPalette::Grey;
//
//constexpr eSDFPalette X_COLOR = eSDFPalette::Red;
//constexpr eSDFPalette Y_COLOR = eSDFPalette::Green;
//constexpr eSDFPalette Z_COLOR = eSDFPalette::Blue;
//constexpr eSDFPalette RING_COLOR = eSDFPalette::Grey;
//
//constexpr float TANGENT_OFFSET_X = -3.5F;
//
//// FUCK - this value depends from DPI
//constexpr float BALL_SENSITIVITY = 1.0F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

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
    GXQuat const rotation = GXQuat::IDENTITY;
    GXVec3 const location ( 0.0F, 0.0F, 12.0F );
    _x.OnParentUpdated ( location, rotation );
    _y.OnParentUpdated ( location, rotation );
    _z.OnParentUpdated ( location, rotation );
    _ring.OnParentUpdated ( location, rotation );
}

RotateTool::TangentLine RotateTool::ResolveTangentLine ( GXVec3 const &ringPosition,
    GXVec3 const &ringDirection,
    GXVec3 const &rayPosition,
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
        lambda.Subtract ( ringPosition, rayPosition );

        GXVec3 g {};
        g.Sum ( rayPosition, lambda.DotProduct ( ringDirection ) / alpha, rayDirection );

        lambda.Subtract ( g, ringPosition );
        lambda.Normalize ();

        result._tangentDirection.CrossProduct ( ringDirection, lambda );
        result._tangentPosition.Sum ( ringPosition, radius, lambda );
    }

    result._distance =
        ResolveSkewLines ( rayPosition, rayDirection, result._tangentPosition, result._tangentDirection );

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
