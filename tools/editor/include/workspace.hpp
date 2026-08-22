#ifndef EDITOR_WORKSPACE_HPP
#define EDITOR_WORKSPACE_HPP


#include "gbuffer_mesh_node.hpp"
#include "gizmo_node.hpp"
#include "history.hpp"
#include "mesh_geometry_ref.hpp"
#include "outline_mesh_node.hpp"
#include <platform/windows/pbr/gizmo_compose_program.hpp>
#include <platform/windows/pbr/gizmo_prepass_program.hpp>
#include <platform/windows/pbr/gpu_buffer.hpp>
#include <platform/windows/pbr/opaque_program.hpp>
#include <platform/windows/pbr/opaque_with_id_program.hpp>
#include <platform/windows/pbr/outline_blur_x_program.hpp>
#include <platform/windows/pbr/outline_border_program.hpp>
#include <platform/windows/pbr/outline_mask_program.hpp>
#include <platform/windows/pbr/swapchain_info.hpp>
#include "point_light_node.hpp"
#include "reflection_probe_global_node.hpp"
#include "reflection_probe_local_node.hpp"
#include "selection.hpp"
#include "stream_buffer_ref.hpp"
#include "viewport_widget.hpp"


namespace editor {

class Workspace final
{
    private:
        constexpr static size_t WORKERS = 4U;

        using GBufferMeshes = std::deque<GBufferMeshInfo*>;
        using GBufferMeshQueue = std::unordered_map<MeshGeometryRef, GBufferMeshes>;
        using GBufferMeshMap = std::unordered_map<GBufferMeshInfo const*, MeshGeometryRef::weak_type>;

        using OutlineMeshes = std::deque<OutlineMeshInfo*>;
        using OutlineMeshQueue = std::unordered_map<MeshGeometryRef, OutlineMeshes>;
        using OutlineMeshMap = std::unordered_map<OutlineMeshInfo const*, MeshGeometryRef::weak_type>;

        using GizmoQueue = std::deque<GizmoInfo*>;

        using PointLightQueue = std::unordered_set<PointLightInfo*>;
        using ReflectionProbeLocalQueue = std::unordered_set<ReflectionProbeLocalInfo*>;
        using ReflectionProbeGlobalQueue = std::unordered_set<ReflectionProbeGlobalInfo*>;

        struct MeshInstance final
        {
            android_vulkan::MeshGeometry*               _mesh = nullptr;
            uint32_t                                    _count = 0U;
        };

    private:
        History                                         _history {};
        Selection                                       _selection {};
        std::unordered_map<Actor const*, ActorRef>      _actors {};

        GBufferMeshQueue                                _opaqueQueue {};
        GBufferMeshMap                                  _opaqueMap {};
        std::vector<MeshInstance>                       _opaqueVisible {};

        GBufferMeshQueue                                _stippleQueue {};
        GBufferMeshMap                                  _stippleMap {};
        std::vector<MeshInstance>                       _stippleVisible {};

        OutlineMeshQueue                                _outlineQueue {};
        OutlineMeshMap                                  _outlineMap {};
        std::vector<MeshInstance>                       _outlineVisible {};

        GizmoQueue                                      _gizmoQueue {};
        size_t                                          _gizmoVisible = 0U;

        PointLightQueue                                 _pointLightQueue {};
        ReflectionProbeLocalQueue                       _reflectionProbeLocalQueue {};
        ReflectionProbeGlobalQueue                      _reflectionProbeGlobalQueue {};

        std::unique_ptr<pbr::GizmoComposeProgram>       _gizmoComposeProgram {};
        std::unique_ptr<pbr::GizmoPrepassProgram>       _gizmoPrepassProgram {};
        std::unique_ptr<pbr::OpaqueProgram>             _opaqueProgram {};
        std::unique_ptr<pbr::OpaqueWithIDProgram>       _opaqueWithIDProgram {};
        std::unique_ptr<pbr::OutlineBlurXProgram>       _outlineBlurXProgram {};
        std::unique_ptr<pbr::OutlineBorderProgram>      _outlineBorderProgram {};
        std::unique_ptr<pbr::OutlineMaskProgram>        _outlineMaskProgram {};

        pbr::OutlineBorderProgram::PushConstants        _outlineBorderPushConstants {};
        pbr::OutlineBlurXProgram::PushConstants         _outlineBlurXPushConstants {};
        pbr::GizmoPrepassProgram::PushConstants         _gizmoPrepassPushConstants {};
        pbr::GizmoComposeProgram::PushConstants         _gizmoComposePushConstants {};

        StreamBufferRef                                 _frameStream {};
        std::optional<VkDeviceAddress>                  _frameInstance = std::nullopt;

