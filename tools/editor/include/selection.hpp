#ifndef EDITOR_SELECTION_HPP
#define EDITOR_SELECTION_HPP


#include "actor.hpp"
#include "message_queue.hpp"
#include <platform/windows/pbr/id_collect_program.hpp>
#include <platform/windows/pbr/id_compress_program.hpp>
#include <rect.hpp>
#include <renderer.hpp>
#include <texture2D.hpp>


namespace editor {

class Selection final
{
    private:
        class Buffer final
        {
            public:
                VkDeviceMemory                      _memory = VK_NULL_HANDLE;
                VkDeviceSize                        _offset = std::numeric_limits<VkDeviceSize>::max ();
                uint64_t*                           _data = nullptr;
                std::optional<uint32_t>             _resourceIdx = std::nullopt;

                VkBufferMemoryBarrier               _barrier
                {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_NONE,
                    .dstAccessMask = VK_ACCESS_NONE,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .buffer = VK_NULL_HANDLE,
                    .offset = 0U,
                    .size = VK_WHOLE_SIZE
                };

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
        std::unique_ptr<pbr::IDCollectProgram>      _idCollectProgram {};
        std::unique_ptr<pbr::IDCompressProgram>     _idCompressProgram {};

        Buffer                                      _idSet {};
        Buffer                                      _idDevice {};
        Buffer                                      _idHost[ 2U ];

        Buffer*                                     _free[ 2U ] = { _idHost, _idHost + 1U };
        Buffer*                                     _counting = nullptr;
        Buffer*                                     _ready = nullptr;

        std::vector<Actor const*>                   _lastSelection {};
        std::deque<Actor const*>                    _items {};
        bool                                        _pendingSelect = false;

        VkExtent2D                                  _idImageResolution
        {
            .width = 0U,
            .height = 0U
        };

        pbr::IDCollectProgram::PushConstants        _collectPushConstants
        {
            ._idImage = 0U,
            ._idSet = 0U,
            ._capacity = 0U
        };

        pbr::IDCompressProgram::PushConstants       _compressPushConstants
        {
            ._idSet = 0U,
            ._uniqueIDs = 0U,
            ._capacity = 0U
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

        [[nodiscard]] bool IsSelectionRequested () const noexcept;
        [[nodiscard]] bool HasSelection () const noexcept;
        void Select ( Rect const &rect, bool invert ) noexcept;
        void ComputeSelect ( VkCommandBuffer commandBuffer ) noexcept;
};

} // namespace editor


#endif // EDITOR_SELECTION_HPP
