#include <shape_box.h>

namespace android_vulkan {

ShapeBox::ShapeBox ( float width, float height, float depth ) noexcept:
    Shape ( eShapeType::Box ),
    _width ( width ),
    _height ( height ),
    _depth ( depth )
{
    GXVec3 half ( width, height, depth );
    half.Multiply ( half, 0.5F );

    _bouldsLocal.AddVertex ( half );

    half.Reverse ();
    _bouldsLocal.AddVertex ( half );

    _boundsWorld = _bouldsLocal;
}

[[maybe_unused]] void ShapeBox::CalculateInertiaTensor ( float mass ) noexcept
{
    // https://en.wikipedia.org/wiki/List_of_moments_of_inertia

    GXVec3 square ( _width, _height, _depth );
    square.Multiply ( square, square );

    GXVec3 diagonal ( square._data[ 1U ], square._data[ 0U ], square._data[ 0U ] );
    diagonal.Sum ( diagonal, GXVec3 ( square._data[ 2U ], square._data[ 2U ], square._data[ 1U ] ) );
    diagonal.Multiply ( diagonal, 8.3333e-2F * mass );

    _inertiaTensor._m[ 0U ][ 0U ] = diagonal._data[ 0U ];
    _inertiaTensor._m[ 1U ][ 1U ] = diagonal._data[ 1U ];
    _inertiaTensor._m[ 2U ][ 2U ] = diagonal._data[ 2U ];

    _inertiaTensor._m[ 0U ][ 1U ] = 0.0F;
    _inertiaTensor._m[ 0U ][ 2U ] = 0.0F;
    _inertiaTensor._m[ 1U ][ 0U ] = 0.0F;
    _inertiaTensor._m[ 1U ][ 2U ] = 0.0F;
    _inertiaTensor._m[ 2U ][ 0U ] = 0.0F;
    _inertiaTensor._m[ 2U ][ 1U ] = 0.0F;
}

[[maybe_unused]] GXVec3 ShapeBox::GetExtremePointWorld ( GXVec3 const &/*direction*/ ) const noexcept
{
    // TODO
    return {};
}

[[maybe_unused]] float ShapeBox::GetWidth () const noexcept
{
    return _width;
}

[[maybe_unused]] float ShapeBox::GetHeight () const noexcept
{
    return _height;
}

[[maybe_unused]] float ShapeBox::GetDepth () const noexcept
{
    return _depth;
}

void ShapeBox::UpdateBounds () noexcept
{
    _boundsWorld.Transform ( _bouldsLocal, _transformWorld );
}

} // namespace android_vulkan
