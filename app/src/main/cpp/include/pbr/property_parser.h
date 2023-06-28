#ifndef PBR_PROPERTY_PARSER_H
#define PBR_PROPERTY_PARSER_H


#include "property.h"
#include "stream.h"

GX_DISABLE_COMMON_WARNINGS

#include <list>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

class PropertyParser final
{
    public:
        struct Result final
        {
            Stream                                                              _newStream;
            Property::eType                                                     _property;

            // The value is trimmed from any whitespace characters from start and end.
            // The value can not be empty.
            std::u32string                                                      _value;
        };

    private:
        static std::unordered_map<std::string_view, Property::eType> const      _properties;
        static std::unordered_map<Property::eType, std::string_view> const      _names;

    public:
        PropertyParser () = delete;

        PropertyParser ( PropertyParser const & ) = delete;
        PropertyParser& operator = ( PropertyParser const & ) = delete;

        PropertyParser ( PropertyParser && ) = delete;
        PropertyParser& operator = ( PropertyParser && ) = delete;

        ~PropertyParser () = delete;

        [[nodiscard]] static std::optional<Result> Parse ( char const* css, Stream stream ) noexcept;
        [[nodiscard]] static char const* ToString ( Property::eType property ) noexcept;
        [[nodiscard]] static std::list<std::u32string> Tokenize ( std::u32string const &value ) noexcept;
    };

} // namespace pbr


#endif // PBR_PROPERTY_PARSER_H
