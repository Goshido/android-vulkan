#ifndef PBR_FULL_SCREEN_TRIANGLE_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_FULL_SCREEN_TRIANGLE_DESCRIPTOR_SET_LAYOUT_HPP


#include <pbr/descriptor_set_layout.hpp>


namespace pbr {

class FullScreenTriangleDescriptorSetLayout final : public DescriptorSetLayout
{
    private:
        bool    _init = false;

    public:
        FullScreenTriangleDescriptorSetLayout () = default;

        FullScreenTriangleDescriptorSetLayout ( FullScreenTriangleDescriptorSetLayout const & ) = delete;
        FullScreenTriangleDescriptorSetLayout &operator = ( FullScreenTriangleDescriptorSetLayout const & ) = delete;

        FullScreenTriangleDescriptorSetLayout ( FullScreenTriangleDescriptorSetLayout && ) = delete;
        FullScreenTriangleDescriptorSetLayout &operator = ( FullScreenTriangleDescriptorSetLayout && ) = delete;

        ~FullScreenTriangleDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout &GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_FULL_SCREEN_TRIANGLE_DESCRIPTOR_SET_LAYOUT_HPP
