#include <shape_box.h>

namespace android_vulkan {

ShapeBox::ShapeBox ( GXVec3 const &size ) noexcept:
    Shape ( eShapeType::Box ),
    _localGeometry {},
    _size ( size )
{
    Init ();
}

ShapeBox::ShapeBox ( float width, float height, float depth ) noexcept:
    Shape ( eShapeType::Box ),
    _localGeometry {},
    _size ( width, height, depth )
{
    Init ();
}

[[maybe_unused]] void ShapeBox::CalculateInertiaTensor ( float mass ) noexcept
{
    // https://en.wikipedia.org/wiki/List_of_moments_of_inertia

    GXVec3 square {};
    square.Multiply ( _size, _size );

    GXVec3 diagonal ( square._data[ 1U ], square._data[ 0U ], square._data[ 0U ] );
    diagonal.Sum ( diagonal, GXVec3 ( square._data[ 2U ], square._data[ 2U ], square._data[ 1U ] ) );
    diagonal.Multiply ( diagonal, 8.3333e-2F * mass );

    _inertiaTensorInverse._m[ 0U ][ 0U ] = 1.0F / diagonal._data[ 0U ];
    _inertiaTensorInverse._m[ 1U ][ 1U ] = 1.0F / diagonal._data[ 1U ];
    _inertiaTensorInverse._m[ 2U ][ 2U ] = 1.0F / diagonal._data[ 2U ];

    _inertiaTensorInverse._m[ 0U ][ 1U ] = 0.0F;
    _inertiaTensorInverse._m[ 0U ][ 2U ] = 0.0F;
    _inertiaTensorInverse._m[ 1U ][ 0U ] = 0.0F;
    _inertiaTensorInverse._m[ 1U ][ 2U ] = 0.0F;
    _inertiaTensorInverse._m[ 2U ][ 0U ] = 0.0F;
    _inertiaTensorInverse._m[ 2U ][ 1U ] = 0.0F;
}

[[maybe_unused]] GXVec3 ShapeBox::GetExtremePointWorld ( GXVec3 const &direction ) const noexcept
{
    float projection = -FLT_MAX;
    GXVec3 alpha {};
    GXVec3 result {};

    for ( auto const& v : _localGeometry )
    {
        _transformWorld.MultiplyAsPoint ( alpha, v );
        float const dot = direction.DotProduct ( alpha );

        if ( dot <= projection )
            continue;

        result = alpha;
        projection = dot;
    }

    return result;
}

[[maybe_unused]] float ShapeBox::GetWidth () const noexcept
{
    return _size._data[ 0U ];
}

[[maybe_unused]] float ShapeBox::GetHeight () const noexcept
{
    return _size._data[ 1U ];
}

[[maybe_unused]] float ShapeBox::GetDepth () const noexcept
{
    return _size._data[ 2U ];
}

void ShapeBox::Init () noexcept
{
    GXVec3 half {};
    half.Multiply ( _size, 0.5F );

    _boundsLocal.AddVertex ( half );

    GXVec3 negHalf ( half );
    negHalf.Reverse ();
    _boundsLocal.AddVertex ( negHalf );

    _boundsWorld = _boundsLocal;

    _localGeometry[ 0U ] = negHalf;
    _localGeometry[ 1U ] = GXVec3 ( half._data[ 0U ], negHalf._data[ 1U ], negHalf._data[ 2U ] );
    _localGeometry[ 2U ] = GXVec3 ( half._data[ 0U ], half._data[ 1U ], negHalf._data[ 2U ] );
    _localGeometry[ 3U ] = GXVec3 ( negHalf._data[ 0U ], half._data[ 1U ], negHalf._data[ 2U ] );

    _localGeometry[ 4U ] = GXVec3 ( negHalf._data[ 0U ], negHalf._data[ 1U ], half._data[ 2U ] );
    _localGeometry[ 5U ] = GXVec3 ( half._data[ 0U ], negHalf._data[ 1U ], half._data[ 2U ] );
    _localGeometry[ 6U ] = half;
    _localGeometry[ 7U ] = GXVec3 ( negHalf._data[ 0U ], half._data[ 1U ], half._data[ 2U ] );
}

void ShapeBox::UpdateBounds () noexcept
{
    _boundsWorld.Transform ( _boundsLocal, _transformWorld );
}

} // namespace android_vulkan
