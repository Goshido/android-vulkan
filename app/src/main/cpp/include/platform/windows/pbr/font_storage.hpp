// FUCK - windows and android separation

#ifndef PBR_WINDOWS_FONT_STORAGE_HPP
#define PBR_WINDOWS_FONT_STORAGE_HPP


#include <half_types.hpp>
#include <pbr/fif_count.hpp>
#include "resource_heap.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <ft2build.h>
#include FT_FREETYPE_H

#include <forward_list>
#include <optional>
#include <shared_mutex>

GX_RESTORE_WARNING_STATE


// FUCK - remove namespace
namespace pbr::windows {

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
            uint16_t                                    _atlas = 0U;

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

        struct Binding final
        {
            GlyphStorage*                               _glyphs = nullptr;
            char32_t                                    _character = 0;
        };

        struct StagingBuffer final
        {
            enum class eState : uint8_t
            {
                FirstLine,
                FullLinePresent
            };

            std::vector<Binding>                        _bindings {};

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

            void AddGlyph ( GlyphStorage &glyphs, char32_t character ) noexcept;
            void BindGlyphs ( uint16_t heapResource ) noexcept;
            void Reset () noexcept;
        };

        struct ImageResource final
        {
            VkImage                                     _image = VK_NULL_HANDLE;
            VkImageView                                 _view = VK_NULL_HANDLE;
            VkDeviceMemory                              _memory = VK_NULL_HANDLE;
            VkDeviceSize                                _memoryOffset = 0U;
            uint16_t                                    _heapResource = ResourceHeap::INVALID_UI_IMAGE;
        };

        class Atlas final
        {
            public:
                std::vector<ImageResource>              _pages {};
                Line                                    _line {};

            public:
                Atlas () = default;

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
        Atlas                                           _atlas {};
        FontResources                                   _fontResources {};
        FontVault                                       _fonts {};

        std::list<StagingBuffer>                        _activeStagingBuffer {};
        std::list<StagingBuffer>                        _freeStagingBuffers {};
        std::list<StagingBuffer>                        _fullStagingBuffers {};

        std::vector<VkImageMemoryBarrier2>              _barriers {};
        FT_Library                                      _library = nullptr;
        std::forward_list<std::string>                  _stringHeap {};

        ResourceHeap                                    &_resourceHeap;

        GlyphInfo                                       _transparentGlyph {};
        std::shared_mutex                               _mutex {};

        VkDependencyInfo                                _dependencies
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

        VkBufferImageCopy                               _imageCopy
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

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;
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
        void TransferPixels ( VkCommandBuffer commandBuffer, ImageResource const* pages ) noexcept;

        [[nodiscard]] static bool CheckFTResult ( FT_Error result, char const* from, char const* message ) noexcept;
        [[nodiscard]] static android_vulkan::Half2 PixToUV ( uint32_t x, uint32_t y ) noexcept;
        [[nodiscard]] static std::optional<EMFontMetrics> ResolveEMFontMetrics ( FT_Face face ) noexcept;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_FONT_STORAGE_HPP
