#ifndef EDITOR_RENDER_SESSION_HPP
#define EDITOR_RENDER_SESSION_HPP


#include "font_storage.hpp"
#include "program_ref.hpp"
#include "mesh_upload_info.hpp"
#include <platform/windows/pbr/exposure_pass.hpp>
#include <platform/windows/pbr/present_pass.hpp>
#include <platform/windows/pbr/tone_mapper_pass.hpp>
#include <platform/windows/pbr/ui_pass.hpp>
#include "resource_heap.hpp"
#include "stream_buffer_ref.hpp"
#include "texture2D_upload_info.hpp"
#include "ui_manager.hpp"
#include "workspace.hpp"


namespace editor {

class RenderSession final
{
    private:
        struct CommandInfo final
        {
            VkSemaphore                         _acquire = VK_NULL_HANDLE;
            VkCommandBuffer                     _buffer = VK_NULL_HANDLE;
            VkFence                             _fence = VK_NULL_HANDLE;
            bool                                _inUse = false;
            VkCommandPool                       _pool = VK_NULL_HANDLE;
        };

        struct MeshStorage final
        {
            std::deque<MeshUploadInfo>          _uploadQueue {};
            std::deque<MeshGeometryRef>         _freeTransferQueue[ pbr::FIF_COUNT ] {};
            std::deque<MeshGeometryRef>         _toDestroy {};
            std::deque<MeshGeometryRef>         _destroyQueue[ pbr::FIF_COUNT ] {};
            size_t                              _count = 0U;
        };

        struct Texture2DStorage final
        {
            std::deque<Texture2DUploadInfo>     _uploadQueue {};
            std::deque<Texture2DRef>            _freeTransferQueue[ pbr::FIF_COUNT ] {};
            std::deque<Texture2DRef>            _toDestroy {};
            std::deque<Texture2DRef>            _destroyQueue[ pbr::FIF_COUNT ] {};
            size_t                              _count = 0U;
        };

        struct ProgramStorage final
        {
            std::deque<ProgramRef>              _toDestroy {};
            std::deque<ProgramRef>              _destroyQueue[ pbr::FIF_COUNT ] {};
            size_t                              _count = 0U;
        };

        struct StreamBufferStorage final
        {
            std::deque<StreamBufferRef>         _toDestroy {};
            std::deque<StreamBufferRef>         _destroyQueue[ pbr::FIF_COUNT ] {};
            size_t                              _count = 0U;
        };

        using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;
        using EnqueueHandle = void ( MessageQueue::* ) ( Message &&message ) noexcept;

    private:
        CommandInfo                             _commandInfo[ pbr::FIF_COUNT ];
        size_t                                  _writingCommandInfo = 0U;

        pbr::ExposurePass                       _exposurePass {};
        pbr::PresentPass                        _presentRenderPass {};

        android_vulkan::Texture2D               _albedoRenderTarget {};
        std::optional<uint32_t>                 _albedoRenderTargetIdx = std::nullopt;

        android_vulkan::Texture2D               _hdrRenderTarget {};
        std::optional<uint32_t>                 _hdrRenderTargetIdx = std::nullopt;

        android_vulkan::Texture2D               _normalRenderTarget {};
        std::optional<uint32_t>                 _normalRenderTargetIdx = std::nullopt;

        android_vulkan::Texture2D               _paramRenderTarget {};
        std::optional<uint32_t>                 _paramRenderTargetIdx = std::nullopt;

        android_vulkan::Texture2D               _idRenderTarget {};
        std::optional<uint32_t>                 _idRenderTargetIdx = std::nullopt;

        android_vulkan::Texture2D               _depthRenderTarget {};
        std::optional<uint32_t>                 _depthRenderTargetIdx = std::nullopt;

        ProgramStorage                          _programStorage {};
        MeshStorage                             _meshStorage {};
        StreamBufferStorage                     _streamBufferStorage {};
        Texture2DStorage                        _texture2DStorage {};
        ResourceHeap                            _resourceHeap {};

        std::mutex                              _submitMutex {};
        std::thread                             _thread {};
        Timestamp                               _timestamp {};

        pbr::ToneMapperPass                     _toneMapper {};

        size_t                                  _uiElements = 0U;

        pbr::UIPass                             _uiPass { ResourceHeap::Instance () };
        FontStorage                             _fontStorage { _uiPass.GetFontStorage () };

        VkViewport                              _viewport {};
        Workspace                               &_workspace;
        UIManager                               &_uiManager;

