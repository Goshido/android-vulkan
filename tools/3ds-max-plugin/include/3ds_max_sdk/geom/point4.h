/**********************************************************************
 *<
   FILE: point4.h

   DESCRIPTION: Class definitions for Point4

   CREATED BY: Dan Silva

   HISTORY:

 *>   Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "GeomExport.h"
#include "point3.h"

#include <cassert>

class AColor;

/*! \sa  Class Point3.\n\n
\par Description:
This class describes a point using float x, y, z and w coordinates. Methods are
provided to add and subtract points, multiply and divide by scalars, and
element by element multiply and divide two points.
\par Data Members:
<b>float x, y, z, w;</b>\n\n
The x, y, z and w components of the point.\n\n
<b>static const Point4 Origin;</b>\n\n
This is equivalent to <b>Point4(0.0f, 0.0f, 0.0f, 0.0f);</b>\n\n
<b>static const Point4 XAxis;</b>\n\n
This is equivalent to <b>Point4(1.0f, 0.0f, 0.0f, 0.0f);</b>\n\n
<b>static const Point4 YAxis;</b>\n\n
This is equivalent to <b>Point4(0.0f,1.0f, 0.0f, 0.0f);</b>\n\n
<b>static const Point4 ZAxis;</b>\n\n
This is equivalent to <b>Point4(0.0f, 0.0f,1.0f, 0.0f);</b>\n\n
<b>static const Point4 WAxis;</b>\n\n
This is equivalent to <b>Point4(0.0f, 0.0f, 0.0f,1.0f);</b>  */
class GEOMEXPORT Point4
{
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

	//! Initializes vector components to zero.
	Point4() = default;
	constexpr Point4(const Point4&) = default;
	constexpr Point4(Point4&&) = default;
	Point4& operator=(const Point4&) = default;
	Point4& operator=(Point4&&) = default;

