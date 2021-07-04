#include <shape.h>


namespace android_vulkan {

constexpr static float const DEFAULT_FRICTION_DYNAMIC = 1.0F;
constexpr static float const DEFAULT_FRICTION_STATIC = 1.0F;
constexpr static float const DEFAULT_RESTITUTION = 0.25F;

constexpr static float const DEFAULT_SIZE = 1.0F;
constexpr static float const DEFAULT_HALF_SIZE = 0.5F * DEFAULT_SIZE;
constexpr static GXVec3 const DEFAULT_BOUNDS_MIN ( -DEFAULT_HALF_SIZE, -DEFAULT_HALF_SIZE, -DEFAULT_HALF_SIZE );
constexpr static GXVec3 const DEFAULT_BOUNDS_MAX ( DEFAULT_HALF_SIZE, DEFAULT_HALF_SIZE, DEFAULT_HALF_SIZE );

GXMat3 const& Shape::GetInertiaTensorInverse () const noexcept
{
    return _inertiaTensorInverse;
}

[[maybe_unused]] GXAABB const& Shape::GetBoundsLocal () const noexcept
{
    return _boundsLocal;
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

void Shape::UpdateCacheData ( GXMat4 const &transform ) noexcept
{
    _transformWorld.Multiply ( _transformRigidBody, transform );
    UpdateBounds ();
}

GXVec3 Shape::FindSupportPoint ( GXVec3 const &direction, Shape const &shapeA, Shape const &shapeB ) noexcept
{
    GXVec3 opposite ( direction );
    opposite.Reverse ();

    GXVec3 result {};
    result.Subtract ( shapeA.GetExtremePointWorld ( direction ), shapeB.GetExtremePointWorld ( opposite ) );

    return result;
}

Shape::Shape ( eShapeType shapeType ) noexcept:
    _type ( shapeType ),
    _boundsLocal {},
    _boundsWorld {},
    _frictionDynamic ( DEFAULT_FRICTION_DYNAMIC ),
    _frictionStatic ( DEFAULT_FRICTION_STATIC ),
    _inertiaTensorInverse {},
    _restitution ( DEFAULT_RESTITUTION ),
    _transformRigidBody {},
    _transformWorld {}
{
    _boundsLocal.AddVertex ( DEFAULT_BOUNDS_MIN );
    _boundsLocal.AddVertex ( DEFAULT_BOUNDS_MAX );

    _transformRigidBody.Identity ();
    _transformWorld = _transformRigidBody;
    _boundsLocal.Transform ( _boundsWorld, _transformWorld );

    _inertiaTensorInverse.Identity ();
}

} // namespace android_vulkan
