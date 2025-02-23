#ifndef PBR_FONT_STORAGE_HPP
#define PBR_FONT_STORAGE_HPP


#include "command_buffer_count.hpp"
#include <renderer.hpp>
#include "ui_vertex_info.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <ft2build.h>
#include FT_FREETYPE_H

#include <forward_list>
#include <optional>

GX_RESTORE_WARNING_STATE


namespace pbr {

class FontStorage final
{
    public:
        struct GlyphInfo final
        {
            UIAtlas                             _topLeft
            {
                ._uv { 0.0F, 0.0F },
                ._layer = 0U

            };

            UIAtlas                             _bottomRight
            {
                ._uv { 0.0F, 0.0F },
                ._layer = 0U
            };

            int32_t                             _width = 0;
            int32_t                             _height = 0;
            int32_t                             _advance = 0;
            int32_t                             _offsetX = 0;
            int32_t                             _offsetY = 0;
        };

        // Coefficients for getting pixel metrics of the font.
        struct EMFontMetrics final
        {
            double                              _ascend = 0.0;
            double                              _baselineToBaseline = 0.0;
            double                              _contentAreaHeight = 0.0;
            double                              _xHeight = 0.0;
        };

        struct PixelFontMetrics final
        {
            int32_t                             _ascend = 0;
            int32_t                             _baselineToBaseline = 0;
            int32_t                             _contentAreaHeight = 0;
        };

        struct FontResource final
        {
            FT_Face                             _face = nullptr;
            std::vector<uint8_t>                _fontAsset;
            EMFontMetrics                       _metrics {};
        };

        using GlyphStorage = std::unordered_map<char32_t, GlyphInfo>;

        struct FontData final
        {
            FontResource*                       _fontResource = nullptr;
            uint32_t                            _fontSize = 0U;
            GlyphStorage                        _glyphs {};
            PixelFontMetrics                    _metrics {};
        };

        using FontHash = size_t;
        using FontVault = std::unordered_map<FontHash, FontData>;
        using Font = FontVault::iterator;

    private:
        using FontResources = std::unordered_map<std::string_view, FontResource>;

        struct Line final
        {
            uint32_t                            _height = 0U;
            uint32_t                            _x = 0U;
            uint32_t                            _y = 0U;
        };

        struct StagingBuffer final
        {
            enum class eState : uint8_t
            {
                FirstLine,
                FullLinePresent
            };

            VkBuffer                            _buffer = VK_NULL_HANDLE;
            uint8_t*                            _data = nullptr;
            VkDeviceMemory                      _memory = VK_NULL_HANDLE;
            VkDeviceSize                        _memoryOffset = 0U;
            eState                              _state = eState::FirstLine;

            Line                                _endLine {};
            Line                                _startLine {};

            [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, uint32_t side ) noexcept;
            void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
            void Reset () noexcept;
        };

        struct ImageResource final
        {
            VkImage                             _image = VK_NULL_HANDLE;
            VkImageView                         _view = VK_NULL_HANDLE;
            VkDeviceMemory                      _memory = VK_NULL_HANDLE;
            VkDeviceSize                        _memoryOffset = 0U;
        };

        class Atlas final
        {
            private:
                ImageResource                   _dyingResources[ DUAL_COMMAND_BUFFER ];

            public:
                ImageResource                   _resource {};

                uint32_t                        _layers = 0U;
                Line                            _line {};

            public:
                Atlas () = default;

                Atlas ( Atlas const & ) = delete;
                Atlas &operator = ( Atlas const & ) = delete;

                Atlas ( Atlas && ) = delete;
                Atlas &operator = ( Atlas && ) = delete;

                ~Atlas () = default;

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
        Atlas                                   _atlas {};
        FontResources                           _fontResources {};
        FontVault                               _fonts {};

        std::list<StagingBuffer>                _activeStagingBuffer {};
        std::list<StagingBuffer>                _freeStagingBuffers {};
        std::list<StagingBuffer>                _fullStagingBuffers {};

        FT_Library                              _library = nullptr;
        std::forward_list<std::string>          _stringHeap {};

        GlyphInfo                               _opaqueGlyph {};
        GlyphInfo                               _transparentGlyph {};

    public:
        FontStorage () = default;

        FontStorage ( FontStorage const & ) = delete;
        FontStorage &operator = ( FontStorage const & ) = delete;

        FontStorage ( FontStorage && ) = delete;
        FontStorage &operator = ( FontStorage && ) = delete;

        ~FontStorage () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] VkImageView GetAtlasImageView () const noexcept;
        [[nodiscard]] std::optional<Font> GetFont ( std::string_view font, uint32_t size ) noexcept;
        [[nodiscard]] GlyphInfo const &GetOpaqueGlyphInfo () const noexcept;
        [[nodiscard]] GlyphInfo const &GetTransparentGlyphInfo () const noexcept;

        [[nodiscard]] GlyphInfo const &GetGlyphInfo ( android_vulkan::Renderer &renderer,
            Font font,
            char32_t character
        ) noexcept;

        [[nodiscard]] static PixelFontMetrics const &GetFontPixelMetrics ( Font font ) noexcept;

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            size_t commandBufferIndex
        ) noexcept;

        [[nodiscard]] static int32_t GetKerning ( Font font, char32_t left, char32_t right ) noexcept;

    private:
        void DestroyAtlas ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] GlyphInfo const &EmbedGlyph ( android_vulkan::Renderer &renderer,
            GlyphStorage &glyphs,
            FT_Face face,
            uint32_t fontSize,
            int32_t ascend,
            char32_t character
        ) noexcept;

        [[nodiscard]] std::optional<FontResource*> GetFontResource ( std::string_view font ) noexcept;
        [[nodiscard]] std::optional<StagingBuffer*> GetStagingBuffer ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool MakeSpecialGlyphs ( android_vulkan::Renderer &renderer ) noexcept;
        void TransferPixels ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] static bool CheckFTResult ( FT_Error result, char const* from, char const* message ) noexcept;
        [[nodiscard]] static UIAtlas PixToUV ( uint32_t x, uint32_t y, uint8_t layer ) noexcept;
};

} // namespace pbr


#endif // PBR_FONT_STORAGE_HPP
