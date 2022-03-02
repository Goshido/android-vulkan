#ifndef PBR_DESCRIPTOR_SET_LAYOUT_H
#define PBR_DESCRIPTOR_SET_LAYOUT_H


#include <renderer.h>


namespace pbr {

// The class successors must create singleton instance of target the VkDescriptorSetLayout object and
// maintain it's presence.
class DescriptorSetLayout
{
    public:
        DescriptorSetLayout () = default;

        DescriptorSetLayout ( DescriptorSetLayout const & ) = delete;
        DescriptorSetLayout& operator = ( DescriptorSetLayout const & ) = delete;

        DescriptorSetLayout ( DescriptorSetLayout && ) = delete;
        DescriptorSetLayout& operator = ( DescriptorSetLayout && ) = delete;

        virtual ~DescriptorSetLayout () = default;

        virtual void Destroy ( VkDevice device ) noexcept = 0;
        [[nodiscard]] virtual bool Init ( android_vulkan::Renderer &render ) noexcept = 0;

        [[nodiscard]] virtual VkDescriptorSetLayout GetLayout () const noexcept = 0;
};

} // namespace pbr


#endif // PBR_DESCRIPTOR_SET_LAYOUT_H
