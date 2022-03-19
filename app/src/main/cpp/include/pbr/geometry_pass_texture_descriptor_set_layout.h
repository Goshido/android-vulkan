#ifndef PBR_GEOMETRY_PASS_TEXTURE_DESCRIPTOR_SET_LAYOUT_H
#define PBR_GEOMETRY_PASS_TEXTURE_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class GeometryPassTextureDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        constexpr static size_t const TEXTURE_SLOTS = 5U;

    public:
        GeometryPassTextureDescriptorSetLayout () = default;

        GeometryPassTextureDescriptorSetLayout ( GeometryPassTextureDescriptorSetLayout const &other ) = delete;
        GeometryPassTextureDescriptorSetLayout& operator = ( GeometryPassTextureDescriptorSetLayout const &other ) = delete;

        GeometryPassTextureDescriptorSetLayout ( GeometryPassTextureDescriptorSetLayout &&other ) = delete;
        GeometryPassTextureDescriptorSetLayout& operator = ( GeometryPassTextureDescriptorSetLayout &&other ) = delete;

        ~GeometryPassTextureDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_GEOMETRY_PASS_TEXTURE_DESCRIPTOR_SET_LAYOUT_H
