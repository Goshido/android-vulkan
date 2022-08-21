#ifndef PBR_GEOMETRY_PASS_TEXTURE_DESCRIPTOR_SET_LAYOUT_H
#define PBR_GEOMETRY_PASS_TEXTURE_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class GeometryPassTextureDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        GeometryPassTextureDescriptorSetLayout () = default;

        GeometryPassTextureDescriptorSetLayout ( GeometryPassTextureDescriptorSetLayout const & ) = delete;
        GeometryPassTextureDescriptorSetLayout& operator = ( GeometryPassTextureDescriptorSetLayout const & ) = delete;

        GeometryPassTextureDescriptorSetLayout ( GeometryPassTextureDescriptorSetLayout && ) = delete;
        GeometryPassTextureDescriptorSetLayout& operator = ( GeometryPassTextureDescriptorSetLayout && ) = delete;

        ~GeometryPassTextureDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_GEOMETRY_PASS_TEXTURE_DESCRIPTOR_SET_LAYOUT_H
