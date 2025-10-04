#ifndef PBR_FONT_STORAGE_HPP
#define PBR_FONT_STORAGE_HPP


#include <pbr/font_storage_base.hpp>
#include "resource_heap.hpp"


namespace pbr {

struct GlyphInfo final
{
    android_vulkan::Half2       _topLeft { 0.0F, 0.0F };
    android_vulkan::Half2       _bottomRight { 0.0F, 0.0F };
    uint16_t                    _pageID = 0U;

    int32_t                     _width = 0;
    int32_t                     _height = 0;
    int32_t                     _advance = 0;
    int32_t                     _offsetX = 0;
    int32_t                     _offsetY = 0;
};

class FontStagingBuffer final : public FontStagingBufferBase
{
    public:
        struct Binding final
        {
            GlyphStorage<GlyphInfo>*            _glyphs = nullptr;
            char32_t                            _character = 0;
        };

    public:
        std::vector<Binding>                    _bindings {};

    public:
        explicit FontStagingBuffer () = default;

        FontStagingBuffer ( FontStagingBuffer const & ) = delete;
        FontStagingBuffer &operator = ( FontStagingBuffer const & ) = delete;

        FontStagingBuffer ( FontStagingBuffer && ) = default;
        FontStagingBuffer &operator = ( FontStagingBuffer && ) = default;

        ~FontStagingBuffer () = default;

        void AddGlyph ( GlyphStorage<GlyphInfo> &glyphs, char32_t character ) noexcept;
        void BindGlyphs ( uint16_t heapResource ) noexcept;
};

class FontStorage final : public FontStorageBase<GlyphInfo, FontStagingBuffer>
{
    private:
        struct ImageResource final
        {
            VkImage                             _image = VK_NULL_HANDLE;
            VkImageView                         _view = VK_NULL_HANDLE;
            VkDeviceMemory                      _memory = VK_NULL_HANDLE;
            VkDeviceSize                        _memoryOffset = 0U;
            uint16_t                            _heapResource = ResourceHeap::INVALID_UI_IMAGE;
        };

        class Atlas final
        {
            public:
                std::vector<ImageResource>      _pages {};
                FontAtlasLine                   _line {};

            public:
                explicit Atlas () = default;

                Atlas ( Atlas const & ) = delete;
                Atlas &operator = ( Atlas const & ) = delete;

                Atlas ( Atlas && ) = delete;
                Atlas &operator = ( Atlas && ) = delete;

                ~Atlas () = default;

                [[nodiscard]] bool AddPages ( android_vulkan::Renderer &renderer,
                    ResourceHeap &resourceHeap,
                    uint32_t pages
                ) noexcept;

                void Destroy ( android_vulkan::Renderer &renderer, ResourceHeap &resourceHeap ) noexcept;
        };

    private:
        Atlas                                   _atlas {};

        std::vector<VkImageMemoryBarrier2>      _barriers {};
        ResourceHeap                            &_resourceHeap;

        VkDependencyInfo                        _dependencies
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

        VkBufferImageCopy                       _imageCopy
        {
            .bufferOffset = 0U,
            .bufferRowLength = 0U,
            .bufferImageHeight = 0U,

            .imageSubresource
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0U,
                .baseArrayLayer = 0U,
                .layerCount = 1U
            },

            .imageOffset
            {
                .x = 0,
                .y = 0,
                .z = 0
            },

            .imageExtent
            {
                .width = 0U,
                .height = 0U,
                .depth = 1U
            }
        };

    public:
        FontStorage () = delete;

        FontStorage ( FontStorage const & ) = delete;
        FontStorage &operator = ( FontStorage const & ) = delete;

        FontStorage ( FontStorage && ) = delete;
        FontStorage &operator = ( FontStorage && ) = delete;

        explicit FontStorage ( ResourceHeap &resourceHeap ) noexcept;

        ~FontStorage () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;

    private:
        [[nodiscard]] GlyphInfo const &InsertGlyph ( GlyphStorage<GlyphInfo> &glyphs,
            FontStagingBufferBase &stagingBuffer,
            char32_t character,
            int32_t offsetX,
            int32_t offsetY,
            int32_t advance,
            int32_t width,
            int32_t height,
            android_vulkan::Half2 topLeft,
            android_vulkan::Half2 bottomRight
        ) noexcept override;

        [[nodiscard]] bool MakeTransparentGlyph ( android_vulkan::Renderer &renderer ) noexcept;
        void TransferPixels ( VkCommandBuffer commandBuffer, ImageResource const* pages ) noexcept;
};

} // namespace pbr


#endif // PBR_FONT_STORAGE_HPP