	/*! \remarks Constructor. x, y, z and w are initialized to the values specified. */
	constexpr Point4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}
	/*! \remarks Constructor. x, y, z and w are initialized to the specified values (cast as floats). */
	constexpr Point4(double X, double Y, double Z, double W) : Point4(float(X), float(Y), float(Z), float(W)) {}
	/*! \remarks Constructor. x, y, z and w are initialized to the specified values (cast as floats). */
	constexpr Point4(int X, int Y, int Z, int W) : Point4(float(X), float(Y), float(Z), float(W)) {}
	/*! \remarks Constructor. x, y, z and w are initialized to the specified Point3 and W. */
	constexpr Point4(const Point3& a, float W = 0.f) : x(a.x), y(a.y), z(a.z), w(W) {}
	/*! \remarks Constructor. x, y, z and w are initialized to af[0], af[1], af[2] and af[3] respectively. */
	constexpr Point4(const float af[4]) : x(af[0]), y(af[1]), z(af[2]), w(af[3]) {}

	explicit Point4(const AColor& c);

	// Data members
	static const Point4 Origin;
	static const Point4 XAxis;
	static const Point4 YAxis;
	static const Point4 ZAxis;
	static const Point4 WAxis;

	/*! \remarks Allows access to x, y, z and w using the subscript operator.
	\return  An value for <b>i</b> of 0 will return x, 1 will return y, 2 will
	return z and 3 will return w. */
	float& operator[](int i)
	{
		assert(i >= 0 && i <= 3);
		return (&x)[i];
	}
	/*! \remarks Allows access to x, y, z and w using the subscript operator.
	\return  An value for <b>i</b> of 0 will return x, 1 will return y, 2 will
	return z and 3 will return w. */
	const float& operator[](int i) const
	{
		assert(i >= 0 && i <= 3);
		return (&x)[i];
	}

	// Conversion function
	/*! \remarks Conversion function. Returns the address of the Point4.x */
	constexpr operator float* ()
	{
		return &x;
	}
	constexpr operator const float* () const
	{
		return &x;
	}

	// Unary operators
	/*! \remarks Unary - operator. Negates x, y, z and w. */
	constexpr Point4 operator-() const
	{
		return Point4(-x, -y, -z, -w);
	}
	/*! \remarks Unary +. Returns the Point4. */
	constexpr Point4 operator+() const
	{
		return *this;
	}

	// Property functions
	constexpr float LengthSquared() const
	{
		return x * x + y * y + z * z + w * w;
	}
	float Length() const
	{
		return (float)sqrt((double)LengthSquared());
	}
	float FLength() const
	{
		return sqrt(LengthSquared());
	}
	int MaxComponent() const;
	int MinComponent() const;
	Point4 Normalize() const; // more accurate than FNormalize()
	Point4 FNormalize() const; // faster than Normalize()

	// Assignment operators
	/*! \remarks Subtracts a Point4 from this Point4.
	\return  A Point4 that is the difference between two Point4s. */
	constexpr Point4& operator-=(const Point4& a)
	{
		x -= a.x;
		y -= a.y;
		z -= a.z;
		w -= a.w;
		return *this;
	}
	/*! \remarks Adds a Point4 to this Point4.
	\return  A Point4 that is the sum of two Point4s. */
	constexpr Point4& operator+=(const Point4& a)
	{
		x += a.x;
		y += a.y;
		z += a.z;
		w += a.w;
		return *this;
	}
	/*! \remarks Element-by-element multiplication of two Point4s:\n\n
	<b>(x*x, y*y, z*z, w*w)</b>.
	\return  A Point4 element-by-element multiplied by another Point4. */
	constexpr Point4& operator*=(const Point4& a)
	{
		x *= a.x;
		y *= a.y;
		z *= a.z;
		w *= a.w;
		return *this;
	}
	/*! \remarks Element-by-element division of two Point4s:\n\n
	<b>(x/x, y/y, z/z, w/w)</b>.
	\return  A Point4 element-by-element divided by another Point4. */
	Point4& operator/=(const Point4& a)
	{
		assert(a.x != 0 && a.y != 0 && a.z != 0 && a.w != 0);
		x /= a.x;
		y /= a.y;
		z /= a.z;
		w /= a.w;
		return *this;
	}

	/*! \remarks Adds floating point value to this Point4.
	\return  A Point4 + a float. */
	constexpr Point4& operator+=(float f)
	{
		x += f;
		y += f;
		z += f;
		w += f;
		return *this;
	}
	/*! \remarks Subtracts a floating point value from this Point4.
	\return  A Point4 - a float. */
	constexpr Point4& operator-=(float f)
	{
		return *this += -f;
	}
	/*! \remarks Multiplies this Point4 by a floating point value.
	\return  A Point4 multiplied by a float. */
	constexpr Point4& operator*=(float f)
	{
		x *= f;
		y *= f;
		z *= f;
		w *= f;
		return *this;
	}
	/*! \remarks Divides this Point4 by a floating point value.
	\return  A Point4 divided by a float. */
	Point4& operator/=(float f)
	{
		assert(f != 0.f);
		if (f == 0.f)
			f = .000001f;
		return *this *= (1.f / f);
	}
	/*! \remarks Sets the x, y, z and w coordinate to the values passed and returns a
	reference to this Point4.
	\par Parameters:
	<b>float X</b>\n\n
	The new x value.\n\n
	<b>float Y</b>\n\n
	The new y value.\n\n
	<b>float Z</b>\n\n
	The new z value.\n\n
	<b>float W</b>\n\n
	The new w value.
	\return  A reference to this <b>Point4</b>. */
	constexpr Point4& Set(float X, float Y, float Z, float W)
	{
		return *this = { X, Y, Z, W };
	}

	// Test for equality
	/*! \remarks Equality operator. Test for equality between two Point4's.
	\return  Nonzero if the Point4's are equal; otherwise 0. */
	constexpr bool operator==(const Point4& p) const
	{
		return p.x == x && p.y == y && p.z == z && p.w == w;
	}
	constexpr bool operator!=(const Point4& p) const
	{
		return !(*this == p);
	}

	/*! \remarks	Compares this Point4 and the specified one to see if the x, y, z and w values
   are within plus or minus the specified tolerance.
   \par Parameters:
   <b>const Point4\& p</b>\n\n
   The point to compare.\n\n
   <b>float epsilon = 1E-6f</b>\n\n
   The tolerance to use in the comparison.
   \return  Nonzero if the points are 'equal'; otherwise zero.
   \par Operators:
   */
	bool Equals(const Point4& p, float epsilon = 1E-6f) const;

	// In-place normalize
	Point4& Unify();
	float LengthUnify(); // returns old Length

	// Binary operators
	/*! \remarks Subtracts a Point4 from a Point4.
	\return  A Point4 that is the difference between two Point4s. */
	constexpr Point4 operator-(const Point4& a) const
	{
		return Point4(*this) -= a;
	}
	/*! \remarks Adds a Point4 to a Point4.
	\return  A Point4 that is the sum of two Point4s. */
	constexpr Point4 operator+(const Point4& a) const
	{
		return Point4(*this) += a;
	}
	/*! \remarks Divides a Point4 by a Point4 element by element.
	\return  A Point4 resulting from dividing a Point4 by a Point4 element by
	element. */
	Point4 operator/(const Point4& a) const
	{
		return Point4(*this) /= a;
	}
	/*! \remarks Multiplies a Point4 by a Point4 element by element.\n\n
	<b>(x*x, y*y, z*z, w*w)</b>.
	\return  A Point4 resulting from the multiplication of a Point4 and a
	Point4. */
	constexpr Point4 operator*(const Point4& a) const
	{
		return Point4(*this) *= a;
	}

	constexpr float DotProd(const Point4& a) const
	{
		return x * a.x + y * a.y + z * a.z + w * a.w;
	}
	constexpr float operator%(const Point4& a) const // DOT PRODUCT
	{
		return DotProd(a);
	}
};

