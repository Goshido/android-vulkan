#include <plugin.hpp>


namespace avp {

ClassDesc* Plugin::GetClassDesc ( int /*idx*/ ) noexcept
{
    // TODO
    return nullptr;
}

void Plugin::Init ( HINSTANCE instance ) noexcept
{
    _instance = instance;
}

} // namespace avp
