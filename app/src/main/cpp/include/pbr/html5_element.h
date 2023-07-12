#ifndef PBR_HTML5_ELEMENT_H
#define PBR_HTML5_ELEMENT_H


#include "css_computed_values.h"
#include "html5_tag.h"
#include "stream.h"

GX_DISABLE_COMMON_WARNINGS

#include <deque>
#include <memory>

GX_RESTORE_WARNING_STATE


namespace pbr {

class HTML5Element
{
    public:
        struct Result final
        {
            std::shared_ptr<HTML5Element>       _element;
            Stream                              _newStream;
        };

    private:
        HTML5Tag                                _tag;

    public:
        HTML5Element () = delete;

        HTML5Element ( HTML5Element const & ) = delete;
        HTML5Element &operator = ( HTML5Element const & ) = delete;

        HTML5Element ( HTML5Element && ) = delete;
        HTML5Element &operator = ( HTML5Element && ) = delete;

        virtual ~HTML5Element () = default;

        [[nodiscard]] virtual bool ApplyCSS ( char const* html, CSSParser const &css ) noexcept;
        [[nodiscard]] HTML5Tag GetTag () const noexcept;

    protected:
        explicit HTML5Element ( HTML5Tag::eTag tag ) noexcept;
};

using HTML5Children = std::deque<std::shared_ptr<HTML5Element>>;

} // namespace pbr


#endif // PBR_HTML5_ELEMENT_H
