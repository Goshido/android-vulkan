#include <pbr/scriptable_gamepad.h>


namespace pbr {

void ScriptableGamepad::CaptureInput () noexcept
{
    android_vulkan::Gamepad& gamepad = android_vulkan::Gamepad::GetInstance ();

    auto bind = [ & ] ( KeyContext*& context, android_vulkan::eGamepadKey key ) noexcept {
        context->_instance = this;
        context->_key = key;
        context->_state = android_vulkan::eButtonState::Down;
        gamepad.BindKey ( context, &ScriptableGamepad::OnKeyDown, key, android_vulkan::eButtonState::Down );
        ++context;

        context->_instance = this;
        context->_key = key;
        context->_state = android_vulkan::eButtonState::Up;
        gamepad.BindKey ( context, &ScriptableGamepad::OnKeyUp, key, android_vulkan::eButtonState::Up );
        ++context;
    };

    KeyContext* context = _keyContexts;

    bind ( context, android_vulkan::eGamepadKey::A );
    bind ( context, android_vulkan::eGamepadKey::B );
    bind ( context, android_vulkan::eGamepadKey::Down );
    bind ( context, android_vulkan::eGamepadKey::Home );
    bind ( context, android_vulkan::eGamepadKey::Left );
    bind ( context, android_vulkan::eGamepadKey::LeftBumper );
    bind ( context, android_vulkan::eGamepadKey::LeftStick );
    bind ( context, android_vulkan::eGamepadKey::Menu );
    bind ( context, android_vulkan::eGamepadKey::Right );
    bind ( context, android_vulkan::eGamepadKey::RightBumper );
    bind ( context, android_vulkan::eGamepadKey::RightStick );
    bind ( context, android_vulkan::eGamepadKey::Up );
    bind ( context, android_vulkan::eGamepadKey::View );
    bind ( context, android_vulkan::eGamepadKey::X );
    bind ( context, android_vulkan::eGamepadKey::Y );
}

// NOLINTNEXTLINE - could be static method
void ScriptableGamepad::ReleaseInput () noexcept
{
    android_vulkan::Gamepad& gamepad = android_vulkan::Gamepad::GetInstance ();
    
    auto unbind = [ & ] ( android_vulkan::eGamepadKey key ) noexcept {
        gamepad.UnbindKey ( key, android_vulkan::eButtonState::Down );
        gamepad.UnbindKey ( key, android_vulkan::eButtonState::Up );
    };

    unbind ( android_vulkan::eGamepadKey::A );
    unbind ( android_vulkan::eGamepadKey::B );
    unbind ( android_vulkan::eGamepadKey::Down );
    unbind ( android_vulkan::eGamepadKey::Home );
    unbind ( android_vulkan::eGamepadKey::Left );
    unbind ( android_vulkan::eGamepadKey::LeftBumper );
    unbind ( android_vulkan::eGamepadKey::LeftStick );
    unbind ( android_vulkan::eGamepadKey::Menu );
    unbind ( android_vulkan::eGamepadKey::Right );
    unbind ( android_vulkan::eGamepadKey::RightBumper );
    unbind ( android_vulkan::eGamepadKey::RightStick );
    unbind ( android_vulkan::eGamepadKey::Up );
    unbind ( android_vulkan::eGamepadKey::View );
    unbind ( android_vulkan::eGamepadKey::X );
    unbind ( android_vulkan::eGamepadKey::Y );
}

void ScriptableGamepad::OnKeyDown ( void* context ) noexcept
{
    auto const& ctx = *static_cast<KeyContext const*> ( context );
    (void)ctx;

    // TODO
}

void ScriptableGamepad::OnKeyUp ( void* context ) noexcept
{
    auto const& ctx = *static_cast<KeyContext const*> ( context );
    (void)ctx;

    // TODO
}

} // namespace pbr
