#ifndef ROTATING_MESH_GAME_H
#define ROTATING_MESH_GAME_H


#include <game.h>
#include <vulkan_utils.h>
#include <GXCommon/GXMath.h>
#include "mesh_geometry.h"
#include "texture2D.h"
#include "uniform_buffer.h"


namespace rotating_mesh {

class Game final : public android_vulkan::Game
{
    private:
AV_DX_ALIGNMENT_BEGIN
        struct MipInfo final
        {
            float                       _level;
            GXVec3                      _padding0_0;
        };
AV_DX_ALIGNMENT_END

    private:
        const Texture2D*                _activeTexture;

        VkCommandPool                   _commandPool;

        VkImage                         _depthStencil;
        VkImageView                     _depthStencilView;
        VkDeviceMemory                  _depthStencilMemory;

        VkDescriptorPool                _descriptorPool;
        VkDescriptorSet                 _descriptorSet;
        VkDescriptorSetLayout           _descriptorSetLayout;

        std::vector<VkFramebuffer>      _framebuffers;

        double                          _mipTimeout;

        VkPipeline                      _pipeline;
        VkPipelineLayout                _pipelineLayout;

        VkRenderPass                    _renderPass;

        VkSemaphore                     _renderPassEndSemaphore;
        VkSemaphore                     _renderTargetAcquiredSemaphore;

        VkSampler                       _sampler09Mips;
        VkSampler                       _sampler10Mips;
        VkSampler                       _sampler11Mips;

        VkShaderModule                  _vertexShaderModule;
        VkShaderModule                  _fragmentShaderModule;

        std::vector<VkCommandBuffer>    _commandBuffers;

        MeshGeometry                    _mesh;

        Texture2D                       _material1Diffuse;
        Texture2D                       _material2Diffuse;
        Texture2D                       _material2Normal;
        Texture2D                       _material3Diffuse;
        Texture2D                       _material3Normal;

        MipInfo                         _mipInfo;
        UniformBuffer                   _mipInfoBuffer;

        UniformBuffer                   _peTransformBuffer;

    public:
        Game ();
        ~Game () override = default;

        Game ( const Game &other ) = delete;
        Game& operator = ( const Game &other ) = delete;

    private:
        bool IsReady () override;

        bool OnInit ( android_vulkan::Renderer &renderer ) override;
        bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) override;
        bool OnDestroy ( android_vulkan::Renderer &renderer ) override;

        bool BeginFrame ( uint32_t &presentationImageIndex, android_vulkan::Renderer &renderer );
        bool EndFrame ( uint32_t presentationImageIndex, android_vulkan::Renderer &renderer );

        bool CreateCommandPool ( android_vulkan::Renderer &renderer );
        void DestroyCommandPool ( android_vulkan::Renderer &renderer );

        bool CreateDescriptorSet ( android_vulkan::Renderer &renderer );
        void DestroyDescriptorSet ( android_vulkan::Renderer &renderer );

        bool CreateFramebuffers ( android_vulkan::Renderer &renderer );
        void DestroyFramebuffers ( android_vulkan::Renderer &renderer );

        bool CreateMeshes ( android_vulkan::Renderer &renderer, VkCommandBuffer* commandBuffers );
        void DestroyMeshes ( android_vulkan::Renderer &renderer );

        bool CreatePipeline ( android_vulkan::Renderer &renderer );
        void DestroyPipeline ( android_vulkan::Renderer &renderer );

        bool CreatePipelineLayout ( android_vulkan::Renderer &renderer );
        void DestroyPipelineLayout ( android_vulkan::Renderer &renderer );

        bool CreateRenderPass ( android_vulkan::Renderer &renderer );
        void DestroyRenderPass ( android_vulkan::Renderer &renderer );

        bool CreateSamplers ( android_vulkan::Renderer &renderer );
        void DestroySamplers ( android_vulkan::Renderer &renderer );

        bool CreateShaderModules ( android_vulkan::Renderer &renderer );
        void DestroyShaderModules ( android_vulkan::Renderer &renderer );

        bool CreateSyncPrimitives ( android_vulkan::Renderer &renderer );
        void DestroySyncPrimitives ( android_vulkan::Renderer &renderer );

        bool CreateTextures ( android_vulkan::Renderer &renderer, VkCommandBuffer* commandBuffers );
        void DestroyTextures ( android_vulkan::Renderer &renderer );

        bool CreateUniformBuffer ( android_vulkan::Renderer &renderer );
        void DestroyUniformBuffer ();

        bool InitCommandBuffers ( android_vulkan::Renderer &renderer );
        bool LoadGPUContent ( android_vulkan::Renderer &renderer );
        bool UpdateUniformBuffer ( double deltaTime );
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_GAME_H
