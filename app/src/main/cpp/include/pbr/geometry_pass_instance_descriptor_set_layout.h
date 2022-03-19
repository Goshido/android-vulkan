#ifndef PBR_GEOMETRY_PASS_INSTANCE_DESCRIPTOR_SET_LAYOUT_H
#define PBR_GEOMETRY_PASS_INSTANCE_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class GeometryPassInstanceDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        GeometryPassInstanceDescriptorSetLayout () = default;

        GeometryPassInstanceDescriptorSetLayout ( GeometryPassInstanceDescriptorSetLayout const & ) = delete;

        GeometryPassInstanceDescriptorSetLayout& operator = (
            GeometryPassInstanceDescriptorSetLayout const &
        ) = delete;

        GeometryPassInstanceDescriptorSetLayout ( GeometryPassInstanceDescriptorSetLayout && ) = delete;
        GeometryPassInstanceDescriptorSetLayout& operator = ( GeometryPassInstanceDescriptorSetLayout && ) = delete;

        ~GeometryPassInstanceDescriptorSetLayout () noexcept override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_GEOMETRY_PASS_INSTANCE_DESCRIPTOR_SET_LAYOUT_H
