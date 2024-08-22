#ifndef EDITOR_CRASH_DUMP_HPP
#define EDITOR_CRASH_DUMP_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <Windows.h>

GX_RESTORE_WARNING_STATE


namespace editor {

class CrashDump final
{
    public:
        CrashDump () = delete;

        CrashDump ( CrashDump const & ) = delete;
        CrashDump &operator = ( CrashDump const & ) = delete;

        CrashDump ( CrashDump && ) = delete;
        CrashDump &operator = ( CrashDump && ) = delete;

        ~CrashDump () = default;

        [[nodiscard]] static bool Install () noexcept;

    private:
        [[nodiscard]] static LONG CALLBACK CrashHandler ( EXCEPTION_POINTERS* exceptionPointers );
        [[nodiscard]] static bool CreateDumpDirectory () noexcept;
};

} // namespace editor


#endif // EDITOR_CRASH_DUMP_HPP
