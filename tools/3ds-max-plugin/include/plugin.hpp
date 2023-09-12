#ifndef AVP_PLUGIN_HPP
#define AVP_PLUGIN_HPP


#include <class_desc.hpp>


namespace avp {

class Plugin final
{
    private:
        ClassDesc       _classDesc {};

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
            return _T ( "android-vulkan" );
        }

        [[nodiscard]] constexpr static int GetNumberClasses () noexcept
        {
            return 1;
        }
};

} // namespace avp


#endif // AVP_PLUGIN_HPP
