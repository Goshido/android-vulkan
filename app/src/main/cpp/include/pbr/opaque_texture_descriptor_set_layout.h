#ifndef PBR_OPAQUE_TEXTURE_DESCRIPTOR_SET_LAYOUT_H
#define PBR_OPAQUE_TEXTURE_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class OpaqueTextureDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        constexpr static size_t const TEXTURE_SLOTS = 5U;

    public:
        OpaqueTextureDescriptorSetLayout () = default;

        OpaqueTextureDescriptorSetLayout ( OpaqueTextureDescriptorSetLayout const &other ) = delete;
        OpaqueTextureDescriptorSetLayout& operator = ( OpaqueTextureDescriptorSetLayout const &other ) = delete;

        OpaqueTextureDescriptorSetLayout ( OpaqueTextureDescriptorSetLayout &&other ) = delete;
        OpaqueTextureDescriptorSetLayout& operator = ( OpaqueTextureDescriptorSetLayout &&other ) = delete;

        ~OpaqueTextureDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) override;
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const override;
};

} // namespace pbr


#endif // PBR_OPAQUE_TEXTURE_DESCRIPTOR_SET_LAYOUT_H
