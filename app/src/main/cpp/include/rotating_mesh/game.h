#ifndef ROTATING_MESH_GAME_H
#define ROTATING_MESH_GAME_H


#include <game.h>
#include <vulkan_utils.h>
#include <mesh_geometry.h>
#include <GXCommon/GXMath.h>
#include "drawcall.h"
#include "uniform_buffer.h"


namespace rotating_mesh {

constexpr const size_t MATERIAL_COUNT = 3U;

class Game : public android_vulkan::Game
{
    private:
        using CommandContext = std::pair<VkCommandBuffer, VkFence>;

        AV_DX_ALIGNMENT_BEGIN

        struct Transform
        {
            GXMat4                      _transform;
            GXMat4                      _normalTransform;
        };

        AV_DX_ALIGNMENT_END

    protected:
        VkCommandPool                   _commandPool;

        VkDescriptorPool                _descriptorPool;
        VkDescriptorSetLayout           _descriptorSetLayout;

        Drawcall                        _drawcalls[ MATERIAL_COUNT ];
        VkPipelineLayout                _pipelineLayout;
        UniformBuffer                   _transformBuffer;

    private:
        float                           _angle;
        android_vulkan::Texture2D       _depthStencilRenderTarget;

        const char*                     _fragmentShader;
        std::vector<VkFramebuffer>      _framebuffers;

        VkPipeline                      _pipeline;
        VkRenderPass                    _renderPass;
        VkSemaphore                     _renderPassEndSemaphore;
        VkSemaphore                     _renderTargetAcquiredSemaphore;

        VkSampler                       _sampler01Mips;
        VkSampler                       _sampler09Mips;
        VkSampler                       _sampler10Mips;
        VkSampler                       _sampler11Mips;

        VkShaderModule                  _vertexShaderModule;
        VkShaderModule                  _fragmentShaderModule;

        std::vector<CommandContext>     _commandBuffers;

        GXMat4                          _projectionMatrix;
        Transform                       _transform;

    protected:
        explicit Game ( const char* fragmentShader );

        Game ( const Game &other ) = delete;
        Game& operator = ( const Game &other ) = delete;

        [[nodiscard]] virtual bool CreateDescriptorSet ( android_vulkan::Renderer &renderer ) = 0;
        [[nodiscard]] virtual bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) = 0;
        [[nodiscard]] virtual bool LoadGPUContent ( android_vulkan::Renderer &renderer ) = 0;

        [[nodiscard]] virtual bool CreateSamplers ( android_vulkan::Renderer &renderer );
        virtual void DestroySamplers ( android_vulkan::Renderer &renderer );
        virtual void DestroyTextures ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateCommonTextures ( android_vulkan::Renderer &renderer, VkCommandBuffer* commandBuffers );
        [[nodiscard]] bool CreateMeshes ( android_vulkan::Renderer &renderer, VkCommandBuffer* commandBuffers );

        static void InitDescriptorPoolSizeCommon ( VkDescriptorPoolSize* features );
        static void InitDescriptorSetLayoutBindingCommon ( VkDescriptorSetLayoutBinding* bindings );

    private:
        bool IsReady () override;

        bool OnInit ( android_vulkan::Renderer &renderer ) override;
        bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) override;
        bool OnDestroy ( android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] bool BeginFrame ( size_t &imageIndex, android_vulkan::Renderer &renderer );
        [[nodiscard]] bool EndFrame ( uint32_t imageIndex, android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer );
        void DestroyCommandPool ( android_vulkan::Renderer &renderer );

        void DestroyDescriptorSet ( android_vulkan::Renderer &renderer );
        void DestroyMeshes ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer );
        void DestroyFramebuffers ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreatePipeline ( android_vulkan::Renderer &renderer );
        void DestroyPipeline ( android_vulkan::Renderer &renderer );

        void DestroyPipelineLayout ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer );
        void DestroyRenderPass ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateShaderModules ( android_vulkan::Renderer &renderer );
        void DestroyShaderModules ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateSyncPrimitives ( android_vulkan::Renderer &renderer );
        void DestroySyncPrimitives ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateUniformBuffer ( android_vulkan::Renderer &renderer );
        void DestroyUniformBuffer ();

        [[nodiscard]] bool InitCommandBuffers ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool UpdateUniformBuffer ( android_vulkan::Renderer &renderer, double deltaTime );
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_GAME_H
