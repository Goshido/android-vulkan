#ifndef PBR_REFLECTION_LOCAL_DESCRIPTOR_SET_LAYOUT_H
#define PBR_REFLECTION_LOCAL_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class ReflectionLocalDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        ReflectionLocalDescriptorSetLayout () = default;

        ReflectionLocalDescriptorSetLayout ( ReflectionLocalDescriptorSetLayout const & ) = delete;
        ReflectionLocalDescriptorSetLayout& operator = ( ReflectionLocalDescriptorSetLayout const & ) = delete;

        ReflectionLocalDescriptorSetLayout ( ReflectionLocalDescriptorSetLayout && ) = delete;
        ReflectionLocalDescriptorSetLayout& operator = ( ReflectionLocalDescriptorSetLayout && ) = delete;

        ~ReflectionLocalDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) override;
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const override;
};

} // namespace pbr


#endif // PBR_REFLECTION_LOCAL_DESCRIPTOR_SET_LAYOUT_H
