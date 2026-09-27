#ifndef ANDROID_VULKAN_SCOPE_GUARD_HPP
#define ANDROID_VULKAN_SCOPE_GUARD_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <utility>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

template<typename T>
class ScopeGuard final
{
    private:
        T       _onScopeEndAction;

    public:
        ScopeGuard () = delete;

        ScopeGuard ( ScopeGuard const & ) = delete;
        ScopeGuard &operator = ( ScopeGuard const & ) = delete;

        ScopeGuard ( ScopeGuard && ) = delete;
        ScopeGuard &operator = ( ScopeGuard && ) = delete;

        explicit ScopeGuard ( T &&onScopeEndAction ) noexcept:
            _onScopeEndAction ( std::move ( onScopeEndAction ) )
        {
            // NOTHING
        }

        ~ScopeGuard () noexcept
        {
            _onScopeEndAction ();
        }
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SCOPE_GUARD_HPP
