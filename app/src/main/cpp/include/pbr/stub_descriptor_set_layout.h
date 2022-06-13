#ifndef PBR_STUB_DESCRIPTOR_SET_LAYOUT_H
#define PBR_STUB_DESCRIPTOR_SET_LAYOUT_H


#include "descriptor_set_layout.h"


namespace pbr {

class StubDescriptorSetLayout final : public DescriptorSetLayout
{
    public:
        StubDescriptorSetLayout () = default;

        StubDescriptorSetLayout ( StubDescriptorSetLayout const & ) = delete;
        StubDescriptorSetLayout& operator = ( StubDescriptorSetLayout const & ) = delete;

        StubDescriptorSetLayout ( StubDescriptorSetLayout && ) = delete;
        StubDescriptorSetLayout& operator = ( StubDescriptorSetLayout && ) = delete;

        ~StubDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_STUB_DESCRIPTOR_SET_LAYOUT_H
