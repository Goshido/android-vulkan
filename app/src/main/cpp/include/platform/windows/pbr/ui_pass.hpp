#ifndef PBR_UI_PASS_HPP
#define PBR_UI_PASS_HPP


#include "font_storage.hpp"
#include "ui_program.hpp"
#include "ui_vertex_info.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace pbr {

class UIPass final
{
    public:
        using UIBufferResponse = std::optional<UIBufferStreams>;

        struct Image final
        {
            uint16_t                        _image = ResourceHeap::INVALID_UI_IMAGE;
            VkExtent2D                      _resolution {};
        };

    private:
        struct Buffer final
        {
            VkBuffer                        _buffer = VK_NULL_HANDLE;
            VkDeviceMemory                  _memory = VK_NULL_HANDLE;
            char const*                     _name = nullptr;
            VkDeviceSize                    _memoryOffset = 0U;

            [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
                size_t size,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags memoryProperties,
                char const* name
            ) noexcept;

            void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        };

        class BufferStream final
        {
            private:
                VkBufferMemoryBarrier       _barriers[ 2U ]
                {
                    {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                        .pNext = nullptr,
                        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .buffer = VK_NULL_HANDLE,
                        .offset = 0U,
                        .size = 0U
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                        .pNext = nullptr,
                        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .buffer = VK_NULL_HANDLE,
                        .offset = 0U,
                        .size = 0U
                    }
                };

                uint8_t*                    _data = nullptr;

                Buffer                      _staging {};
                Buffer                      _gpuBuffer {};

                VkDeviceAddress             _bdaStream0 = 0U;
                VkDeviceAddress             _bdaStream1 = 0U;

            public:
                explicit BufferStream () = default;

                BufferStream ( BufferStream const & ) = delete;
                BufferStream &operator = ( BufferStream const & ) = delete;

                BufferStream ( BufferStream && ) = delete;
                BufferStream &operator = ( BufferStream && ) = delete;

                ~BufferStream () = default;

                [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
                void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

                [[nodiscard]] VkDeviceAddress GetStream0Address () const noexcept;
                [[nodiscard]] VkDeviceAddress GetStream1Address () const noexcept;
                [[nodiscard]] UIBufferStreams GetData ( size_t startIndex, size_t neededVertices ) const noexcept;
                void UpdateGeometry ( VkCommandBuffer commandBuffer, size_t readIdx, size_t writeIdx ) noexcept;
        };

        struct InUseImageTracker final
        {
            using Entry = std::unordered_map<uint16_t, size_t>;

            Entry                           _registry[ FIF_COUNT ];

            void Destroy () noexcept;
            void CollectGarbage ( size_t commandBufferIndex ) noexcept;
            void MarkInUse ( uint16_t image, size_t commandBufferIndex ) noexcept;
        };

    private:
        GXVec2                              _bottomRight {};
        float                               _brightnessBalance = 0.0F;

        VkExtent2D                          _currentResolution
        {
            .width = 0U,
            .height = 0U
        };

        uint16_t                            _textLUT = ResourceHeap::INVALID_UI_IMAGE;
        InUseImageTracker                   _inUseImageTracker {};

        size_t                              _readVertexIndex = 0U;
        size_t                              _writeVertexIndex = 0U;
        uint32_t                            _vertices = 0U;
        std::vector<uint16_t>               _usedImages {};

        FontStorage                         _fontStorage;
        ResourceHeap                        &_resourceHeap;

        bool                                _hasChanges = false;
        bool                                _isTransformChanged = false;

        BufferStream                        _uiVertices {};

        UIProgram                           _program {};
        UIProgram::PushConstants            _pushConstants {};

    public:
        UIPass () = delete;

        UIPass ( UIPass const & ) = delete;
        UIPass &operator = ( UIPass const & ) = delete;

        UIPass ( UIPass && ) = delete;
        UIPass &operator = ( UIPass && ) = delete;

        explicit UIPass ( ResourceHeap &resourceHeap ) noexcept;

        ~UIPass () = default;

        [[nodiscard]] bool Execute ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept;

        [[nodiscard]] FontStorage &GetFontStorage () noexcept;
        [[nodiscard]] size_t GetUsedVertexCount () const noexcept;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept;
        void OnSwapchainDestroyed () noexcept;

        void RequestEmptyUI () noexcept;
        [[nodiscard]] UIBufferResponse RequestUIBuffer ( size_t neededVertices ) noexcept;

        [[nodiscard]] bool SetBrightness ( android_vulkan::Renderer &renderer, float brightnessBalance ) noexcept;

        void SubmitImage ( uint16_t image ) noexcept;
        void SubmitNonImage () noexcept;

        [[nodiscard]] bool UploadGPUFontData ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        ) noexcept;

        void UploadGPUGeometryData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] constexpr static size_t GetVerticesPerRectangle () noexcept
        {
            return 6U;
        }

        static void AppendImage ( UIVertexStream0* stream0,
            UIVertexStream1* stream1,
            GXColorUNORM color,
            GXVec2 const &topLeft,
            GXVec2 const &bottomRight,
            uint16_t image
        ) noexcept;

        static void AppendRectangle ( UIVertexStream0* stream0,
            UIVertexStream1* stream1,
            GXColorUNORM color,
            GXVec2 const &topLeft,
            GXVec2 const &bottomRight
        ) noexcept;

        static void AppendText ( UIVertexStream0* stream0,
            UIVertexStream1* stream1,
            GXColorUNORM color,
            GXVec2 const &topLeft,
            GXVec2 const &bottomRight,
            android_vulkan::Half2 const &glyphTopLeft,
            android_vulkan::Half2 const &glyphBottomRight,
            uint16_t atlas
        ) noexcept;

        static void ReleaseImage ( uint16_t image ) noexcept;
        [[nodiscard]] static std::optional<Image> RequestImage ( std::string const &asset ) noexcept;

    private:
        void UpdateGeometry ( VkCommandBuffer commandBuffer ) noexcept;
        void UpdateTransform ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif //  PBR_UI_PASS_HPP
