#ifndef PBR_GEOMETRY_PASS_INSTANCE_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_GEOMETRY_PASS_INSTANCE_DESCRIPTOR_SET_LAYOUT_HPP


#include "descriptor_set_layout.hpp"


namespace pbr {

class GeometryPassInstanceDescriptorSetLayout final : public DescriptorSetLayout
{
    private:
        bool    _init = false;

    public:
        GeometryPassInstanceDescriptorSetLayout () = default;

        GeometryPassInstanceDescriptorSetLayout ( GeometryPassInstanceDescriptorSetLayout const & ) = delete;

        GeometryPassInstanceDescriptorSetLayout &operator = (
            GeometryPassInstanceDescriptorSetLayout const &
        ) = delete;

        GeometryPassInstanceDescriptorSetLayout ( GeometryPassInstanceDescriptorSetLayout && ) = delete;
        GeometryPassInstanceDescriptorSetLayout &operator = ( GeometryPassInstanceDescriptorSetLayout && ) = delete;

        ~GeometryPassInstanceDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout &GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_GEOMETRY_PASS_INSTANCE_DESCRIPTOR_SET_LAYOUT_HPP
