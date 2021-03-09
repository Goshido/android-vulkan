#ifndef PBR_REFLECTION_GLOBAL_PASS_H
#define PBR_REFLECTION_GLOBAL_PASS_H


#include "reflection_global_program.h"
#include "types.h"


namespace pbr {

class [[maybe_unused]] ReflectionGlobalPass final
{
    private:
        std::vector<TextureCubeRef>     _prefilters;
        ReflectionGlobalProgram         _program;

    public:
        ReflectionGlobalPass () noexcept;

        ReflectionGlobalPass ( ReflectionGlobalPass const & ) = delete;
        ReflectionGlobalPass& operator = ( ReflectionGlobalPass const & ) = delete;

        ReflectionGlobalPass ( ReflectionGlobalPass && ) = delete;
        ReflectionGlobalPass& operator = ( ReflectionGlobalPass && ) = delete;

        ~ReflectionGlobalPass () = default;

        void Append ( TextureCubeRef &prefilter );
        [[maybe_unused]] void Execute ( GXMat4 const &viewToWorld );

        [[maybe_unused, nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &viewport
        );

        [[maybe_unused]] void Destroy ( VkDevice device );

        void Reset ();
};

} // namespace pbr

#endif // PBR_REFLECTION_GLOBAL_PASS_H
