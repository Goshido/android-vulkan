#ifndef EDITOR_RENDER_SESSION_HPP
#define EDITOR_RENDER_SESSION_HPP


#include "font_storage.hpp"
#include "hello_triangle_program.hpp"
#include "mesh_upload_info.hpp"
#include <platform/windows/mesh_geometry.hpp>
#include <platform/windows/pbr/exposure_pass.hpp>
#include <platform/windows/pbr/present_pass.hpp>
#include <platform/windows/pbr/tone_mapper_pass.hpp>
#include <platform/windows/pbr/ui_pass.hpp>
#include "resource_heap.hpp"
#include "texture2D_upload_info.hpp"
#include "ui_manager.hpp"
#include "workspace.hpp"


namespace editor {

class RenderSession final
{
    private:
        struct CommandInfo final
        {
            VkSemaphore                                     _acquire = VK_NULL_HANDLE;
            VkCommandBuffer                                 _buffer = VK_NULL_HANDLE;
            VkFence                                         _fence = VK_NULL_HANDLE;
            bool                                            _inUse = false;
            VkCommandPool                                   _pool = VK_NULL_HANDLE;
        };

        struct MeshStorage final
        {
            std::deque<MeshUploadInfo>                      _uploadQueue {};
            std::deque<MeshGeometryRef>                     _freeTransferQueue[ pbr::FIF_COUNT ] {};
            std::deque<MeshGeometryRef>                     _toDestroy {};
            std::deque<MeshGeometryRef>                     _destroyQueue[ pbr::FIF_COUNT ] {};
            size_t                                          _count = 0U;
        };

        struct Texture2DStorage final
        {
            std::deque<Texture2DUploadInfo>                 _uploadQueue {};
            std::deque<Texture2DRef>                        _freeTransferQueue[ pbr::FIF_COUNT ] {};
            std::deque<Texture2DRef>                        _toDestroy {};
            std::deque<Texture2DRef>                        _destroyQueue[ pbr::FIF_COUNT ] {};
            size_t                                          _count = 0U;
        };

        using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;
        using EnqueueHandle = void ( MessageQueue::* ) ( Message &&message ) noexcept;

    private:
        CommandInfo                                         _commandInfo[ pbr::FIF_COUNT ];
        size_t                                              _writingCommandInfo = 0U;

        pbr::ExposurePass                                   _exposurePass {};

        std::unique_ptr<HelloTriangleProgram>               _helloTriangleProgram {};
        std::unique_ptr<android_vulkan::MeshGeometry>       _helloTriangleGeometry {};

        pbr::PresentPass                                    _presentRenderPass {};

        android_vulkan::Texture2D                           _renderTarget {};
        uint32_t                                            _renderTargetIdx = 0U;

        MeshStorage                                         _meshStorage {};
        Texture2DStorage                                    _texture2DStorage {};
        ResourceHeap                                        _resourceHeap {};

        std::mutex                                          _submitMutex {};
        std::thread                                         _thread {};
        Timestamp                                           _timestamp {};

        pbr::ToneMapperPass                                 _toneMapper {};

        size_t                                              _uiElements = 0U;

        pbr::UIPass                                         _uiPass { ResourceHeap::Instance () };
        FontStorage                                         _fontStorage { _uiPass.GetFontStorage () };

        VkViewport                                          _viewport {};
        Workspace                                           &_workspace;
        UIManager                                           &_uiManager;

        bool                                                _broken = false;

        // When app is in shutdown state the enqueue back mush be switched to enqueue front to avoid deadlock.
        EnqueueHandle                                       _enqueueHandle = &MessageQueue::EnqueueBack;

        VkRenderingAttachmentInfo                           _colorAttachment
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

        VkRenderingInfo                                     _renderingInfo
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

        VkImageMemoryBarrier                                _barrier
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

        explicit RenderSession ( UIManager &uiManager, Workspace &workspace ) noexcept;

        ~RenderSession () = default;

        void Init () noexcept;
        void Destroy () noexcept;

    private:
        [[nodiscard]] bool AllocateCommandBuffers ( VkDevice device ) noexcept;
        void FreeCommandBuffers ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateRenderTarget () noexcept;
        [[nodiscard]] bool CreateRenderTargetImage ( VkExtent2D const &resolution ) noexcept;
        void EventLoop () noexcept;
        [[nodiscard]] bool InitModules () noexcept;

        void FreeMeshTransferQueue ( MessageQueue &messageQueue, size_t commandBufferIndex ) noexcept;
        void FreeTexture2DTransferQueue ( MessageQueue &messageQueue, size_t commandBufferIndex ) noexcept;

        void DestroyMeshes (  MessageQueue &messageQueue, size_t commandBufferIndex ) noexcept;
        void DestroyTexture2DInstances ( MessageQueue &messageQueue, size_t commandBufferIndex ) noexcept;

        void UploadMeshes ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept;
        void UploadTexture2DInstances ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept;

        void OnDestroyMesh ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnDestroyTexture2D ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnHelloTriangleReady ( void* params ) noexcept;
        void OnInvokeRenderSession ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnRenderFrame ( MessageQueue &messageQueue ) noexcept;
        void OnShutdown ( MessageQueue &messageQueue, Message &&refund ) noexcept;
        void OnSwapchainCreated ( MessageQueue &messageQueue ) noexcept;
        void OnUIAppendChildElement ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnUIDeleteElement ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnUIElementCreated ( MessageQueue &messageQueue ) noexcept;
        void OnUIHideElement ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnUIShowElement ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnUIPrependChildElement ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnUISetText ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnUIUpdateElement ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnUploadMesh ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnUploadTexture2D ( MessageQueue &messageQueue, Message &&message ) noexcept;

        void NotifyRecreateSwapchain ( MessageQueue &messageQueue ) const noexcept;

        [[nodiscard]] static bool PrepareCommandBuffer ( VkDevice device, CommandInfo &info ) noexcept;
};

} // namespace editor


#endif // EDITOR_RENDER_SESSION_HPP
