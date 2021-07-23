#include <shape.h>


namespace android_vulkan {

constexpr static uint32_t const DEFAULT_COLLISION_GROUPS = 0b11111111'11111111'11111111'11111111U;
constexpr static float const DEFAULT_FRICTION = 0.8F;
constexpr static float const DEFAULT_RESTITUTION = 0.25F;

//----------------------------------------------------------------------------------------------------------------------

GXMat3 const& Shape::GetInertiaTensorInverse () const noexcept
{
    return _inertiaTensorInverse;
}

[[maybe_unused]] GXAABB const& Shape::GetBoundsLocal () const noexcept
{
    return _boundsLocal;
}

GXAABB const& Shape::GetBoundsWorld () const noexcept
{
    return _boundsWorld;
}

uint32_t Shape::GetCollisionGroups () const noexcept
{
    return _collisionGroups;
}

[[maybe_unused]] void Shape::SetCollisionGroups ( uint32_t groups ) noexcept
{
    _collisionGroups = groups;
}

float Shape::GetFriction () const noexcept
{
    return _friction;
}

[[maybe_unused]] void Shape::SetFriction ( float friction ) noexcept
{
    _friction = friction;
}

float Shape::GetRestitution () const noexcept
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
    _collisionGroups ( DEFAULT_COLLISION_GROUPS ),
    _friction ( DEFAULT_FRICTION ),
    _restitution ( DEFAULT_RESTITUTION ),
    _type ( shapeType ),
    _boundsLocal {},
    _boundsWorld {},
    _inertiaTensorInverse {},
    _transformRigidBody {},
    _transformWorld {}
{
    _transformRigidBody.Identity ();
    _transformWorld = _transformRigidBody;
    _inertiaTensorInverse.Identity ();
}

} // namespace android_vulkan
