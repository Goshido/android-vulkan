#ifndef PBR_DESCRIPTOR_SET_LAYOUT_H
#define PBR_DESCRIPTOR_SET_LAYOUT_H


#include <renderer.h>


namespace pbr {

class DescriptorSetLayout
{
    public:
        DescriptorSetLayout () noexcept = default;

        DescriptorSetLayout ( DescriptorSetLayout const & ) = delete;
        DescriptorSetLayout& operator = ( DescriptorSetLayout const & ) = delete;

        DescriptorSetLayout ( DescriptorSetLayout && ) = delete;
        DescriptorSetLayout& operator = ( DescriptorSetLayout && ) = delete;

        virtual ~DescriptorSetLayout () = default;

        virtual void Destroy ( android_vulkan::Renderer &render ) = 0;
        [[nodiscard]] virtual bool Init ( android_vulkan::Renderer &render ) = 0;

        [[nodiscard]] virtual VkDescriptorSetLayout GetLayout () const = 0;
};

} // namespace pbr


#endif // PBR_DESCRIPTOR_SET_LAYOUT_H
