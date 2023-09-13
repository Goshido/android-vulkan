#ifndef AVP_UTILITY_HPP
#define AVP_UTILITY_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <shtypes.h>
#include <unordered_map>
#include <utilapi.h>

GX_RESTORE_WARNING_STATE


namespace avp {

class Utility final : public UtilityObj
{
    private:
        using ButtonHandler = void ( Utility::* ) () noexcept;
        using ButtonHandlers = std::unordered_map<uint16_t, ButtonHandler>;

        using MessageHandler = INT_PTR ( Utility::* ) ( UINT msg, WPARAM wParam, LPARAM lParam ) noexcept;
        using MessageHandlers = std::unordered_map<UINT, MessageHandler>;

    private:
        ButtonHandlers      _buttonHandlers {};
        HINSTANCE           _instance = nullptr;
        Interface*          _interfacePointer = nullptr;
        MessageHandlers     _messageHandlers {};
        HWND                _ui = nullptr;

    public:
        Utility () = default;

        Utility ( Utility const & ) = delete;
        Utility &operator = ( Utility const & ) = delete;

        Utility ( Utility && ) = delete;
        Utility &operator = ( Utility && ) = delete;

        ~Utility () override = default;

        void Init ( HINSTANCE instance ) noexcept;

    private:
        void BeginEditParams ( Interface* interfacePointer, IUtil* util ) override;
        void EndEditParams ( Interface* interfacePointer, IUtil* util ) override;
        void SelectionSetChanged ( Interface* interfacePointer, IUtil* util ) override;
        void DeleteThis () override;

        [[nodiscard]] bool CheckCOMResult ( HRESULT result, char const* message ) const noexcept;
        void GetPath ( COMDLG_FILTERSPEC filter, wchar_t const* extention, int targetID ) noexcept;

        [[nodiscard]] INT_PTR OnCommand ( UINT msg, WPARAM wParam, LPARAM lParam ) noexcept;
        [[nodiscard]] INT_PTR OnMouseEvent ( UINT msg, WPARAM wParam, LPARAM lParam ) noexcept;

        void OnBrowseAnimationFile () noexcept;
        void OnBrowseMeshFile () noexcept;
        void OnBrowseSkeletonFile () noexcept;
        void OnBrowseSkinFile () noexcept;

        void OnExportAnimation () noexcept;
        void OnExportMesh () noexcept;
        void OnExportSkeleton () noexcept;
        void OnExportSkin () noexcept;

        [[nodiscard]] static INT_PTR CALLBACK DialogProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
};

} // namespace avp


#endif // AVP_UTILITY_HPP