GEOMEXPORT Point4 CrossProd(const Point4& a, const Point4& b, const Point4& c); // CROSS PRODUCT

inline int MaxComponent(const Point4& a) // the component with the maximum abs value
{
	return a.MaxComponent();
}
inline int MinComponent(const Point4& a) // the component with the minimum abs value
{
	return a.MinComponent();
}

inline Point4 Normalize(const Point4& a) // Accurate normalize
{
	return a.Normalize();
}
inline Point4 FNormalize(const Point4& a) // Fast normalize
{
	return a.FNormalize();
}

inline float Length(const Point4& v)
{
	return v.Length();
}
inline float FLength(const Point4& v)
{
	return v.FLength();
}
constexpr float LengthSquared(const Point4& v)
{
	return v.LengthSquared();
}

/*! \remarks Returns a Point4 that is the specified Point4 multiplied by the
specified float. */
constexpr Point4 operator*(const Point4& a, float f)
{
	return Point4(a) *= f;
}
constexpr Point4 operator*(float f, const Point4& a)
{
	return a * f;
}

/*! \remarks Returns a Point4 that is the specified Point4 divided by the
specified float. */
inline Point4 operator/(const Point4& a, float f)
{
	return Point4(a) /= f;
}

/*! \remarks Returns a Point4 that is the specified Point4 with the specified
floating point valued added to each component x, y, z and w. */
constexpr Point4 operator+(const Point4& a, float f)
{
	return Point4(a) += f;
}
constexpr Point4 operator+(float f, const Point4& a)
{
	return a + f;
}
constexpr Point4 operator-(const Point4& a, float f)
{
	return a + -f;
}

constexpr float DotProd(const Point4& a, const Point4& b)
{
	return a % b;
}
