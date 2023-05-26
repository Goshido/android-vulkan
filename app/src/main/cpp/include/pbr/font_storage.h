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
            [[maybe_unused]] uint32_t                   _atlasLayer;
            [[maybe_unused]] GXVec2                     _topLeft;
            [[maybe_unused]] GXVec2                     _bottomRight;
            [[maybe_unused]] int32_t                    _width;
            [[maybe_unused]] int32_t                    _height;
            [[maybe_unused]] int32_t                    _advance;
            [[maybe_unused]] int32_t                    _offsetY;
        };

        struct FontResource final
        {
            FT_Face                                     _face;
            std::vector<uint8_t>                        _fontAsset;
        };

        struct FontData final
        {
            [[maybe_unused]] FontResource*              _fontResource;
            std::unordered_map<char32_t, GlyphInfo>     _glyphs;
        };

        using FontHash = size_t;
        using FontVault = std::unordered_map<FontHash, FontData>;
        using Font = FontVault::iterator;

    private:
        using FontResources = std::unordered_map<std::string_view, FontResource>;

        struct StagingBuffer final
        {
            enum class eState : uint8_t
            {
                FirstLine,
                FullLinePresent [[maybe_unused]]
            };

            [[maybe_unused]] VkBuffer                   _buffer = VK_NULL_HANDLE;
            [[maybe_unused]] VkDeviceMemory             _memory = VK_NULL_HANDLE;
            [[maybe_unused]] eState                     _state = eState::FirstLine;

            [[maybe_unused]] uint32_t                   _startX = 0U;
            [[maybe_unused]] uint32_t                   _startY = 0U;

            [[maybe_unused]] uint32_t                   _endX = 0U;
            [[maybe_unused]] uint32_t                   _endY = 0U;
        };

        struct Atlas final
        {
            [[maybe_unused]] VkBuffer                   _buffer = VK_NULL_HANDLE;
            [[maybe_unused]] uint32_t                   _layers = 0U;
            [[maybe_unused]] VkDeviceMemory             _memory = VK_NULL_HANDLE;
            [[maybe_unused]] VkExtent2D                 _resolution {};

            [[maybe_unused, nodiscard]] bool AddLayer ( android_vulkan::Renderer &renderer,
                VkCommandBuffer commandBuffer
            ) noexcept;

            void Destroy () noexcept;
            void SetResolution ( VkExtent2D const &resolution ) noexcept;
        };

    private:
        Atlas                                           _atlas {};
        FontVault                                       _fonts {};
        FontResources                                   _fontResources {};
        FT_Library                                      _library = nullptr;
        std::vector<StagingBuffer>                      _stagingBuffers {};
        std::forward_list<std::string>                  _stringHeap {};

    public:
        FontStorage () = default;

        FontStorage ( FontStorage const & ) = delete;
        FontStorage& operator = ( FontStorage const & ) = delete;

        FontStorage ( FontStorage && ) = delete;
        FontStorage& operator = ( FontStorage && ) = delete;

        ~FontStorage () = default;

        [[nodiscard]] bool Init ( VkExtent2D const &nativeViewport ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] std::optional<Font> GetFont ( std::string_view font, uint32_t size ) noexcept;

        [[nodiscard]] GlyphInfo const& GetGlyphInfo ( android_vulkan::Renderer &renderer,
            Font font,
            char32_t character
        ) noexcept;

        [[nodiscard]] static bool CheckFTResult ( FT_Error result, char const* from, char const* message ) noexcept;

    private:
        [[nodiscard]] std::optional<FontResource*> GetFontResource ( std::string_view font ) noexcept;
};

} // namespace pbr


#endif // PBR_FONT_STORAGE_H