        // When app is in shutdown state the enqueue back must be switched to enqueue front to avoid deadlock.
        EnqueueHandle                           _enqueueHandle = &MessageQueue::EnqueueBack;

        bool                                    _broken = false;

        VkRenderingAttachmentInfo               _colorAttachments[ 5U ] =
        {
            // Albedo
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
                        .float32 { 0.0F, 0.0F, 0.0F, 0.0F }
                    }
                }
            },
            // HDR
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
                        .float32 { 0.0F, 0.0F, 0.0F, 1.0F }
                    }
                }
            },
            // Normal
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
                        .float32 { 0.5F, 0.5F, 0.5F, 0.0F }
                    }
                }
            },
            // Param
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
                        .float32 { 0.5F, 0.5F, 0.5F, 0.0F }
                    }
                }
            },
            // ID
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
                        .uint32 { 0U, 0U, 0U, 0U }
                    }
                }
            }
        };

        VkRenderingAttachmentInfo               _depthAttachment
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = VK_NULL_HANDLE,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,

            .clearValue
            {
                .depthStencil
                {
                    .depth = 0.0F,
                    .stencil = 0U
                }
            }
        };

        VkRenderingInfo                         _renderingInfo
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
            .colorAttachmentCount = static_cast<uint32_t> ( std::size ( _colorAttachments ) ),
            .pColorAttachments = _colorAttachments,
            .pDepthAttachment = &_depthAttachment,
            .pStencilAttachment = nullptr
        };

        VkImageMemoryBarrier2                   _barrierStart[ 6U ] =
        {
            // Albedo
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
            },
            // HDR
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
            },
            // Normal
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .pNext = nullptr,
                    .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
            },
            // Param
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
            },
            // Depth
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,

                .srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,

                .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT |
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,

                .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,

                .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,

                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = VK_NULL_HANDLE,

                .subresourceRange
                {
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    .baseMipLevel = 0U,
                    .levelCount = 1U,
                    .baseArrayLayer = 0U,
                    .layerCount = 1U
                }
            },
            // ID
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
            }
        };

        VkImageMemoryBarrier2                   _barrierFinish[ 6U ] =
        {
            // Albedo
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
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
            },
            // HDR
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_GENERAL,
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
            },
            // Normal
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .pNext = nullptr,
                    .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                    .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
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
            },
            // Param
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
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
            },
            // Depth
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,

                .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,

                .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,

                .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = VK_NULL_HANDLE,

                .subresourceRange
                {
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    .baseMipLevel = 0U,
                    .levelCount = 1U,
                    .baseArrayLayer = 0U,
                    .layerCount = 1U
                }
            },
            // ID
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_GENERAL,
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
            }
        };

        VkDependencyInfo                        _depInfo
        {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = 0U,
            .memoryBarrierCount = 0U,
            .pMemoryBarriers = nullptr,
            .bufferMemoryBarrierCount = 0U,
            .pBufferMemoryBarriers = nullptr,
            .imageMemoryBarrierCount = 6U,
            .pImageMemoryBarriers = _barrierStart
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

        [[nodiscard]] bool CreateRenderTargets () noexcept;
        [[nodiscard]] bool CreateRenderTargetImages ( VkExtent2D const &resolution ) noexcept;
        void EventLoop () noexcept;
        [[nodiscard]] bool InitModules () noexcept;

        void FreeMeshTransferQueue ( MessageQueue &messageQueue, size_t fif ) noexcept;
        void FreeTexture2DTransferQueue ( MessageQueue &messageQueue, size_t fif ) noexcept;

        void DestroyPrograms ( MessageQueue &messageQueue, size_t fif ) noexcept;
        void DestroyMeshes ( MessageQueue &messageQueue, size_t fif ) noexcept;
        void DestroyStreamBuffers ( MessageQueue &messageQueue, size_t fif ) noexcept;
        void DestroyTexture2DInstances ( MessageQueue &messageQueue, size_t fif ) noexcept;

        void UploadMeshes ( VkCommandBuffer commandBuffer, size_t fif ) noexcept;
        void UploadTexture2DInstances ( VkCommandBuffer commandBuffer, size_t fif ) noexcept;

        void RenderScene ( VkCommandBuffer commandBuffer ) noexcept;
        void RenderSceneWithID ( VkCommandBuffer commandBuffer ) noexcept;

        void OnDestroyMesh ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnDestroyProgram ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnDestroyStreamBuffer ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnDestroyTexture2D ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnInvokeRenderSession ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnNewProgram ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnNewStreamBuffer ( MessageQueue &messageQueue, Message &&message ) noexcept;
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
