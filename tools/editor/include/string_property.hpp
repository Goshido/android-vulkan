#ifndef EDITOR_STRING_PROPERTY_HPP
#define EDITOR_STRING_PROPERTY_HPP


#include "property.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <string_view>

GX_RESTORE_WARNING_STATE


namespace editor {

class StringProperty;
[[nodiscard]] std::string operator + ( char const* left, StringProperty const &right ) noexcept;

class StringProperty final : public Property
{
    friend std::string operator + ( char const* left, StringProperty const &right ) noexcept;

    private:
        std::string     _value {};

    public:
        explicit StringProperty () = default;

        StringProperty ( StringProperty const & ) = delete;
        StringProperty &operator = ( StringProperty const & ) = delete;

        StringProperty ( StringProperty && ) = delete;
        StringProperty &operator = ( StringProperty && ) = delete;
        StringProperty &operator = ( std::string &&value ) noexcept;

        StringProperty ( std::string &&value ) noexcept;

        ~StringProperty () override = default;

        [[nodiscard]] operator std::string const &() const noexcept;
        [[nodiscard]] operator std::string_view () const noexcept;

        [[nodiscard]] std::string operator + ( char const* other ) const noexcept;
};

} // namespace editor


#endif // EDITOR_STRING_PROPERTY_HPP
