//**************************************************************************/
// Copyright (c) 2011 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

/*! This file contains windows version constants for 3ds Max. 
 *	The earliest version of Windows that 3ds Max supports is defined below
 *	For other system requirements, see www.autodesk.com for details.
 */

#ifndef WINVER				
#define WINVER 0x0601		//!< 3ds Max 2017 specifically targets Windows 7 or higher.
#endif

#ifndef _WIN32_WINNT		
#define _WIN32_WINNT 0x0601	 //!< 3ds Max 2017 specifically targets Windows 7 or higher.
#endif						

#ifndef _WIN32_WINDOWS		
#define _WIN32_WINDOWS 0x0601 //!< 3ds Max 2017 specifically targets Windows 7 or higher.
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0800	//!< 3ds Max 2017 specifically requires Internet Explorer version 8.0 or higher
#endif

