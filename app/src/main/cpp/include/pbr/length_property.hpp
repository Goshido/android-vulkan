#ifndef PBR_LENGTH_PROPERTY_HPP
#define PBR_LENGTH_PROPERTY_HPP


#include "length_value.hpp"
#include "property.hpp"


namespace pbr {

class LengthProperty final : public Property
{
    private:
        LengthValue const       _value;

    public:
        LengthProperty () = delete;

        LengthProperty ( LengthProperty const & ) = delete;
        LengthProperty &operator = ( LengthProperty const & ) = delete;

        LengthProperty ( LengthProperty && ) = delete;
        LengthProperty &operator = ( LengthProperty && ) = delete;

        explicit LengthProperty ( eType type, LengthValue value ) noexcept;

        ~LengthProperty () override = default;

        [[nodiscard]] LengthValue GetValue () const noexcept;
};

} // namespace pbr


#endif // PBR_LENGTH_PROPERTY_HPP
