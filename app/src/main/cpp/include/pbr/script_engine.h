#ifndef PBR_SCRIPT_ENGINE_H
#define PBR_SCRIPT_ENGINE_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

// Note "lstate.h" defines 'ispow2' macro which breaks bit.h C++ include.
#include <string_view>
#include <renderer.h>
#include <sound_mixer.h>

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptEngine final
{
    private:
        using Deleter = void ( * ) ( lua_State* state ) noexcept;

    private:
        std::unique_ptr<lua_State, Deleter>     _vm { nullptr, &ScriptEngine::Free };
        static ScriptEngine*                    _instance;

    public:
        ScriptEngine ( ScriptEngine const & ) = delete;
        ScriptEngine& operator = ( ScriptEngine const & ) = delete;

        ScriptEngine ( ScriptEngine && ) = delete;
        ScriptEngine& operator = ( ScriptEngine && ) = delete;

        [[nodiscard]] lua_State& GetVirtualMachine () noexcept;
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, android_vulkan::SoundMixer &soundMixer ) noexcept;

        [[nodiscard]] static ScriptEngine& GetInstance () noexcept;
        static void Destroy () noexcept;

        [[nodiscard]] constexpr static int GetErrorHandlerIndex () noexcept
        {
            return 1;
        }

    private:
        ScriptEngine () = default;
        ~ScriptEngine () = default;

        void BanFunctions () const noexcept;

        [[nodiscard]] bool ExtendFrontend ( android_vulkan::Renderer &renderer,
            android_vulkan::SoundMixer &soundMixer
        ) const noexcept;

        [[nodiscard]] bool InitInterfaceFunctions ( android_vulkan::Renderer &renderer,
            android_vulkan::SoundMixer &soundMixer
        ) noexcept;

        [[nodiscard]] bool InitLua () noexcept;
        void InitLibraries () const noexcept;
        void InitLogFacility () const noexcept;
        [[nodiscard]] bool InitCustomLoader () const noexcept;

        static void Free ( lua_State* state ) noexcept;

        [[nodiscard]] static bool LoadScript ( lua_State* vm,
            std::string_view const &physicalPath,
            std::string_view const &logicalPath
        ) noexcept;

        // Lua VM handlers.
        [[nodiscard]] static void* OnAlloc ( void* ud, void* ptr, size_t osize, size_t nsize );
        [[nodiscard]] static int OnErrorHandler ( lua_State* state );
        [[nodiscard]] static int OnPanic ( lua_State* state );
        [[nodiscard]] static int OnRequire ( lua_State* state );
        static void OnWarning ( void* ud, char const* message, int tocont );
};

} // namespace pbr


#endif // PBR_SCRIPT_ENGINE_H
