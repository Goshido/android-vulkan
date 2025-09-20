#ifndef EDITOR_RENDER_SESSION_HPP
#define EDITOR_RENDER_SESSION_HPP


#include "hello_triangle_program.hpp"
#include "hello_triangle_program_ext.hpp"
#include "message_queue.hpp"
#include <pbr/default_texture_manager.hpp>

// FUCK - remove namespace
#include <platform/android/pbr/exposure_pass.hpp>
#include <platform/android/pbr/present_pass.hpp>
#include <platform/android/pbr/tone_mapper_pass.hpp>

#include <platform/windows/mesh_geometry.hpp>
#include <platform/windows/pbr/exposure_pass.hpp>
#include <platform/windows/pbr/present_pass.hpp>
#include <platform/windows/pbr/resource_heap.hpp>
#include <platform/windows/pbr/tone_mapper_pass.hpp>
#include <platform/windows/pbr/ui_pass.hpp>
#include "ui_manager.hpp"


namespace editor {

class RenderSession final
{
    private:
        struct CommandInfo final
        {
            VkSemaphore                                             _acquire = VK_NULL_HANDLE;
            VkCommandBuffer                                         _buffer = VK_NULL_HANDLE;
            VkFence                                                 _fence = VK_NULL_HANDLE;
            bool                                                    _inUse = false;
            VkCommandPool                                           _pool = VK_NULL_HANDLE;
        };

        using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;

    private:
        bool                                                        _broken = false;

        CommandInfo                                                 _commandInfo[ pbr::FIF_COUNT ];
        size_t                                                      _writingCommandInfo = 0U;

        pbr::DefaultTextureManager                                  _defaultTextureManager {};

        // FUCK - remove it
        pbr::android::ExposurePass                                  _exposurePass {};

        // FUCK - remove namespace
        pbr::windows::ExposurePass                                  _exposurePassEXT {};

        // FUCK - remote it
        std::unique_ptr<HelloTriangleProgram>                       _helloTriangleProgram {};

        std::unique_ptr<HelloTriangleProgramExt>                    _helloTriangleProgramExt {};

        // FUCK
        std::unique_ptr<android_vulkan::android::MeshGeometry>      _fuckHelloTriangleGeometry {};
        std::unique_ptr<android_vulkan::windows::MeshGeometry>      _helloTriangleGeometry {};

        MessageQueue                                                &_messageQueue;
        android_vulkan::Renderer                                    &_renderer;

        // FUCK - remove namespace
        pbr::android::PresentPass                                   _presentRenderPass {};
        pbr::windows::PresentPass                                   _presentRenderPassEXT {};

        // FUCK - refactor
        VkRenderPassBeginInfo                                       _renderPassInfo {};

        android_vulkan::Texture2D                                   _renderTarget {};
        uint32_t                                                    _renderTargetIdx = 0U;

        // FUCK - call update GPU
        pbr::windows::ResourceHeap                                  _resourceHeap {};

        // FUCK - remove
        pbr::SamplerManager                                         _samplerManager {};

        std::mutex                                                  _submitMutex {};
        std::thread                                                 _thread {};
        Timestamp                                                   _timestamp {};

        // FUCK - remove namespace
        pbr::android::ToneMapperPass                                _toneMapper {};
        pbr::windows::ToneMapperPass                                _toneMapperEXT {};

        size_t                                                      _uiElements = 0U;

        // FUCK - remove namespace
        pbr::android::UIPass                                        _uiPass {};
        pbr::windows::UIPass                                        _uiPassEXT { _resourceHeap };

        UIManager                                                   &_uiManager;
        VkViewport                                                  _viewport {};

        VkRenderingAttachmentInfo                                   _colorAttachment
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = VK_NULL_HANDLE,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,

            .clearValue
            {
                .color
                {
                    .float32 { 0.5F, 0.5F, 0.5F, 1.0F }
                }
            }
        };

        VkRenderingInfo                                             _renderingInfo
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext = nullptr,
            .flags = 0U,

            .renderArea
            {
                .offset
                {
                    .x = 0,
                    .y = 0
                },

                .extent
                {
                    .width = 0U,
                    .height = 0U
                }
            },

            .layerCount = 1U,
            .viewMask = 0U,
            .colorAttachmentCount = 1U,
            .pColorAttachments = &_colorAttachment,
            .pDepthAttachment = nullptr,
            .pStencilAttachment = nullptr
        };

        VkImageMemoryBarrier                                        _barrier
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_NONE,
            .dstAccessMask = VK_ACCESS_NONE,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = VK_NULL_HANDLE,

            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0U,
                .levelCount = 1U,
                .baseArrayLayer = 0U,
                .layerCount = 1U
            }
        };

    public:
        RenderSession () = delete;

        RenderSession ( RenderSession const & ) = delete;
        RenderSession &operator = ( RenderSession const & ) = delete;

        RenderSession ( RenderSession && ) = delete;
        RenderSession &operator = ( RenderSession && ) = delete;

        explicit RenderSession ( MessageQueue &messageQueue,
            android_vulkan::Renderer &renderer,
            UIManager &uiManager
        ) noexcept;

        ~RenderSession () = default;

        void Init () noexcept;
        void Destroy () noexcept;

        // FUCK - remove it
        [[nodiscard]] pbr::android::FontStorage &GetFontStorage () noexcept;

        // FUCK - remove namespace
        [[nodiscard]] pbr::windows::FontStorage &GetFontStorageEXT () noexcept;

    private:
        [[nodiscard]] bool AllocateCommandBuffers ( VkDevice device ) noexcept;
        void FreeCommandBuffers ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateFramebuffer ( VkDevice device, VkExtent2D const &resolution ) noexcept;
        [[nodiscard]] bool CreateRenderPass ( VkDevice device ) noexcept;
        [[nodiscard]] bool CreateRenderTarget () noexcept;
        [[nodiscard]] bool CreateRenderTargetImage ( VkExtent2D const &resolution ) noexcept;
        void EventLoop () noexcept;
        [[nodiscard]] bool InitModules () noexcept;

        void OnHelloTriangleReady ( void* params ) noexcept;
        void OnRenderFrame () noexcept;
        void OnShutdown ( Message &&refund ) noexcept;
        void OnSwapchainCreated () noexcept;
        void OnUIAppendChildElement ( Message &&message ) noexcept;

        // FUCK - remove it
        void OnUIDeleteElement ( Message &&message ) noexcept;

        // FUCK - rename
        void OnUIDeleteElementEXT ( Message &&message ) noexcept;

        void OnUIElementCreated () noexcept;

        // FUCK - remove it
        void OnUIHideElement ( Message &&message ) noexcept;

        // FUCK - rename
        void OnUIHideElementEXT ( Message &&message ) noexcept;

        // FUCK - remove it
        void OnUIShowElement ( Message &&message ) noexcept;

        // FUCK - rename
        void OnUIShowElementEXT ( Message &&message ) noexcept;

        void OnUIPrependChildElement ( Message &&message ) noexcept;
        void OnUISetText ( Message &&message ) noexcept;

        // FUCK - remove it
        void OnUIUpdateElement ( Message &&message ) noexcept;

        // FUCK - rename
        void OnUIUpdateElementEXT ( Message &&message ) noexcept;

        void NotifyRecreateSwapchain () const noexcept;

        [[nodiscard]] static bool PrepareCommandBuffer ( VkDevice device, CommandInfo &info ) noexcept;
};

} // namespace editor


#endif // EDITOR_RENDER_SESSION_HPP
