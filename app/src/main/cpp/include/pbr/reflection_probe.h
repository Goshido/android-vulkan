#ifndef PBR_REFLECTION_PROBE_H
#define PBR_REFLECTION_PROBE_H


#include "light.h"
#include "types.h"


namespace pbr {

class ReflectionProbe : public Light
{
    private:
        TextureCubeRef      _prefilter;

    public:
        ReflectionProbe () = delete;

        ReflectionProbe ( ReflectionProbe const & ) = delete;
        ReflectionProbe& operator = ( ReflectionProbe const & ) = delete;

        ReflectionProbe ( ReflectionProbe && ) = delete;
        ReflectionProbe& operator = ( ReflectionProbe && ) = delete;

        void FreeTransferResources ( VkDevice device ) noexcept;
        [[nodiscard]] TextureCubeRef& GetPrefilter () noexcept;

    protected:
        explicit ReflectionProbe ( eLightType type, TextureCubeRef &prefilter ) noexcept;
        ~ReflectionProbe () override = default;
};

} // namespace pbr


#endif // PBR_REFLECTION_PROBE_H
