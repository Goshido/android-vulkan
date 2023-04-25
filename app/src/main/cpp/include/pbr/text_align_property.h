#ifndef PBR_TEXT_ALIGN_PROPERTY_H
#define PBR_TEXT_ALIGN_PROPERTY_H


#include "property.h"


namespace pbr {

class TextAlignProperty final : public Property
{
    public:
        enum class eValue : uint8_t
        {
            Center,
            Left,
            Right
        };

    private:
        eValue      _value = eValue::Left;

    public:
        TextAlignProperty () = delete;

        TextAlignProperty ( TextAlignProperty const & ) = default;
        TextAlignProperty& operator = ( TextAlignProperty const & ) = default;

        TextAlignProperty ( TextAlignProperty && ) = default;
        TextAlignProperty& operator = ( TextAlignProperty && ) = default;

        explicit TextAlignProperty ( eValue value ) noexcept;

        ~TextAlignProperty () override = default;

        [[nodiscard]] eValue GetValue () const noexcept;
};

} // namespace pbr


#endif // PBR_TEXT_ALIGN_PROPERTY_H
