/**********************************************************************
 *<
	FILE: point2.h

	DESCRIPTION: Class definition for Point2

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "GeomExport.h"
#include "ipoint2.h"

#include <cassert>
#include <cmath>
#include <iosfwd>

/*! \sa  Class IPoint2.\n\n
\par Description:
This class describes a 2D point using float x and y coordinates. Methods are
provided to add and subtract points, multiply and divide by scalars, normalize
and compute the dot product of two Point2s. All methods are implemented by the
system.
\par Data Members:
<b>float x,y;</b>\n\n
The x and y components of the point.\n\n
<b>static const Point2 Origin;</b>\n\n
This is equivalent to <b>Point2(0.0f, 0.0f);</b>\n\n
<b>static const Point2 XAxis;</b>\n\n
This is equivalent to <b>Point2(1.0f, 0.0f);</b>\n\n
<b>static const Point2 YAxis;</b>\n\n
This is equivalent to <b>Point2(0.0f, 1.0f);</b> <br>  Constructors */
class GEOMEXPORT Point2
{
public:
	float x = 0.f;
	float y = 0.f;

	Point2() = default;
	constexpr Point2(const Point2&) = default;
	constexpr Point2(Point2&&) = default;
	Point2& operator=(const Point2&) = default;
	Point2& operator=(Point2&&) = default;

	/*! \remarks Constructor. Data members are initialized to X and Y. */
	constexpr Point2(float X, float Y) : x(X), y(Y) {}
	/*! \remarks Constructor. Data members are initialized to X and Y cast as floats. */
	constexpr Point2(double X, double Y) : x((float)X), y((float)Y) {}
	/*! \remarks Constructor. Data members are initialized to X and Y cast as floats. */
	constexpr Point2(int X, int Y) : x((float)X), y((float)Y) {}
	/*! \remarks Constructor. Data members are initialized to a.x and a.y. */
	constexpr explicit Point2(const IPoint2& a) : x((float)a.x), y((float)a.y) {}
	/*! \remarks Constructor. Data members are initialized as x = af[0] and y = af[1]. */
	constexpr Point2(const float af[2]) : x(af[0]), y(af[1]) {}

	// Data members
	static const Point2 Origin;
	static const Point2 XAxis;
	static const Point2 YAxis;

	/*! \remarks Allows access to x, y using the subscript operator.
	\return  A value for <b>i</b> of 0 will return x, 1 will return y. */
	float& operator[](int i)
	{
		assert((i == 0) || (i == 1));
		return (i == 0) ? x : y;
	}
	const float& operator[](int i) const
	{
		assert((i == 0) || (i == 1));
		return (i == 0) ? x : y;
	}

	// Conversion function
	/*! \remarks Returns the address of the Point2.x */
	constexpr operator float*()
	{
		return &x;
	}
	constexpr operator const float*() const
	{
		return &x;
	}

	// Unary operators
	/*! \remarks Unary -. Negates both x and y.*/
	constexpr Point2 operator-() const
	{
		return Point2(-x, -y);
	}
	/*! \remarks Unary +. Returns the point unaltered.*/
	constexpr Point2 operator+() const
	{
		return *this;
	}

	// Property functions
	/*! \remarks The 'Length' squared of the point. This is <b>v.x*v.x+v.y*v.y.</b> */
	constexpr float LengthSquared() const
	{
		return x * x + y * y;
	}
	/*! \remarks Returns the length of the point. This is <b>sqrt(v.x*v.x+v.y*v.y);</b> */
	float Length() const
	{
		return sqrtf(LengthSquared());
	}
	/*! \remarks Returns the component with the maximum abs value. 0=x, 1=y. */
	int MaxComponent() const;
	/*! \remarks Returns the component with the minimum abs value. 0=x, 1=y. */
	int MinComponent() const;
	/*! \remarks This method returns a normalized version of this Point2. This method is
	more accurate than <b>*this/Length()</b> (internal computations are done in	double precision). */
	Point2 Normalize() const; // more accurate than *this/Length();

	// Assignment operators
	//! Member-wise, in-place addition of this vector (x+a.x, y+a.y)
	constexpr Point2& operator+=(const Point2& a)
	{
		x += a.x;
		y += a.y;
		return *this;
	}
	//! Member-wise, in-place subtraction of this vector (x-a.x, y-a.y)
	constexpr Point2& operator-=(const Point2& a)
	{
		x -= a.x;
		y -= a.y;
		return *this;
	}

	//! Member-wise, in-place multiplication of this vector: (x*a.x, y*a.y)
	constexpr Point2& operator*=(const Point2& a)
	{
		x *= a.x;
		y *= a.y;
		return *this;
	}

	//! Member-wise, in-place division of this vector: (x/a.x, y/a.y)
	Point2& operator/=(const Point2& a)
	{
		assert(a.x != 0 && a.y != 0);
		x /= a.x;
		y /= a.y;
		return *this;
	}

