// Copyright (c) 2017 Autodesk, Inc.
// All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.

#pragma once
#include "GeomExport.h"
#include "point2.h"

#include <cassert>
#include <iosfwd>


/**
Description:
This class describes a 2D point using double precision x and y coordinates.
Methods are provided to add and subtract points, multiply and divide by
scalars, and element by element multiply and divide two points. All methods are
implemented by the system.
*/
class GEOMEXPORT DPoint2
{
public:
	double x = 0.0;
	double y = 0.0;

	constexpr DPoint2() = default;
	constexpr DPoint2(const DPoint2&) = default;
	constexpr DPoint2(DPoint2&&) = default;
	DPoint2& operator=(const DPoint2&) = default;
	DPoint2& operator=(DPoint2&&) = default;

	/*! \remarks Constructor. Data members are initialized to X and Y. */
	constexpr DPoint2(double X, double Y) : x(X), y(Y) {}
	/*! \remarks Constructor. Data members are initialized to a.x and a.y. */
	constexpr DPoint2(const Point2& a) : x(a.x), y(a.y) {}
	/*! \remarks Constructor. Data members are initialized as x = af[0] and y = af[1]. */
	constexpr DPoint2(const double af[2]) : x(af[0]), y(af[1]) {}
	/** Assign a Point2 to this DPoint2 */
	constexpr DPoint2& operator=(const Point2& a)
	{
		x = a.x;
		y = a.y;
		return *this;
	}

	static const DPoint2 Origin; /**  const for (0,0) */
	static const DPoint2 XAxis; /**  const for (1,0) */
	static const DPoint2 YAxis; /**  const for (0,1) */

	// Access operators

	/*! \remarks Allows access to x, y using the subscript operator.
	\return  An index of 0 will return x, 1 will return y. */
	double& operator[](int i)
	{
		assert((i == 0) || (i == 1));
		return (i == 0) ? x : y;
	}
	const double& operator[](int i) const
	{
		assert((i == 0) || (i == 1));
		return (i == 0) ? x : y;
	}

	// Conversion function

	/**  Conversion function. Returns the address of the DPoint2.x
	\return a pointer to the x component. */
	constexpr operator double*()
	{
		return &x;
	}
	constexpr operator const double*() const
	{
		return &x;
	}

	/** Convert DPoint2 to Point2 */
	constexpr operator Point2()
	{
		return Point2(x, y);
	}

	// Unary operators

	/** Unary - operator. Negates both x and y */
	constexpr DPoint2 operator-() const
	{
		return DPoint2(-x, -y);
	}

	/** Unary +. Returns this point unaltered.*/
	constexpr DPoint2 operator+() const
	{
		return *this;
	}

	/**  Subtracts a DPoint2 from this DPoint2.
	\param a the value to subtract from this DPoint2
	\return the resulting DPoint2 */
	constexpr DPoint2& operator-=(const DPoint2& a)
	{
		x -= a.x;
		y -= a.y;
		return *this;
	}

	/**  Adds a DPoint2 to this DPoint2.
	\param a the value to add to this DPoint2
	\return the resulting DPoint2 */
	constexpr DPoint2& operator+=(const DPoint2& a)
	{
		x += a.x;
		y += a.y;
		return *this;
	}

	/** Member-wise multiplication of two vectors: (x*x, y*y)
	\param a the multiplier
	\return the resulting DPoint2 */
	constexpr DPoint2& operator*=(const DPoint2& a)
	{
		x *= a.x;
		y *= a.y;
		return *this;
	}

	/** Member-wise division of two vectors
	\param a the multiplier
	\return the resulting DPoint2 */
	DPoint2& operator/=(const DPoint2& a)
	{
		assert(a.x != 0.0 && a.y != 0.0);
		x /= a.x;
		y /= a.y;
		return *this;
	}

	/** Each element of this DPoint2 is increased by the specified double.*/
	constexpr DPoint2& operator+=(double a)
	{
		x += a;
		y += a;
		return *this;
	}

