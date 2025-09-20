#ifndef PBR_POINT_LIGHT_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_POINT_LIGHT_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class PointLightDescriptorSetLayout final : public DescriptorSetLayout
{
    private:
        bool    _init = false;

    public:
        PointLightDescriptorSetLayout () = default;

        PointLightDescriptorSetLayout ( PointLightDescriptorSetLayout const & ) = delete;
        PointLightDescriptorSetLayout &operator = ( PointLightDescriptorSetLayout const & ) = delete;

        PointLightDescriptorSetLayout ( PointLightDescriptorSetLayout && ) = delete;
        PointLightDescriptorSetLayout &operator = ( PointLightDescriptorSetLayout && ) = delete;

        ~PointLightDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout &GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_DESCRIPTOR_SET_LAYOUT_HPP
