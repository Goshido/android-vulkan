#ifndef ROTATING_MESH_GAME_HPP
#define ROTATING_MESH_GAME_HPP


#include <game.hpp>
#include <vulkan_utils.hpp>
#include <mesh_geometry.hpp>
#include <uniform_buffer.hpp>
#include <GXCommon/GXMath.hpp>
#include "drawcall.hpp"


namespace rotating_mesh {

constexpr const size_t MATERIAL_COUNT = 3U;

class Game : public android_vulkan::Game
{
    private:
        AV_DX_ALIGNMENT_BEGIN

        struct Transform
        {
            GXMat4                          _transform;
            GXMat4                          _normalTransform;
        };

        AV_DX_ALIGNMENT_END

        struct CommandInfo final
        {
            VkCommandPool                   _pool = VK_NULL_HANDLE;
            VkCommandBuffer                 _buffer = VK_NULL_HANDLE;
            VkFence                         _fence = VK_NULL_HANDLE;
            VkSemaphore                     _acquire = VK_NULL_HANDLE;
        };

        struct FramebufferInfo final
        {
            VkFramebuffer                   _framebuffer = VK_NULL_HANDLE;
            VkSemaphore                     _renderPassEnd = VK_NULL_HANDLE;
        };

    protected:
        constexpr static size_t             DUAL_COMMAND_BUFFER = 2U;

        CommandInfo                         _commandInfo[ DUAL_COMMAND_BUFFER ];
        size_t                              _writingCommandInfo = 0U;

        VkDescriptorPool                    _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout               _fixedDSLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout               _onceDSLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout               _materialDSLayout = VK_NULL_HANDLE;

        VkDescriptorSet                     _fixedDS = VK_NULL_HANDLE;
        VkDescriptorSet                     _materialDS[ MATERIAL_COUNT ];
        VkDescriptorSet                     _onceDS[ DUAL_COMMAND_BUFFER ];

        Drawcall                            _drawcalls[ MATERIAL_COUNT ] {};
        VkPipelineLayout                    _pipelineLayout = VK_NULL_HANDLE;
        android_vulkan::UniformBuffer       _transformBuffer {};

        VkSampler                           _sampler = VK_NULL_HANDLE;

    private:
        float                               _angle = 0.0F;
        android_vulkan::Texture2D           _depthStencilRenderTarget {};

        char const*                         _fragmentShader;
        std::vector<FramebufferInfo>        _framebufferInfo {};

        VkPipeline                          _pipeline = VK_NULL_HANDLE;
        VkRenderPass                        _renderPass = VK_NULL_HANDLE;

        VkShaderModule                      _vertexShaderModule = VK_NULL_HANDLE;
        VkShaderModule                      _fragmentShaderModule = VK_NULL_HANDLE;

        GXMat4                              _projectionMatrix {};
        Transform                           _transform {};

    public:
        Game () = delete;

        Game ( Game const & ) = delete;
        Game &operator = ( Game const & ) = delete;

        Game ( Game && ) = delete;
        Game &operator = ( Game && ) = delete;

    protected:
        explicit Game ( char const* fragmentShader ) noexcept;
        ~Game () override = default;

        [[nodiscard]] virtual bool CreateMaterialDescriptorSetLayout (
            android_vulkan::Renderer &renderer ) noexcept = 0;

        [[nodiscard]] virtual bool CreateDescriptorSet ( android_vulkan::Renderer &renderer ) noexcept = 0;

        [[nodiscard]] virtual bool LoadGPUContent ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool
        ) noexcept = 0;

        virtual void DestroyTextures ( android_vulkan::Renderer &renderer ) noexcept;
        [[nodiscard]] bool CreateSamplers ( VkDevice device ) noexcept;
        void DestroySamplers ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateCommonTextures ( android_vulkan::Renderer &renderer,
            VkCommandBuffer* commandBuffers
        ) noexcept;

        [[nodiscard]] bool CreateMeshes ( android_vulkan::Renderer &renderer,
            VkCommandBuffer* commandBuffers
        ) noexcept;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool BeginFrame ( android_vulkan::Renderer &renderer,
            size_t &imageIndex,
            VkSemaphore acquire
        ) noexcept;

        [[nodiscard]] bool EndFrame ( android_vulkan::Renderer &renderer, uint32_t imageIndex ) noexcept;

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateCommonDescriptorSetLayouts ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommonDescriptorSetLayouts ( VkDevice device ) noexcept;

        void DestroyDescriptorSet ( VkDevice device ) noexcept;
        void DestroyMeshes ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyFramebuffers ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool CreatePipeline ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyPipeline ( VkDevice device ) noexcept;

        void DestroyPipelineLayout ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyRenderPass ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateShaderModules ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyShaderModules ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateUniformBuffer ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyUniformBuffer ( android_vulkan::Renderer &renderer ) noexcept;

        void UpdateUniformBuffer ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkDescriptorSet ds,
            double deltaTime
        ) noexcept;
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_GAME_HPP
