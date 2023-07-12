#ifndef PBR_POINT_LIGHT_COMPONENT_HPP
#define PBR_POINT_LIGHT_COMPONENT_HPP


#include "point_light.h"
#include "point_light_component_desc.h"
#include "renderable_component.h"
#include "transformable.h"


namespace pbr {

class PointLightComponent final : public RenderableComponent, public Transformable
{
    private:
        LightRef    _pointLight;

    public:
        PointLightComponent () noexcept;

        PointLightComponent ( PointLightComponent const & ) = delete;
        PointLightComponent &operator = ( PointLightComponent const & ) = delete;

        PointLightComponent ( PointLightComponent && ) = delete;
        PointLightComponent &operator = ( PointLightComponent && ) = delete;

        explicit PointLightComponent ( PointLightComponentDesc const &desc, uint8_t const* data ) noexcept;

        ~PointLightComponent () override = default;

        void Submit ( RenderSession &renderSession ) noexcept override;

        void SetBoundDimensions ( float width, float height, float depth ) noexcept;
        void SetBoundDimensions ( GXVec3 const &dimensions ) noexcept;
        [[maybe_unused]] void SetHue ( GXColorRGB const &hue ) noexcept;
        void SetIntensity ( float intensity ) noexcept;
        void SetLocation ( GXVec3 const &location ) noexcept;

    private:
        [[nodiscard]] ComponentRef &GetReference () noexcept override;
        void OnTransform ( GXMat4 const &transformWorld ) noexcept override;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_COMPONENT_HPP
