#ifndef PBR_NUMBER_PARSER_H
#define PBR_NUMBER_PARSER_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <optional>
#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class NumberParser final
{
    public:
        struct Result final
        {
            std::u32string_view     _tail;
            float                   _value;
        };

    public:
        NumberParser () = delete;

        NumberParser ( NumberParser const & ) = delete;
        NumberParser& operator = ( NumberParser const & ) = delete;

        NumberParser ( NumberParser && ) = delete;
        NumberParser& operator = ( NumberParser && ) = delete;

        ~NumberParser () = delete;

        [[nodiscard]] static std::optional<Result> Parse ( char const* css,
            size_t line,
            std::u32string_view value
        ) noexcept;
};

} // namespace pbr


#endif // PBR_NUMBER_PARSER_H
