#ifndef PBR_COMMON_CSS_RULE_HPP
#define PBR_COMMON_CSS_RULE_HPP


#include "css_rule.hpp"
#include "parse_result.hpp"


namespace pbr {

class CommonCSSRule final
{
    public:
        enum class eType : uint8_t
        {
            Class,
            ID
        };

    public:
        CommonCSSRule () = delete;

        CommonCSSRule ( CommonCSSRule const & ) = delete;
        CommonCSSRule &operator = ( CommonCSSRule const & ) = delete;

        CommonCSSRule ( CommonCSSRule && ) = delete;
        CommonCSSRule &operator = ( CommonCSSRule && ) = delete;

        ~CommonCSSRule () = default;

        [[nodiscard]] static ParseResult Parse ( char const* css,
            Stream stream,
            eType type,
            CSSRules &cssRules
        ) noexcept;
};

} // namespace pbr


#endif // PBR_COMMON_CSS_RULE_HPP
