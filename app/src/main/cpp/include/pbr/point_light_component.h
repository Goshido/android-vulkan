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
        PointLightComponent () = delete;

        PointLightComponent ( PointLightComponent const & ) = delete;
        PointLightComponent& operator = ( PointLightComponent const & ) = delete;

        PointLightComponent ( PointLightComponent && ) = delete;
        PointLightComponent& operator = ( PointLightComponent && ) = delete;

        explicit PointLightComponent ( PointLightComponentDesc const &desc ) noexcept;

        ~PointLightComponent () override = default;

    private:
        void Submit ( RenderSession &renderSession ) override;
        void FreeTransferResources ( android_vulkan::Renderer &renderer ) override;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_COMPONENT_H
