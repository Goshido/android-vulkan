#include <mesh_exporter.hpp>
#include <resource.hpp>
#include <utility.hpp>
#include <version.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <maxapi.h>
#include <Shobjidl.h>

GX_RESTORE_WARNING_STATE


#define AVP_STR( x )            L#x
#define AVP_STRINGAZE( x )      AVP_STR ( x )

#define AVP_CAPTION                         \
    L"android-vulkan v"                     \
    AVP_STRINGAZE ( AVP_VERSION_MAJOR )     \
    L"."                                    \
    AVP_STRINGAZE ( AVP_VERSION_MINOR )     \
    L"."                                    \
    AVP_STRINGAZE ( AVP_VERSION_RELEASE )   \
    L"."                                    \
    AVP_STRINGAZE ( AVP_VERSION_BUILD )

//----------------------------------------------------------------------------------------------------------------------

namespace avp {

namespace {

class AutoRelease final
{
    private:
        IUnknown*       _com;

    public:
        AutoRelease () = delete;

        AutoRelease ( AutoRelease const & ) = delete;
        AutoRelease &operator = ( AutoRelease const & ) = delete;

        AutoRelease ( AutoRelease && ) = delete;
        AutoRelease &operator = ( AutoRelease && ) = delete;

        explicit AutoRelease ( IUnknown* com ) noexcept;

