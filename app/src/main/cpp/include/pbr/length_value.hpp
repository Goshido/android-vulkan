#ifndef PBR_LENGTH_VALUE_HPP
#define PBR_LENGTH_VALUE_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE


namespace pbr {

class LengthValue final
{
    public:
        enum class eType : uint8_t
        {
            Auto,
            EM,
            Inherit,
            MM,
            PT,
            PX,
            Percent,
            Unitless
        };

        struct CalcRest final
        {
            float                   _value = 1.0F;
            eType                   _type = eType::MM;
        };

    private:
        eType                       _type = eType::MM;
        float                       _value = 1.0F;
        std::vector<CalcRest>       _calcRest {};

    public:
        LengthValue () = default;

        LengthValue ( LengthValue const & ) = default;
        LengthValue &operator = ( LengthValue const & ) = default;

        LengthValue ( LengthValue && ) = default;
        LengthValue &operator = ( LengthValue && ) = default;

        explicit LengthValue ( eType type, float value ) noexcept;

        // Constructor for 'calc' CSS constraint.
        // See <repo>/doc/ui-system.md#calc
        explicit LengthValue ( eType firstType, float firstValue, std::vector<CalcRest> &&rest ) noexcept;

        ~LengthValue () = default;

        [[nodiscard]] eType GetType () const noexcept;
        [[nodiscard]] float GetValue () const noexcept;
        [[nodiscard]] std::vector<CalcRest> const &GetCalcInfo () const noexcept;
};

} // namespace pbr


#endif // PBR_LENGTH_VALUE_HPP
