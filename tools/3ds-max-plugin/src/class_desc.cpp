#include <class_desc.hpp>


namespace avp {

void ClassDesc::Init ( HINSTANCE instance ) noexcept
{
    _instance = instance;
}

int ClassDesc::IsPublic ()
{
    return TRUE;
}

void* ClassDesc::Create ( BOOL /*loading*/ )
{
    // TODO
    return nullptr;
}

MCHAR const* ClassDesc::ClassName ()
{
    return _T ( "android-vulkan" );
}

MCHAR const* ClassDesc::NonLocalizedClassName ()
{
    return _T ( "AndroidVulkanExporter" );
}

MCHAR const* ClassDesc::InternalName ()
{
    constexpr MCHAR const* nonExportableToScripts = nullptr;
    return nonExportableToScripts;
}

SClass_ID ClassDesc::SuperClassID ()
{
    return UTILITY_CLASS_ID;
}

Class_ID ClassDesc::ClassID ()
{
    return Class_ID ( 0x5BA96F0EU, 0x44345F2DU );
}

MCHAR const* ClassDesc::Category ()
{
    return nullptr;
}

HINSTANCE ClassDesc::HInstance ()
{
    return _instance;
}

} // namespace avp
