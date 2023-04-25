#ifndef PBR_IMG_HTML5_ELEMENT_H
#define PBR_IMG_HTML5_ELEMENT_H


#include "html5_element.h"

GX_DISABLE_COMMON_WARNINGS

#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace pbr {

class IMGHTML5Element final : public HTML5Element
{
    public:
        CSSComputedValues                           _cssComputedValues {};

    private:
        std::string                                 _assetPath {};
        std::unordered_set<std::u32string> const    _classes {};
        std::u32string const                        _id {};

    public:
        IMGHTML5Element () = delete;

        IMGHTML5Element ( IMGHTML5Element const & ) = delete;
        IMGHTML5Element& operator = ( IMGHTML5Element const & ) = delete;

        IMGHTML5Element ( IMGHTML5Element && ) = delete;
        IMGHTML5Element& operator = ( IMGHTML5Element && ) = delete;

        explicit IMGHTML5Element ( std::u32string &&id,
            std::unordered_set<std::u32string> &&classes,
            std::string &&assetPath
        ) noexcept;

        ~IMGHTML5Element () override = default;

        [[nodiscard]] std::u32string const& GetID () const noexcept;

        [[nodiscard]] static std::optional<Result> Parse ( char const* html,
            Stream stream,
            const char* assetRoot,
            std::unordered_set<std::u32string> &idRegistry
        ) noexcept;

    private:
        [[nodiscard]] bool ApplyCSS ( char const* html, CSSParser const &css ) noexcept override;
};

} // namespace pbr


#endif // PBR_IMG_HTML5_ELEMENT_H
