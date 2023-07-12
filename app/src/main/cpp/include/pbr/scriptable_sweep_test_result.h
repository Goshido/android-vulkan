#ifndef PBR_SCRIPTABLE_SWEEP_TEST_RESULT_H
#define PBR_SCRIPTABLE_SWEEP_TEST_RESULT_H


#include <physics.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptableSweepTestResult final
{
    public:
        ScriptableSweepTestResult () = delete;

        ScriptableSweepTestResult ( ScriptableSweepTestResult const & ) = delete;
        ScriptableSweepTestResult &operator = ( ScriptableSweepTestResult const & ) = delete;

        ScriptableSweepTestResult ( ScriptableSweepTestResult && ) = delete;
        ScriptableSweepTestResult &operator = ( ScriptableSweepTestResult && ) = delete;

        ~ScriptableSweepTestResult () = delete;

        [[nodiscard]] static bool Init ( lua_State &vm ) noexcept;
        static void Destroy ( lua_State &vm ) noexcept;

        [[nodiscard]] static bool PublishResult ( lua_State &vm,
            std::vector<android_vulkan::RigidBodyRef> const &sweepTestResult
        ) noexcept;
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_SWEEP_TEST_RESULT_H
