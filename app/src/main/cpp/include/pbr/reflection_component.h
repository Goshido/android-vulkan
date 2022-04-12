#ifndef PBR_REFLECTION_COMPONENT_H
#define PBR_REFLECTION_COMPONENT_H


#include "renderable_component.h"
#include "reflection_component_desc.h"


namespace pbr {

class ReflectionComponent final : public RenderableComponent
{
    private:
        LightRef    _probe;

    public:
        ReflectionComponent () = delete;

        ReflectionComponent ( ReflectionComponent const & ) = delete;
        ReflectionComponent& operator = ( ReflectionComponent const & ) = delete;

        ReflectionComponent ( ReflectionComponent && ) = delete;
        ReflectionComponent& operator = ( ReflectionComponent && ) = delete;

        explicit ReflectionComponent ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            ReflectionComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        ~ReflectionComponent () override = default;

    private:
        void FreeTransferResources ( VkDevice device ) noexcept override;
        void Submit ( RenderSession &renderSession ) noexcept override;
};

} // namespace pbr


#endif // PBR_REFLECTION_COMPONENT_H
