//**************************************************************************/
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: File Resolution Manager => resolves the physical location of
// an asset file
// AUTHOR: Peter Ochodo
// DATE: 2008-06-04 (YYYY-MM-DD) 
//***************************************************************************/

#pragma once

#include <guiddef.h>
#include <CGuid.h>
#include "../maxheap.h"

namespace MaxSDK
{
	namespace AssetManagement
	{
		struct AssetId : GUID, public MaxHeapOperators
		{
			AssetId() { *static_cast<GUID*>(this) = CLSID_NULL; }
		};

		static AssetId kInvalidId;
	}
}

