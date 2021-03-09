#ifndef PBR_OPAQUE_INSTANCE_DESCRIPTOR_SET_LAYOUT_H
#define PBR_OPAQUE_INSTANCE_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class OpaqueInstanceDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        OpaqueInstanceDescriptorSetLayout () = default;

        OpaqueInstanceDescriptorSetLayout ( OpaqueInstanceDescriptorSetLayout const & ) = delete;
        OpaqueInstanceDescriptorSetLayout& operator = ( OpaqueInstanceDescriptorSetLayout const & ) = delete;

        OpaqueInstanceDescriptorSetLayout ( OpaqueInstanceDescriptorSetLayout && ) = delete;
        OpaqueInstanceDescriptorSetLayout& operator = ( OpaqueInstanceDescriptorSetLayout && ) = delete;

        ~OpaqueInstanceDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) override;
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const override;
};

} // namespace pbr


#endif // PBR_OPAQUE_INSTANCE_DESCRIPTOR_SET_LAYOUT_H
