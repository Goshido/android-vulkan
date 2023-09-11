/**********************************************************************
 *<
	FILE: ipoint2.h

	DESCRIPTION: Class definintion for IPoint2: Integer 2D point.

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once
#include "GeomExport.h"

#include <cassert>
#include <cmath>
#include <iosfwd>


/*! \sa  Class Point2.\n\n
\par Description:
This class describes a 2D point using int x and y coordinates. Methods are
provided to add and subtract points, multiply and divide by scalars, normalize
and compute the dot product of two IPoint2s. All methods are implemented by the
system.
\par Data Members:
<b>int x,y;</b>  */
class GEOMEXPORT IPoint2
{
public:
	int x = 0;
	int y = 0;

	IPoint2() = default;
	constexpr IPoint2(const IPoint2&) = default;
	constexpr IPoint2(IPoint2&&) = default;
	IPoint2& operator=(const IPoint2&) = default;
	IPoint2& operator=(IPoint2&&) = default;

	/*! \remarks Constructor. Data members are initialized to X and Y. */
	constexpr IPoint2(int X, int Y) : x(X), y(Y) {}
	/*! \remarks Constructor. Data members are initialized as x = af[0] and y = af[1]. */
	constexpr IPoint2(int af[2]) : x(af[0]), y(af[1]) {}

	static const IPoint2 Origin;
	static const IPoint2 XAxis;
	static const IPoint2 YAxis;

	// Access operators
	/*! \remarks Allows access to x, y using the subscript operator.
	\return  An index of 0 will return x, 1 will return y. */
	int& operator[](int i)
	{
		assert((i == 0) || (i == 1));
		return (i == 0) ? x : y;
	}
	const int& operator[](int i) const
	{
		assert((i == 0) || (i == 1));
		return (i == 0) ? x : y;
	}

	// Conversion function
	/*! \remarks Returns the address of the IPoint2.x. */
	constexpr operator int*()
	{
		return &x;
	}
	constexpr operator const int*() const
	{
		return &x;
	}

	// Unary operators
	/*! \remarks Unary -. Negates both x and y. */
	constexpr IPoint2 operator-() const
	{
		return IPoint2(-x, -y);
	}
	/*! \remarks Unary +. Returns the Ipoint2 unaltered. */
	constexpr IPoint2 operator+() const
	{
		return *this;
	}

	/*! \remarks Returns the length squared of the IPoint2*/
	constexpr int LengthSquared() const
	{
		return x * x + y * y;
	}

	/*! \remarks Returns the length of the IPoint2*/
	float Length() const
	{
		return (float)sqrt((double)LengthSquared());
	}

	/*! \remarks Returns the component with the maximum abs value. 0=x, 1=y. */
	int MaxComponent() const;
	/*! \remarks Returns the component with the minimum abs value. 0=x, 1=y. */
	int MinComponent() const;

	// Assignment operators
	/*! \remarks Subtracts a IPoint2 from this IPoint2. */
	constexpr IPoint2& operator-=(const IPoint2& a)
	{
		x -= a.x;
		y -= a.y;
		return *this;
	}

	/*! \remarks Adds a IPoint2 to this IPoint2. */
	constexpr IPoint2& operator+=(const IPoint2& a)
	{
		x += a.x;
		y += a.y;
		return *this;
	}

	//! Member-wise, in-place multiplication of this vector: (x*x, y*y)
	constexpr IPoint2& operator*=(const IPoint2& a)
	{
		x *= a.x;
		y *= a.y;
		return *this;
	}

	//! Member-wise, in-place division of this vector:
	constexpr IPoint2& operator/=(const IPoint2& a)
	{
		x /= a.x;
		y /= a.y;
		return *this;
	}

	/*! \remarks Multiplies this IPoint2 by an integer value. */
	constexpr IPoint2& operator*=(int f)
	{
		x *= f;
		y *= f;
		return *this;
	}

	/*! \remarks Divides this IPoint2 by an integer value. */
	constexpr IPoint2& operator/=(int f)
	{
		x /= f;
		y /= f;
		return *this;
	}

	//! \remarks Adds floating point value to this Point2.
	constexpr IPoint2& operator+=(int f)
	{
		x += f;
		y += f;
		return *this;
	}

	//! \remarks Subtracts floating point value from this Point2.
	constexpr IPoint2& operator-=(int f)
	{
		return *this += -f;
	}

	// Binary operators
	/*! \remarks Subtracts IPoint2 from this IPoint2. */
	constexpr IPoint2 operator-(const IPoint2& a) const
	{
		return IPoint2(x - a.x, y - a.y);
	}
	/*! \remarks Adds IPoint2 to this IPoint2. */
	constexpr IPoint2 operator+(const IPoint2& a) const
	{
		return IPoint2(x + a.x, y + a.y);
	}

	//! Member-wise multiplication of two vectors: (x*x, y*y)
	constexpr IPoint2 operator*(const IPoint2& a) const
	{
		return IPoint2(x * a.x, y * a.y);
	}

	//! Member-wise division of two vectors:
	constexpr IPoint2 operator/(const IPoint2& a) const
	{
		return IPoint2(x / a.x, y / a.y);
	}

	/*! \remarks Returns the dot product of two IPoint2's. */
	constexpr int DotProd(const IPoint2& a) const
	{
		return x * a.x + y * a.y;
	}
	constexpr int operator%(const IPoint2& a) const
	{
		return DotProd(a);
	}

	// Relational operators
	  /*! \remarks Equality operator. Compares two Point2's.*/
	constexpr bool operator==(const IPoint2& a) const
	{
		return x == a.x && y == a.y;
	}
	constexpr bool operator!=(const IPoint2& a) const
	{
		return !(*this == a);
	}
};

// Inlines:

/*! \remarks Returns the component with the maximum abs value. 0=x, 1=y. */
inline int MaxComponent(const IPoint2& p)
{
	return p.MaxComponent();
}
/*! \remarks Returns the component with the minimum abs value. 0=x, 1=y. */
inline int MinComponent(const IPoint2& p)
{
	return p.MinComponent();
}

/*! \remarks Returns the length of the IPoint2 */
inline float Length(const IPoint2& p)
{
	return p.Length();
}
/*! \remarks Returns the length squared of the IPoint2 */
constexpr int LengthSquared(const IPoint2& p)
{
	return p.LengthSquared();
}

/*! \remarks Returns dot product of two IPoint2 */
constexpr int DotProd(const IPoint2& a, const IPoint2& b)
{
	return a % b;
}

/*! \remarks Returns an IPoint2 multiplied by a scalar. */
constexpr IPoint2 operator*(const IPoint2& a, int f)
{
	return IPoint2(a.x * f, a.y * f);
}
constexpr IPoint2 operator*(int f, const IPoint2& a)
{
	return a * f;
}

/*! \remarks Returns an IPoint2 offset by (f,f). */
constexpr IPoint2 operator+(const IPoint2& a, int f)
{
	return IPoint2(a.x + f, a.y + f);
}
constexpr IPoint2 operator+(int f, const IPoint2& a)
{
	return a + f;
}

/*! \remarks Returns an IPoint2 offset by (-f,-f). */
constexpr IPoint2 operator-(const IPoint2& a, int f)
{
	return a + -f;
}

/*! \remarks Returns an IPoint2 whose x and y members are divided by a scalar.*/
constexpr IPoint2 operator/(const IPoint2& a, int f)
{
	return IPoint2(a.x / f, a.y / f);
}

