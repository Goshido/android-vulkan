#ifndef ANDROID_VULKAN_SHAPE_SPHERE_HPP
#define ANDROID_VULKAN_SHAPE_SPHERE_HPP


#include "shape.hpp"


namespace android_vulkan {

class [[maybe_unused]] ShapeSphere final : public Shape
{
    private:
        float const     _radius;

    public:
        ShapeSphere () = delete;

        ShapeSphere ( ShapeSphere const & ) = delete;
        ShapeSphere &operator = ( ShapeSphere const & ) = delete;

        ShapeSphere ( ShapeSphere && ) = delete;
        ShapeSphere &operator = ( ShapeSphere && ) = delete;

        [[maybe_unused]] explicit ShapeSphere ( float radius ) noexcept;

        ~ShapeSphere () override = default;

        [[maybe_unused, nodiscard]] float GetRadius () const noexcept;

    private:
        void CalculateInertiaTensor ( float mass ) noexcept override;
        [[nodiscard]] GXVec3 GetExtremePointWorld ( GXVec3 const &direction ) const noexcept override;
        void UpdateBounds () noexcept override;
};

} // namespace android_vulkan

#endif // ANDROID_VULKAN_SHAPE_SPHERE_HPP
