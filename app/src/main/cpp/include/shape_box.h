#ifndef ANDROID_VULKAN_SHAPE_BOX_H
#define ANDROID_VULKAN_SHAPE_BOX_H

#include "shape.h"

namespace android_vulkan {

class [[maybe_unused]] ShapeBox final : public Shape
{
    private:
        GXVec3 const    _size;

    public:
        ShapeBox () = delete;

        ShapeBox ( ShapeBox const & ) = delete;
        ShapeBox& operator = ( ShapeBox const & ) = delete;

        ShapeBox ( ShapeBox && ) = delete;
        ShapeBox& operator = ( ShapeBox && ) = delete;

        explicit ShapeBox ( GXVec3 const &size ) noexcept;
        explicit ShapeBox ( float width, float height, float depth ) noexcept;

        ~ShapeBox () override = default;

        [[maybe_unused]] void CalculateInertiaTensor ( float mass ) noexcept override;
        [[maybe_unused, nodiscard]] GXVec3 GetExtremePointWorld ( GXVec3 const &direction ) const noexcept override;

        [[maybe_unused, nodiscard]] float GetWidth () const noexcept;
        [[maybe_unused, nodiscard]] float GetHeight () const noexcept;
        [[maybe_unused, nodiscard]] float GetDepth () const noexcept;

    private:
        void Init () noexcept;
        void UpdateBounds () noexcept override;
};

} // namespace android_vulkan

#endif // ANDROID_VULKAN_SHAPE_BOX_H