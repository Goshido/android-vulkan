/**********************************************************************
 *<
	FILE: ipoint3.h

	DESCRIPTION: Class definitions for IPoint3

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once
#include "GeomExport.h"

#include <cassert>
#include <cmath>
#include <iosfwd>

/*! \sa  Class Point3.\n\n
\par Description:
This class describes a 3D point using integer x, y and z coordinates. Methods
are provided to add and subtract points, multiply and divide by scalars, and
element by element multiply and divide two points. All methods are implemented
by the system. <br>
\par Data Members:
<b>int x,y,z;</b> */
class GEOMEXPORT IPoint3
{
public:
	int x = 0;
	int y = 0;
	int z = 0;

	//! Initializes all vector components to zero.
	IPoint3() = default;
	constexpr IPoint3(const IPoint3&) = default;
	constexpr IPoint3(IPoint3&&) = default;
	IPoint3& operator=(const IPoint3&) = default;
	IPoint3& operator=(IPoint3&&) = default;

	static const IPoint3 Origin;
	static const IPoint3 XAxis;
	static const IPoint3 YAxis;
	static const IPoint3 ZAxis;

	/*! \remarks Constructor. x, y, and z are initialized to the values
	specified. */
	constexpr IPoint3(int X, int Y, int Z) : x(X), y(Y), z(Z) {}
	/*! \remarks Constructor. x, y, and z are initialized to. ai[0], ai[1],
	and ai[2] respectively. */
	constexpr IPoint3(const int ai[3]) : x(ai[0]), y(ai[1]), z(ai[2]) {}

	// Access operators
	/*! \remarks Allows access to x, y and z using the [ ] operator.
	\return  An index of 0 will return x, 1 will return y, 2 will return z. */
	int& operator[](int i)
	{
		assert((i >= 0) && (i <= 2));
		return (&x)[i];
	}
	/*! \remarks Allows access to x, y and z using the [ ] operator.
	\return  An index of 0 will return x, 1 will return y, 2 will return z. */
	const int& operator[](int i) const
	{
		assert((i >= 0) && (i <= 2));
		return (&x)[i];
	}

	// Conversion function
	/*! \remarks Conversion function. Returns the address of the IPoint3. */
	constexpr operator int*()
	{
		return &x;
	}
	constexpr operator const int*() const
	{
		return &x;
	}

	// Unary operators
	/*! \remarks Unary - operator. Negates x, y and z. */
	constexpr IPoint3 operator-() const
	{
		return IPoint3(-x, -y, -z);
	}
	/*! \remarks Unary +. Returns the point unaltered. */
	constexpr IPoint3 operator+() const
	{
		return *this;
	}

	/*! \remarks Returns the length squared of the IPoint3*/
	constexpr int LengthSquared() const
	{
		return x * x + y * y + z * z;
	}
	/*! \remarks Returns the length of the IPoint3*/
	float Length() const
	{
		return sqrtf(float(LengthSquared()));
	}

	/*! \remarks Returns the component with the maximum abs value. 0=x, 1=y, 2=z. */
	int MaxComponent() const;
	/*! \remarks Returns the component with the minimum abs value. 0=x, 1=y, 2=z. */
	int MinComponent() const;

	// Assignment operators
	/*! \remarks Subtracts a IPoint3 from this IPoint3. */
	constexpr IPoint3& operator-=(const IPoint3& p)
	{
		x -= p.x;
		y -= p.y;
		z -= p.z;
		return *this;
	}
	/*! \remarks Adds a IPoint3 to this IPoint3. */
	constexpr IPoint3& operator+=(const IPoint3& p)
	{
		x += p.x;
		y += p.y;
		z += p.z;
		return *this;
	}
	//! Member-wise, in-place multiplication of this vector
	constexpr IPoint3& operator*=(const IPoint3& p)
	{
		x *= p.x;
		y *= p.y;
		z *= p.z;
		return *this;
	}
	//! Member-wise, in-place division of this vector
	IPoint3& operator/=(const IPoint3& p)
	{
		assert(p.x != 0 && p.y != 0 && p.z != 0);
		x /= p.x;
		y /= p.y;
		z /= p.z;
		return *this;
	}

