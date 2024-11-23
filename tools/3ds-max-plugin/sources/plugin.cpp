#include <precompiled_headers.hpp>
#include <plugin.hpp>


namespace avp {

ClassDesc* Plugin::GetClassDesc ( int idx ) noexcept
{
    ClassDesc* cases[] = { &_classDesc, nullptr };
    return cases[ static_cast<size_t> ( idx >= GetNumberClasses () ) ];
}

void Plugin::Init ( HINSTANCE instance ) noexcept
{
    _classDesc.Init ( instance );
}

} // namespace avp