	//! \remarks Adds floating point value to this Point2.
	constexpr Point2& operator+=(float f)
	{
		x += f;
		y += f;
		return *this;
	}

	//! \remarks Subtracts floating point value from this Point2.
	constexpr Point2& operator-=(float f)
	{
		return *this += -f;
	}

	/*! \remarks Multiplies this Point2 by a floating point value.*/
	constexpr Point2& operator*=(float f)
	{
		x *= f;
		y *= f;
		return *this;
	}
	/*! \remarks Divides this Point2 by a floating point value. */
	Point2& operator/=(float f)
	{
		assert(f != 0.0f);
		return (*this) *= (1.0f / f);
	}

	/*! \remarks Sets the x and y coordinate and returns a reference to this Point2.*/
	constexpr Point2& Set(float X, float Y)
	{
		x = X;
		y = Y;
		return *this;
	}

	// Binary operators
	//! Member-wise subtraction of two vectors: (x-a.x, y-a.y)
	constexpr Point2 operator-(const Point2& a) const
	{
		return Point2(x - a.x, y - a.y);
	}
	//! Member-wise addition of two vectors: (x+a.x, y+a.y)
	constexpr Point2 operator+(const Point2& a) const
	{
		return Point2(x + a.x, y + a.y);
	}
	//! Member-wise multiplication of two vectors: (x*a.x, y*a.y)
	constexpr Point2 operator*(const Point2& a)const
	{
		return Point2(x * a.x, y * a.y);
	}
	//! Member-wise division of two vectors: (x/a.x, y/a.y)
	Point2 operator/(const Point2& a)const
	{
		assert(a.x != 0 && a.y != 0);
		return Point2(x / a.x, y / a.y);
	}
	//! Returns the dot product of two Point2's
	constexpr float DotProd(const Point2& a) const
	{
		return x * a.x + y * a.y;
	}
	//! Returns the dot product of two Point2's
	constexpr float operator%(const Point2& a) const
	{
		return DotProd(a);
	}

	// Relational operators
	/*! \remarks Equality operator. Compares two Point2's.*/
	constexpr bool operator==(const Point2& p) const
	{
		return x == p.x && y == p.y;
	}
	constexpr bool operator!=(const Point2& p) const
	{
		return !(*this == p);
	}

	/*! \remarks Returns true if the absolute difference between point components
	is less or equal to the given epsilon for each component*/
	bool Equals(const Point2& p, float epsilon = 1E-6f) const;

	// In-place normalize
	/*! \remarks This method is used to unify (or normalize) this Point2 (in place) and
	return the result. Internal computations are done in double precision. */
	Point2& Unify();
	/*! \remarks This method is used to unify (or normalize) this Point2 (in place) and
	return the previous length. Internal computations are done in double precision.
	*/
	float LengthUnify(); // returns old Length
};

// Inlines:
/*! \remarks Returns the length of the point. This is <b>sqrt(v.x*v.x+v.y*v.y);</b> */
inline float Length(const Point2& v)
{
	return v.Length();
}
/*! \remarks The 'Length' squared of the point. This is <b>v.x*v.x+v.y*v.y.</b> */
constexpr float LengthSquared(const Point2& v)
{
	return v.LengthSquared();
}

/*! \remarks Returns the component with the maximum absolute value. 0=x, 1=y.*/
inline int MaxComponent(const Point2& v)
{
	return v.MaxComponent();
}
/*! \remarks Returns the component with the minimum absolute value. 0=x, 1=y.*/
inline int MinComponent(const Point2& v)
{
	return v.MinComponent();
}

/*! \remarks Returns a unit vector. This is a Point2 with each component
divided by the point <b>Length()</b>. */
inline Point2 Normalize(const Point2& v)
{
	return v.Normalize();
}

/*! \remarks Returns the dot product of two Point2's */
constexpr float DotProd(const Point2& a, const Point2& b)
{
	return a % b;
}

/*! \remarks Returns a Point2 multiplied by a scalar. */
constexpr Point2 operator*(const Point2& a, float f)
{
	return Point2(a.x * f, a.y * f);
}
constexpr Point2 operator*(float f, const Point2& a)
{
	return a * f;
}

/*! \remarks Returns a copy of the Point2 argument offset by (f, f). */
constexpr Point2 operator+(const Point2& a, float f)
{
	return Point2(a.x + f, a.y + f);
}
constexpr Point2 operator+(float f, const Point2& a)
{
	return a + f;
}

/*! \remarks Returns a copy of the Point2 argument offset by (-f, -f). */
constexpr Point2 operator-(const Point2& a, float f)
{
	return a + -f;
}

/*! \remarks Returns a Point2 whose x and y members are divided by a scalar.*/
inline Point2 operator/(const Point2& a, float f)
{
	assert(f != 0.0f);
	return a * (1.0f / f);
}

