#ifndef PBR_POINT_LIGHT_LIGHTUP_H
#define PBR_POINT_LIGHT_LIGHTUP_H


#include "mesh_geometry.h"
#include "light_volume.h"
#include "point_light_lightup_program.h"
#include "sampler.h"
#include "uniform_buffer_pool.h"


namespace pbr {

class PointLightPass;
class PointLightLightup final
{
    private:
        VkCommandBuffer                         _transferCommandBuffer;
        VkCommandPool                           _commandPool;
        VkDescriptorPool                        _descriptorPool;
        std::vector<VkDescriptorSet>            _descriptorSets;
        std::vector<VkDescriptorImageInfo>      _imageInfo;
        PointLightLightupProgram                _program;
        Sampler                                 _sampler;
        VkSubmitInfo                            _submitInfoTransfer;
        std::vector<VkDescriptorBufferInfo>     _uniformInfo;
        UniformBufferPool                       _uniformPool;
        android_vulkan::MeshGeometry            _volumeMesh;
        std::vector<VkWriteDescriptorSet>       _writeSets;

    public:
        PointLightLightup () noexcept;

        PointLightLightup ( PointLightLightup const & ) = delete;
        PointLightLightup& operator = ( PointLightLightup const & ) = delete;

        PointLightLightup ( PointLightLightup && ) = delete;
        PointLightLightup& operator = ( PointLightLightup && ) = delete;

        ~PointLightLightup () = default;

        [[maybe_unused]] [[nodiscard]] bool Execute ( PointLightPass const &pointLightPass,
            LightVolume &lightVolume,
            GXMat4 const &viewerLocal,
            GXMat4 const &view,
            android_vulkan::Renderer &renderer
        );

        [[nodiscard]] bool Init ( VkCommandBuffer commandBuffer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &resolution,
            android_vulkan::Renderer &renderer
        );

        void Destroy ( android_vulkan::Renderer &renderer );

    private:
        [[nodiscard]] bool AllocateNativeDescriptorSets ( size_t neededSets, android_vulkan::Renderer &renderer );
        void DestroyDescriptorPool ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool UpdateGPUData ( PointLightPass const &pointLightPass,
            GXMat4 const &viewerLocal,
            GXMat4 const &view,
            android_vulkan::Renderer &renderer
        );
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_LIGHTUP_H
