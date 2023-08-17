#ifndef PBR_SPD_MIP_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_SPD_MIP_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class SPDMipDescriptorSetLayout final
{
    public:
        VkDescriptorSetLayout       _layout = VK_NULL_HANDLE;

    private:
        std::atomic<size_t>         _references = 0U;
        uint32_t                    _relaxedMipViews;

    public:
        SPDMipDescriptorSetLayout () = delete;

        SPDMipDescriptorSetLayout ( SPDMipDescriptorSetLayout const & ) = delete;
        SPDMipDescriptorSetLayout &operator = ( SPDMipDescriptorSetLayout const & ) = delete;

        SPDMipDescriptorSetLayout ( SPDMipDescriptorSetLayout && ) = delete;
        SPDMipDescriptorSetLayout &operator = ( SPDMipDescriptorSetLayout && ) = delete;

        explicit SPDMipDescriptorSetLayout ( uint32_t relaxedMipViews ) noexcept;

        ~SPDMipDescriptorSetLayout () = default;

        void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
};

} // namespace pbr


#endif // PBR_SPD_MIP_DESCRIPTOR_SET_LAYOUT_HPP
