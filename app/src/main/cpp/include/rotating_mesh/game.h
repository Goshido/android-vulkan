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
        VkCommandPool                       _commandPool = VK_NULL_HANDLE;

        VkDescriptorPool                    _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout               _descriptorSetLayout = VK_NULL_HANDLE;

        Drawcall                            _drawcalls[ MATERIAL_COUNT ] {};
        VkPipelineLayout                    _pipelineLayout = VK_NULL_HANDLE;
        android_vulkan::UniformBuffer       _transformBuffer {};

    private:
        float                               _angle = 0.0F;
        android_vulkan::Texture2D           _depthStencilRenderTarget {};

        VkFence                             _fence = VK_NULL_HANDLE;
        char const*                         _fragmentShader;
        std::vector<VkFramebuffer>          _framebuffers {};

        VkPipeline                          _pipeline = VK_NULL_HANDLE;
        VkRenderPass                        _renderPass = VK_NULL_HANDLE;
        VkSemaphore                         _renderPassEndSemaphore = VK_NULL_HANDLE;
        VkSemaphore                         _renderTargetAcquiredSemaphore = VK_NULL_HANDLE;

        VkSampler                           _sampler01Mips = VK_NULL_HANDLE;
        VkSampler                           _sampler09Mips = VK_NULL_HANDLE;
        VkSampler                           _sampler10Mips = VK_NULL_HANDLE;
        VkSampler                           _sampler11Mips = VK_NULL_HANDLE;

        VkShaderModule                      _vertexShaderModule = VK_NULL_HANDLE;
        VkShaderModule                      _fragmentShaderModule = VK_NULL_HANDLE;

        std::vector<CommandContext>         _commandBuffers {};

        GXMat4                              _projectionMatrix {};
        Transform                           _transform {};

    public:
        Game () = delete;

        Game ( Game const & ) = delete;
        Game& operator = ( Game const & ) = delete;

        Game ( Game && ) = delete;
        Game& operator = ( Game && ) = delete;

    protected:
        explicit Game ( char const* fragmentShader ) noexcept;
        ~Game () override = default;

        [[nodiscard]] virtual bool CreateDescriptorSet ( android_vulkan::Renderer &renderer ) noexcept = 0;
        [[nodiscard]] virtual bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) noexcept = 0;
        [[nodiscard]] virtual bool LoadGPUContent ( android_vulkan::Renderer &renderer ) noexcept = 0;

        [[nodiscard]] virtual bool CreateSamplers ( android_vulkan::Renderer &renderer ) noexcept;
        virtual void DestroySamplers ( VkDevice device ) noexcept;
        virtual void DestroyTextures ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool CreateCommonTextures ( android_vulkan::Renderer &renderer,
            VkCommandBuffer* commandBuffers
        ) noexcept;

        [[nodiscard]] bool CreateMeshes ( android_vulkan::Renderer &renderer,
            VkCommandBuffer* commandBuffers
        ) noexcept;

        static void InitDescriptorPoolSizeCommon ( VkDescriptorPoolSize* features ) noexcept;
        static void InitDescriptorSetLayoutBindingCommon ( VkDescriptorSetLayoutBinding* bindings ) noexcept;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool BeginFrame ( android_vulkan::Renderer &renderer, size_t &imageIndex ) noexcept;
        [[nodiscard]] bool EndFrame ( android_vulkan::Renderer &renderer, uint32_t imageIndex ) noexcept;

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandPool ( VkDevice device ) noexcept;

        void DestroyDescriptorSet ( VkDevice device ) noexcept;
        void DestroyMeshes ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyFramebuffers ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool CreatePipeline ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyPipeline ( VkDevice device ) noexcept;

        void DestroyPipelineLayout ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyRenderPass ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateShaderModules ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyShaderModules ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateSyncPrimitives ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroySyncPrimitives ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateUniformBuffer ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyUniformBuffer ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateCommandBuffers ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandBuffers ( VkDevice device ) noexcept;

        [[nodiscard]] bool UpdateUniformBuffer ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept;
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_GAME_H