	/** Each element of this DPoint2 is decreased by the specified double.*/
	constexpr DPoint2& operator-=(double a)
	{
		return *this += -a;
	}

	/** Each element of this DPoint2 is multiplied by the specified
	double.
	\param a the multiplier
	\return the resulting DPoint2 */
	constexpr DPoint2& operator*=(double a)
	{
		x *= a;
		y *= a;
		return *this;
	}

	/** Each element of this DPoint2 is divided by the specified
	double.
	\param a the divisor
	\return the resulting DPoint2 */
	DPoint2& operator/=(double a)
	{
		assert(a != 0.0);
		return *this *= (1.0 / a);
	}

	// Binary operators

	/*!  Subtracts a DPoint2 from a DPoint2.
	\param a the value to subtract from this DPoint2
	\return the resulting DPoint2 */
	constexpr DPoint2 operator-(const DPoint2& a) const
	{
		return DPoint2(x - a.x, y - a.y);
	}

	/*!  Adds a DPoint2 to a DPoint2.
	\param a the value to add to this DPoint2
	\return the resulting DPoint2 */
	constexpr DPoint2 operator+(const DPoint2& a) const
	{
		return DPoint2(x + a.x, y + a.y);
	}

	/** Member-wise multiplication of two vectors: (x*x, y*y)
	\param a the multiplier
	\return the resulting DPoint2 */
	constexpr DPoint2 operator*(const DPoint2& a) const
	{
		return DPoint2(x * a.x, y * a.y);
	}

	/** Member-wise division of two vectors:
	\param a the multiplier
	\return the resulting DPoint2 */
	DPoint2 operator/(const DPoint2& a) const
	{
		assert(a.x != 0.0 && a.y != 0.0);
		return DPoint2(x / a.x, y / a.y);
	}

	/*!  Returns the dot product of two DPoint2s. */
	constexpr double DotProd(const DPoint2& a) const
	{
		return x * a.x + y * a.y;
	}
	/*!  Returns the dot product of two DPoint2s. */
	constexpr double operator%(const DPoint2& a) const
	{
		return DotProd(a);
	}

	/** The 'Length' squared of this point. This is v.x*v.x+v.y*v.y.
	\return the length value */
	constexpr double LengthSquared() const
	{
		return x * x + y * y;
	}

	/*!  Returns the 'Length' of this point (vector). This is:\n\n
	sqrt(v.x*v.x+v.y*v.y).
	\return the length value */
	double Length() const
	{
		return sqrt(LengthSquared());
	}

	/** In place normalize
	\return the normalized value */
	DPoint2& Unify();

	double LengthUnify();

	/*! \remarks Returns the component with the maximum abs value. 0=x, 1=y. */
	int MaxComponent() const;
	/*! \remarks Returns the component with the minimum abs value. 0=x, 1=y. */
	int MinComponent() const;

	/** Returns a unit vector. This is a DPoint2 with each component divided by the point Length().
	\return the unit vector for the specified DPoint2*/
	DPoint2 Normalize() const;

	// Relational operators
	/*! \remarks Equality operator. Compares two Point2's.*/
	constexpr bool operator==(const DPoint2& p) const
	{
		return p.x == x && p.y == y;
	}
	constexpr bool operator!=(const DPoint2& p) const
	{
		return !(*this == p);
	}
	/*! \remarks Returns true if the absolute difference between point components
	is less or equal to the given epsilon for each component*/
	bool Equals(const DPoint2& p, double epsilon = 1E-6) const;
};

/*!  Returns the 'Length' of the point. This is sqrt(v.x*v.x+v.y*v.y)
\param p the DPoint2 to test
\return the length value */
inline double Length(const DPoint2& p)
{
	return p.Length();
}
/** The 'Length' squared of this point. This is v.x*v.x+v.y*v.y.
\param p the DPoint2 to test
\return the length value */
constexpr double LengthSquared(const DPoint2& p)
{
	return p.LengthSquared();
}

