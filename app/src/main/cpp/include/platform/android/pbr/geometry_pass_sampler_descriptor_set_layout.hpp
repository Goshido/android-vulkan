#ifndef PBR_GEOMETRY_PASS_SAMPLER_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_GEOMETRY_PASS_SAMPLER_DESCRIPTOR_SET_LAYOUT_HPP


#include <pbr/descriptor_set_layout.hpp>


namespace pbr {

class GeometryPassSamplerDescriptorSetLayout final : public DescriptorSetLayout
{
    private:
        bool    _init = false;

    public:
        GeometryPassSamplerDescriptorSetLayout () = default;

        GeometryPassSamplerDescriptorSetLayout ( GeometryPassSamplerDescriptorSetLayout const & ) = delete;
        GeometryPassSamplerDescriptorSetLayout &operator = ( GeometryPassSamplerDescriptorSetLayout const & ) = delete;

        GeometryPassSamplerDescriptorSetLayout ( GeometryPassSamplerDescriptorSetLayout && ) = delete;
        GeometryPassSamplerDescriptorSetLayout &operator = ( GeometryPassSamplerDescriptorSetLayout && ) = delete;

        ~GeometryPassSamplerDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout &GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_GEOMETRY_PASS_SAMPLER_DESCRIPTOR_SET_LAYOUT_HPP
