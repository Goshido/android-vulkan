#ifndef PBR_POINT_LIGHT_COMPONENT_H
#define PBR_POINT_LIGHT_COMPONENT_H


#include "component.h"
#include "point_light.h"
#include "point_light_component_desc.h"
#include "types.h"


namespace pbr {

class PointLightComponent final : public Component
{
    private:
        LightRef    _pointLight;

    public:
        PointLightComponent () noexcept;

        PointLightComponent ( PointLightComponent const & ) = delete;
        PointLightComponent& operator = ( PointLightComponent const & ) = delete;

        PointLightComponent ( PointLightComponent && ) = delete;
        PointLightComponent& operator = ( PointLightComponent && ) = delete;

        explicit PointLightComponent ( PointLightComponentDesc const &desc ) noexcept;

        ~PointLightComponent () override = default;

        [[maybe_unused]] void SetBoundDimensions ( float width, float height, float depth ) noexcept;
        [[maybe_unused]] void SetHue ( GXColorRGB const &hue ) noexcept;
        [[maybe_unused]] void SetIntensity ( float intensity ) noexcept;
        [[maybe_unused]] void SetLocation ( GXVec3 const &location ) noexcept;

    private:
        void Submit ( RenderSession &renderSession ) override;
        void FreeTransferResources ( VkDevice device ) override;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_COMPONENT_H
