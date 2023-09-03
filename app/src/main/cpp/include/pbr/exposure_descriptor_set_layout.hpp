#ifndef PBR_EXPOSURE_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_EXPOSURE_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class ExposureDescriptorSetLayout final : public DescriptorSetLayout
{
    private:
        bool    _init = false;

    public:
        ExposureDescriptorSetLayout () = default;

        ExposureDescriptorSetLayout ( ExposureDescriptorSetLayout const & ) = delete;
        ExposureDescriptorSetLayout &operator = ( ExposureDescriptorSetLayout const & ) = delete;

        ExposureDescriptorSetLayout ( ExposureDescriptorSetLayout && ) = delete;
        ExposureDescriptorSetLayout &operator = ( ExposureDescriptorSetLayout && ) = delete;

        ~ExposureDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_EXPOSURE_DESCRIPTOR_SET_LAYOUT_HPP
