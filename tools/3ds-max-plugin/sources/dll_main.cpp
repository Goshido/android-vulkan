#include <plugin.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <Windows.h>
#include <3ds_max_sdk/plugin.h>
#include <3ds_max_sdk/plugapi.h>

GX_RESTORE_WARNING_STATE


namespace {

avp::Plugin g_plugin {};

} // end of anonymous namespace

BOOL WINAPI DllMain ( HINSTANCE instance, DWORD /*reason*/, LPVOID /*reserved*/ )
{
    g_plugin.Init ( instance );
    return TRUE;
}

__declspec ( dllexport ) TCHAR const* LibDescription ()
{
    return avp::Plugin::GetDescription ();
}

__declspec ( dllexport ) int LibNumberClasses ()
{
    return avp::Plugin::GetNumberClasses ();
}

__declspec ( dllexport ) ClassDesc* LibClassDesc ( int idx )
{
    return g_plugin.GetClassDesc ( idx );
}

__declspec ( dllexport ) ULONG LibVersion ()
{
    return VERSION_3DSMAX;
}

__declspec ( dllexport ) ULONG CanAutoDefer ()
{
    return avp::Plugin::CouldAutoDefer ();
}
