#ifndef PBR_SCRIPTABLE_MATERIAL_HPP
#define PBR_SCRIPTABLE_MATERIAL_HPP


#include "types.hpp"

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

#include <unordered_map>
#include <vector>

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptableMaterial final
{
    private:
        static size_t                                               _commandBufferIndex;
        static std::vector<VkCommandBuffer>                         _commandBuffers;
        static VkCommandPool                                        _commandPool;
        static std::vector<VkFence>                                 _fences;
        static std::unordered_map<Material const*, MaterialRef>     _materials;
        static android_vulkan::Renderer*                            _renderer;

    public:
        ScriptableMaterial () = delete;

        ScriptableMaterial ( ScriptableMaterial const & ) = delete;
        ScriptableMaterial &operator = ( ScriptableMaterial const & ) = delete;

        ScriptableMaterial ( ScriptableMaterial && ) = delete;
        ScriptableMaterial &operator = ( ScriptableMaterial && ) = delete;

        ~ScriptableMaterial () = delete;

        [[nodiscard]] static bool Init ( lua_State &vm, android_vulkan::Renderer &renderer ) noexcept;
        static void Destroy () noexcept;

        [[nodiscard]] static MaterialRef &GetReference ( Material const &handle ) noexcept;

        // Waiting until all texture data will be uploaded to GPU.
        [[nodiscard]] static bool Sync () noexcept;

    private:
        [[nodiscard]] static bool AllocateCommandBuffers ( size_t amount ) noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_MATERIAL_HPP
