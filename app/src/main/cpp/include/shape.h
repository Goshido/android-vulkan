#ifndef ANDROID_VULKAN_SHAPE_H
#define ANDROID_VULKAN_SHAPE_H


#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>
#include <memory>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

enum class eShapeType : uint8_t
{
    Sphere = 0U,
    Box = 1U
};

class Shape
{
    private:
        uint32_t        _collisionGroups;
        float           _friction;
        float           _restitution;
        eShapeType      _type;

    protected:
        GXAABB          _boundsLocal;
        GXAABB          _boundsWorld;

        GXMat3          _inertiaTensorInverse;

        GXMat4          _transformRigidBody;
        GXMat4          _transformWorld;

    public:
        Shape () = delete;

        Shape ( Shape const & ) = delete;
        Shape& operator = ( Shape const & ) = delete;

        Shape ( Shape && ) = delete;
        Shape& operator = ( Shape && ) = delete;

        virtual void CalculateInertiaTensor ( float mass ) noexcept = 0;
        [[nodiscard]] virtual GXVec3 GetExtremePointWorld ( GXVec3 const &directionWorld ) const noexcept = 0;

        [[nodiscard]] GXMat3 const& GetInertiaTensorInverse () const noexcept;

        [[maybe_unused, nodiscard]] GXAABB const& GetBoundsLocal () const noexcept;
        [[nodiscard]] GXAABB const& GetBoundsWorld () const noexcept;

        [[nodiscard]] uint32_t GetCollisionGroups () const noexcept;
        [[maybe_unused]] void SetCollisionGroups ( uint32_t groups ) noexcept;

        [[nodiscard]] float GetFriction () const noexcept;
        [[maybe_unused]] void SetFriction ( float friction ) noexcept;

        [[nodiscard]] float GetRestitution () const noexcept;
        [[maybe_unused]] void SetRestitution ( float restitution ) noexcept;

        [[maybe_unused, nodiscard]] GXMat4 const& GetTransformWorld () const noexcept;
        [[maybe_unused, nodiscard]] eShapeType GetType () const noexcept;

        void UpdateCacheData ( GXMat4 const &transform ) noexcept;

        [[nodiscard]] static GXVec3 FindSupportPoint ( GXVec3 const &direction,
            Shape const &shapeA,
            Shape const &shapeB
        ) noexcept;

    protected:
        explicit Shape ( eShapeType shapeType ) noexcept;
        virtual ~Shape () = default;

        virtual void UpdateBounds () noexcept = 0;
};

using ShapeRef = std::shared_ptr<Shape>;

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SHAPE_H
