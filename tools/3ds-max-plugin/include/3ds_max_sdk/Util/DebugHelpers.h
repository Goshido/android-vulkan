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

#include <cstdarg>
#include <cstdio>
#include <tchar.h>
#include <WTypes.h>
#include "../BuildWarnings.h"
#include "../dbgprint.h"

namespace MaxSDK
{
    namespace Util
    {
        #if (_MANAGED == 1) || (_M_CEE == 1)
            namespace DebugHelpersCLR
            {
                /*! \brief Sets the name of the current thread.
                    The given name will appear in the thread view of the debugger, enabling easier debugging.
                    \param pcszThreadName The string to use as the name of the thread. */
	            static void SetThreadName(LPCSTR pcszThreadName);
            };
        #endif

        namespace DebugHelpers
        {
            /*! \brief Sets the name of the given thread.
                The given name will appear in the thread view of the debugger, enabling easier debugging.
                \param dwThreadID The ID of the thread for which to set the name.
                \param pcszThreadName The string to use as the name of the thread. */
	        void SetThreadName(DWORD dwThreadID, LPCSTR pcszThreadName);

            /*! \brief Sets the name of the current thread.
                The given name will appear in the thread view of the debugger, enabling easier debugging.
                \param pcszThreadName The string to use as the name of the thread. */
	        void SetThreadName(LPCSTR pcszThreadName);

            /*! \brief Sets the name of the given thread.
                The given name will appear in the thread view of the debugger, enabling easier debugging.
                If the thread name shouldn't appear in release builds (e.g. because of security risks), then use this
                method instead of SetThreadName().
                \param dwThreadID The ID of the thread for which to set the name.
                \param pcszThreadName The string to use as the name of the thread. */
	        void SetThreadNameDebug(DWORD dwThreadID, LPCSTR pcszThreadName);

            /*! \brief Sets the name of the current thread, but only if building for debug.
                The given name will appear in the thread view of the debugger, enabling easier debugging.
                If the thread name shouldn't appear in release builds (e.g. because of security risks), then use this
                method instead of SetThreadName().
                \param pcszThreadName The string to use as the name of the thread. */
	        void SetThreadNameDebug(LPCSTR pcszThreadName);

	        /*! Send a string to the debugger's output window, but only in debug builds.
	            More convenient than having to \#ifndef NDEBUG all over the place.
	            It also supports a variable number of arguments, as you can see.
                \param lpctszMsg The string to send to the debugger's output window. */
	        void Trace(LPCMSTR lpctszMsg, ...);

	        /*! Identical to Trace(), but without the variable argument list. */
	        void Trace0(LPCMSTR lpctszMsg);

        }       // namespace DebugHelpers
    }   //namespace Util
}       // namespace MaxSDK

#include "DebugHelpers.inline.h"