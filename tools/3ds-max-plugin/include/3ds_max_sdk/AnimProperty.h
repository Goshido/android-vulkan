//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include "maxheap.h"
#include <WTypes.h>

/*! \sa  Class Animatable.\n\n
\par Description:
This is the base class for classes that can be hung off an animatable's
property list. When an animatable is deleted, its properties will be deleted
and their virtual destructor will be called.  */
class AnimProperty : public MaxHeapOperators {
	public:
		/*! \remarks When the animatable is destroyed it will normally delete all the
		properties. If a plug-in wants to add a property to the list that it
		does not want deleted it can implement this method to return TRUE.
		\return  TRUE if the item should not be deleted; otherwise FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL DontDelete() {return FALSE;}
		virtual ~AnimProperty() {}
		/*! \remarks Returns the ID of the property. Values above <b>PROPID_USER</b> can be
		used by plug-ins. Note: that a plug-in should only put user defined
		properties on its own list. So IDs only have to be unique within a
		plug-in. If a plug-in needs to attach data to another object, it can do
		so via APP_DATA. */
		virtual DWORD ID()=0;
	};