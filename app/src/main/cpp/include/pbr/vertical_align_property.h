#ifndef PBR_VERTICAL_ALIGN_PROPERTY_H
#define PBR_VERTICAL_ALIGN_PROPERTY_H


#include "property.h"


namespace pbr {

class VerticalAlignProperty final : public Property
{
    public:
        enum class eValue : uint8_t
        {
            Bottom,
            Middle,
            Top,
            Inherit
        };

    private:
        eValue      _value = eValue::Inherit;

    public:
        VerticalAlignProperty () = delete;

        VerticalAlignProperty ( VerticalAlignProperty const & ) = default;
        VerticalAlignProperty &operator = ( VerticalAlignProperty const & ) = default;

        VerticalAlignProperty ( VerticalAlignProperty && ) = default;
        VerticalAlignProperty &operator = ( VerticalAlignProperty && ) = default;

        explicit VerticalAlignProperty ( eValue value ) noexcept;

        ~VerticalAlignProperty () override = default;

        [[nodiscard]] eValue GetValue () const noexcept;
};

} // namespace pbr


#endif // PBR_VERTICAL_ALIGN_PROPERTY_H
