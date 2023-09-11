//
// Copyright 2009 Autodesk, Inc.  All rights reserved. 
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
#pragma once
#include "tab.h"
#include "geom/point3.h"

// Pre defined Tab Containers
typedef Tab<int>   IntTab;
typedef Tab<float> floatTab;
typedef Tab<float> FloatTab;
typedef Tab<DWORD> DWTab;
typedef Tab<Point3> Point3Tab;
/*! \sa  Class Tab, Class AdjEdgeList.\n\n
\par Description:
This class is simply a table of DWORDs (32-bit values.) */
class DWORDTab : public Tab<DWORD> {};
