#ifndef PBR_SKIN_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_SKIN_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class SkinDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        SkinDescriptorSetLayout () = default;

        SkinDescriptorSetLayout ( SkinDescriptorSetLayout const & ) = delete;
        SkinDescriptorSetLayout &operator = ( SkinDescriptorSetLayout const & ) = delete;

        SkinDescriptorSetLayout ( SkinDescriptorSetLayout && ) = delete;
        SkinDescriptorSetLayout &operator = ( SkinDescriptorSetLayout && ) = delete;

        ~SkinDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_SKIN_DESCRIPTOR_SET_LAYOUT_HPP
