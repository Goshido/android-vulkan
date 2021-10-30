#ifndef ANDROID_VULKAN_SHAPE_BOX_H
#define ANDROID_VULKAN_SHAPE_BOX_H


#include "shape.h"


namespace android_vulkan {

class [[maybe_unused]] ShapeBox final : public Shape
{
    private:
        GXVec3          _localGeometry[ 8U ];
        GXVec3 const    _size;

    public:
        ShapeBox () = delete;

        ShapeBox ( ShapeBox const & ) = delete;
        ShapeBox& operator = ( ShapeBox const & ) = delete;

        ShapeBox ( ShapeBox && ) = delete;
        ShapeBox& operator = ( ShapeBox && ) = delete;

        [[maybe_unused]] explicit ShapeBox ( GXVec3 const &size ) noexcept;
        [[maybe_unused]] explicit ShapeBox ( float width, float height, float depth ) noexcept;

        ~ShapeBox () override = default;

        [[maybe_unused, nodiscard]] float GetWidth () const noexcept;
        [[maybe_unused, nodiscard]] float GetHeight () const noexcept;
        [[maybe_unused, nodiscard]] float GetDepth () const noexcept;

        void Test () noexcept;

    private:
        void CalculateInertiaTensor ( float mass ) noexcept override;
        [[nodiscard]] GXVec3 GetExtremePointWorld ( GXVec3 const &direction ) const noexcept override;
        void UpdateBounds () noexcept override;

        void Init () noexcept;
};

} // namespace android_vulkan

#endif // ANDROID_VULKAN_SHAPE_BOX_H
