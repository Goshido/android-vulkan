#include <result_checker.hpp>


namespace avp {

bool CheckResult ( bool result, HWND parent, char const* message, UINT icon ) noexcept
{
    if ( result )
        return true;

    MessageBoxA ( parent, message, "android-vulkan", icon );
    return false;
}

} // namespace avp
