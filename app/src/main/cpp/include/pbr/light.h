#ifndef PBR_LIGHT_H
#define PBR_LIGHT_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>

GX_RESTORE_WARNING_STATE


namespace pbr {

enum class eLightType : uint8_t
{
    PointLight,
    ReflectionGlobal,
    ReflectionLocal
};

class Light
{
    private:
        eLightType      _type;

    public:
        Light () = delete;

        Light ( Light const & ) = delete;
        Light& operator = ( Light const & ) = delete;

        Light ( Light && ) = delete;
        Light& operator = ( Light && ) = delete;

        [[nodiscard]] eLightType GetType () const;

    protected:
        explicit Light ( eLightType type ) noexcept;
        virtual ~Light () = default;
};

} // namespace pbr


#endif // PBR_LIGHT_H
