#ifndef PBR_UI_PASS_TRANSFORM_DESCRIPTOR_SET_LAYOUT_H
#define PBR_UI_PASS_TRANSFORM_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class UIPassTransformDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        UIPassTransformDescriptorSetLayout () = default;

        UIPassTransformDescriptorSetLayout ( UIPassTransformDescriptorSetLayout const & ) = delete;
        UIPassTransformDescriptorSetLayout &operator = ( UIPassTransformDescriptorSetLayout const & ) = delete;

        UIPassTransformDescriptorSetLayout ( UIPassTransformDescriptorSetLayout && ) = delete;
        UIPassTransformDescriptorSetLayout &operator = ( UIPassTransformDescriptorSetLayout && ) = delete;

        ~UIPassTransformDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_UI_PASS_TRANSFORM_DESCRIPTOR_SET_LAYOUT_H
