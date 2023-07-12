#ifndef PBR_CSS_PARSER_H
#define PBR_CSS_PARSER_H


#include "css_rule.h"
#include "parse_result.h"

GX_DISABLE_COMMON_WARNINGS

#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

class CSSParser final
{
    private:
        CSSRules                                            _classes {};
        std::string                                         _cssSource {};
        std::unordered_map<std::u32string, std::string>     _fonts;
        CSSRules                                            _ids {};

    public:
        CSSParser () = default;

        CSSParser ( CSSParser const & ) = delete;
        CSSParser &operator = ( CSSParser const & ) = delete;

        CSSParser ( CSSParser && ) = default;
        CSSParser &operator = ( CSSParser && ) = default;

        ~CSSParser () = default;

        [[nodiscard]] std::optional<CSSProps const*> FindClass ( std::u32string const &name ) const noexcept;

        [[nodiscard]] std::optional<std::string const*> FindFontFile (
            std::u32string const &fontFamily
        ) const noexcept;

        [[nodiscard]] std::optional<CSSProps const*> FindID ( std::u32string const &id ) const noexcept;

        // Method returns source file path from which CSS rules were constructed.
        [[nodiscard]] std::string const &GetSource () const noexcept;

        [[nodiscard]] bool Parse ( char const* css, Stream stream, std::string &&assetRoot ) noexcept;
};

} // namespace pbr


#endif // PBR_CSS_PARSER_H
