#ifndef PBR_POINT_LIGHT_H
#define PBR_POINT_LIGHT_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <array>

GX_RESTORE_WARNING_STATE

#include <half_types.h>
#include <vulkan_utils.h>
#include "gpgpu_limits.inc"
#include "light.h"


namespace pbr {

class PointLight final : public Light
{
    public:
        using Matrices = std::array<GXMat4, PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT>;

    private:
        // Matrices mapping is connected with native cube map representation
        // see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap15.html#_cube_map_face_selection_and_transformations
        enum class eFaceIndex : size_t
        {
            PositiveX = 0U,
            NegativeX = 1U,
            PositiveY = 2U,
            NegativeY = 3U,
            PositiveZ = 4U,
            NegativeZ = 5U,
        };

    private:
        GXAABB                      _bounds;
        GXVec3                      _dimensions;
        android_vulkan::Half3       _hue;
        android_vulkan::Half        _intensity;
        bool                        _isNeedUpdate;
        GXVec3                      _location;
        Matrices                    _matrices;
        GXMat4                      _projection;

    public:
        PointLight () noexcept;

        PointLight ( PointLight const & ) = delete;
        PointLight& operator = ( PointLight const & ) = delete;

        PointLight ( PointLight && ) = delete;
        PointLight& operator = ( PointLight && ) = delete;

        explicit PointLight ( android_vulkan::Half3 const &hue,
            android_vulkan::Half intensity,
            GXVec3 const &location,
            GXAABB const &bounds
        ) noexcept;

        ~PointLight () override = default;

        [[nodiscard]] GXAABB const& GetBounds () const noexcept;
        [[nodiscard]] android_vulkan::Half3 const& GetHue () const noexcept;

        [[nodiscard]] android_vulkan::Half GetIntensity () const noexcept;
        void SetIntensity ( float intensity ) noexcept;

        [[nodiscard]] GXVec3 const& GetLocation () const noexcept;
        [[nodiscard]] Matrices const& GetMatrices () noexcept;
        [[nodiscard]] GXMat4 const& GetProjection () noexcept;

        void SetLocation ( GXVec3 const location ) noexcept;

    private:
        void UpdateMatrices () noexcept;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_H
