#ifndef PBR_REFLECTION_COMPONENT_HPP
#define PBR_REFLECTION_COMPONENT_HPP


#include "renderable_component.hpp"
#include "transformable.hpp"
#include <android_vulkan_sdk/pbr/reflection_component_desc.hpp>


namespace pbr {

class ReflectionComponent final : public RenderableComponent, public Transformable
{
    private:
        LightRef    _probe;
        bool        _isGlobal;

    public:
        ReflectionComponent () = delete;

        ReflectionComponent ( ReflectionComponent const & ) = delete;
        ReflectionComponent &operator = ( ReflectionComponent const & ) = delete;

        ReflectionComponent ( ReflectionComponent && ) = delete;
        ReflectionComponent &operator = ( ReflectionComponent && ) = delete;

        explicit ReflectionComponent ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            ReflectionComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        ~ReflectionComponent () override = default;

        [[nodiscard]] bool IsGlobalReflection () const noexcept;

    private:
        void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept override;
        [[nodiscard]] ComponentRef &GetReference () noexcept override;
        void Submit ( RenderSession &renderSession ) noexcept override;
        void OnTransform ( GXMat4 const &transformWorld ) noexcept override;
};

} // namespace pbr


#endif // PBR_REFLECTION_COMPONENT_HPP