        StreamBufferRef                                 _transformStream {};
        StreamBufferRef                                 _shadingStream {};
        StreamBufferRef                                 _idStream {};
        StreamBufferRef                                 _outlineStream {};
        StreamBufferRef                                 _sdfVertexStream {};
        StreamBufferRef                                 _sdfPixelStream {};
        StreamBufferRef                                 _sdfShapeStream {};

        Texture2DRef                                    _defaultAlbedo {};
        Texture2DRef                                    _defaultEmission {};
        Texture2DRef                                    _defaultMask {};
        Texture2DRef                                    _defaultParam {};
        Texture2DRef                                    _defaultNormal {};

        Texture2DRef                                    _swapchainDepth {};
        Texture2DRef                                    _idMask {};
        Texture2DRef                                    _border {};
        Texture2DRef                                    _blurX {};

        pbr::GPUBuffer                                  _tileCounters {};
        pbr::GPUBuffer                                  _tileSamples {};

        VkViewport                                      _idViewport {};

        VkExtent3D                                      _outlineDispatch {};
        VkExtent3D                                      _gizmoComposeDispatch {};

        ViewportWidget*                                 _viewport = nullptr;
        std::mutex                                      _mutex {};

        Hotkey                                          _delete {};
        Hotkey                                          _openWorkspace {};
        Hotkey                                          _saveWorkspace {};
        Hotkey                                          _saveAsWorkspace {};
        Hotkey                                          _undo {};
        Hotkey                                          _redo {};

        bool                                            _ready = false;

        VkRenderingAttachmentInfo                       _idMaskAttachment
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
        };

        VkRenderingAttachmentInfo                       _idDepthAttachment
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = VK_NULL_HANDLE,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

