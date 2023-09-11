//**************************************************************************/
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include <WTypes.h>

/*!	The permanent ID of a Parameter.
	see class ParamDef */
typedef short ParamID;
/*!	The permanent ID of the parameter block.
	see class ParamBlockDesc2 */
typedef short BlockID;
/*!	Parameter Map ID.
	see class class ParamBlockDesc2 and class IParamMap2 */
typedef short MapID;
/*!	Resource String ID, that points to the fixed internal name for a function published definition.
	see class ParamBlockDesc2, FPFunctionDef, FPActionDef, FPPropDef, FPParamDef, etc.. */
typedef INT_PTR StringResID;
/*!	Resource ID.
	see class ParamBlockDesc2 */
typedef int ResID;
