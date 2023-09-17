#ifndef AVP_RESULT_CHECKER_HPP
#define AVP_RESULT_CHECKER_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <Windows.h>

GX_RESTORE_WARNING_STATE


namespace avp {

[[nodiscard]] bool CheckResult ( bool result, HWND parent, char const* message, UINT icon ) noexcept;

} // namespace avp


#endif // AVP_RESULT_CHECKER_HPP
