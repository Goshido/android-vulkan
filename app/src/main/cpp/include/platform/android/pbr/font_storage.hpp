#ifndef PBR_FONT_STORAGE_HPP
#define PBR_FONT_STORAGE_HPP


#include <pbr/fif_count.hpp>
#include <pbr/font_storage_base.hpp>


namespace pbr {

struct GlyphInfo final
{
    android_vulkan::Half2       _topLeft { 0.0F, 0.0F };
    android_vulkan::Half2       _bottomRight { 0.0F, 0.0F };
    uint8_t                     _pageID = 0U;

    int32_t                     _width = 0;
    int32_t                     _height = 0;
    int32_t                     _advance = 0;
    int32_t                     _offsetX = 0;
    int32_t                     _offsetY = 0;
};

class FontStorage final : public FontStorageBase<GlyphInfo, FontStagingBufferBase>
{
    private:
        struct ImageResource final
        {
            VkImage                                     _image = VK_NULL_HANDLE;
            VkImageView                                 _view = VK_NULL_HANDLE;
            VkDeviceMemory                              _memory = VK_NULL_HANDLE;
            VkDeviceSize                                _memoryOffset = 0U;
        };

        class Atlas final
        {
            private:
                ImageResource                           _dyingResources[ FIF_COUNT ];

            public:
                ImageResource                           _resource {};

                uint32_t                                _layers = 0U;
                FontAtlasLine                           _line {};

            public:
                Atlas () = default;

                Atlas ( Atlas const & ) = delete;
                Atlas &operator = ( Atlas const & ) = delete;

                Atlas ( Atlas && ) = delete;
                Atlas &operator = ( Atlas && ) = delete;

                ~Atlas () = default;

                // Note: All layers will be moved in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL as the side effect.
                [[nodiscard]] bool AddLayers ( android_vulkan::Renderer &renderer,
                    VkCommandBuffer commandBuffer,
                    size_t commandBufferIndex,
                    uint32_t layers
                ) noexcept;

                void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
                void Cleanup ( android_vulkan::Renderer &renderer, size_t commandBufferIndex ) noexcept;

            private:
                void Copy ( VkCommandBuffer commandBuffer, ImageResource &oldResource, uint32_t newLayers ) noexcept;
                static void FreeImageResource ( android_vulkan::Renderer &renderer, ImageResource &resource ) noexcept;
        };

    private:
        Atlas                                           _atlas {};

    public:
        explicit FontStorage () = default;

        FontStorage ( FontStorage const & ) = delete;
        FontStorage &operator = ( FontStorage const & ) = delete;

        FontStorage ( FontStorage && ) = delete;
        FontStorage &operator = ( FontStorage && ) = delete;

        ~FontStorage () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] VkImageView GetAtlasImageView () const noexcept;

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            size_t commandBufferIndex
        ) noexcept;

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

        void DestroyAtlas ( android_vulkan::Renderer &renderer ) noexcept;
        [[nodiscard]] bool MakeTransparentGlyph ( android_vulkan::Renderer &renderer ) noexcept;
        void TransferPixels ( VkCommandBuffer commandBuffer, uint32_t targetLayer ) noexcept;
};

} // namespace pbr


#endif // PBR_FONT_STORAGE_HPP