        ~AutoRelease () noexcept;
};

AutoRelease::AutoRelease ( IUnknown* com ) noexcept:
    _com ( com )
{
    // NOTHING
}

AutoRelease::~AutoRelease ()
{
    if ( _com )
    {
        _com->Release ();
    }
}

} // end of anonymous namespace

void Utility::Init ( HINSTANCE instance ) noexcept
{
    _instance = instance;

    _buttonHandlers.emplace ( static_cast<uint16_t> ( AVP_UI_BUTTON_BROWSE_ANIMATION_FILE ),
        &Utility::OnBrowseAnimationFile
    );

    _buttonHandlers.emplace ( static_cast<uint16_t> ( AVP_UI_BUTTON_BROWSE_MESH_FILE ), &Utility::OnBrowseMeshFile );

    _buttonHandlers.emplace ( static_cast<uint16_t> ( AVP_UI_BUTTON_BROWSE_SKELETON_FILE ),
        &Utility::OnBrowseSkeletonFile
    );

    _buttonHandlers.emplace ( static_cast<uint16_t> ( AVP_UI_BUTTON_BROWSE_SKIN_FILE ), &Utility::OnBrowseSkinFile );

    _buttonHandlers.emplace ( static_cast<uint16_t> ( AVP_UI_BUTTON_EXPORT_ANIMATION ), &Utility::OnExportAnimation );
    _buttonHandlers.emplace ( static_cast<uint16_t> ( AVP_UI_BUTTON_EXPORT_MESH ), &Utility::OnExportMesh );
    _buttonHandlers.emplace ( static_cast<uint16_t> ( AVP_UI_BUTTON_EXPORT_SKELETON ), &Utility::OnExportSkeleton );
    _buttonHandlers.emplace ( static_cast<uint16_t> ( AVP_UI_BUTTON_EXPORT_SKIN ), &Utility::OnExportSkin );

    _messageHandlers.emplace ( static_cast<UINT> ( WM_COMMAND ), &Utility::OnCommand );
    _messageHandlers.emplace ( static_cast<UINT> ( WM_LBUTTONDOWN ), &Utility::OnMouseEvent );
    _messageHandlers.emplace ( static_cast<UINT> ( WM_LBUTTONUP ), &Utility::OnMouseEvent );
    _messageHandlers.emplace ( static_cast<UINT> ( WM_MOUSEMOVE ), &Utility::OnMouseEvent );
}

void Utility::BeginEditParams ( Interface* interfacePointer, IUtil* /*util*/ )
{
    _interfacePointer = interfacePointer;

    _ui = interfacePointer->AddRollupPage ( _instance,
        MAKEINTRESOURCE ( AVP_IDD_DIALOG ),
        &Utility::DialogProc,
        AVP_CAPTION,
        reinterpret_cast<LPARAM> ( this )
    );
}

void Utility::EndEditParams ( Interface* interfacePointer, IUtil* /*util*/ )
{
    interfacePointer->DeleteRollupPage ( _ui );
}

void Utility::SelectionSetChanged ( Interface* /*interfacePointer*/, IUtil* /*util*/ )
{
    // NOTHING
}

void Utility::DeleteThis ()
{
    // NOTHING
}

bool Utility::CheckCOMResult ( HRESULT result, char const *message ) const noexcept
{
    if ( SUCCEEDED ( result ) )
        return true;

    char buffer[ 1024U ];
    std::snprintf ( buffer, std::size ( buffer ), "%s. Error: 0x%08X", message, static_cast<uint32_t> ( result ) );
    MessageBoxA ( _ui, buffer, "android-vulkan error", MB_ICONERROR );
    return false;
}

void Utility::GetPath ( COMDLG_FILTERSPEC filter, wchar_t const* extention, int targetID ) noexcept
{
    IFileSaveDialog* fileSaveDialog = nullptr;

    bool result = CheckCOMResult (
        CoCreateInstance ( CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS ( &fileSaveDialog ) ),
        "Can't acquire system save dialog"
    );

    if ( !result )
        return;

    AutoRelease const autoRelease0 ( fileSaveDialog );
    FILEOPENDIALOGOPTIONS flags;

    result = CheckCOMResult ( fileSaveDialog->GetOptions ( &flags ), "Can't get save dialog settings" ) &&

        CheckCOMResult ( fileSaveDialog->SetOptions ( flags | FOS_FORCEFILESYSTEM ),
            "Can't setup save dialog settings"
        ) &&

        CheckCOMResult ( fileSaveDialog->SetDefaultExtension ( extention ), "Can't setup save dialog extension" ) &&
        CheckCOMResult ( fileSaveDialog->SetFileTypes ( 1U, &filter ), "Can't setup save dialog filter" );

    if ( !result )
        return;

    HRESULT const status = fileSaveDialog->Show ( _ui );

    if ( HRESULT_CODE ( status ) == ERROR_CANCELLED || !CheckCOMResult ( status, "Can't show save dialog" ) )
        return;

    IShellItem* item = nullptr;

    if ( !CheckCOMResult ( fileSaveDialog->GetResult ( &item ), "Can't get path item" ) )
        return;

    AutoRelease const autoRelease1 ( item );
    PWSTR path = nullptr;

    if ( !CheckCOMResult ( item->GetDisplayName ( SIGDN_FILESYSPATH, &path ), "Can't get path" ) )
        return;

    ICustEdit* edit = GetICustEdit ( GetDlgItem ( _ui, targetID ) );
    edit->SetText ( path );
    ReleaseICustEdit ( edit );

    CoTaskMemFree ( path );
}

INT_PTR Utility::OnCommand ( UINT /*msg*/, WPARAM wParam, LPARAM /*lParam*/ ) noexcept
{
    auto const findResult = _buttonHandlers.find ( static_cast<uint16_t> ( LOWORD ( wParam ) ) );

    if ( findResult == _buttonHandlers.cend () )
        return FALSE;

    ButtonHandler const handler = findResult->second;

    // Calling method by pointer C++ syntax.
    ( this->*handler ) ();
    return TRUE;
}

INT_PTR Utility::OnMouseEvent ( UINT msg, WPARAM wParam, LPARAM lParam ) noexcept
{
    _interfacePointer->RollupMouseMessage ( _ui, msg, wParam, lParam );
    return TRUE;
}

void Utility::OnBrowseAnimationFile () noexcept
{
    GetPath (
        COMDLG_FILTERSPEC
        {
            .pszName = L"animation",
            .pszSpec = L"*.animation"
        },

        L"animation",
        AVP_UI_EDITBOX_ANIMATION_FILE
    );
}

void Utility::OnBrowseMeshFile () noexcept
{
    GetPath (
        COMDLG_FILTERSPEC
        {
            .pszName = L"mesh2",
            .pszSpec = L"*.mesh2"
        },

        L"mesh2",
        AVP_UI_EDITBOX_MESH_FILE
    );
}

void Utility::OnBrowseSkeletonFile () noexcept
{
    GetPath (
        COMDLG_FILTERSPEC
        {
            .pszName = L"skeleton",
            .pszSpec = L"*.skeleton"
        },

        L"skeleton",
        AVP_UI_EDITBOX_SKELETON_FILE
    );
}

void Utility::OnBrowseSkinFile () noexcept
{
    GetPath (
        COMDLG_FILTERSPEC
        {
            .pszName = L"skin",
            .pszSpec = L"*.skin"
        },

        L"skin",
        AVP_UI_EDITBOX_SKIN_FILE
    );
}

void Utility::OnExportAnimation () noexcept
{
    MessageBoxA ( _ui, "OnExportAnimation", "android-vulkan", MB_ICONINFORMATION );
}

void Utility::OnExportMesh () noexcept
{
    ICustEdit* edit = GetICustEdit ( GetDlgItem ( _ui, AVP_UI_EDITBOX_MESH_FILE ) );

    MSTR path {};
    edit->GetText ( path );

    MeshExporter const exporter ( _ui,
        path,
        static_cast<bool> ( IsDlgButtonChecked ( _ui, AVP_UI_CHECKBOX_CURRENT_POSE ) )
    );

    ReleaseICustEdit ( edit );
}

void Utility::OnExportSkeleton () noexcept
{
    // TODO
    MessageBoxA ( _ui, "OnExportSkeleton", "android-vulkan", MB_ICONINFORMATION );
}

void Utility::OnExportSkin () noexcept
{
    // TODO
    MessageBoxA ( _ui, "OnExportSkin", "android-vulkan", MB_ICONINFORMATION );
}

INT_PTR CALLBACK Utility::DialogProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    auto* userData = reinterpret_cast<void*> ( GetWindowLongPtr ( hwnd, GWLP_USERDATA ) );

    if ( !userData )
    {
        if ( msg != WM_INITDIALOG )
            return FALSE;

        //  Finishing initialization. 'lParam' contains pointer to Utility instance.
        SetWindowLongPtr ( hwnd, GWLP_USERDATA, static_cast<LONG_PTR> ( lParam ) );
        SendDlgItemMessage ( hwnd, AVP_UI_CHECKBOX_CURRENT_POSE, BM_SETCHECK, BST_UNCHECKED, 0 );
        return TRUE;
    }

    auto &utility = *reinterpret_cast<Utility*> ( userData );
    MessageHandlers const &messageHandlers = utility._messageHandlers;
    auto const findResult = messageHandlers.find ( msg );

    if ( findResult == messageHandlers.cend () )
        return FALSE;

    MessageHandler const handler = findResult->second;

    // Calling method by pointer C++ syntax.
    return ( utility.*handler ) ( msg, wParam, lParam );
}

} // namespace avp
