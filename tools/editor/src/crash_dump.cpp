#include <crash_dump.hpp>
#include <logger.hpp>
#include <os_utils.hpp>
#include <version.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <DbgHelp.h>

GX_RESTORE_WARNING_STATE


namespace editor {

namespace {

constexpr char const CRASH_DIRECTORY_TARGET[] = R"__(%APPDATA%\Goshido Inc\Editor\Crash dumps)__";
char CRASH_DIRECTORY[ 256U ];

} // end of anonymous namespace

bool CrashDump::Install () noexcept
{
    std::filesystem::create_directories ( OSUtils::ResolvePath ( CRASH_DIRECTORY ) );
    SetUnhandledExceptionFilter ( &CrashDump::CrashHandler );
    return true;
}

LONG CALLBACK CrashDump::CrashHandler ( EXCEPTION_POINTERS* exceptionPointers )
{
    // Note code here is executed in very unstable environment: stack corruption, heap corruption.
    // Note all valiables are allocated in global memory (not stack) by design to successfully generate crash dump file.
    // see https://stackoverflow.com/questions/5028781/how-to-write-a-sample-code-that-will-crash-and-produce-dump-file
    static FILETIME utcFileTime {};
    GetSystemTimeAsFileTime ( &utcFileTime );

    static FILETIME localFileTime {};
    FileTimeToLocalFileTime ( &utcFileTime, &localFileTime );

    static SYSTEMTIME currentTime {};
    FileTimeToSystemTime ( &localFileTime, &currentTime );

    ExpandEnvironmentStringsA ( CRASH_DIRECTORY_TARGET,
        CRASH_DIRECTORY,
        static_cast<DWORD> ( std::size ( CRASH_DIRECTORY ) )
    );

    static char dumpFilePath[ 256U ];

    std::snprintf ( dumpFilePath,
        std::size ( dumpFilePath ),
        R"__(%s\%d.%d.%d.%d %04hu.%02hu.%02hu %02hu-%02hu-%02hu.dmp)__",
        CRASH_DIRECTORY,
        EDITOR_VERSION_MAJOR,
        EDITOR_VERSION_MINOR,
        EDITOR_VERSION_RELEASE,
        EDITOR_VERSION_BUILD,
        currentTime.wYear,
        currentTime.wMonth,
        currentTime.wDay,
        currentTime.wHour,
        currentTime.wMinute,
        currentTime.wSecond
    );

    static HANDLE dumpFile = CreateFileA ( dumpFilePath,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if ( !dumpFile ) [[unlikely]]
        return EXCEPTION_CONTINUE_SEARCH;

    static MINIDUMP_EXCEPTION_INFORMATION exceptionInfo {
        .ThreadId = GetCurrentThreadId (),
        .ExceptionPointers = exceptionPointers,
        .ClientPointers = FALSE
    };

    static HMODULE dbghelp = LoadLibraryA ( "dbghelp.dll" );

    if ( !dbghelp ) [[unlikely]]
        return EXCEPTION_CONTINUE_SEARCH;

    using MiniDumpWriteDumpProc = BOOL ( WINAPI* ) ( HANDLE hProcess,
        DWORD ProcessId,
        HANDLE hFile,
        MINIDUMP_TYPE DumpType,
        PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
        PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
        PMINIDUMP_CALLBACK_INFORMATION CallbackParam
    );

    static auto MiniDumpWriteDump = static_cast<MiniDumpWriteDumpProc> (
        static_cast<void*> ( GetProcAddress ( dbghelp, "MiniDumpWriteDump" ) )
    );

    if ( !MiniDumpWriteDump ) [[unlikely]]
        return EXCEPTION_CONTINUE_SEARCH;

    MiniDumpWriteDump ( GetCurrentProcess (),
        GetCurrentProcessId (),
        dumpFile,
        MINIDUMP_TYPE::MiniDumpNormal,
        exceptionPointers ? &exceptionInfo : nullptr,
        nullptr,
        nullptr
    );

    CloseHandle ( dumpFile );
    FreeLibrary ( dbghelp );
    return EXCEPTION_CONTINUE_SEARCH;
}

} // namespace editor
