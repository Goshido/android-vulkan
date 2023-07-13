#ifndef PBR_POSITION_PROPERTY_HPP
#define PBR_POSITION_PROPERTY_HPP


#include "property.hpp"


namespace pbr {

class PositionProperty final : public Property
{
    public:
        enum class eValue : uint8_t
        {
            Absolute,
            Static
        };

    private:
        eValue const    _value;

    public:
        PositionProperty () = delete;

        PositionProperty ( PositionProperty const & ) = delete;
        PositionProperty &operator = ( PositionProperty const & ) = delete;

        PositionProperty ( PositionProperty && ) = delete;
        PositionProperty &operator = ( PositionProperty && ) = delete;

        explicit PositionProperty ( eValue value ) noexcept;

        ~PositionProperty () override = default;

        [[nodiscard]] eValue GetValue () const noexcept;
};

} // namespace pbr


#endif // PBR_POSITION_PROPERTY_HPP
