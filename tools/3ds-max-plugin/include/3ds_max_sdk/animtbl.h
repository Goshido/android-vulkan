//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// FILE:        animtbl.h
// DESCRIPTION: Defines Animatable Classes
// AUTHOR:      Rolf Berteig & Dan Silva
// HISTORY:     created 9 September 1994
//**************************************************************************/
#pragma once

#include "WindowsDefines.h"
#include "GetCOREInterface.h"


#include "AnimatableFlags.h"

#include "TimeMap.h"

#include "TrackHitRecord.h"

#include "TrackFlags.h"

#include "TrackScreenUtils.h"

#include "TrackClipObject.h"

#include "ParamDimension.h"

#include "AnimatableInterfaceIDs.h"

#include "AnimProperty.h"

#include "AnimPropertyList.h"

// Property IDs
#include "AnimPropertyID.h"

// BeginEditParams flags values
#include "EditParamFlags.h"

#include "AssetEnumCallback.h"

class NoteTrack;

#include "SysNodeContext.h"

#include "Animatable.h"

//
// Callback for EnumAnimTree:
//
#include "AnimEnum.h"

#include "SubObjAxisCallback.h"

// --- AppData ---------------------------------------------

#include "AppDataChunk.h"

// This list is maintained by the systems. Plug-ins need not concern themselves with it.
#include "AnimAppData.h"

#include "SchematicViewProperty.h"
 
#include "CoreFunctions.h"
