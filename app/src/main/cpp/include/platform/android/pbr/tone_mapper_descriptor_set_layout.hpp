#ifndef PBR_TONE_MAPPER_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_TONE_MAPPER_DESCRIPTOR_SET_LAYOUT_HPP


#include <pbr/descriptor_set_layout.hpp>


namespace pbr {

class ToneMapperDescriptorSetLayout final : public DescriptorSetLayout
{
    private:
        bool    _init = false;

    public:
        ToneMapperDescriptorSetLayout () = default;

        ToneMapperDescriptorSetLayout ( ToneMapperDescriptorSetLayout const & ) = delete;
        ToneMapperDescriptorSetLayout &operator = ( ToneMapperDescriptorSetLayout const & ) = delete;

        ToneMapperDescriptorSetLayout ( ToneMapperDescriptorSetLayout && ) = delete;
        ToneMapperDescriptorSetLayout &operator = ( ToneMapperDescriptorSetLayout && ) = delete;

        ~ToneMapperDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout &GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_TONE_MAPPER_DESCRIPTOR_SET_LAYOUT_HPP
