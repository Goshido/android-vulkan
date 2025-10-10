#ifndef PBR_POINT_LIGHT_LIGHTUP_HPP
#define PBR_POINT_LIGHT_LIGHTUP_HPP


#include <platform/android/mesh_geometry.hpp>
#include "point_light.hpp"
#include "point_light_lightup_program.hpp"
#include "uma_uniform_buffer.hpp"


namespace pbr {

class PointLightPass;
class PointLightLightup final
{
    private:
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSets {};
        std::vector<VkDescriptorImageInfo>      _imageInfo {};

        size_t                                  _itemBaseIndex = 0U;
        size_t                                  _itemReadIndex = 0U;
        size_t                                  _itemWriteIndex = 0U;
        size_t                                  _itemWritten = 0U;

        PointLightLightupProgram                _program {};
        VkMappedMemoryRange                     _ranges[ 2U ]{};
        UMAUniformBuffer                        _uniformPool {};
        std::vector<VkWriteDescriptorSet>       _writeSets {};

    public:
        explicit PointLightLightup () = default;

        PointLightLightup ( PointLightLightup const & ) = delete;
        PointLightLightup &operator = ( PointLightLightup const & ) = delete;

        PointLightLightup ( PointLightLightup && ) = delete;
        PointLightLightup &operator = ( PointLightLightup && ) = delete;

        ~PointLightLightup () = default;

        void Commit () noexcept;
        void BindProgram ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            VkExtent2D const &resolution
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        void Lightup ( VkCommandBuffer commandBuffer, VkDescriptorSet transform, uint32_t volumeVertices ) noexcept;

        [[nodiscard]] bool UpdateGPUData ( VkDevice device,
            PointLightPass const &pointLightPass,
            GXMat4 const &viewerLocal,
            GXMat4 const &view
        ) noexcept;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_LIGHTUP_HPP
