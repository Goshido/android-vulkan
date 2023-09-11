//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once
#include "coreexp.h"
#include "BuildWarnings.h"

/*! \remarks Returns TRUE if the application is running under a trial license, as
opposed to a full, authorized license; otherwise FALSE. */
extern CoreExport bool IsTrialLicense();
//! Returns true if the application is running under a network license.
extern CoreExport bool IsNetworkLicense();


