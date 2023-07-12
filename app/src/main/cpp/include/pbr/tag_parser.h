#ifndef PBR_TAG_PARSER_HPP
#define PBR_TAG_PARSER_HPP


#include "html5_tag.h"
#include "stream.h"


namespace pbr {

class TagParser final
{
    public:
        struct Result final
        {
            Stream      _newStream;
            HTML5Tag    _tag;
        };

    public:
        TagParser () = delete;

        TagParser ( TagParser const & ) = delete;
        TagParser &operator = ( TagParser const & ) = delete;

        TagParser ( TagParser && ) = delete;
        TagParser &operator = ( TagParser && ) = delete;

        ~TagParser () = delete;

        [[nodiscard]] static std::optional<Result> IsEndTag ( char const* html, Stream stream ) noexcept;
        [[nodiscard]] static std::optional<Result> Parse ( char const* html, Stream stream ) noexcept;
};

} // namespace pbr


#endif // PBR_TAG_PARSER_HPP
