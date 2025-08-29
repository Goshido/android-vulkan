// FUCK - rename include guard
#ifndef PBR_WINDOWS_UI_PASS_HPP
#define PBR_WINDOWS_UI_PASS_HPP


#include "font_storage.hpp"
#include <pbr/types.hpp>
#include "ui_program.hpp"
#include "ui_vertex_info.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


// FUCK - remove namespace
namespace pbr::windows {

class UIPass final
{
    public:
        using UIBufferResponse = std::optional<UIVertexBuffer>;

    private:
        struct Buffer final
        {
            VkBuffer                                    _buffer = VK_NULL_HANDLE;
            VkDeviceMemory                              _memory = VK_NULL_HANDLE;
            char const*                                 _name = nullptr;
            VkDeviceSize                                _memoryOffset = 0U;

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
                VkBufferMemoryBarrier                   _barrier
                {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .buffer = VK_NULL_HANDLE,
                    .offset = 0U,
                    .size = 0U
                };

                uint8_t*                                _data = nullptr;
                size_t const                            _elementSize = 0U;

                Buffer                                  _staging {};
                Buffer                                  _vertex {};

            public:
                BufferStream () = delete;

                BufferStream ( BufferStream const & ) = delete;
                BufferStream &operator = ( BufferStream const & ) = delete;

                BufferStream ( BufferStream && ) = delete;
                BufferStream &operator = ( BufferStream && ) = delete;

                explicit BufferStream ( size_t elementSize ) noexcept;

                ~BufferStream () = default;

                [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
                    char const *vertexName,
                    char const *stagingName
                ) noexcept;

                void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

                [[nodiscard]] VkBuffer GetBuffer () const noexcept;
                [[nodiscard]] void *GetData ( size_t startIndex ) const noexcept;
                void UpdateGeometry ( VkCommandBuffer commandBuffer, size_t readIdx, size_t writeIdx ) noexcept;
        };

        struct InUseImageTracker final
        {
            using Entry = std::unordered_map<Texture2DRef, size_t>;

            Entry                                       _registry[ FIF_COUNT ];

            void Destroy () noexcept;

            void CollectGarbage ( size_t commandBufferIndex ) noexcept;
            void MarkInUse ( Texture2DRef const &texture, size_t commandBufferIndex );
        };

        struct Job final
        {
            Texture2DRef const*                         _texture = nullptr;
            uint32_t                                    _vertices {};
        };

    private:
        GXVec2                                          _bottomRight {};
        float                                           _brightnessBalance = 0.0F;

        VkExtent2D                                      _currentResolution
        {
            .width = 0U,
            .height = 0U
        };

        android_vulkan::Texture2D                       _textLUT {};

        InUseImageTracker                               _inUseImageTracker {};

        size_t                                          _readVertexIndex = 0U;
        size_t                                          _writeVertexIndex = 0U;

        FontStorage                                     _fontStorage;

        bool                                            _hasChanges = false;
        bool                                            _isTransformChanged = false;
        std::vector<Job>                                _jobs {};

        BufferStream                                    _positions { sizeof ( GXVec2 ) };
        BufferStream                                    _rest { sizeof ( UIVertex ) };

        UIProgram                                       _program {};
        UIProgram::UIInfo                               _uiInfo {};

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

        void SubmitImage ( Texture2DRef const &texture ) noexcept;
        void SubmitRectangle () noexcept;
        void SubmitText ( size_t usedVertices ) noexcept;

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] constexpr static size_t GetVerticesPerRectangle () noexcept
        {
            return 6U;
        }

        static void AppendImage ( GXVec2* targetPositions,
            UIVertex* targetVertices,
            GXColorUNORM color,
            GXVec2 const &topLeft,
            GXVec2 const &bottomRight,
            uint16_t image
        ) noexcept;

        static void AppendRectangle ( GXVec2* targetPositions,
            UIVertex* targetVertices,
            GXColorUNORM color,
            GXVec2 const &topLeft,
            GXVec2 const &bottomRight
        ) noexcept;

        static void AppendText ( GXVec2* targetPositions,
            UIVertex* targetVertices,
            GXColorUNORM color,
            GXVec2 const &topLeft,
            GXVec2 const &bottomRight,
            android_vulkan::Half2 const &glyphTopLeft,
            android_vulkan::Half2 const &glyphBottomRight,
            uint16_t atlas
        ) noexcept;

        static void ReleaseImage ( Texture2DRef const &image ) noexcept;
        [[nodiscard]] static std::optional<Texture2DRef const> RequestImage ( std::string const &asset ) noexcept;

    private:
        void SubmitNonImage ( size_t usedVertices ) noexcept;
        void UpdateGeometry ( VkCommandBuffer commandBuffer ) noexcept;
        void UpdateTransform ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr::windows


#endif //  PBR_WINDOWS_UI_PASS_HPP
