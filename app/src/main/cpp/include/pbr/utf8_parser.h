#ifndef PBR_UTF8_PARSER_H
#define PBR_UTF8_PARSER_H


#include "stream.h"

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <optional>

GX_RESTORE_WARNING_STATE


namespace pbr {

class UTF8Parser final
{
    public:
        struct Result final
        {
            char32_t    _character;
            Stream      _newStream;
        };

    public:
        UTF8Parser () = delete;

        UTF8Parser ( UTF8Parser const & ) = delete;
        UTF8Parser& operator = ( UTF8Parser const & ) = delete;

        UTF8Parser ( UTF8Parser && ) = delete;
        UTF8Parser& operator = ( UTF8Parser && ) = delete;

        ~UTF8Parser () = delete;

        [[nodiscard]] static std::optional<Result> Parse ( char const* where, Stream stream ) noexcept;
        [[nodiscard]] static std::optional<std::u32string> ToU32String ( std::string_view string ) noexcept;
        [[nodiscard]] static std::optional<std::string> ToUTF8 ( std::u32string_view string ) noexcept;
};

} // namespace pbr


#endif // PBR_UTF8_PARSER_H