/** Returns the component with the maximum absolute value.
\param p the DPoint2 to test
\return the maximum component (0=x, 1=y). */
inline int MaxComponent(const DPoint2& p)
{
	return p.MaxComponent();
}
/** Returns the component with the minimum absolute value. 0=x, 1=y.
\param p the DPoint2 to test
\return the minimum component (0=x, 1=y). */
inline int MinComponent(const DPoint2& p)
{
	return p.MinComponent();
}

/** Returns a unit vector. This is a DPoint2 with each component
divided by the point Length().
\param p the DPoint2 to test
\return the unit vector for the specified DPoint2*/
inline DPoint2 Normalize(const DPoint2& p)
{
	return p.Normalize();
}

/*!  Returns the dot product of two DPoint2s. */
constexpr double DotProd(const DPoint2& a, const DPoint2& b)
{
	return a % b;
}

/** Multiply a DPoint2 by a scalar
\param a the DPoint2
\param f the multiplier
\return the resulting DPoint2 */
constexpr DPoint2 operator*(const DPoint2& a, double f)
{
	return DPoint2(a.x * f, a.y * f);
}
constexpr DPoint2 operator*(double f, const DPoint2& a)
{
	return a * f;
}

/*! \remarks Returns a copy of the DPoint2 argument offset by (f, f). */
constexpr DPoint2 operator+(const DPoint2& a, double f)
{
	return DPoint2(a.x + f, a.y + f);
}
constexpr DPoint2 operator+(double f, const DPoint2& a)
{
	return a + f;
}

/*! \remarks Returns a copy of the DPoint2 argument offset by (-f, -f). */
constexpr DPoint2 operator-(const DPoint2& a, float f)
{
	return a + -f;
}

/** Divide a DPoint2 by a scalar
\param a the DPoint2
\param f the divisor
\return the resulting DPoint2 */
inline DPoint2 operator/(const DPoint2& a, double f)
{
	assert(f != 0.0);
	return a * (1.0 / f);
}

// Helper for converting DPoint2 to Point2
constexpr Point2 Point2FromDPoint2(const DPoint2& from)
{
	return Point2(from.x, from.y);
}


// Handy DPoint2 uses...

/** This class describes a vector in space using an origin point p, and a
unit direction vector in double precision. */
class DRay2
{
	// Warning - instances of this class are saved as a binary blob to scene file
	// Adding/removing members will break file i/o
public:
	// point of origin
	DPoint2 p;
	// unit vector
	DPoint2 dir;
};

/** Double-Precision Line Segment Intersection test.
Determines if two line segments intersect.
\param seg1Start endpoint 1 for the first line segment
\param seg1End endpoint 2 for the first line segment
\param seg2Start endpoint 1 for the second line segment
\param seg2End endpoint 2 for the second line segment
\param intersectPoint if the lines intersect, this will return the intersection point.
\return  Intersection determination (0: No intersection, 1: Lines intersect (intersection location in 'intersectPoint'),
2: Lines parallel (no intersection)) */
GEOMEXPORT int DoublePrecisionLineSegmentIntersection(const DPoint2& seg1Start, const DPoint2& seg1End,
		const DPoint2& seg2Start, const DPoint2& seg2End, DPoint2& intersectPoint);

/** Double-Precision Line Intersection test.
Determines if two infinite lines cross, and if so, where.
\param line1PointA point 1 on the first line
\param line1PointB point 2 on the first line
\param line2PointA point 1 on the second line
\param line2PointB point 2 on the second line
\param intersectPoint if the lines intersect, this will return the intersection point.
\return Intersection determination (0: Lines parallel, 1: Lines intersect (intersection location in 'intersectPoint'))
*/
GEOMEXPORT int DoublePrecisionLineIntersection(const DPoint2& line1PointA, const DPoint2& line1PointB,
		const DPoint2& line2PointA, const DPoint2& line2PointB, DPoint2& intersectPoint);
