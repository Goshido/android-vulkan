// Copyright (c) 2017 Autodesk, Inc.
// All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.

#pragma once
#include "GeomExport.h"
#include "ipoint2.h"
#include "dpoint2.h"

#include <limits>

/*! \see  Class DPoint2.\n\n
\details
This class represents a 2D box described by two double-precision 2D corner coordinates.
DBox2 provides methods that return individual coordinates of the box, scale and
translate it, retrieve its center, modify its size, expand it to include points
or other boxes, and determine if points are inside the box. All methods are
implemented by the system. */
class GEOMEXPORT DBox2
{
	// Warning - instances of this class are saved as a binary blob to scene file
	// Adding/removing members will break file i/o
public:
	/** The minimum 2D point in this box */
	DPoint2 pmin{ DBL_MAX, DBL_MAX };

	/** The maximum 2D point in this box */
	DPoint2 pmax{ DBL_MIN, DBL_MIN };

	/** Constructor. The corners of the box are initialized such
	that the box is 'empty'. See \see <b>IsEmpty()</b>. */
	DBox2() = default;

	DBox2(const DBox2&) = default;
	DBox2(DBox2&&) = default;
	DBox2& operator=(const DBox2&) = default;
	DBox2& operator=(DBox2&&) = default;

	/** Constructor. The corners of the box are initialized to
	the points passed.
	\param p The first corner of the box
	\param q The second corner of the box */
	DBox2(const DPoint2& p, const DPoint2& q)
	{
		pmin = p;
		pmax = q;
	}

	/** Initializes this box such that <b>pmin</b> is a very
	large value while <b>pmax</b> is a small value. Thus the box is
	'empty'. See \see <b>IsEmpty()</b>. */
	void Init();

	// Access

	/** Returns the minimum corner value.
	\return the value of corner <b>pmin</b> */
	DPoint2 Min() const
	{
		return pmin;
	}

	/** Returns the maximum corner value.
	\return the value of corner <b>pmax</b>. */
	DPoint2 Max() const
	{
		return pmax;
	}

	/** Returns the center of this DBox2 as a DPoint2.
	\return the center of the box */
	DPoint2 Center() const
	{
		return (pmin + pmax) / 2.0;
	}

	/** Returns the width of the box as a DPoint2. This is
	<b>pmax-pmin</b>.
	\return the width of the box */
	DPoint2 Width() const
	{
		return (pmax - pmin);
	}

	// Modifiers

	/** Expands this DBox2 to include the DPoint2 <b>p</b>.
	\param p the DPoint2 to include in the new size
	\return the resulting box */
	DBox2& operator+=(const DPoint2& p);

	/** Expands this DBox2 to include the specified DBox2.
	\param b the box to include in the new size
	\return the resulting box */
	DBox2& operator+=(const DBox2& b);

	/** Scales this box about its center by the specified scale.
	\param s Specifies the scale factor for this DBox2. */
	void Scale(double s);

	/** Translates this box by the distance specified. The point
	is added to each corner.
	\param p Specifies the distance to translate the box. */
	void Translate(const DPoint2& p);

	/** Enlarges this box. A DPoint2 is created from <b>s</b> as
	DPoint2(s,s) and added to <b>pmax</b> and subtracted from <b>pmin</b>.
	If the box is 'empty', the box is centered at (0,0) and then
	enlarged.
	\param s the amount to enlarge the box on all sides */
	void EnlargeBy(double s);

	// Tests

	/** Determines if the box is empty. This indicates the box
	has not had specific values set by the developer.
	\return  Nonzero if the box is empty; otherwise 0. */
	int IsEmpty() const;

	/** Determines if the specified point <b>p</b> is contained
	in this box.
	\param p Specifies the point to check.
	\return  Nonzero if the specified point is contained in this box, otherwise 0. */
	int Contains(const DPoint2& p) const;

	/** Determines if the specified DBox2 is contained totally
	within this box.
	\param b Specifies the box to check.
	\return  Nonzero if the specified box is entirely contained within this	box; otherwise 0. */
	int Contains(const DBox2& b) const;

	/** Determines if the specified DBox2 overlaps this box.
	\param b Specifies the box to check.
	\return  Nonzero if the specified box overlaps this	box; otherwise 0. */
	int Overlaps(const DBox2& b) const;
};
