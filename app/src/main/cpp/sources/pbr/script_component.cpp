#include <pbr/script_component.h>
#include <pbr/script_engine.h>


namespace pbr {

ScriptComponent::ScriptComponent ( std::string &&script ) noexcept:
    Component ( ClassID::Script ),
    _script ( std::move ( script ) )
{
    // NOTHING
}

ScriptComponent::ScriptComponent ( std::string &&script, std::string &&params ) noexcept:
    Component ( ClassID::Script ),
    _script ( std::move ( script ) ),
    _params ( std::move ( params ) )
{
    // NOTHING
}

bool ScriptComponent::Register () noexcept
{
    if ( _params.empty () )
        return ScriptEngine::GetInstance ().AppendScript ( this, _script );

    return ScriptEngine::GetInstance ().AppendScript ( this, _script, _params );
}

bool ScriptComponent::IsRenderable () const noexcept
{
    return false;
}

} // namespace pbr
