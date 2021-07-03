#ifndef PBR_COMPONENT_H
#define PBR_COMPONENT_H


#include "component_desc.h"
#include "render_session.h"
#include "types.h"


namespace pbr {

class Component
{
    private:
        ClassID     _classID;

    public:
        Component () = delete;

        Component ( Component const & ) = delete;
        Component& operator = ( Component const & ) = delete;

        Component ( Component && ) = delete;
        Component& operator = ( Component && ) = delete;

        virtual ~Component () = default;

        virtual void Submit ( RenderSession &renderSession ) = 0;
        virtual void FreeTransferResources ( VkDevice device ) = 0;

        [[nodiscard]] ClassID GetClassID () const;

        [[nodiscard]] static ComponentRef Create ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

    protected:
        explicit Component ( ClassID classID ) noexcept;
};

} // namespace pbr


#endif // PBR_COMPONENT_H
