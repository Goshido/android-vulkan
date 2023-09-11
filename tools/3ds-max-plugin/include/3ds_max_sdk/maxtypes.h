//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// FILE:        maxtypes.h
// DESCRIPTION: Typedefs for general jaguar types.
// AUTHOR:      Rolf Berteig
// HISTORY:     created 19 November 1994
//**************************************************************************/

#pragma once
#include "maxheap.h"
#include "WindowsDefines.h"
#include "geom/maxcolors.h"

#include <cstdint>

using ulong = unsigned long;
using uchar = unsigned char;
using UBYTE = uchar;
using UWORD = unsigned short;
using MtlID = UWORD;


/* Time:
*/
using TimeValue = int;
/*! Number of ticks in a second. */
#define TIME_TICKSPERSEC 4800
/*! Converts from ticks to seconds. */
constexpr float TicksToSec(TimeValue ticks) { return ticks / static_cast<float>(TIME_TICKSPERSEC); }
/*! Converts from seconds to ticks. */
constexpr TimeValue SecToTicks(double secs) { return static_cast<TimeValue>(secs * TIME_TICKSPERSEC); }
/*! Converts a number of seconds plus a number of ticks to a TimeValue. */
constexpr TimeValue TicksSecToTime(TimeValue ticks, double secs) { return ticks + SecToTicks(secs); }
/*! Converts a TimeValue to a number of seconds plus a number of ticks.  */
constexpr void TimeToTicksSec(TimeValue time, TimeValue& ticks, TimeValue& secs)
{
	ticks = time % TIME_TICKSPERSEC;
	secs = time / TIME_TICKSPERSEC;
}
/*! Returns a time value that represents positive infinity. */
#define TIME_PosInfinity TimeValue(0x7fffffff)
/*! Returns a time value that represents negative infinity. */
#define TIME_NegInfinity TimeValue(0x80000000)


//-----------------------------------------------------
// Class_ID
//-----------------------------------------------------
/*! \sa  ~{ Class Descriptors }~,
class ID definititions in plugapi.h</a>\n\n
\par Description:
This class represents the unique class ID for a 3ds Max plug-in. A plug-ins
Class_ID must be <b>unique</b>. A program is provided with the SDK to generate
these ClassIDs. It is VERY important you use this program to create the
ClassIDs for your plug-ins. To generate a random Class_ID and optionally copy
it to the clipboard, run the <b>gencid.exe</b> program. A Class_ID consists of
two unsigned 32-bit quantities. The constructor assigns a value to each of
these, for example <b>Class_ID(0xCAD834E2, 0x27E47C5A)</b>.\n\n
All the methods of this class are implemented by the system.   */
class Class_ID : public MaxHeapOperators
{
	ulong a = 0xffffffff;
	ulong b = 0xffffffff;

public:
	/*! \remarks Constructor.\n\n
	Assigns a value of <b>0xFFFFFFFF</b> to each 32-bit quantity. */
	constexpr Class_ID() = default;
	/*! \remarks Constructor.\n\n
	Creates a new class ID with the same values as the argument.
	\par Parameters:
	<b>const Class_ID\& cid</b>\n\n
	A reference to the Class ID to copy. */
	constexpr Class_ID(const Class_ID& cid) = default;
	/*! \remarks Constructor.\n\n
	This is the standard constructor to be used by 3ds Max plug-ins. Each
	of the 32-bit quantities may be assigned separately.
	\par Parameters:
	<b>ulong aa</b>\n\n
	Assigned to the first 32-bit quantity.\n\n
	<b>ulong bb</b>\n\n
	Assigned to the second 32-bit quantity. */
	constexpr Class_ID(ulong aa, ulong bb) : a(aa), b(bb) {}
	/*! \remarks Returns the first unsigned 32-bit quantity. */
	constexpr ulong PartA() const { return a; }
	/*! \remarks Returns the second unsigned 32-bit quantity.*/
	constexpr ulong PartB() const { return b; }
	// These non-const overrides are required by .NET Wrapper
	ulong PartA() { return a; }
	ulong PartB() { return b; }
	constexpr void SetPartA(ulong aa) { a = aa; }
	constexpr void SetPartB(ulong bb) { b = bb; }
	/* \par Operators :	*/
	/*! \remarks Checks for equality between two Class IDs. */
	constexpr bool operator==(const Class_ID& cid) const { return (a == cid.a) && (b == cid.b); }
	/*! \remarks Check for Inequality between two Class IDs. */
	constexpr bool operator!=(const Class_ID& cid) const { return !operator==(cid); }
	/*! \remarks Assignment operator. Note: In release 3.0 and later this
	method checks for self-assignment. */
	constexpr Class_ID& operator=(const Class_ID& cid) { a = cid.a; b = cid.b; return *this; };
	// less operator - allows for ordering Class_IDs (used by stl maps for example) 
	/*! \remarks This operator is available in release 4.0 and later
	only.\n\n
	Less than operator. This returns true if the specified Class_ID's two
	parts are numerically less than this Class_ID's; false if not. */
	constexpr bool operator<(const Class_ID& rhs) const { return (a < rhs.a || (a == rhs.a && b < rhs.b)); }
};

