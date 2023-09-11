//
// Copyright [2015] Autodesk, Inc.  All rights reserved. 
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//

#pragma once

#include "../strclass.h"
#include "../dbgprint.h"
#include "../noncopyable.h"

namespace MaxSDK
{
	namespace Util
	{
		/*! A diagnostic class used for tracing.
			This class prints a message to the debug output window in visual studio. 
			It ensures that a message is automatically printed when the execution leaves the scope. 
			Using the macro's below, this class can be used to trace when a function starts
			and when a function exits. 
			For example:
			\code
			void foo(int val)
			{
				MaxSDK::Util::ScopedTrace trace(_M("foo"));
				int i = val;
				if (i == 6)
					return;
				
				i = 7;
			}
			\endcode
			Will print 
			Scope: foo - START
			Scope: foo - END
			
			So no matter what, whenever the function returns, the destructor will guarantee that a ending statement will be printed.
		*/
		class ScopedTrace : public MaxSDK::Util::Noncopyable
		{
			private:
				const MSTR mFunction;
				const MSTR mMessage;
				ScopedTrace();
			public:
				ScopedTrace(const MCHAR* function_name) : mFunction(function_name)
				{
					DebugPrint(_M("Scope: %s - START\n"), mFunction.data());
				}
				ScopedTrace(const MCHAR* function_name, const MCHAR* extra_message) : mFunction(function_name), mMessage(extra_message)
				{
					DebugPrint(_M("Scope: %s - START - %s\n"), mFunction.data(), mMessage.data());
				}
				void Print(const MCHAR* message)
				{
					DebugPrint(_M("Scope: %s - %s\n"), mFunction.data(), message);
				}
				~ScopedTrace()
				{
					if (mMessage.isNull())
						DebugPrint(_M("Scope: %s - END\n"), mFunction.data());
					else
						DebugPrint(_M("Scope: %s - END - %s\n"), mFunction.data(), mMessage.data());
				}
		};
	}
}

