#ifndef PBR_FONT_STORAGE_BASE_HPP
#define PBR_FONT_STORAGE_BASE_HPP


#include <half_types.hpp>
#include <renderer.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <ft2build.h>
#include FT_FREETYPE_H

#include <forward_list>
#include <optional>
#include <shared_mutex>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static inline uint16_t FONT_ATLAS_RESOLUTION = 1024U;

template <typename G>
using GlyphStorage = std::unordered_map<char32_t, G>;

struct FontAtlasLine final
{
    uint16_t    _height = 0U;
    uint16_t    _x = 0U;
    uint16_t    _y = 0U;
};

class FontStagingBufferBase
{
    public:
        enum class eState : uint8_t
        {
            FirstLine,
            FullLinePresent
        };

    public:
        VkBuffer            _buffer = VK_NULL_HANDLE;
        uint8_t*            _data = nullptr;
        VkDeviceMemory      _memory = VK_NULL_HANDLE;
        VkDeviceSize        _memoryOffset = 0U;

        FontAtlasLine       _endLine {};
        FontAtlasLine       _startLine {};

        eState              _state = eState::FirstLine;
        bool                _hasNewGlyphs = false;

    public:
        explicit FontStagingBufferBase () = default;

        FontStagingBufferBase ( FontStagingBufferBase const & ) = delete;
        FontStagingBufferBase &operator = ( FontStagingBufferBase const & ) = delete;

        FontStagingBufferBase ( FontStagingBufferBase && ) = default;
        FontStagingBufferBase &operator = ( FontStagingBufferBase && ) = default;

        ~FontStagingBufferBase () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( android_vulkan::Renderer& renderer ) noexcept;
        void Reset () noexcept;
};

template <typename G, typename S>
class FontStorageBase
{
    public:
        // Each element contains offset in native pixels from string start to according symbol start.
        // Useful side effect: the last element is total string length in native pixels. The total amount of elements is
        // target string length plus one. The font kerning is taking into account.
        using StringMetrics = std::vector<float>;

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
            GlyphStorage<G>                             _glyphs {};
            PixelFontMetrics                            _metrics {};
        };

        using FontHash = size_t;
        using FontVault = std::unordered_map<FontHash, FontData>;
        using Font = FontVault::iterator;

        // Class provide thread safe shared access to font structures.
        // The adding new font into the system will be blocked until at least one instance FontStagingBufferBase is present.
        class FontLock final
        {
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

                explicit FontLock ( Font font, std::shared_lock<std::shared_mutex> &&lock ) noexcept;
                ~FontLock () = default;
        };

    protected:
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

        struct MakeFontInfo final
        {
            FT_Face                                     _face = nullptr;
            EMFontMetrics*                              _metrics = nullptr;
        };

        using MakeFontResult = std::optional<MakeFontInfo>;

    protected:
        FT_Library                                      _library = nullptr;
        std::shared_mutex                               _mutex {};
        FontResources                                   _fontResources {};
        std::forward_list<std::string>                  _stringHeap {};

        std::list<S>                                    _activeStagingBuffer {};
        std::list<S>                                    _freeStagingBuffers {};
        std::list<S>                                    _fullStagingBuffers {};

        FontVault                                       _fonts {};
        G                                               _transparentGlyph {};

    public:
        FontStorageBase ( FontStorageBase const & ) = delete;
        FontStorageBase &operator = ( FontStorageBase const & ) = delete;

        FontStorageBase ( FontStorageBase && ) = delete;
        FontStorageBase &operator = ( FontStorageBase && ) = delete;

        [[nodiscard]] std::optional<FontLock> GetFont ( std::string_view font, uint32_t size ) noexcept;

        [[nodiscard]] G const &GetGlyphInfo ( android_vulkan::Renderer &renderer,
            Font font,
            char32_t character
        ) noexcept;

        void GetStringMetrics ( StringMetrics &result,
            std::string_view font,
            uint32_t size,
            std::u32string_view string
        ) noexcept;

        [[nodiscard]] static PixelFontMetrics const &GetFontPixelMetrics ( Font font ) noexcept;
        [[nodiscard]] static int32_t GetKerning ( Font font, char32_t left, char32_t right ) noexcept;

    protected:
        explicit FontStorageBase () = default;
        ~FontStorageBase () = default;

        [[nodiscard]] virtual G const &InsertGlyph ( GlyphStorage<G> &glyphs,
            FontStagingBufferBase &stagingBuffer,
            char32_t character,
            int32_t offsetX,
            int32_t offsetY,
            int32_t advance,
            int32_t width,
            int32_t height,
            android_vulkan::Half2 topLeft,
            android_vulkan::Half2 bottomRight
        ) noexcept = 0;

        [[nodiscard]] bool InitBase () noexcept;
        void DestroyBase ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] std::optional<S*> GetStagingBuffer (
            android_vulkan::Renderer &renderer
        ) noexcept;

        [[nodiscard]] static android_vulkan::Half2 PixToUV ( uint32_t x, uint32_t y ) noexcept;

    private:
        [[nodiscard]] bool MakeFont ( FontHash hash, std::string_view font, uint32_t size ) noexcept;

        [[nodiscard]] G const &EmbedGlyph ( android_vulkan::Renderer &renderer,
            GlyphStorage<G> &glyphs,
            FT_Face face,
            uint32_t fontSize,
            int32_t ascend,
            char32_t character
        ) noexcept;
 
        [[nodiscard]] MakeFontResult MakeFontInternal ( std::string_view font, uint32_t size ) noexcept;

        [[nodiscard]] static bool CheckFTResult ( FT_Error result, char const* from, char const* message ) noexcept;
        [[nodiscard]] static FontHash MakeFontHash ( std::string_view font, uint32_t size ) noexcept;
        [[nodiscard]] static std::optional<EMFontMetrics> ResolveEMFontMetrics ( FT_Face face ) noexcept;
};

} // namespace pbr


#include <pbr/font_storage_base.ipp>


#endif // PBR_FONT_STORAGE_BASE_HPP