// SuperClass ID
using SClass_ID = ulong;

/*! \sa  Class FPInterface, ~{ Function Publishing }~.\n\n
\par Description:
This class is the interface ID for the Function Publishing System of 3ds Max.
This class is structurally very similar to a Class_ID, containing two
randomly-chosen longwords to provide a unique global ID. The various
constructors assign a value to each of these. There are also methods to assign
and retrieve the individual parts and operators to check for equality or
inequality.\n\n
All the methods of this class are implemented by the system.  */
class Interface_ID : public MaxHeapOperators
{
	ulong a = 0xffffffff;
	ulong b = 0xffffffff;

public:
	/*! \remarks Constructor. The two parts of the ID are initialized to
	<b>0xffffffff</b>. */
	constexpr Interface_ID() = default;
	/*! \remarks Constructor. The two parts of the ID are initialized from
	the corresponding parts of the Interface_ID passed.
	\par Parameters:
	<b>const Interface_ID\& iid</b>\n\n
	The ID whose parts are used to initialize this ID. */
	constexpr Interface_ID(const Interface_ID& iid) = default;
	/*! \remarks Constructor. The two parts of the ID are initialized from
	the from the parts passed.\n\n
	\par Parameters:
	<b>ulong aa</b>\n\n
	Passed to initialize the first part of the ID.\n\n
	<b>ulong bb</b>\n\n
	Passed to initialize the second part of the ID. */
	constexpr Interface_ID(ulong aa, ulong bb) : a(aa), b(bb) {}
	/*! \remarks Returns the first part of the ID. */
	constexpr ulong PartA() const { return a; }
	/*! \remarks Returns the second part of the ID. */
	constexpr ulong PartB() const { return b; }
	// These non-const overrides are required by .NET Wrapper
	ulong PartA() { return a; }
	ulong PartB() { return b; }
	/*! \remarks Sets the first part of the ID.
	\par Parameters:
	<b>ulong aa</b>\n\n
	Passed to set the first part. */
	constexpr void SetPartA(ulong aa) { a = aa; }
	/*! \remarks Sets the second part of the ID.
	\par Parameters:
	<b>ulong bb</b>\n\n
	Passed to set the second part. */
	constexpr void SetPartB(ulong bb) { b = bb; }
	/*! \remarks Equality operator. Returns nonzero if the two parts of
	the ID are equal to the ID passed; otherwise zero.
	\par Parameters:
	<b>const Interface_ID\& iid</b>\n\n
	The ID to check. */
	constexpr bool operator==(const Interface_ID& iid) const { return (a == iid.a) && (b == iid.b); }
	/*! \remarks Inequality operator. Returns nonzero if either of the
	parts of the ID are NOT equal to the ID passed; otherwise zero.
	\par Parameters:
	<b>const Interface_ID\& iid</b>\n\n
	The ID to check. */
	constexpr bool operator!=(const Interface_ID& iid) const { return !operator==(iid); }
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const Interface_ID\& iid</b>\n\n
	The ID to assign from. */
	constexpr Interface_ID& operator=(const Interface_ID& iid) { a = iid.a; b = iid.b; return *this; };
	// less operator - allows for ordering Class_IDs (used by stl maps for example) 
	constexpr bool operator<(const Interface_ID& rhs) const { return a < rhs.a || (a == rhs.a && b < rhs.b); }
};

// Types used by ISave, ILoad, AppSave, AppLoad

/*! \defgroup ioResults I/O Results */
//!@{
enum IOResult : int
{
	//! The result was acceptable - no errors.
	IO_OK = 0,	
	/*!	Indicates the end of the chunks at a certain level have been reached. It 
	is used as a signal to terminates the processing of chunks at that level. 
	Returned from ILoad::OpenChunk()*/
	IO_END = 1,
	/*!	Returned when an error occurred. Note that the plug-in should not put up a
	message box if a read error occurred. It should simply return the error 
	status. This prevents a excess of messages from appearing. */
	IO_ERROR = 2,
	/*!	Returned when clicking 'Esc' in busy auto-save process. It is used as
	a signal to terminates the busy process.*/
	IO_INTERRUPT = 3
};
//!@}

enum ChunkType
{
	NEW_CHUNK       = 0,
	CONTAINER_CHUNK = 1,
	DATA_CHUNK      = 2
};

enum FileIOType
{
	/*!	\brief File IO concerns a scene file (.max, .viz). 
	See \ref NOTIFY_FILE_PRE_OPEN, \ref NOTIFY_FILE_POST_OPEN */
	IOTYPE_MAX    = 0, 
	/*!	\brief File IO concerns a material library file (.mat). 
	See \ref NOTIFY_FILE_PRE_OPEN, \ref NOTIFY_FILE_POST_OPEN */
	IOTYPE_MATLIB = 1, 
	/*!	\brief File IO concerns a render preset file (.rps). 
	See \ref NOTIFY_FILE_PRE_OPEN, \ref NOTIFY_FILE_POST_OPEN */
	IOTYPE_RENDER_PRESETS = 2
};
