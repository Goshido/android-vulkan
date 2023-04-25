#ifndef PBR_DIV_HTML5_ELEMENT_H
#define PBR_DIV_HTML5_ELEMENT_H


#include "html5_element.h"

GX_DISABLE_COMMON_WARNINGS

#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace pbr {

class DIVHTML5Element final : public HTML5Element
{
    public:
        CSSComputedValues                           _cssComputedValues {};

    private:
        HTML5Childs                                 _childs {};
        std::unordered_set<std::u32string> const    _classes {};
        std::u32string const                        _id {};

    public:
        DIVHTML5Element () = delete;

        DIVHTML5Element ( DIVHTML5Element const & ) = delete;
        DIVHTML5Element& operator = ( DIVHTML5Element const & ) = delete;

        DIVHTML5Element ( DIVHTML5Element && ) = delete;
        DIVHTML5Element& operator = ( DIVHTML5Element && ) = delete;

        explicit DIVHTML5Element ( std::u32string &&id,
            std::unordered_set<std::u32string> &&classes,
            HTML5Childs &&childs
        ) noexcept;

        ~DIVHTML5Element () override = default;

        [[nodiscard]] std::u32string const& GetID () const noexcept;

        [[nodiscard]] static std::optional<Result> Parse ( char const* html,
            Stream stream,
            char const* assetRoot,
            std::unordered_set<std::u32string> &idRegistry
        ) noexcept;

    private:
        [[nodiscard]] bool ApplyCSS ( char const* html, CSSParser const &css ) noexcept override;
};

} // namespace pbr


#endif // PBR_DIV_HTML5_ELEMENT_H
