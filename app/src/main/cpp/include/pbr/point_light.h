#ifndef PBR_POINT_LIGHT_H
#define PBR_POINT_LIGHT_H


#include <half_types.h>
#include <vulkan_utils.h>
#include <pbr/light.h>


namespace pbr {

class [[maybe_unused]] PointLight final : public Light
{
    private:
        GXAABB                      _bounds;
        android_vulkan::Half3       _hue;
        android_vulkan::Half        _intensity;
        GXVec3                      _location;

    public:
        [[maybe_unused]] PointLight () noexcept;

        PointLight ( PointLight const & ) = delete;
        PointLight& operator = ( PointLight const & ) = delete;

        PointLight ( PointLight && ) = delete;
        PointLight& operator = ( PointLight && ) = delete;

        [[maybe_unused]] explicit PointLight ( android_vulkan::Half3 const &hue,
            android_vulkan::Half intensity,
            GXVec3 const &location,
            GXAABB const &bounds
        ) noexcept;

        ~PointLight () override = default;

        [[maybe_unused]] [[nodiscard]] GXAABB const& GetBounds () const;
        [[maybe_unused]] [[nodiscard]] android_vulkan::Half3 const& GetHue () const;
        [[maybe_unused]] [[nodiscard]] android_vulkan::Half GetIntensity () const;
        [[maybe_unused]] [[nodiscard]] GXVec3 const& GetLocation () const;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_H
