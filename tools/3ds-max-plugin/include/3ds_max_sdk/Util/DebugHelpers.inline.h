//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2015 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#if (_MANAGED == 1) || (_M_CEE == 1)
    #using <mscorlib.dll>
#endif

#include <vadefs.h>

namespace MaxSDK
{;
namespace Util
{;

#if (_MANAGED == 1) || (_M_CEE == 1)
    #pragma managed
    namespace DebugHelpersCLR
    {
	    inline static void SetThreadName(LPCSTR pcszThreadName) 
        {
		    System::Threading::Thread::CurrentThread->Name = gcnew System::String(pcszThreadName);
	    }
    };
    #pragma unmanaged
#endif

namespace DebugHelpers
{;

// See the MSDN help topic "How to: Set a thread name"
//  ===================   SetThreadName  =====================
//
// Usage: SetThreadName (-1, _T("MainThread"));
//
// Once you set the thread's name, you'll see it in your debugger
// which is great when you have tons of threads.
#pragma pack(push,8)
struct THREADNAME_INFO
{
    DWORD    dwType;     // must be 0x1000
    LPCSTR   szName;     // pointer to name (in user addr space)
    DWORD    dwThreadID; // thread ID (-1=caller thread)
    DWORD    dwFlags;    // reserved for future use, must be zero

    inline THREADNAME_INFO(LPCSTR pcszThreadName, DWORD dwID) : 
    dwType(0x1000), 
        szName(pcszThreadName),
        dwThreadID(dwID),
        dwFlags(0) {}
};
#pragma pack(pop)

inline void SetThreadName(DWORD dwThreadID, LPCSTR pcszThreadName)
{
    THREADNAME_INFO info(pcszThreadName, dwThreadID);

    enum { MS_VC_EXCEPTION_ID = 0x406D1388 };
#ifdef _MSC_VER
    __try
#endif
    {
        RaiseException( MS_VC_EXCEPTION_ID, 0, 
            sizeof(info)/ sizeof(ULONG_PTR), 
            reinterpret_cast<const ULONG_PTR*>(&info) );
    }
#ifdef _MSC_VER
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
        // empty
    }
#endif
}

inline void SetThreadName(LPCSTR pcszThreadName)
{
#if (_MANAGED == 1) || (_M_CEE == 1)
    DebugHelpersCLR::SetThreadName(pcszThreadName);
#else
    SetThreadName((DWORD)-1, pcszThreadName);
#endif
}

// In release build enabling this might be a security risk -- named
// threads might be easier to hack...  If you're worried about it, use this
// version, which in release builds doesn't do anytning.
inline void SetThreadNameDebug(DWORD dwThreadID, LPCSTR pcszThreadName)
{
#ifdef _DEBUG
    SetThreadName(dwThreadID, pcszThreadName);
#else
    UNUSED_PARAM(dwThreadID);
    UNUSED_PARAM(pcszThreadName);
#endif
}

inline void SetThreadNameDebug(LPCSTR pcszThreadName)
{
#ifdef _DEBUG

#if (_MANAGED == 1) || (_M_CEE == 1)
    MaxSDK::Util::DebugHelpersCLR::SetThreadName(pcszThreadName);
#else
    SetThreadName((DWORD)-1, pcszThreadName);
#endif

#else
    UNUSED_PARAM(pcszThreadName);
#endif
}

// Send a string to the debugger's output window, but only in debug builds.
// More convenient than having to #ifndef NDEBUG all over the place.
// It also supports a variable number of arguments, as you can see.
inline void Trace(LPCMSTR lpctszMsg, ...)
{
#ifndef NDEBUG
    MCHAR tszBuffer[4096];

    va_list args;
    va_start(args, lpctszMsg);

    _vsntprintf_s(tszBuffer, 4096, lpctszMsg, args);
    DebugOutputString(tszBuffer);

    va_end(args);
#else
    UNUSED_PARAM(lpctszMsg);
#endif
}

inline void Trace0(LPCMSTR lpctszMsg)
{
#ifndef NDEBUG
    DebugOutputString(lpctszMsg);
#else
    UNUSED_PARAM(lpctszMsg);
#endif
}

}}}      // namespaces
