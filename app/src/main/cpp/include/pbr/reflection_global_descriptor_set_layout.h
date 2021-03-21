#ifndef PBR_REFLECTION_GLOBAL_DESCRIPTIOR_SET_LAYOUT_H
#define PBR_REFLECTION_GLOBAL_DESCRIPTIOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class ReflectionGlobalDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        ReflectionGlobalDescriptorSetLayout () = default;

        ReflectionGlobalDescriptorSetLayout ( ReflectionGlobalDescriptorSetLayout const & ) = delete;
        ReflectionGlobalDescriptorSetLayout& operator = ( ReflectionGlobalDescriptorSetLayout const & ) = delete;

        ReflectionGlobalDescriptorSetLayout ( ReflectionGlobalDescriptorSetLayout && ) = delete;
        ReflectionGlobalDescriptorSetLayout& operator = ( ReflectionGlobalDescriptorSetLayout && ) = delete;

        ~ReflectionGlobalDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) override;
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const override;
};

} // namespace pbr


#endif // PBR_REFLECTION_GLOBAL_DESCRIPTIOR_SET_LAYOUT_H
