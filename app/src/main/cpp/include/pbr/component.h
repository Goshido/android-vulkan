#ifndef PBR_COMPONENT_H
#define PBR_COMPONENT_H


#include "component_desc.h"
#include "render_session.h"
#include "types.h"


namespace pbr {

class Component
{
    public:
        Component ( Component const &other ) = delete;
        Component& operator = ( Component const &other ) = delete;

        Component ( Component &&other ) = delete;
        Component& operator = ( Component &&other ) = delete;

        virtual ~Component () = default;

        virtual void Submit ( RenderSession &renderSession ) = 0;
        virtual void FreeTransferResources ( android_vulkan::Renderer &renderer ) = 0;

        [[nodiscard]] static ComponentRef Create ( size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer const* commandBuffers
        );

    protected:
        Component () = default;
};

} // namespace pbr


#endif // PBR_COMPONENT_H
