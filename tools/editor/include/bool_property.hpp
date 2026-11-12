#ifndef EDITOR_BOOL_PROPERTY_HPP
#define EDITOR_BOOL_PROPERTY_HPP


#include "property.hpp"


namespace editor {

class BoolProperty final : public Property
{
    private:
        bool    _value = false;

    public:
        explicit BoolProperty () = default;

        BoolProperty ( BoolProperty const & ) = delete;
        BoolProperty &operator = ( BoolProperty const & ) = delete;
        BoolProperty &operator = ( bool value ) noexcept;

        BoolProperty ( BoolProperty && ) = delete;
        BoolProperty &operator = ( BoolProperty && ) = delete;

        BoolProperty ( bool value ) noexcept;

        ~BoolProperty () override = default;

        [[nodiscard]] operator bool () const noexcept;
};

} // namespace editor


#endif // EDITOR_BOOL_PROPERTY_HPP