            .clearValue
            {
                .depthStencil
                {
                    .depth = 0.0F,
                    .stencil = 0U
                }
            }
        };

        VkRenderingInfo                                 _idRenderingInfo
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
            .pColorAttachments = &_idMaskAttachment,
            .pDepthAttachment = &_idDepthAttachment,
            .pStencilAttachment = nullptr
        };

        VkImageMemoryBarrier2                           _outlineBarrier0[ 2U ] =
        {
            // ID mask
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
            // Swapchain depth
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,

                .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,

                .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
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
            }
        };

        VkImageMemoryBarrier2                           _outlineBarrier1[ 2U ] =
        {
            // ID mask
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
            // Border
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
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

        VkImageMemoryBarrier2                           _outlineBarrier2[ 2U ] =
        {
            // Border
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
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
            // BlurX
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
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

        VkImageMemoryBarrier2                           _blurXBarrier
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
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
        };

        VkBufferMemoryBarrier2                          _tileBarriers[ 2U ]
        {
            // Tile counters
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = VK_NULL_HANDLE,
                .offset = 0U,
                .size = VK_WHOLE_SIZE
            },
            // Tile samplers
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = VK_NULL_HANDLE,
                .offset = 0U,
                .size = VK_WHOLE_SIZE
            }
        };

        VkBufferMemoryBarrier2                          _tileCounterBarrier
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccessMask =  VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = VK_NULL_HANDLE,
            .offset = 0U,
            .size = VK_WHOLE_SIZE
        };

        VkImageMemoryBarrier2                           _gizmoBarriers0[ 2U ] =
        {
            // Swapchain color
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,

                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT |
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,

                .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT |
                    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,

                .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
            // Swapchain depth
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,

                .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR |
                    VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,

                .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                    VK_ACCESS_2_SHADER_READ_BIT,

                .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR |
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
            }
        };

        VkImageMemoryBarrier2                           _gizmoBarriers1[ 2U ] =
        {
            // Swapchain color
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
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
            // Swapchain depth
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext = nullptr,

                .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR |
                    VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,

                .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,

                .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
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
            }
        };

        VkImageMemoryBarrier2                           _gizmoEndBarrier
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
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
        };

        VkDependencyInfo                                _depInfo
        {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = 0U,
            .memoryBarrierCount = 0U,
            .pMemoryBarriers = nullptr,
            .bufferMemoryBarrierCount = 0U,
            .pBufferMemoryBarriers = nullptr,
            .imageMemoryBarrierCount = 0U,
            .pImageMemoryBarriers = nullptr
        };

        VkRenderingAttachmentInfo                       _gizmoColorAttachment
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = VK_NULL_HANDLE,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,

            .clearValue
            {
                .color
                {
                    .float32 { 0.0F, 0.0F, 0.0F, 0.0F }
                }
            }
        };

        VkRenderingAttachmentInfo                       _gizmoDepthAttachment
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
                    .depth = 1.0F,
                    .stencil = 0U
                }
            }
        };

        VkRenderingInfo                                 _gizmoRenderingInfo
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
            .pColorAttachments = &_gizmoColorAttachment,
            .pDepthAttachment = &_gizmoDepthAttachment,
            .pStencilAttachment = nullptr
        };

        static Workspace*                               _instance;

    public:
        explicit Workspace () noexcept;

        Workspace ( Workspace const & ) = delete;
        Workspace &operator = ( Workspace const & ) = delete;

        Workspace ( Workspace && ) = delete;
        Workspace &operator = ( Workspace && ) = delete;

        ~Workspace () = default;

        void Init () noexcept;
        void Destroy () noexcept;

        void Load ( std::string_view scene ) noexcept;
        void Close () noexcept;

        void UploadGPUData ( VkCommandBuffer commandBuffer, float deltaTime ) noexcept;
        void PrepareIDBuffer ( VkCommandBuffer commandBuffer ) noexcept;
        void FillGBuffer ( VkCommandBuffer commandBuffer ) noexcept;

        void PrepareGizmo ( VkCommandBuffer commandBuffer ) noexcept;
        void DrawGizmo ( VkCommandBuffer commandBuffer, pbr::SwapchainInfo const &swapchain ) noexcept;
        [[nodiscard]] bool HasGizmo () const noexcept;

        void DrawOutline ( VkCommandBuffer commandBuffer ) noexcept;
        [[nodiscard]] std::optional<uint32_t> GetOutlineBlurX () noexcept;

        void OnGBufferResolutionChanged ( android_vulkan::Texture2D &idImage, uint32_t idResourceIdx ) noexcept;

        void ComputeSelect ( VkCommandBuffer commandBuffer ) noexcept;
        [[nodiscard]] Selection &GetSelection () noexcept;

        [[nodiscard]] GBufferMeshNode RegisterOpaqueMesh ( MeshGeometryRef &mesh ) noexcept;
        [[nodiscard]] GBufferMeshNode RegisterStippleMesh ( MeshGeometryRef &mesh ) noexcept;
        [[nodiscard]] OutlineMeshNode RegisterOutline ( MeshGeometryRef &mesh ) noexcept;
        [[nodiscard]] GizmoNode RegisterGizmo ( eSDFShape shape, GizmoNode::UpdateHandler &&update ) noexcept;
        [[nodiscard]] PointLightNode RegisterPointLight () noexcept;
        [[nodiscard]] ReflectionProbeLocalNode RegisterReflectionProbeLocal () noexcept;
        [[nodiscard]] ReflectionProbeGlobalNode RegisterReflectionProbeGlobal () noexcept;

        void UnregisterOpaque ( GBufferMeshNode &node ) noexcept;
        void UnregisterStipple ( GBufferMeshNode &node ) noexcept;
        void UnregisterOutline ( OutlineMeshNode &node ) noexcept;
        void Unregister ( GizmoNode &node ) noexcept;
        void Unregister ( PointLightNode &node ) noexcept;
        void Unregister ( ReflectionProbeLocalNode &node ) noexcept;
        void Unregister ( ReflectionProbeGlobalNode &node ) noexcept;

        [[nodiscard]] static Workspace &Instance () noexcept;

    private:
        [[nodiscard]] VkDeviceAddress AcquireFrameInstance () noexcept;
        void FreeSwapchainResources () noexcept;

        void FUCK () noexcept;

        void ComputeTransformGBufferOnly ( GXProjectionClipPlanes const &frustum ) noexcept;
        void ComputeTransformGBufferWithID ( GXProjectionClipPlanes const &frustum ) noexcept;
        void ComputeTransformOutline ( GXProjectionClipPlanes const &frustum ) noexcept;
        void ComputeTransformGizmo ( GXMat4 const &viewProjection, GXMat4 const &cameraLocal ) noexcept;

        void FillGBufferOnly ( VkCommandBuffer commandBuffer ) noexcept;
        void FillGBufferWithID ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] bool IsReady () noexcept;

        void InitGraphicsResources () noexcept;
        void InitHotkeys () noexcept;
        void InitWidgets () noexcept;

        [[nodiscard]] GBufferMeshNode RegisterMesh ( MeshGeometryRef &mesh,
            GBufferMeshQueue &meshQueue,
            GBufferMeshMap &meshMap,
            GBufferMeshInfo &node
        ) noexcept;

        void UnregisterMesh ( GBufferMeshQueue &meshQueue,
            GBufferMeshMap &meshMap,
            GBufferMeshNode &node
        ) noexcept;
};

} // namespace editor


#endif // EDITOR_WORKSPACE_HPP
