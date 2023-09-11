// Copyright (c) 1998-2011 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.

#pragma once
#include "3dsmaxconfig.h"

#ifdef BLD_UTIL
#define UtilExport __declspec( dllexport )
#else
#define UtilExport __declspec( dllimport )
#endif

