#ifndef ANDROID_VULKAN_SHAPE_H
#define ANDROID_VULKAN_SHAPE_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>
#include <memory>

GX_RESTORE_WARNING_STATE

#include <GXCommon/GXMath.h>


namespace android_vulkan {

enum class eShapeType : uint8_t
{
    Sphere [[maybe_unused]] = 0U,
    Box = 1U,
    Rectangle [[maybe_unused]] = 2U
};

class Shape
{
    private:
        eShapeType      _type;

    protected:
        GXAABB          _boundsLocal;
        GXAABB          _boundsWorld;

        float           _frictionDynamic;
        float           _frictionStatic;

        GXMat3          _inertiaTensor;

        float           _restitution;

        GXMat4          _transformRigidBody;
        GXMat4          _transformWorld;

    public:
        Shape () = delete;

        Shape ( Shape const & ) = delete;
        Shape& operator = ( Shape const & ) = delete;

        Shape ( Shape && ) = delete;
        Shape& operator = ( Shape && ) = delete;

        [[maybe_unused]] virtual void CalculateInertiaTensor ( float mass ) noexcept = 0;

        [[maybe_unused, nodiscard]] virtual GXVec3 GetExtremePointWorld (
            GXVec3 const &directionWorld
        ) const noexcept = 0;

        [[nodiscard]] GXMat3 const& GetInertiaTensor () const noexcept;

        [[maybe_unused, nodiscard]] GXAABB const& GetBoundsLocal () const noexcept;
        [[maybe_unused, nodiscard]] GXAABB const& GetBoundsWorld () const noexcept;

        [[maybe_unused, nodiscard]] float GetFrictionDynamic () const noexcept;
        [[maybe_unused]] void SetFrictionDynamic ( float friction ) noexcept;

        [[maybe_unused, nodiscard]] float GetFrictionStatic () const noexcept;
        [[maybe_unused]] void SetFrictionStatic ( float friction ) noexcept;

        [[maybe_unused, nodiscard]] float GetRestitution () const noexcept;
        [[maybe_unused]] void SetRestitution ( float restitution ) noexcept;

        [[maybe_unused, nodiscard]] GXMat4 const& GetTransformWorld () const noexcept;
        [[maybe_unused, nodiscard]] eShapeType GetType () const noexcept;

        void UpdateCacheData ( GXMat4 const &transform ) noexcept;

    protected:
        explicit Shape ( eShapeType shapeType ) noexcept;
        virtual ~Shape () = default;

        virtual void UpdateBounds () noexcept = 0;
};

using ShapeRef = std::shared_ptr<Shape>;

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SHAPE_H
