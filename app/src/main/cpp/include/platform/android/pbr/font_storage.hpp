// FUCK - windows and android separation
#ifndef PBR_ANDROID_FONT_STORAGE_HPP
#define PBR_ANDROID_FONT_STORAGE_HPP


#include <half_types.hpp>
#include <pbr/fif_count.hpp>
#include <renderer.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <ft2build.h>
#include FT_FREETYPE_H

#include <forward_list>
#include <optional>
#include <shared_mutex>

GX_RESTORE_WARNING_STATE


// FUCK - remove namespace
namespace pbr::android {

class FontStorage final
{
    public:
        // Each element contains offset in native pixels from string start to according symbol start.
        // Useful side effect: the last element is total string length in native pixels. The total amount of elements is
        // target string length plus one. The font kerning is taking into account.
        using StringMetrics = std::vector<float>;

        struct GlyphInfo final
        {
            android_vulkan::Half2                       _topLeft { 0.0F, 0.0F };
            android_vulkan::Half2                       _bottomRight { 0.0F, 0.0F };
            uint8_t                                     _layer = 0U;

            int32_t                                     _width = 0;
            int32_t                                     _height = 0;
            int32_t                                     _advance = 0;
            int32_t                                     _offsetX = 0;
            int32_t                                     _offsetY = 0;
        };

        using GlyphStorage = std::unordered_map<char32_t, GlyphInfo>;

        struct PixelFontMetrics final
        {
            int32_t                                     _ascend = 0;
            int32_t                                     _baselineToBaseline = 0;
            int32_t                                     _contentAreaHeight = 0;
        };

        struct FontData final
        {
            FT_Face                                     _face = nullptr;
            uint32_t                                    _fontSize = 0U;
            GlyphStorage                                _glyphs {};
            PixelFontMetrics                            _metrics {};
        };

        using FontHash = size_t;
        using FontVault = std::unordered_map<FontHash, FontData>;
        using Font = FontVault::iterator;

        // Class provide thread safe shared access to font structures.
        // The adding new font into the system will be blocked until at least one instance FontLock is present.
        class FontLock final
        {
            friend FontStorage;

            public:
                Font                                    _font {};

            private:
                std::shared_lock<std::shared_mutex>     _lock {};

            public:
                FontLock () = delete;

                FontLock ( FontLock const & ) = delete;
                FontLock &operator = ( FontLock const & ) = delete;

                FontLock ( FontLock && ) = default;
                FontLock &operator = ( FontLock && ) = default;

                ~FontLock () = default;

            private:
                explicit FontLock ( Font font, std::shared_lock<std::shared_mutex> &&lock ) noexcept;
        };

    private:
        // Coefficients for getting pixel metrics of the font.
        struct EMFontMetrics final
        {
            double                                      _ascend = 0.0;
            double                                      _baselineToBaseline = 0.0;
            double                                      _contentAreaHeight = 0.0;
            double                                      _xHeight = 0.0;
        };

        struct FontResource final
        {
            std::vector<uint8_t>                        _fontAsset {};
            EMFontMetrics                               _metrics {};
        };

        using FontResources = std::unordered_map<std::string_view, FontResource>;

        struct Line final
        {
            uint16_t                                    _height = 0U;
            uint16_t                                    _x = 0U;
            uint16_t                                    _y = 0U;
        };

        struct StagingBuffer final
        {
            enum class eState : uint8_t
            {
                FirstLine,
                FullLinePresent
            };

            VkBuffer                                    _buffer = VK_NULL_HANDLE;
            uint8_t*                                    _data = nullptr;
            VkDeviceMemory                              _memory = VK_NULL_HANDLE;
            VkDeviceSize                                _memoryOffset = 0U;

            Line                                        _endLine {};
            Line                                        _startLine {};

            eState                                      _state = eState::FirstLine;
            bool                                        _hasNewGlyphs = false;

            [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
            void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
            void Reset () noexcept;
        };

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
                Line                                    _line {};

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
        FontResources                                   _fontResources {};
        FontVault                                       _fonts {};

        std::list<StagingBuffer>                        _activeStagingBuffer {};
        std::list<StagingBuffer>                        _freeStagingBuffers {};
        std::list<StagingBuffer>                        _fullStagingBuffers {};

        FT_Library                                      _library = nullptr;
        std::forward_list<std::string>                  _stringHeap {};

        GlyphInfo                                       _transparentGlyph {};
        std::shared_mutex                               _mutex {};

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
        [[nodiscard]] std::optional<FontLock> GetFont ( std::string_view font, uint32_t size ) noexcept;

        [[nodiscard]] GlyphInfo const &GetGlyphInfo ( android_vulkan::Renderer &renderer,
            Font font,
            char32_t character
        ) noexcept;

        void GetStringMetrics ( StringMetrics &result,
            std::string_view font,
            uint32_t size,
            std::u32string_view string
        ) noexcept;

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            size_t commandBufferIndex
        ) noexcept;

        [[nodiscard]] static PixelFontMetrics const &GetFontPixelMetrics ( Font font ) noexcept;
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

        [[nodiscard]] std::optional<StagingBuffer*> GetStagingBuffer ( android_vulkan::Renderer &renderer ) noexcept;
        [[nodiscard]] bool MakeFont ( FontHash hash, std::string_view font, uint32_t size ) noexcept;
        [[nodiscard]] bool MakeTransparentGlyph ( android_vulkan::Renderer &renderer ) noexcept;
        void TransferPixels ( VkCommandBuffer commandBuffer, uint32_t targetLayer ) noexcept;

        [[nodiscard]] static bool CheckFTResult ( FT_Error result, char const* from, char const* message ) noexcept;
        [[nodiscard]] static android_vulkan::Half2 PixToUV ( uint32_t x, uint32_t y ) noexcept;
        [[nodiscard]] static std::optional<EMFontMetrics> ResolveEMFontMetrics ( FT_Face face ) noexcept;
};

} // namespace pbr::android


#endif // PBR_ANDROID_FONT_STORAGE_HPP
