#ifndef PBR_REFLECTION_LOCAL_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_REFLECTION_LOCAL_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class ReflectionLocalDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        ReflectionLocalDescriptorSetLayout () = default;

        ReflectionLocalDescriptorSetLayout ( ReflectionLocalDescriptorSetLayout const & ) = delete;
        ReflectionLocalDescriptorSetLayout &operator = ( ReflectionLocalDescriptorSetLayout const & ) = delete;

        ReflectionLocalDescriptorSetLayout ( ReflectionLocalDescriptorSetLayout && ) = delete;
        ReflectionLocalDescriptorSetLayout &operator = ( ReflectionLocalDescriptorSetLayout && ) = delete;

        ~ReflectionLocalDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;
        [[nodiscard]] VkDescriptorSetLayout &GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_REFLECTION_LOCAL_DESCRIPTOR_SET_LAYOUT_HPP
