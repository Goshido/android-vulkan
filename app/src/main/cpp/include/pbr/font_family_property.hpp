#ifndef PBR_FONT_FAMILY_PROPERTY_HPP
#define PBR_FONT_FAMILY_PROPERTY_HPP


#include "property.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class FontFamilyProperty final : public Property
{
    private:
        std::u32string const    _value;

    public:
        FontFamilyProperty () = delete;

        FontFamilyProperty ( FontFamilyProperty const & ) = delete;
        FontFamilyProperty &operator = ( FontFamilyProperty const & ) = delete;

        FontFamilyProperty ( FontFamilyProperty && ) = delete;
        FontFamilyProperty &operator = ( FontFamilyProperty && ) = delete;

        explicit FontFamilyProperty ( std::u32string &&value ) noexcept;

        ~FontFamilyProperty () override = default;

        [[nodiscard]] std::u32string const &GetValue () const noexcept;
};

} // namespace pbr


#endif // PBR_FONT_FAMILY_PROPERTY_HPP
