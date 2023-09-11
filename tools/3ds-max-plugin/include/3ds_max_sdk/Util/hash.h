//
// Copyright 2018 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//
//

#pragma once
#include "fnv1a.hpp"

namespace MaxSDK {
	namespace Util {

		/*! 
		 Template struct Bitwise_hash.
		 A hash functor for plain old data.\n\n 
		 */
		template<class _Kty>
		struct Bitwise_hash
		{
			typedef _Kty   argument_type;
			typedef size_t result_type;

			/*! Computes a Fowler-Noll-Vo FNV-1A hash on the input data, suitable for noncryptographic uses.
			 \param  _Keyval Constant reference to an object of the template class _Kty.
			 \return Returns an FNV-1A hash of size size_t.
			 \sa Function fnv1a_hash_bytes()
			*/
			size_t operator()(const _Kty& _Keyval) const
			{  // hash _Keyval to size_t value by pseudorandomizing transform
				return (fnv1a_hash_bytes((const unsigned char*)&_Keyval, sizeof(_Kty)));
			}
		};

	}  // namespace Util

}  // namespace MaxSDK