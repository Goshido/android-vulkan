//**************************************************************************/
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: used for importing and exporting functions in assetmanagement.dll
// AUTHOR: Peter Ochodo
// DATE: 2008-07-16 
//***************************************************************************/

#pragma once

#ifdef BLD_ASSETMANAGEMENT
#define AssetMgmntExport __declspec( dllexport )
#else
#define AssetMgmntExport __declspec( dllimport )
#endif

