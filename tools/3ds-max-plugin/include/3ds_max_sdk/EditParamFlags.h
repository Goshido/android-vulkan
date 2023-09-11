//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include "buildver.h"

/** \defgroup Object-derived Class Flags
	A set of flags that is passed to the BeginEditParams virtual method on classes that derive from ::Object. */
//!@{
#define BEGIN_EDIT_CREATE       (1<<0)			//!< An object is displayed in the create command panel.
#define BEGIN_EDIT_MOTION		(1<<1)			//!< A Controller is being edited in the motion command panel.
#define BEGIN_EDIT_HIERARCHY	(1<<2)			//!< Same as BEGIN_EDIT_IK
#define BEGIN_EDIT_IK			(1<<2)			//!< A Controller is being edited in the IK subtask of the hierarchy command panel
#define BEGIN_EDIT_LINKINFO		(1<<3)			//!< A Controller is being edited in the Link Info  subtask of the hierarchy command panel
#define BEGIN_EDIT_SHAPE_NO_RENDPARAM (1<<5)	//!< Used by LoftSpline
//!@}

//! Flag passed to the EndEditParams virtual method
#define END_EDIT_REMOVEUI	(1<<0)
