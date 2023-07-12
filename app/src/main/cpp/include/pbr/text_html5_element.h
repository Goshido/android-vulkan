#ifndef PBR_TEXT_HTML5_ELEMENT_H
#define PBR_TEXT_HTML5_ELEMENT_H


#include "html5_element.h"


namespace pbr {

class TextHTML5Element final : public HTML5Element
{
    private:
        std::u32string      _text;

    public:
        TextHTML5Element () = delete;

        TextHTML5Element ( TextHTML5Element const & ) = delete;
        TextHTML5Element &operator = ( TextHTML5Element const & ) = delete;

        TextHTML5Element ( TextHTML5Element && ) = delete;
        TextHTML5Element &operator = ( TextHTML5Element && ) = delete;

        explicit TextHTML5Element ( std::u32string &&text ) noexcept;

        ~TextHTML5Element () override = default;

        [[nodiscard]] std::u32string &GetText () noexcept;

        [[nodiscard]] static std::optional<Result> Parse ( char const* html, Stream stream ) noexcept;
};

} // namespace pbr


#endif // PBR_TEXT_HTML5_ELEMENT_H
