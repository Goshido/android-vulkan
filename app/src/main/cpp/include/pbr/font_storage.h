#ifndef PBR_FONT_STORAGE_H
#define PBR_FONT_STORAGE_H


#include <renderer.h>

GX_DISABLE_COMMON_WARNINGS

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class FontStorage final
{
    public:
        struct GlyphInfo final
        {
            [[maybe_unused]] uint32_t       _atlasLayer;
            [[maybe_unused]] GXVec2         _topLeft;
            [[maybe_unused]] GXVec2         _bottomRight;
            [[maybe_unused]] int32_t        _width;
            [[maybe_unused]] int32_t        _height;
            [[maybe_unused]] int32_t        _advance;
            [[maybe_unused]] int32_t        _offsetY;
        };

    private:
        FT_Library                          _library = nullptr;

    public:
        FontStorage () = default;

        FontStorage ( FontStorage const & ) = delete;
        FontStorage& operator = ( FontStorage const & ) = delete;

        FontStorage ( FontStorage && ) = delete;
        FontStorage& operator = ( FontStorage && ) = delete;

        ~FontStorage () = default;

        [[nodiscard]] bool Init () noexcept;
        void Destroy () noexcept;

        [[nodiscard]] GlyphInfo const& GetGlyphInfo ( android_vulkan::Renderer &renderer,
            std::string const &font,
            uint32_t size,
            char32_t character
        ) noexcept;

        [[nodiscard]] static bool CheckFTResult ( FT_Error result, char const* from, char const* message ) noexcept;
};

} // namespace pbr


#endif // PBR_FONT_STORAGE_H
