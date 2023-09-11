//
// Copyright [2010] Autodesk, Inc.  All rights reserved. 
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//
//
#pragma once

namespace MaxSDK
{
	namespace Util
	{
		/*!	An assert that works at compile time.
			This validates that an expression is either true or false. The expression is
			passed to the template parameter.
			In the code example below, the compiler will fail with an error if T is not 'Plain Old Data'. 
			That is if the type of class T is not an int, float, character string, pointer, etc... it will
			not compile. This is because is_pod<> will return false. And since there is no definition
			of a StaticAssert struct with a false template parameter, the compiler will fail with an
			error stating that StaticAssert has no body.
			For instance:
			\code
				#include <staticassert.h>
				
				template<typename T> foo
				{
					public:
						foo()
						{
							StaticAssert< std::is_pod<T>::value >();
						}
				};
			\endcode
			
			Similarly, you can use other C++ TR1 extensions that comes with visual C++ 9.0 to help
			in validating assumptions. 
			For instance, you can verify that a type is an enum, as shown below:
			\code
			enum baz
			{
				a,
				b,
				c
			};
			baz a;
			StaticAssert< std::is_enum<baz>::value >();
			\endcode

		*/
		template<bool> struct StaticAssert;
		template<> struct StaticAssert<true> { };
	}
}
