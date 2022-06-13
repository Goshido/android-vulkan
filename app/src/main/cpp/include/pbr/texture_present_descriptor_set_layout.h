#ifndef PBR_TEXTURE_PRESENT_DESCRIPTOR_SET_LAYOUT_H
#define PBR_TEXTURE_PRESENT_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class TexturePresentDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        TexturePresentDescriptorSetLayout () = default;

        TexturePresentDescriptorSetLayout ( TexturePresentDescriptorSetLayout const &other ) = delete;
        TexturePresentDescriptorSetLayout& operator = ( TexturePresentDescriptorSetLayout const &other ) = delete;

        TexturePresentDescriptorSetLayout ( TexturePresentDescriptorSetLayout &&other ) = delete;
        TexturePresentDescriptorSetLayout& operator = ( TexturePresentDescriptorSetLayout &&other ) = delete;

        ~TexturePresentDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_TEXTURE_PRESENT_DESCRIPTOR_SET_LAYOUT_H
