#ifndef PBR_MATERIAL_H
#define PBR_MATERIAL_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>

GX_RESTORE_WARNING_STATE


namespace pbr {

enum class eMaterialType : uint8_t
{
    Opaque = 0U,
    Stipple = 1U
};

class Material
{
    private:
        eMaterialType       _type;

    public:
        Material () = delete;

        [[nodiscard]] eMaterialType GetMaterialType () const;

    protected:
        explicit Material ( eMaterialType type ) noexcept;
        virtual ~Material () = default;

        Material ( const Material & ) = default;
        Material& operator = ( const Material & ) = default;
};

} // namespace pbr


#endif // PBR_MATERIAL_H
