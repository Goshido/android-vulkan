//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Utility "base class" to prevent compiler generation of a
//    copy constructor and assignment operator.
// AUTHOR: Nicolas Desjardins
// DATE: 2006/03/20 
//***************************************************************************/

#pragma once
#include "maxheap.h"

namespace MaxSDK
{

namespace Util
{

/*!
 * \brief Noncopyable is a handy utility mix-in base class that makes any class
 * derived from it non-copyable.
 *
 * Rather than explicitly disabling the copy constructor and assignment operator
 * for a class by declaring them private and leaving them unimplemented, 
 * deriving from this class has the same effect.  The compiler cannot generate
 * a copy constructor or assignment operator for a class derived from 
 * Noncopyable because Noncopyable's are private and unimplemented.
 */
class Noncopyable
: public MaxHeapOperators {
protected:
	Noncopyable() {};
	~Noncopyable() {};

private:
	Noncopyable(const Noncopyable&);
	Noncopyable& operator=(const Noncopyable&);
};

} // end of namespace Util
}
