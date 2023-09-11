#ifndef AVP_PLUGIN_HPP
#define AVP_PLUGIN_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <plugapi.h>

GX_RESTORE_WARNING_STATE


namespace avp {

class Plugin final
{
    private:
        [[maybe_unused]] HINSTANCE      _instance = nullptr;

    public:
        Plugin () = default;

        Plugin ( Plugin const & ) = delete;
        Plugin &operator = ( Plugin const & ) = delete;

        Plugin ( Plugin && ) = delete;
        Plugin &operator = ( Plugin && ) = delete;

        ~Plugin () = default;

        [[nodiscard]] ClassDesc* GetClassDesc ( int idx ) noexcept;
        void Init ( HINSTANCE instance ) noexcept;

        [[nodiscard]] constexpr static ULONG CouldAutoDefer () noexcept
        {
            return 1U;
        }

        [[nodiscard]] constexpr static TCHAR const* GetDescription () noexcept
        {
            return TEXT ( "android-vulkan" );
        }

        [[nodiscard]] constexpr static int GetNumberClasses () noexcept
        {
            return 1;
        }
};

} // namespace avp


#endif // AVP_PLUGIN_HPP
