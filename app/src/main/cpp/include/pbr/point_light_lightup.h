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
        VkCommandBuffer                         _transferCommandBuffer;
        VkCommandPool                           _commandPool;
        VkDescriptorPool                        _descriptorPool;
        std::vector<VkDescriptorSet>            _descriptorSets;
        std::vector<VkDescriptorImageInfo>      _imageInfo;
        PointLightLightupProgram                _program;
        Sampler                                 _sampler;
        VkSubmitInfo                            _submitInfoTransfer;
        std::vector<VkDescriptorBufferInfo>     _uniformInfoLightData;
        UniformBufferPool                       _uniformPoolLightData;
        android_vulkan::MeshGeometry            _volumeMesh;
        std::vector<VkWriteDescriptorSet>       _writeSets;

    public:
        PointLightLightup () noexcept;

        PointLightLightup ( PointLightLightup const & ) = delete;
        PointLightLightup& operator = ( PointLightLightup const & ) = delete;

        PointLightLightup ( PointLightLightup && ) = delete;
        PointLightLightup& operator = ( PointLightLightup && ) = delete;

        ~PointLightLightup () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &resolution
        );

        void Destroy ( VkDevice device );

        [[nodiscard]] android_vulkan::MeshGeometry const& GetLightVolume () const;

        void Lightup ( VkCommandBuffer commandBuffer,
            size_t lightIndex,
            VkDescriptorSet volumeData
        );

        void SetCommonDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet set );

        [[nodiscard]] bool UpdateGPUData ( android_vulkan::Renderer &renderer,
            PointLightPass const &pointLightPass,
            GXMat4 const &viewerLocal,
            GXMat4 const &view
        );

    private:
        [[nodiscard]] bool AllocateNativeDescriptorSets ( android_vulkan::Renderer &renderer, size_t neededSets );
        void DestroyDescriptorPool ( VkDevice device );
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_LIGHTUP_H
