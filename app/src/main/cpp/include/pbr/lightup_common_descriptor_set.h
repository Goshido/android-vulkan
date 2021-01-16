#ifndef PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_H
#define PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_H


#include <uniform_buffer.h>
#include <pbr/lightup_common_descriptor_set_layout.h>


namespace pbr {

class LightupCommonDescriptorSet final
{
    private:
        VkCommandPool                       _commandPool;
        VkDescriptorPool                    _descriptorPool;
        LightupCommonDescriptorSetLayout    _layout;
        VkDescriptorSet                     _set;
        android_vulkan::UniformBuffer       _uniformBuffer;

    public:
        LightupCommonDescriptorSet () noexcept;

        LightupCommonDescriptorSet ( LightupCommonDescriptorSet const & ) = delete;
        LightupCommonDescriptorSet& operator = ( LightupCommonDescriptorSet const & ) = delete;

        LightupCommonDescriptorSet ( LightupCommonDescriptorSet && ) = delete;
        LightupCommonDescriptorSet& operator = ( LightupCommonDescriptorSet && ) = delete;

        ~LightupCommonDescriptorSet () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
        void Destroy ( VkDevice device );
        [[maybe_unused, nodiscard]] VkDescriptorSet GetSet () const;

        [[nodiscard]] bool Update ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution,
            GXMat4 const &cvvToView
        );
};

} // namespace pbr


#endif // PBR_LIGHTUP_COMMON_DESCRIPTOR_SET_H
