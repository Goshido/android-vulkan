#ifndef PBR_SPD_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_SPD_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class SPDDescriptorSetLayout final
{
    public:
        VkDescriptorSetLayout       _layout = VK_NULL_HANDLE;

    private:
        std::atomic<size_t>         _references = 0U;
        uint32_t                    _relaxedMipViews;

    public:
        SPDDescriptorSetLayout () = delete;

        SPDDescriptorSetLayout ( SPDDescriptorSetLayout const & ) = delete;
        SPDDescriptorSetLayout &operator = ( SPDDescriptorSetLayout const & ) = delete;

        SPDDescriptorSetLayout ( SPDDescriptorSetLayout && ) = delete;
        SPDDescriptorSetLayout &operator = ( SPDDescriptorSetLayout && ) = delete;

        explicit SPDDescriptorSetLayout ( uint32_t relaxedMipViews ) noexcept;

        ~SPDDescriptorSetLayout () = default;

        void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
};

} // namespace pbr


#endif // PBR_SPD_DESCRIPTOR_SET_LAYOUT_HPP
