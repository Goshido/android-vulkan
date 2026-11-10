#ifndef EDITOR_STRING_PROPERTY_HPP
#define EDITOR_STRING_PROPERTY_HPP


#include "property.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace editor {

class StringProperty final : public Property
{
    private:
        std::string     _value {};

    public:
        explicit StringProperty () = default;

        StringProperty ( StringProperty const & ) = delete;
        StringProperty &operator = ( StringProperty const & ) = delete;

        StringProperty ( StringProperty && ) = delete;
        StringProperty &operator = ( StringProperty && ) = delete;

        explicit StringProperty ( std::string &&value ) noexcept;

        ~StringProperty () override = default;

        [[nodiscard]] std::string const &Get () const noexcept;
        void Set ( std::string &&value ) noexcept;
};

} // namespace editor


#endif // EDITOR_STRING_PROPERTY_HPP