	/*! \remarks Multiplies this IPoint3 by an integer value. */
	constexpr IPoint3& operator*=(int f)
	{
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}
	/*! \remarks Divides this IPoint3 by an integer value. */
	IPoint3& operator/=(int f)
	{
		assert(f != 0);
		x /= f;
		y /= f;
		z /= f;
		return *this;
	}
	//! \remarks Adds int point value to this IPoint3.
	constexpr IPoint3& operator+=(int f)
	{
		x += f;
		y += f;
		z += f;
		return *this;
	}
	//! \remarks Subtracts int point value from this IPoint3.
	constexpr IPoint3& operator-=(int f)
	{
		return *this += -f;
	}

	// Binary operators
	/*! \remarks Subtracts a IPoint3 from a IPoint3. */
	constexpr IPoint3 operator-(const IPoint3& p) const
	{
		return IPoint3(x - p.x, y - p.y, z - p.z);
	}
	/*! \remarks Adds a IPoint3 to a IPoint3. */
	constexpr IPoint3 operator+(const IPoint3& p) const
	{
		return IPoint3(x + p.x, y + p.y, z + p.z);
	}
	//! Member-wise multiplication of two vectors: (x*x, y*y, z*z)
	constexpr IPoint3 operator*(const IPoint3& p) const
	{
		return IPoint3(x * p.x, y * p.y, z * p.z);
	}
	//! Member-wise division of two vectors: (x/x, y/y, z/z)
	IPoint3 operator/(const IPoint3& p) const
	{
		assert(p.x != 0 && p.y != 0 && p.z != 0);
		return IPoint3(x / p.x, y / p.y, z / p.z);
	}
	/*! \remarks Returns the dot product of two IPoint3s. */
	constexpr int DotProd(const IPoint3& p) const
	{
		return x * p.x + y * p.y + z * p.z;
	}
	constexpr int operator%(const IPoint3& p) const
	{
		return DotProd(p);
	}

	/*! \remarks The cross product of two IPoint3's (vectors). */
	constexpr IPoint3 CrossProd(const IPoint3& p) const
	{
		return IPoint3(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x);
	}
	/*! \remarks The cross product of two IPoint3's (vectors). */
	constexpr IPoint3 operator^(const IPoint3& p) const
	{
		return CrossProd(p);
	}

	// Relational operators
	/*! \remarks Test for equality between two IPoint3's.
	\return  true if the IPoint3's are equal; otherwise false.\n\n
	  */
	constexpr bool operator==(const IPoint3& p) const
	{
		return x == p.x && y == p.y && z == p.z;
	}
	constexpr bool operator!=(const IPoint3& p) const
	{
		return !(*this == p);
	}
};

/*! \remarks Returns the component with the maximum abs value. 0=x, 1=y, 2=z. */
inline int MaxComponent(const IPoint3& p)
{
	return p.MaxComponent();
}
/*! \remarks Returns the component with the minimum abs value. 0=x, 1=y, 2=z. */
inline int MinComponent(const IPoint3& p)
{
	return p.MinComponent();
}

/*! \remarks Returns the length of the IPoint3 */
inline float Length(const IPoint3& p)
{
	return p.Length();
}
/*! \remarks Returns the length squared of the IPoint3 */
constexpr int LengthSquared(const IPoint3& p)
{
	return p.LengthSquared();
}

/*! \remarks Returns dot product of two IPoint3 */
constexpr int DotProd(const IPoint3& a, const IPoint3& b)
{
	return a % b;
}

/*! \remarks Returns cross product of two IPoint3 */
constexpr IPoint3 CrossProd(const IPoint3& a, const IPoint3& b)
{
	return a ^ b;
}

/*! \remarks Returns an IPoint3 multiplied by a scalar. */
constexpr IPoint3 operator*(const IPoint3& p, int f)
{
	return IPoint3(p.x * f, p.y * f, p.z * f);
}
constexpr IPoint3 operator*(int f, const IPoint3& p)
{
	return p * f;
}

/*! \remarks Returns an IPoint3 offset by (f,f,f). */
constexpr IPoint3 operator+(const IPoint3& p, int f)
{
	return IPoint3(p.x + f, p.y + f, p.z + f);
}
constexpr IPoint3 operator+(int f, const IPoint3& p)
{
	return p + f;
}

/*! \remarks Returns an IPoint3 offset by (-f,-f,-f). */
constexpr IPoint3 operator-(const IPoint3& p, int f)
{
	return p + -f;
}

/*! \remarks Returns an IPoint3 whose members are divided by a scalar.*/
inline IPoint3 operator/(const IPoint3& p, int f)
{
	assert(f != 0);
	return IPoint3(p.x / f, p.y / f, p.z / f);
}

