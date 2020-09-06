#ifndef PBR_DESCRIPTOR_SET_LAYOUT_H
#define PBR_DESCRIPTOR_SET_LAYOUT_H


#include <renderer.h>


namespace pbr {

class DescriptorSetLayout
{
    public:
        DescriptorSetLayout () = default;

        DescriptorSetLayout ( DescriptorSetLayout const &other ) = delete;
        DescriptorSetLayout& operator = ( DescriptorSetLayout const &other ) = delete;

        DescriptorSetLayout ( DescriptorSetLayout &&other ) = delete;
        DescriptorSetLayout& operator = ( DescriptorSetLayout &&other ) = delete;

        virtual ~DescriptorSetLayout () = default;

        virtual void Destroy ( android_vulkan::Renderer &render ) = 0;
        [[nodiscard]] virtual bool Init ( android_vulkan::Renderer &render ) = 0;

        [[nodiscard]] virtual VkDescriptorSetLayout GetLayout () const = 0;
};

} // namespace pbr


#endif // PBR_DESCRIPTOR_SET_LAYOUT_H
