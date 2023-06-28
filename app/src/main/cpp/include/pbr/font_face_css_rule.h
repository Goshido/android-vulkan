#ifndef PBR_FONT_FACE_CSS_RULE_H
#define PBR_FONT_FACE_CSS_RULE_H


#include "parse_result.h"

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

class FontFaceCSSRule final
{
    public:
        FontFaceCSSRule () = delete;

        FontFaceCSSRule ( FontFaceCSSRule const & ) = delete;
        FontFaceCSSRule& operator = ( FontFaceCSSRule const & ) = delete;

        FontFaceCSSRule ( FontFaceCSSRule && ) = delete;
        FontFaceCSSRule& operator = ( FontFaceCSSRule && ) = delete;

        ~FontFaceCSSRule () = default;

        [[nodiscard]] static ParseResult Parse ( char const* css,
            Stream stream,
            std::string const &assetRoot,
            std::unordered_map<std::u32string, std::string> &fonts
        ) noexcept;
};

} // namespace pbr


#endif // PBR_FONT_FACE_CSS_RULE_H
