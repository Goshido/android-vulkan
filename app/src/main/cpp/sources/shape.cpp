#include <shape.h>


namespace android_vulkan {

constexpr static float const DEFAULT_FRICTION_DYNAMIC = 1.0F;
constexpr static float const DEFAULT_FRICTION_STATIC = 1.0F;
constexpr static float const DEFAULT_RESTITUTION = 0.25F;

constexpr static float const DEFAULT_SIZE = 1.0F;
constexpr static float const DEFAULT_HALF_SIZE = 0.5F * DEFAULT_SIZE;
constexpr static GXVec3 const DEFAULT_BOUNDS_MIN ( -DEFAULT_HALF_SIZE, -DEFAULT_HALF_SIZE, -DEFAULT_HALF_SIZE );
constexpr static GXVec3 const DEFAULT_BOUNDS_MAX ( DEFAULT_HALF_SIZE, DEFAULT_HALF_SIZE, DEFAULT_HALF_SIZE );

constexpr static GXVec3 const DEFAULT_ORIGIN_WORLD ( 0.0F, -777.777F, 0.0F );

GXMat3 const& Shape::GetInertiaTensor () const noexcept
{
    return _inertiaTensor;
}

[[maybe_unused]] GXAABB const& Shape::GetBoundsLocal () const noexcept
{
    return _bouldsLocal;
}

[[maybe_unused]] GXAABB const& Shape::GetBoundsWorld () const noexcept
{
    return _boundsWorld;
}

[[maybe_unused]] float Shape::GetFrictionDynamic () const noexcept
{
    return _frictionDynamic;
}

[[maybe_unused]] void Shape::SetFrictionDynamic ( float friction ) noexcept
{
    _frictionDynamic = friction;
}

[[maybe_unused]] float Shape::GetFrictionStatic () const noexcept
{
    return _frictionStatic;
}

[[maybe_unused]] void Shape::SetFrictionStatic ( float friction ) noexcept
{
    _frictionStatic = friction;
}

[[maybe_unused]] float Shape::GetRestitution () const noexcept
{
    return _restitution;
}

[[maybe_unused]] void Shape::SetRestitution ( float restitution ) noexcept
{
    _restitution = restitution;
}

[[maybe_unused]] GXMat4 const& Shape::GetTransformWorld () const noexcept
{
    return _transformWorld;
}

[[maybe_unused]] eShapeType Shape::GetType () const noexcept
{
    return _type;
}

[[maybe_unused]] void Shape::UpdateCacheData ( GXMat4 const &transform ) noexcept
{
    _transformWorld.Multiply ( _transformRigidBody, transform );
    UpdateBounds ();
}

Shape::Shape ( eShapeType shapeType ) noexcept:
    _type ( shapeType ),
    _bouldsLocal {},
    _boundsWorld {},
    _frictionDynamic ( DEFAULT_FRICTION_DYNAMIC ),
    _frictionStatic ( DEFAULT_FRICTION_STATIC ),
    _inertiaTensor {},
    _restitution ( DEFAULT_RESTITUTION ),
    _transformRigidBody {},
    _transformWorld {}
{
    _bouldsLocal.AddVertex ( DEFAULT_BOUNDS_MIN );
    _bouldsLocal.AddVertex ( DEFAULT_BOUNDS_MAX );

    _transformRigidBody.Translation ( DEFAULT_ORIGIN_WORLD._data[ 0U ], DEFAULT_ORIGIN_WORLD._data[ 1U ],
        DEFAULT_ORIGIN_WORLD._data[ 2U ]
    );

    _transformWorld = _transformRigidBody;
    _bouldsLocal.Transform ( _boundsWorld, _transformWorld );

    _inertiaTensor.Identity ();
}

} // namespace android_vulkan
