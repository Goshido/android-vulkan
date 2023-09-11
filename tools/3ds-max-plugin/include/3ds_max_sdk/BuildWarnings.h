// Copyright (c) 1998-2011 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.

#pragma once
#include "3dsmaxconfig.h"
 
// The recommended way to disable\enable warnings, or to change their level
// is via the following files 
// maxsdk\ProjectSettings\AdditionalCompilerOptions.txt - for x32 build targets
// maxsdk\ProjectSettings\AdditionalCompilerOptions64.txt - for x64 build targets

// Macro used to avoid the C4100 compiler warning. Use this for function parameters
// that are not used.
#define UNUSED_PARAM(x) (x)

