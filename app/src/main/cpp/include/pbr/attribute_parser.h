#ifndef PBR_ATTRIBUTE_PARSER_HPP
#define PBR_ATTRIBUTE_PARSER_HPP


#include "stream.h"

GX_DISABLE_COMMON_WARNINGS

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace pbr {

enum class eAttribute : uint32_t
{
    Class,
    HREF,
    ID,
    No_Attribute,
    REL,
    SRC
};

class AttributeParser final
{
    public:
        struct Result final
        {
            eAttribute                                                      _attribute;
            Stream                                                          _newStream;
            std::u32string                                                  _value;
        };

    private:
        static std::unordered_map<std::string_view, eAttribute> const       _attributes;
        static std::unordered_map<eAttribute, std::string_view> const       _names;

    public:
        AttributeParser () = delete;

        AttributeParser ( AttributeParser const & ) = delete;
        AttributeParser &operator = ( AttributeParser const & ) = delete;

        AttributeParser ( AttributeParser && ) = delete;
        AttributeParser &operator = ( AttributeParser && ) = delete;

        ~AttributeParser () = delete;

        [[nodiscard]] static std::optional<Result> Parse ( char const* html, Stream stream ) noexcept;
        [[nodiscard]] static char const* ToString ( eAttribute attribute ) noexcept;

        [[nodiscard]] static std::optional<std::unordered_set<std::u32string>> Tokenize ( char const* html,
            size_t line,
            char const* entity,
            std::u32string const &value
        ) noexcept;
};

} // namespace pbr


#endif // PBR_ATTRIBUTE_PARSER_HPP
