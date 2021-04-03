#ifndef PBR_REFLECTION_LOCAL_PASS_H
#define PBR_REFLECTION_LOCAL_PASS_H


#include "reflection_local_program.h"
#include "types.h"


namespace pbr {

class ReflectionLocalPass final
{
    private:
        struct Call final
        {
            GXVec3                  _location;
            TextureCubeRef          _prefilter;
            float                   _size;

            Call () = default;

            Call ( Call const & ) = default;
            Call& operator = ( Call const & ) = default;

            Call ( Call && ) = default;
            Call& operator = ( Call && ) = default;

            explicit Call ( GXVec3 const &location, TextureCubeRef &prefilter, float size );

            ~Call () = default;
        };

    private:
        std::vector<Call>           _calls;
        ReflectionLocalProgram      _program;

    public:
        ReflectionLocalPass () noexcept;

        ReflectionLocalPass ( ReflectionLocalPass const & ) = delete;
        ReflectionLocalPass& operator = ( ReflectionLocalPass const & ) = delete;

        ReflectionLocalPass ( ReflectionLocalPass && ) = delete;
        ReflectionLocalPass& operator = ( ReflectionLocalPass && ) = delete;

        ~ReflectionLocalPass () = default;

        void Append ( TextureCubeRef &prefilter, GXVec3 const &location, float size );
        [[nodiscard]] size_t GetReflectionLocalCount () const;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        );

        void Destroy ( VkDevice device );

        void Reset ();
};

} // namespace pbr

#endif // PBR_REFLECTION_LOCAL_PASS_H
