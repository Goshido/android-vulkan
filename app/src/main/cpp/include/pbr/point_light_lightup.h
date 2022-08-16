#ifndef PBR_POINT_LIGHT_LIGHTUP_H
#define PBR_POINT_LIGHT_LIGHTUP_H


#include "mesh_geometry.h"
#include "point_light.h"
#include "point_light_lightup_program.h"
#include "sampler.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class PointLightPass;
class PointLightLightup final
{
    private:
        std::vector<VkBufferMemoryBarrier>      _barriers {};
        std::vector<VkDescriptorBufferInfo>     _bufferInfo {};
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSets;
        std::vector<VkDescriptorImageInfo>      _imageInfo {};

        size_t                                  _itemBaseIndex = 0U;
        size_t                                  _itemReadIndex = 0U;
        size_t                                  _itemWriteIndex = 0U;
        size_t                                  _itemWritten = 0U;

        PointLightLightupProgram                _program {};
        Sampler                                 _sampler {};
        UniformBufferPool                       _uniformPool { eUniformPoolSize::Tiny_4M, false };
        std::vector<VkWriteDescriptorSet>       _writeSets {};

    public:
        PointLightLightup () = default;

        PointLightLightup ( PointLightLightup const & ) = delete;
        PointLightLightup& operator = ( PointLightLightup const & ) = delete;

        PointLightLightup ( PointLightLightup && ) = delete;
        PointLightLightup& operator = ( PointLightLightup && ) = delete;

        ~PointLightLightup () = default;

        void Commit () noexcept;
        void BindProgram ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &resolution
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        void Lightup ( VkCommandBuffer commandBuffer,
            VkDescriptorSet transform,
            android_vulkan::MeshGeometry &unitCube
        ) noexcept;

        void UpdateGPUData ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            PointLightPass const &pointLightPass,
            GXMat4 const &viewerLocal,
            GXMat4 const &view
        ) noexcept;

    private:
        [[nodiscard]] bool AllocateDescriptorSets ( android_vulkan::Renderer &renderer ) noexcept;
        void IssueSync ( VkDevice device, VkCommandBuffer commandBuffer ) const noexcept;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_LIGHTUP_H
