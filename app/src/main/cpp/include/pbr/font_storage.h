#ifndef PBR_FONT_STORAGE_H
#define PBR_FONT_STORAGE_H


#include <renderer.h>

GX_DISABLE_COMMON_WARNINGS

#include <ft2build.h>
#include FT_FREETYPE_H

#include <forward_list>
#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class FontStorage final
{
    public:
        struct GlyphInfo final
        {
            [[maybe_unused]] uint32_t       _atlasLayer = 0U;
            GXVec2                          _topLeft = GXVec2 ( 0.0F, 0.0F );
            GXVec2                          _bottomRight = GXVec2 ( 0.0F, 0.0F );
            [[maybe_unused]] int32_t        _width = 0;
            int32_t                         _height = 0;
            int32_t                         _advance = 0;
            [[maybe_unused]] int32_t        _offsetY = 0;
        };

        struct FontResource final
        {
            FT_Face                         _face;
            std::vector<uint8_t>            _fontAsset;
        };

        using GlyphStorage = std::unordered_map<char32_t, GlyphInfo>;

        struct FontData final
        {
            FontResource*                   _fontResource;
            uint32_t                        _fontSize;
            GlyphStorage                    _glyphs;
            int32_t                         _lineHeight;
        };

        using FontHash = size_t;
        using FontVault = std::unordered_map<FontHash, FontData>;
        using Font = FontVault::iterator;

    private:
        using FontResources = std::unordered_map<std::string_view, FontResource>;

        struct Line final
        {
            uint32_t                        _height = 0U;
            uint32_t                        _x = 0U;
            uint32_t                        _y = 0U;
        };

        struct StagingBuffer final
        {
            enum class eState : uint8_t
            {
                FirstLine,
                FullLinePresent
            };

            VkBuffer                        _buffer = VK_NULL_HANDLE;
            uint8_t*                        _data = nullptr;
            VkDeviceMemory                  _memory = VK_NULL_HANDLE;
            VkDeviceSize                    _memoryOffset = 0U;
            eState                          _state = eState::FirstLine;

            Line                            _endLine {};
            Line                            _startLine {};

            [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, uint32_t side ) noexcept;
            void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
            void Reset () noexcept;
        };

        struct ImageResource final
        {
            VkImage                         _image = VK_NULL_HANDLE;
            VkImageView                     _view = VK_NULL_HANDLE;
            VkDeviceMemory                  _memory = VK_NULL_HANDLE;
            VkDeviceSize                    _memoryOffset = 0U;
        };

        struct Atlas final
        {
            ImageResource                   _resource {};
            ImageResource                   _oldResource {};

            uint32_t                        _layers = 0U;
            Line                            _line {};
            uint32_t                        _side = 0U;

            [[nodiscard]] bool AddLayers ( android_vulkan::Renderer &renderer,
                VkCommandBuffer commandBuffer,
                uint32_t layers
            ) noexcept;

            void Cleanup ( android_vulkan::Renderer &renderer ) noexcept;
            void Copy ( VkCommandBuffer commandBuffer, uint32_t newLayers ) noexcept;
            void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        };

    private:
        Atlas                               _atlas {};
        FontResources                       _fontResources {};
        FontVault                           _fonts {};

        std::list<StagingBuffer>            _activeStagingBuffer {};
        std::list<StagingBuffer>            _freeStagingBuffers {};
        std::list<StagingBuffer>            _fullStagingBuffers {};

        FT_Library                          _library = nullptr;
        std::forward_list<std::string>      _stringHeap {};

        GXVec2                              _halfPixelUV {};
        float                               _pixToUV {};

        GXVec2                              _opaqueGlyphUV {};
        GXVec2                              _transparentGlyphUV {};

    public:
        FontStorage () = default;

        FontStorage ( FontStorage const & ) = delete;
        FontStorage& operator = ( FontStorage const & ) = delete;

        FontStorage ( FontStorage && ) = delete;
        FontStorage& operator = ( FontStorage && ) = delete;

        ~FontStorage () = default;

        [[nodiscard]] bool Init () noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] std::optional<Font> GetFont ( std::string_view font, uint32_t size ) noexcept;

        [[nodiscard]] GlyphInfo const& GetGlyphInfo ( android_vulkan::Renderer &renderer,
            Font font,
            char32_t character
        ) noexcept;

        [[nodiscard]] bool SetMediaResolution ( android_vulkan::Renderer &renderer,
            VkExtent2D const &nativeViewport
        ) noexcept;

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] static int32_t GetKerning ( Font font, char32_t left, char32_t right ) noexcept;

    private:
        void DestroyAtlas ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] GlyphInfo const& EmbedGlyph ( android_vulkan::Renderer &renderer,
            GlyphStorage &glyphs,
            FT_Face face,
            uint32_t fontSize,
            char32_t character
        ) noexcept;

        [[nodiscard]] std::optional<FontResource*> GetFontResource ( std::string_view font ) noexcept;
        [[nodiscard]] std::optional<StagingBuffer*> GetStagingBuffer ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool MakeSpecialGlyphs ( android_vulkan::Renderer &renderer ) noexcept;
        [[nodiscard]] GXVec2 PixToUV ( uint32_t x, uint32_t y ) const noexcept;
        void TransferPixels ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] static bool CheckFTResult ( FT_Error result, char const* from, char const* message ) noexcept;
};

} // namespace pbr


#endif // PBR_FONT_STORAGE_H
