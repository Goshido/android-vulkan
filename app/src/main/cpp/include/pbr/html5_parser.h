#ifndef PBR_HTML5_PARSER_HPP
#define PBR_HTML5_PARSER_HPP


#include "html5_element.h"
#include "parse_result.h"

GX_DISABLE_COMMON_WARNINGS

#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace pbr {

class HTML5Parser final
{
    private:
        bool                                    _bodyElementParsed = false;
        bool                                    _headElementParsed = false;

        std::unordered_set<std::u32string>      _bodyClasses {};
        HTML5Children                           _bodyChildren {};
        std::u32string                          _bodyID {};

        CSSParser                               _css {};
        CSSComputedValues                       _cssComputedValues {};

    public:
        HTML5Parser () = default;

        HTML5Parser ( HTML5Parser const & ) = delete;
        HTML5Parser &operator = ( HTML5Parser const & ) = delete;

        HTML5Parser ( HTML5Parser && ) = delete;
        HTML5Parser &operator = ( HTML5Parser && ) = delete;

        ~HTML5Parser () = default;

        [[nodiscard]] CSSComputedValues &GetBodyCSS () noexcept;
        [[nodiscard]] HTML5Children &GetBodyChildren () noexcept;
        [[nodiscard]] std::u32string &GetBodyID () noexcept;
        [[nodiscard]] CSSParser &GetCSSParser () noexcept;

        [[nodiscard]] bool Parse ( char const* html, Stream stream, char const* assetRoot ) noexcept;

    private:
        void ApplyDefaultCSS () noexcept;

        [[nodiscard]] ParseResult ParseBodyElement ( char const* html, Stream stream, char const* assetRoot ) noexcept;
        [[nodiscard]] bool ParseHTMLElement ( char const* html, Stream stream, char const* assetRoot ) noexcept;
        [[nodiscard]] ParseResult ParseHeadElement ( char const* html, Stream stream, char const* assetRoot ) noexcept;
};

} // namespace pbr


#endif // PBR_HTML5_PARSER_HPP
