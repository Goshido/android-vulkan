//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include <WTypes.h>
#include "maxheap.h"
#include "tab.h"

/*! \sa  Class Animatable
\par Description:
When hit testing is done, this class has data members used to identify the key
that was hit. The manner these are used is up to the developer. For example,
the hit member may be used as an index into the table of keys. If a developer
needed additional information, the flags could be used. The system does not use
these itself - the developer just needs to establish a consistent way to
recognize its own keys and tag them as hit.\n\n
There is a typedef that defines a table of these <b>TrackHitRecords</b>. It is
defined as:\n\n
<b>typedef Tab\<TrackHitRecord\> TrackHitTab;</b> */
class TrackHitRecord: public MaxHeapOperators  {
	public:
		DWORD	hit;
		DWORD	flags;
		/*! \remarks Constructor. The data members are initialized to the
		values passed. */
		TrackHitRecord(DWORD h=0,DWORD f=0) {hit=h;flags=f;}
	};

typedef Tab<TrackHitRecord> TrackHitTab;