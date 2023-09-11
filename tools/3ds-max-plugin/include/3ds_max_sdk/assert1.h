//**************************************************************************/
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "buildver.h"	// Pulls in ALPHABETA
#include "utilexp.h"
#include "maxheap.h"
#include "strbasic.h"
#include <tchar.h>

/*! \brief Method to be called to signal a violation of some assumption in the code
*
* This method should not be called directly, but rather via the various assertion 
* macros such as DbgAssert, MaxAssert, DbgVerify, etc.
* \param line - the line number where the assertion occurred
* \param file - the file path and name where the assertion occurred
* \param function - the non-decorated name of the function in which the assertion occurred
* \param expr - the expression that was asserted 
* \return Currently, this method either returns TRUE or it triggers the shutdown of 3ds Max */
UtilExport int assert1(int line, const char* file, const char* function = NULL, const char* expr = NULL );
UtilExport int assert1(int line, const wchar_t* file, const wchar_t* function = NULL, const wchar_t* expr = NULL );
UtilExport int assert1(int line, const wchar_t* file, const char* function = NULL, const wchar_t* expr = NULL);

#undef assert

// DbgAssert defaults: disabled for Release builds, enabled for Hybrid and Debug builds
#if (defined( NDEBUG )  && defined( MAX_ASSERTS_ACTIVE_IN_RELEASE_BUILD )) || \
    (defined( _HYBRID ) && !defined( MAX_ASSERTS_NOT_ACTIVE_IN_HYBRID_BUILD )) || \
    (defined( _DEBUG )  && !defined( MAX_ASSERTS_NOT_ACTIVE_IN_DEBUG_BUILD ))
#  ifdef  _ADESK_3DSMAX_MAC_
#    define _3DSMAX_CURRENT_FUNCTION_ (__FUNCTION__)
#    undef MAX_ASSERTS_ACTIVE
#  else
#    define _3DSMAX_CURRENT_FUNCTION_ (_M(__FUNCTION__))
#    define MAX_ASSERTS_ACTIVE 
#  endif
#else    
#  undef  MAX_ASSERTS_ACTIVE
#endif

/*! \defgroup AssertMacros Assert macros
 * The following macros can be used in 3ds Max plug-ins to assert assumptions made by the plug-in's code.
 * Asserts are by default enabled in Debug and Hybrid builds, and are by default disabled
 * in release builds. They can be enabled in Release builds of a plug-in by defining 
 * the MAX_ASSERTS_ACTIVE_IN_RELEASE_BUILD pre-processor symbol. 
 * One way to do this is: set CL=/DMAX_ASSERTS_ACTIVE_IN_RELEASE_BUILD
 * They can be disabled in Hybrid and Debug builds of a plug-in by defining
 * the MAX_ASSERTS_NOT_ACTIVE_IN_HYBRID_BUILD and MAX_ASSERTS_NOT_ACTIVE_IN_DEBUG_BUILD pre-processor symbol.
 * One way to do this is: set CL=/DMAX_ASSERTS_NOT_ACTIVE_IN_HYBRID_BUILD
 *
 * All assert macros below are equivalent. The 32 and 64 bit versions of these macros are also equivalent.
 * The reason why several macros exist are historical...
 * Plug-ins are encouraged to use the DbgAssert macro.
 * 
 * Code that needs to be compiled only when asserts are active (enabled) can be 
 * conditionally defined using MAX_ASSERT_ACTIVE. One example when this can be useful is when 
 * certain variables are used for debugging purposes only. In this case they can be conditionally
 * declared and updated based on the MAX_ASSERT_ACTIVE symbol, and can be part of expressions that
 * are evaluated in asserts.
 * \code
 * #ifdef MAX_ASSERT_ACTIVE
 * int counter = 0;
 * // update counter
 * #endif
 * DbgAssert(counter != 0);
 * \endcode
 */
//!@{
#ifdef MAX_ASSERTS_ACTIVE
#  define assert( expr )    ( (expr) || assert1( __LINE__, _M(__FILE__), _3DSMAX_CURRENT_FUNCTION_, (_M(#expr)) ) )
#  define DbgAssert( expr ) (void((expr) || assert1( __LINE__, _M(__FILE__), _3DSMAX_CURRENT_FUNCTION_, (_M(#expr)))))
#  define DbgVerify( expr ) ( (expr) || (assert1( __LINE__, _M(__FILE__), _3DSMAX_CURRENT_FUNCTION_, (_M(#expr))), false) )
#else    
#  define assert( expr )      ((void *)0)
#  define DbgAssert( expr )
#  define DbgVerify( expr ) ( expr )
#endif
//!@}

//-----------------------------------------------------------------------------

/*! \brief Callback class that allows for hooking into 3ds Max's assertion mechanims
 *
 * Plugins can derive from this class and overwrite the CAssertCB::AssertEx method
 * if they wish to execute a custom action, such as logging, when an assertion fails 
 * during execution of 3ds Max. See CAssertCB::AssertResult for more information on
 * the customization options this callback mechanism offers.
*/
class CAssertCB : public MaxHeapOperators 
{
	public:
	/*! \brief Possible return values for callback method (CAssertCB::AssertEx) invoked 
	 * when an assertion fails.
	 */
	enum AssertResult 
	{
		ASSERTCB_IGNORE,	//!< Ignore the assertion and continue. No assertion dialog is displayed.
		ASSERTCB_CANCEL,	//!< Cancel the execution. 3ds Max will be exit.
		ASSERTCB_CONTINUE	//!< Proceed with the built in handling of the assertion (display assert dialog)
	};

	/*! \brief Callback method invoked when an assertion fails
	 *
	 * \param line - the line number where the assertion occurred
	 * \param file - the file path and name where the assertion occurred
	 * \param function - the non-decorated name of the function in which the assertion occurred
	 * \param expression - the expression that was asserted 
	 * \return One of the possible AssertResult values.
	 */
	virtual AssertResult AssertEx(int line, const MCHAR* file, const MCHAR* function = NULL, const MCHAR* expression = NULL) =0;
};

/*! \brief Sets a callback object that allows for performing a custom action when 
 * an assertion fails (is triggered).
 *
 * A plugin can supply an object that allows for executing a custom action when 
 * an assertion fails in 3ds Max. For details on the extent of possible customization 
 * see class CAssertCB.
 * Currently, only one callback object can be active at any given time. 
 * This means that the callback installed by a plugin can be de-installed by another plugin.
 * Thus, plugins should keep their callbacks active for as short of a time period 
 * as possible, and restore the previous callback object when they de-activate their own.
 * \param cb - The new callback object to use 
 * \return The old callback object or NULL if there's none set
*/
UtilExport CAssertCB* SetAssertCB(CAssertCB* cb);

