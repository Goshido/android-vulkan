#ifndef EDITOR_SELECTION_HPP
#define EDITOR_SELECTION_HPP


#include "actor.hpp"
#include "message_queue.hpp"
#include <platform/windows/pbr/id_collect_program.hpp>
#include <platform/windows/pbr/id_compress_program.hpp>
#include "rect.hpp"
#include <renderer.hpp>
#include <texture2D.hpp>


namespace editor {

class Selection final
{
    public:
        enum class eMode : uint8_t
        {
            Add,
            New,
            Remove,
            Toggle
        };

        using Items = std::unordered_set<Actor*>;

    private:
        class Buffer final
        {
            public:
                VkBuffer                            _buffer = VK_NULL_HANDLE;
                VkDeviceMemory                      _memory = VK_NULL_HANDLE;
                VkDeviceSize                        _offset = std::numeric_limits<VkDeviceSize>::max ();
                uint64_t*                           _data = nullptr;
                std::optional<uint32_t>             _resourceIdx = std::nullopt;

            public:
                explicit Buffer () = default;

                Buffer ( Buffer const & ) = delete;
                Buffer &operator = ( Buffer const & ) = delete;

                Buffer ( Buffer && ) = delete;
                Buffer &operator = ( Buffer && ) = delete;

                ~Buffer () = default;

                [[nodiscard]] bool Init (  android_vulkan::Renderer &renderer,
                    size_t size,
                    VkBufferUsageFlags usage,
                    bool map,
                    char const* name
                ) noexcept;

                void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        };

    private:
        constexpr static size_t ID_PREFETCH_ADDRESSES = 64UZ;

        constexpr static auto ID_PREFETCH_SIZE =
            static_cast<VkDeviceSize> ( sizeof ( uint64_t ) + ID_PREFETCH_ADDRESSES * sizeof ( uint64_t ) );

        std::unique_ptr<pbr::IDCollectProgram>      _idCollectProgram {};
        std::unique_ptr<pbr::IDCompressProgram>     _idCompressProgram {};

        Buffer                                      _idSet {};
        Buffer                                      _idDevice {};
        Buffer                                      _idHost[ 2U ];

        Buffer*                                     _free[ 2U ] = { _idHost, _idHost + 1U };
        Buffer*                                     _counting = nullptr;
        Buffer*                                     _ready = nullptr;

        std::vector<Actor*>                         _lastSelection {};
        Items                                       _items {};
        Items                                       _lastItems {};
        VkOffset2D                                  _begin {};
        std::optional<Rect>                         _area = std::nullopt;

        GXVec4                                      _areaConv {};
        eMode                                       _lastMode = eMode::New;

        VkExtent2D                                  _idImageResolution
        {
            .width = 0U,
            .height = 0U
        };

        pbr::IDCollectProgram::PushConstants        _collectPushConstants
        {
            ._idImage = 0U,

            ._offset
            {
                .width = 0U,
                .height = 0U
            },

            ._size
            {
                .width = 0U,
                .height = 0U
            },

            ._idSet = 0U,
            ._capacity = 0U
        };

        pbr::IDCompressProgram::PushConstants       _compressPushConstants
        {
            ._idSet = 0U,
            ._uniqueIDs = 0U,
            ._capacity = 0U
        };

        VkDependencyInfo                            _depInfo
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

        VkBufferMemoryBarrier2                      _idSetBarrier
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = VK_NULL_HANDLE,
            .offset = 0U,
            .size = VK_WHOLE_SIZE
        };

        VkBufferMemoryBarrier2                      _idDeviceBarrier
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = VK_NULL_HANDLE,
            .offset = 0U,
            .size = ID_PREFETCH_SIZE
        };

        VkBufferMemoryBarrier2                      _idHostBarrier0
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT,
            .srcAccessMask = VK_ACCESS_2_HOST_READ_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = VK_NULL_HANDLE,
            .offset = ID_PREFETCH_SIZE,
            .size = 0U
        };

        VkBufferMemoryBarrier2                      _idHostBarrier1
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT,
            .dstAccessMask = VK_ACCESS_2_HOST_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = VK_NULL_HANDLE,
            .offset = 0U,
            .size = ID_PREFETCH_SIZE
        };

        VkBufferMemoryBarrier2                      _barriers0[ 3U ]
        {
            // ID Set
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = VK_NULL_HANDLE,
                .offset = 0U,
                .size = VK_WHOLE_SIZE
            },
            // ID (device)
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,

                .srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT |
                    VK_ACCESS_2_SHADER_READ_BIT |
                    VK_ACCESS_2_SHADER_WRITE_BIT,

                .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = VK_NULL_HANDLE,
                .offset = 0U,
                .size = static_cast<VkDeviceSize> ( sizeof ( uint64_t ) )
            },
            // ID (host)
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT,
                .dstAccessMask = VK_ACCESS_2_HOST_READ_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = VK_NULL_HANDLE,
                .offset = ID_PREFETCH_SIZE,
                .size = 0U
            }
        };

        VkBufferMemoryBarrier2                      _barriers1[ 3U ]
        {
            // ID Set
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = VK_NULL_HANDLE,
                .offset = 0U,
                .size = VK_WHOLE_SIZE
            },
            // ID (device)
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = VK_NULL_HANDLE,
                .offset = 0U,
                .size = VK_WHOLE_SIZE
            },
            // ID (host)
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT | VK_ACCESS_2_HOST_READ_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = VK_NULL_HANDLE,
                .offset = 0U,
                .size = ID_PREFETCH_SIZE
            }
        };

    public:
        Selection () = default;

        Selection ( Selection const & ) = delete;
        Selection &operator = ( Selection const & ) = delete;

        Selection ( Selection && ) = delete;
        Selection &operator = ( Selection && ) = delete;

        ~Selection () = default;

        [[nodiscard]] bool Init ( MessageQueue &messageQueue, android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( MessageQueue &messageQueue, android_vulkan::Renderer &renderer ) noexcept;
        [[nodiscard]] bool IsReady () const noexcept;
        [[nodiscard]] uint32_t GetIDImageResourceIndex () const noexcept;

        void PrepareIDBuffer ( VkCommandBuffer commandBuffer ) noexcept;
        void OnGBufferResolutionChanged ( android_vulkan::Texture2D &idImage, uint32_t idResourceIdx ) noexcept;

        void Begin ( VkOffset2D const &mouse, eMode mode ) noexcept;
        [[nodiscard]] std::optional<Rect> Update ( VkOffset2D const &mouse, eMode mode ) noexcept;
        void End ( VkOffset2D const &mouse, eMode mode ) noexcept;

        [[nodiscard]] bool IsSelectionRequested () const noexcept;
        [[nodiscard]] bool HasSelection () const noexcept;
        void ComputeSelect ( VkCommandBuffer commandBuffer ) noexcept;
        void CommitSelect () noexcept;
        void CommitArea ( Rect &&canvasArea, eMode mode ) noexcept;

        void ProcessAdd ( Items &&selected ) noexcept;
        void ProcessNew ( Items &&selected ) noexcept;
        void ProcessRemove ( Items &&selected ) noexcept;
        void ProcessToggle ( std::vector<Actor*> const &selected ) noexcept;
};

} // namespace editor


#endif // EDITOR_SELECTION_HPP
