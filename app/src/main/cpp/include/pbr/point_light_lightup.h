#ifndef PBR_POINT_LIGHT_LIGHTUP_H
#define PBR_POINT_LIGHT_LIGHTUP_H


#include "mesh_geometry.h"
#include "light_volume.h"
#include "point_light.h"
#include "point_light_lightup_program.h"
#include "sampler.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class PointLightPass;
class PointLightLightup final
{
    private:
        VkCommandBuffer                         _transferCommandBuffer = VK_NULL_HANDLE;
        VkCommandPool                           _commandPool = VK_NULL_HANDLE;
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSets;
        std::vector<VkDescriptorImageInfo>      _imageInfo {};
        PointLightLightupProgram                _program {};
        Sampler                                 _sampler {};
        VkSubmitInfo                            _submitInfoTransfer {};
        std::vector<VkDescriptorBufferInfo>     _uniformInfoLightData {};
        UniformBufferPool                       _uniformPoolLightData { eUniformPoolSize::Tiny_4M };
        std::vector<VkWriteDescriptorSet>       _writeSets {};

    public:
        PointLightLightup () = default;

        PointLightLightup ( PointLightLightup const & ) = delete;
        PointLightLightup& operator = ( PointLightLightup const & ) = delete;

        PointLightLightup ( PointLightLightup && ) = delete;
        PointLightLightup& operator = ( PointLightLightup && ) = delete;

        ~PointLightLightup () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &resolution
        ) noexcept;

        void Destroy ( VkDevice device ) noexcept;

        void Lightup ( VkCommandBuffer commandBuffer,
            android_vulkan::MeshGeometry &unitCube,
            size_t lightIndex
        ) noexcept;

        [[nodiscard]] bool UpdateGPUData ( android_vulkan::Renderer &renderer,
            PointLightPass const &pointLightPass,
            GXMat4 const &viewerLocal,
            GXMat4 const &view
        ) noexcept;

    private:
        [[nodiscard]] bool AllocateNativeDescriptorSets ( android_vulkan::Renderer &renderer,
            size_t neededSets
        ) noexcept;

        void DestroyDescriptorPool ( VkDevice device ) noexcept;
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_LIGHTUP_H
