#include <precompiled_headers.hpp>
#include <gizmo_box_collider.hpp>


namespace editor {

namespace {

constexpr float MISS = std::numeric_limits<float>::max ();

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

GizmoBoxCollider::GizmoBoxCollider ( GXQuat const &orientation, GXVec3 const &size ) noexcept:
    _size ( size )
{
    _toBox.InverseFast ( orientation );
}

float GizmoBoxCollider::Raycast ( GXVec3 const &rayOrigin,
    GXVec3 const &rayDirection,
    GXVec3 const &boxCenter
) const noexcept
{
    GXVec3 bMax {};
    // Trick: the order was changed to get reversed|negative vector.
    bMax.Subtract ( boxCenter, rayOrigin );

    GXVec3 alpha {};
    _toBox.TransformFast ( alpha, bMax );

    bMax.Sum ( alpha, 0.5F, _size );

    GXVec3 bMin {};
    bMin.Subtract ( bMax, _size );

    _toBox.Transform ( alpha, rayDirection );
    GXVec3 const invD ( 1.0F / alpha._data[ 0U ], 1.0F / alpha._data[ 1U ], 1.0F / alpha._data[ 2U ] );

    GXVec2 const casesT[] =
    {
        GXVec2 ( bMax._data[ 0U ], bMin._data[ 0U ] ),
        GXVec2 ( bMin._data[ 0U ], bMax._data[ 0U ] )
    };

    GXVec2 t = casesT[ static_cast<size_t> ( invD._data[ 0U ] >= 0.0F ) ];
    t.Multiply ( t, invD._data[ 0U ] );

    GXVec2 const casesI[] =
    {
        GXVec2 ( bMax._data[ 1U ], bMin._data[ 1U ] ),
        GXVec2 ( bMin._data[ 1U ], bMax._data[ 1U ] )
    };

    GXVec2 i = casesI[ static_cast<size_t> ( invD._data[ 1U ] >= 0.0F ) ];
    i.Multiply ( i, invD._data[ 1U ] );

    if ( ( t._data[ 0U ] > i._data[ 1U ] ) | ( i._data[ 0U ] > t._data[ 1U ] ) )
        return MISS;

    float const casesX[] = { t._data[ 0U ], i._data[ 0U ] };
    t._data[ 0U ] = casesX[ static_cast<size_t> ( i._data[ 0U ] > t._data[ 0U ] ) ];

    float const casesY[] = { t._data[ 1U ], i._data[ 1U ] };
    t._data[ 1U ] = casesY[ static_cast<size_t> ( i._data[ 1U ] > t._data[ 1U ] ) ];

    GXVec2 const casesZ[] =
    {
        GXVec2 ( bMax._data[ 2U ], bMin._data[ 2U ] ),
        GXVec2 ( bMin._data[ 2U ], bMax._data[ 2U ] )
    };

    i = casesZ[ static_cast<size_t> ( invD._data[ 2U ] >= 0.0F ) ];
    i.Multiply ( i, invD._data[ 2U ] );

    if ( ( t._data[ 0U ] > i._data[ 1U ] ) | ( i._data[ 0U ] > t._data[ 1U ] ) )
        return MISS;

    float const casesResult[] = { t._data[ 0U ], i._data[ 0U ] };
    return casesResult[ static_cast<size_t> ( i._data[ 0U ] > t._data[ 0U ] ) ];
}

} // namespace editor
