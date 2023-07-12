#ifndef PBR_DISPLAY_PROPERTY_H
#define PBR_DISPLAY_PROPERTY_H


#include "property.h"


namespace pbr {

class DisplayProperty final : public Property
{
    public:
        enum class eValue : uint8_t
        {
            Block,
            InlineBlock,
            None
        };

    private:
        eValue const    _value;

    public:
        DisplayProperty () = delete;

        DisplayProperty ( DisplayProperty const & ) = delete;
        DisplayProperty &operator = ( DisplayProperty const & ) = delete;

        DisplayProperty ( DisplayProperty && ) = delete;
        DisplayProperty &operator = ( DisplayProperty && ) = delete;

        explicit DisplayProperty ( eValue value ) noexcept;

        ~DisplayProperty () override = default;

        [[nodiscard]] eValue GetValue () const noexcept;
};

} // namespace pbr


#endif // PBR_DISPLAY_PROPERTY_H
