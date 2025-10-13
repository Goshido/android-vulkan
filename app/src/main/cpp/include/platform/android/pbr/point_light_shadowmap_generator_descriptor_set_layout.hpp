#ifndef PBR_POINT_LIGHT_SHADOWMAP_GENERATOR_DESCRIPTOR_SET_LAYOUT_HPP
#define PBR_POINT_LIGHT_SHADOWMAP_GENERATOR_DESCRIPTOR_SET_LAYOUT_HPP


#include <pbr/descriptor_set_layout.hpp>


namespace pbr {

class PointLightShadowmapGeneratorDescriptorSetLayout final : public DescriptorSetLayout
{
    private:
        bool    _init = false;

    public:
        explicit PointLightShadowmapGeneratorDescriptorSetLayout () = default;

        PointLightShadowmapGeneratorDescriptorSetLayout (
            PointLightShadowmapGeneratorDescriptorSetLayout const &
        ) = delete;

        PointLightShadowmapGeneratorDescriptorSetLayout &operator = (
            PointLightShadowmapGeneratorDescriptorSetLayout const &
        ) = delete;

        PointLightShadowmapGeneratorDescriptorSetLayout ( PointLightShadowmapGeneratorDescriptorSetLayout && ) = delete;

        PointLightShadowmapGeneratorDescriptorSetLayout &operator = (
            PointLightShadowmapGeneratorDescriptorSetLayout &&
        ) = delete;

        ~PointLightShadowmapGeneratorDescriptorSetLayout () override = default;

        void Destroy ( VkDevice device ) noexcept override;
        [[nodiscard]] bool Init ( VkDevice device ) noexcept override;

        [[nodiscard]] VkDescriptorSetLayout &GetLayout () const noexcept override;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_SHADOWMAP_GENERATOR_DESCRIPTOR_SET_LAYOUT_HPP
