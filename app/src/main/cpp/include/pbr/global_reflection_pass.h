#ifndef PBR_GLOBAL_REFLECTION_PASS_H
#define PBR_GLOBAL_REFLECTION_PASS_H


#include "types.h"


namespace pbr {

class [[maybe_unused]] GlobalReflectionPass final
{
    private:
        std::vector<TextureCubeRef>     _prefilters;

    public:
        GlobalReflectionPass () noexcept;

        GlobalReflectionPass ( GlobalReflectionPass const & ) = delete;
        GlobalReflectionPass& operator = ( GlobalReflectionPass const & ) = delete;

        GlobalReflectionPass ( GlobalReflectionPass && ) = delete;
        GlobalReflectionPass& operator = ( GlobalReflectionPass && ) = delete;

        ~GlobalReflectionPass () = default;

        void Append ( TextureCubeRef &prefilter );
        void Reset ();
};

} // namespace pbr

#endif // PBR_GLOBAL_REFLECTION_PASS_H
