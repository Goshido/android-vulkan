#ifndef PBR_REFLECTION_GLOBAL_PASS_H
#define PBR_REFLECTION_GLOBAL_PASS_H


#include "types.h"


namespace pbr {

class [[maybe_unused]] ReflectionGlobalPass final
{
    private:
        std::vector<TextureCubeRef>     _prefilters;

    public:
        ReflectionGlobalPass () noexcept;

        ReflectionGlobalPass ( ReflectionGlobalPass const & ) = delete;
        ReflectionGlobalPass& operator = ( ReflectionGlobalPass const & ) = delete;

        ReflectionGlobalPass ( ReflectionGlobalPass && ) = delete;
        ReflectionGlobalPass& operator = ( ReflectionGlobalPass && ) = delete;

        ~ReflectionGlobalPass () = default;

        void Append ( TextureCubeRef &prefilter );
        void Reset ();
};

} // namespace pbr

#endif // PBR_REFLECTION_GLOBAL_PASS_H
