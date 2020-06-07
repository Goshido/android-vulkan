#ifndef ROTATING_MESH_GAME_H
#define ROTATING_MESH_GAME_H


#include <game.h>
#include <vulkan_utils.h>
#include <GXCommon/GXMath.h>
#include "drawcall.h"
#include "mesh_geometry.h"
#include "texture2D.h"
#include "uniform_buffer.h"


namespace rotating_mesh {

constexpr const size_t MATERIAL_COUNT = 3U;

class Game final : public android_vulkan::Game
{
    private:
        using CommandContext = std::pair<VkCommandBuffer, VkFence>;

        AV_DX_ALIGNMENT_BEGIN

        struct Transform
        {
            GXMat4      _transform;
            GXMat4      _normalTransform;
        };

        AV_DX_ALIGNMENT_END

    private:
        float                           _angle;

        VkCommandPool                   _commandPool;

        VkImage                         _depthStencil;
        VkImageView                     _depthStencilView;
        VkDeviceMemory                  _depthStencilMemory;

        VkDescriptorPool                _descriptorPool;
        VkDescriptorSetLayout           _descriptorSetLayout;

        Drawcall                        _drawcalls[ MATERIAL_COUNT ];

        std::vector<VkFramebuffer>      _framebuffers;

        VkPipeline                      _pipeline;
        VkPipelineLayout                _pipelineLayout;

        VkRenderPass                    _renderPass;

        VkSemaphore                     _renderPassEndSemaphore;
        VkSemaphore                     _renderTargetAcquiredSemaphore;

        VkSampler                       _sampler01Mips;
        VkSampler                       _sampler09Mips;
        VkSampler                       _sampler10Mips;
        VkSampler                       _sampler11Mips;

        VkSampler                       _specularLUTSampler;
        Texture2D                       _specularLUTTexture;

        VkShaderModule                  _vertexShaderModule;
        VkShaderModule                  _fragmentShaderModule;

        std::vector<CommandContext>     _commandBuffers;

        GXMat4                          _projectionMatrix;

        UniformBuffer                   _transformBuffer;
        Transform                       _transform;

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

        bool BeginFrame ( size_t &imageIndex, android_vulkan::Renderer &renderer );
        bool EndFrame ( uint32_t imageIndex, android_vulkan::Renderer &renderer );

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

        bool CreateSpecularLUTTexture ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer );
        void DestroySpecularLUTTexture ( android_vulkan::Renderer &renderer );

        bool CreateSyncPrimitives ( android_vulkan::Renderer &renderer );
        void DestroySyncPrimitives ( android_vulkan::Renderer &renderer );

        bool CreateTextures ( android_vulkan::Renderer &renderer, VkCommandBuffer* commandBuffers );
        void DestroyTextures ( android_vulkan::Renderer &renderer );

        bool CreateUniformBuffer ( android_vulkan::Renderer &renderer );
        void DestroyUniformBuffer ();

        bool InitCommandBuffers ( android_vulkan::Renderer &renderer );
        bool LoadGPUContent ( android_vulkan::Renderer &renderer );
        bool UpdateUniformBuffer ( android_vulkan::Renderer &renderer, double deltaTime );
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_GAME_H
