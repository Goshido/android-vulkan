#include <pbr/script_component.h>
#include <pbr/script_engine.h>
#include <guid_generator.h>


namespace pbr {

[[maybe_unused]] int ScriptComponent::_registerScriptComponentIndex = std::numeric_limits<int>::max ();

ScriptComponent::ScriptComponent ( std::string &&script ) noexcept:
    Component ( ClassID::Script, android_vulkan::GUID::GenerateAsString ( "Script" ) ),
    _script ( std::move ( script ) )
{
    // NOTHING
}

ScriptComponent::ScriptComponent ( std::string &&script, std::string &&params ) noexcept:
    Component ( ClassID::Script, android_vulkan::GUID::GenerateAsString ( "Script" ) ),
    _script ( std::move ( script ) ),
    _params ( std::move ( params ) )
{
    // NOTHING
}

bool ScriptComponent::Register ( ScriptEngine &scriptEngine ) noexcept
{
    if ( _params.empty () )
        return scriptEngine.AppendScript ( this, _script );

    return scriptEngine.AppendScript ( this, _script, _params );
}

bool ScriptComponent::Init ( lua_State &/*vm*/ ) noexcept
{
    // TODO
    return true;
}

int ScriptComponent::OnCreate ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

} // namespace pbr
