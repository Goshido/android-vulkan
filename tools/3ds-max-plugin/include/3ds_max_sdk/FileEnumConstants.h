//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

/*! \defgroup EnumAuxFiles Auxiliary File Enumeration Flags
Some scene entities depend on data files other then .max files. These files 
are referred to as "auxiliary" files. The following are flags that used to 
customize the enumeration of these files. See Animatable::EnumAuxFiles(), 
ReferenceMaker::EnumAuxFiles() and Interface::EnumAuxFiles() for more details. 
Sample code that implements the EnumAuxFiles and handles both accessor and 
non-accessor handling of assets, and the handling of sub files is in
<b>/maxsdk/samples/modifiers/pointcache/PointCache.cpp</b>. */
//!@{

//! \brief Enumerate inactive files
/*! Inactive files are those that aren't being used currently. For instance, 
a texture map file that is present, but not activated in the materials editor 
user interface, is considered inactive. */
#define FILE_ENUM_INACTIVE 		(1<<0)
//! \brief Enumerate video post files.
#define FILE_ENUM_VP			(1<<1)
//! \brief Enumerate render files. 
#define FILE_ENUM_RENDER		(1<<2)
//! \brief Enumerate ALL files.
#define FILE_ENUM_ALL  (FILE_ENUM_INACTIVE|FILE_ENUM_VP|FILE_ENUM_RENDER)
//! \brief Enumerate references to be saved to scene files. 
#define FILE_ENUM_FILE_SAVE		(1<<3)
//! \brief Enumerate missing files only
#define FILE_ENUM_MISSING_ONLY	(1<<8)
//! \brief Just enumerate 1st missing sub file, for example frame files specified by IFL and point cache .xml, if enumerating missing files
/*! Trying to resolve missing files can be slow. If all that is really 
cared about is whether any sub files are missing, specify this option. */
#define FILE_ENUM_1STSUB_MISSING (1<<9)
//! \brief Do not enumerate references.
#define FILE_ENUM_DONT_RECURSE   (1<<10)
//! \brief Do not enumerate things with flag A_WORK1 set.
#define FILE_ENUM_CHECK_AWORK1   (1<<11)
//! \brief Do not enumerate custom attributes.
#define FILE_ENUM_DONTCHECK_CUSTATTR  (1<<12)
//! \brief Do not enumerate files needed only for viewport rendering.
/*!  If the flag is set, then any files that are needed only for viewport display 
(not for rendering) will be excluded from the enumeration. This is useful in 
network render mode, when you do not want to consider a render a failure just 
because some viewport-only files are missing. */
#define FILE_ENUM_SKIP_VPRENDER_ONLY (1<<13)
//! \brief The callback object passed through is an IEnumAuxAssetsCallback derived object.
#define FILE_ENUM_ACCESSOR_INTERFACE (1<<14)
//! \brief FILE_ENUM_RESERVED_1 was marked as internal use only, was used to skip IFL sub files
#define FILE_ENUM_RESERVED_1 (1<<15)
//! \brief Do not enumerate sub files, for example frame files specified by IFL and point cache .xml
#define FILE_ENUM_SKIP_SUB_FILES FILE_ENUM_RESERVED_1
//! \brief Enumerate all active but missing files.
#define FILE_ENUM_MISSING_ACTIVE (FILE_ENUM_VP|FILE_ENUM_RENDER|FILE_ENUM_MISSING_ONLY)
//! \brief Enumerate all active but missing files.
/*! But only enumerate first sub file pointed to by an IFL (Image File List) or other asset with sub files
\note Enumerating all of them can be very slow. */
#define FILE_ENUM_MISSING_ACTIVE1 (FILE_ENUM_MISSING_ACTIVE|FILE_ENUM_1STSUB_MISSING )

//!@}	END OF EnumAuxFiles