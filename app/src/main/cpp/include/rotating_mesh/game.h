#ifndef ROTATING_MESH_GAME_H
#define ROTATING_MESH_GAME_H


#include <game.h>
#include <vulkan_utils.h>
#include <mesh_geometry.h>
#include <uniform_buffer.h>
#include <GXCommon/GXMath.h>
#include "drawcall.h"


namespace rotating_mesh {

constexpr const size_t MATERIAL_COUNT = 3U;

class Game : public android_vulkan::Game
{
    private:
        using CommandContext = std::pair<VkCommandBuffer, VkFence>;

        AV_DX_ALIGNMENT_BEGIN

        struct Transform
        {
            GXMat4                          _transform;
            GXMat4                          _normalTransform;
        };

        AV_DX_ALIGNMENT_END

    protected:
        VkCommandPool                       _commandPool;

        VkDescriptorPool                    _descriptorPool;
        VkDescriptorSetLayout               _descriptorSetLayout;

        Drawcall                            _drawcalls[ MATERIAL_COUNT ];
        VkPipelineLayout                    _pipelineLayout;
        android_vulkan::UniformBuffer       _transformBuffer;

    private:
        float                               _angle;
        android_vulkan::Texture2D           _depthStencilRenderTarget;

        const char*                         _fragmentShader;
        std::vector<VkFramebuffer>          _framebuffers;

        VkPipeline                          _pipeline;
        VkRenderPass                        _renderPass;
        VkSemaphore                         _renderPassEndSemaphore;
        VkSemaphore                         _renderTargetAcquiredSemaphore;

        VkSampler                           _sampler01Mips;
        VkSampler                           _sampler09Mips;
        VkSampler                           _sampler10Mips;
        VkSampler                           _sampler11Mips;

        VkShaderModule                      _vertexShaderModule;
        VkShaderModule                      _fragmentShaderModule;

        std::vector<CommandContext>         _commandBuffers;

        GXMat4                              _projectionMatrix;
        Transform                           _transform;

    protected:
        explicit Game ( const char* fragmentShader ) noexcept;

        Game ( Game const & ) = delete;
        Game& operator = ( Game const & ) = delete;

        Game ( Game && ) = delete;
        Game& operator = ( Game && ) = delete;

        [[nodiscard]] virtual bool CreateDescriptorSet ( android_vulkan::Renderer &renderer ) = 0;
        [[nodiscard]] virtual bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) = 0;
        [[nodiscard]] virtual bool LoadGPUContent ( android_vulkan::Renderer &renderer ) = 0;

        [[nodiscard]] virtual bool CreateSamplers ( android_vulkan::Renderer &renderer );
        virtual void DestroySamplers ( VkDevice device );
        virtual void DestroyTextures ( VkDevice device );

        [[nodiscard]] bool CreateCommonTextures ( android_vulkan::Renderer &renderer, VkCommandBuffer* commandBuffers );
        [[nodiscard]] bool CreateMeshes ( android_vulkan::Renderer &renderer, VkCommandBuffer* commandBuffers );

        static void InitDescriptorPoolSizeCommon ( VkDescriptorPoolSize* features );
        static void InitDescriptorSetLayoutBindingCommon ( VkDescriptorSetLayoutBinding* bindings );

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( VkDevice device ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( VkDevice device ) noexcept override;

        [[nodiscard]] bool BeginFrame ( android_vulkan::Renderer &renderer, size_t &imageIndex );
        [[nodiscard]] bool EndFrame ( android_vulkan::Renderer &renderer, uint32_t imageIndex );

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer );
        void DestroyCommandPool ( VkDevice device );

        void DestroyDescriptorSet ( VkDevice device );
        void DestroyMeshes ( VkDevice device );

        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer );
        void DestroyFramebuffers ( VkDevice device );

        [[nodiscard]] bool CreatePipeline ( android_vulkan::Renderer &renderer );
        void DestroyPipeline ( VkDevice device );

        void DestroyPipelineLayout ( VkDevice device );

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer );
        void DestroyRenderPass ( VkDevice device );

        [[nodiscard]] bool CreateShaderModules ( android_vulkan::Renderer &renderer );
        void DestroyShaderModules ( VkDevice device );

        [[nodiscard]] bool CreateSyncPrimitives ( android_vulkan::Renderer &renderer );
        void DestroySyncPrimitives ( VkDevice device );

        [[nodiscard]] bool CreateUniformBuffer ( android_vulkan::Renderer &renderer );
        void DestroyUniformBuffer ( VkDevice device );

        [[nodiscard]] bool CreateCommandBuffers ( android_vulkan::Renderer &renderer );
        void DestroyCommandBuffers ( VkDevice device );

        [[nodiscard]] bool UpdateUniformBuffer ( android_vulkan::Renderer &renderer, double deltaTime );
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_GAME_H
