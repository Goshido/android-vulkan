#ifndef PBR_LENGTH_VALUE_PARSER_H
#define PBR_LENGTH_VALUE_PARSER_H


#include "length_value.h"

GX_DISABLE_COMMON_WARNINGS

#include <optional>
#include <string>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

class LengthValueParser final
{
    private:
        static std::unordered_map<std::u32string_view, LengthValue::eType>      _types;

    public:
        LengthValueParser () = delete;

        LengthValueParser ( LengthValueParser const & ) = delete;
        LengthValueParser &operator = ( LengthValueParser const & ) = delete;

        LengthValueParser ( LengthValueParser && ) = delete;
        LengthValueParser &operator = ( LengthValueParser && ) = delete;

        ~LengthValueParser () = delete;

        [[nodiscard]] static std::optional<LengthValue> Parse ( char const* css,
            size_t line,
            std::u32string_view value
        ) noexcept;
};

} // namespace pbr


#endif // PBR_LENGTH_VALUE_PARSER_H
