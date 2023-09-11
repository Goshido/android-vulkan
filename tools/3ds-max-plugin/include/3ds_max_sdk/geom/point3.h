/**********************************************************************
 *<
   FILE: point3.h

   DESCRIPTION: Class definitions for Point3

   CREATED BY: Dan Silva

   HISTORY:

 *>   Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "GeomExport.h"
#include "point2.h"

#include <cassert>
#include <cmath>

class Color;
class AColor;
class Point4;

/*! \sa  Class IPoint3, Class DPoint3, Class Matrix3.\n\n
\par Description:
This class describes a 3D point using float x, y and z coordinates. Methods are
provided to add and subtract points, multiply and divide by scalars, and
element by element multiply and divide two points.\n\n
This class is also frequently used to simply store three floating point values
that may not represent a point. For example, a color value where x=red,
y=green, and z=blue. For color, the range of values is 0.0 to 1.0, where 0 is 0
and 1.0 is 255. All methods are implemented by the system.\n\n
Note: In 3ds Max, all vectors are assumed to be row vectors. Under this
assumption, multiplication of a vector with a matrix can be written either way
(Matrix*Vector or Vector*Matrix), for ease of use, and the result is the same
-- the (row) vector transformed by the matrix.
\par Data Members:
<b>float x, y, z;</b>\n\n
The x, y and z components of the point.\n\n
<b>static const Point3 Origin;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is equivalent to <b>Point3(0.0f, 0.0f, 0.0f);</b>\n\n
<b>static const Point3 XAxis;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is equivalent to <b>Point3(1.0f, 0.0f, 0.0f);</b>\n\n
<b>static const Point3 YAxis;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is equivalent to <b>Point3(0.0f, 1.0f, 0.0f);</b>\n\n
<b>static const Point3 ZAxis;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is equivalent to <b>Point3(0.0f, 0.0f, 1.0f);</b>  */
class GEOMEXPORT Point3
{
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	//! Initializes all vector components to zero.
	Point3() = default;
	/*! \remarks Constructor. x, y, and z are initialized to the specified Point3. */
	constexpr Point3(const Point3&) = default;
	/*! \remarks Move Constructor. x, y, and z are initialized to the specified Point3. */
	constexpr Point3(Point3&&) = default;
	/*! \remarks Copy Assignement Operator. x, y, and z are initialized to the specified Point3. */
	Point3& operator=(const Point3&) = default;
	/*! \remarks Move Assignement Operator. x, y, and z are initialized to the specified Point3. */
	Point3& operator=(Point3&&) = default;

	/*! \remarks Constructor. x, y, and z are initialized to the values specified. */
	constexpr Point3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
	/*! \remarks Constructor. x, y, and z are initialized to af[0], af[1], and af[2] respectively.*/
	constexpr Point3(const float af[3]) : x(af[0]), y(af[1]), z(af[2]) {}
	/*! \remarks Constructor. x, y, and z are initialized to the specified values (cast as floats). */
	constexpr Point3(double X, double Y, double Z) : x(float(X)), y(float(Y)), z(float(Z)) {}
	/*! \remarks Constructor. x, y, and z are initialized to the specified values (cast as floats). */
	constexpr Point3(int X, int Y, int Z) : x(float(X)), y(float(Y)), z(float(Z)) {}
	/*! \remarks Color Constructor. x = r, y = g, and z = b. */
	Point3(const Color& c);
	/*! \remarks AColor Constructor. x = r, y = g, and z = b. */
	Point3(const AColor& c);
	/*! \remarks Point4 Constructor. x = x, y = y, and z = z. */
	Point3(const Point4& p);

	// Data members
	static const Point3 Origin;
	static const Point3 XAxis;
	static const Point3 YAxis;
	static const Point3 ZAxis;

	/*! \remarks Allows access to x, y and z using the subscript operator.
	\return  An value for <b>i</b> of 0 will return x, 1 will return y, 2 will
	return z. */
	float& operator[](int i)
	{
		assert((i >= 0) && (i <= 2));
		return (&x)[i];
	}
	/*! \remarks Allows access to x, y and z using the subscript operator.
	\return  An value for <b>i</b> of 0 will return x, 1 will return y, 2 will
	return z. */
	const float& operator[](int i) const
	{
		assert((i >= 0) && (i <= 2));
		return (&x)[i];
	}

	// Conversion function
	/*! \remarks Conversion function. Returns the address of the Point3.x */
	constexpr operator float*()
	{
		return &x;
	}
	constexpr operator const float*() const
	{
		return &x;
	}

	// Unary operators
	/*! \remarks Unary - operator. Negates x, y and z. */
	constexpr Point3 operator-() const
	{
		return Point3(-x, -y, -z);
	}
	/*! \remarks Unary +. Returns the Point3. */
	constexpr Point3 operator+() const
	{
		return *this;
	}

	// Property functions
	/*! \remarks The 'Length' squared of this point. This is
	 <b>v.x*v.x+v.y*v.y+v.z*v.z.</b> */
	constexpr float LengthSquared() const
	{
		return x * x + y * y + z * z;
	}
	/*! \remarks Returns the 'Length' of this point (vector). */
	float Length() const
	{
		return (float)sqrt((double)LengthSquared());
	}
	/*! \remarks Returns the 'Length' of this point (vector) using float square root. */
	float FLength() const
	{
		return (float)sqrt(LengthSquared());
	}

	/*! \remarks Returns the component with the maximum abs value. 0=x, 1=y, 2=z. */
	int MaxComponent() const;
	/*! \remarks Returns the component with the minimum abs value. 0=x, 1=y, 2=z. */
	int MinComponent() const;

	/*! \remarks Normalizes point using float precision */
	Point3 Normalize() const;
	/*! \remarks Normalizes point using double precision */
	Point3 FNormalize() const;

	// Assignment operators
	/*! \remarks Subtracts a Point3 from this Point3. */
	constexpr Point3& operator-=(const Point3& p)
	{
		x -= p.x;
		y -= p.y;
		z -= p.z;
		return *this;
	}
	/*! \remarks Adds a Point3 to this Point3. */
	constexpr Point3& operator+=(const Point3& p)
	{
		x += p.x;
		y += p.y;
		z += p.z;
		return *this;
	}
	/*! \remarks Element-by-element multiplication of two Point3s */
	constexpr Point3& operator*=(const Point3& p)
	{
		x *= p.x;
		y *= p.y;
		z *= p.z;
		return *this;
	}
	//! Member-wise, in-place division of this vector
	Point3& operator/=(const Point3& p)
	{
		assert(p.x != 0 && p.y != 0 && p.z != 0);
		x /= p.x;
		y /= p.y;
		z /= p.z;
		return *this;
	}

	//! \remarks Adds floating point value to this Point3.
	constexpr Point3& operator+=(float f)
	{
		x += f;
		y += f;
		z += f;
		return *this;
	}
	//! \remarks Subtracts floating point value from this Point3.
	constexpr Point3& operator-=(float f)
	{
		return *this += -f;
	}
	/*! \remarks Multiplies this Point3 by a floating point value. */
	constexpr Point3& operator*=(float f)
	{
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}
	/*! \remarks Divides this Point3 by a floating point value. */
	Point3& operator/=(float f)
	{
		assert(f != 0);
		return *this *= (1.f/ f);
	}

	constexpr Point3& Set(float X, float Y, float Z)
	{
		x = X;
		y = Y;
		z = Z;
		return *this;
	}

	// Test for equality
	/*! \remarks Equality operator. Test for equality between two Point3's.
   \return  Nonzero if the Point3's are equal; otherwise 0. */
	constexpr bool operator==(const Point3& p) const
	{
		return p.x == x && p.y == y && p.z == z;
	}
	constexpr bool operator!=(const Point3& p) const
	{
		return !(*this == p);
	}
	/*! \remarks Returns true if the absolute difference between point components
	is less or equal to the given epsilon for each component*/
	bool Equals(const Point3& p, float epsilon = 1E-6f) const;

	// In-place normalize
	Point3& Unify();
	float LengthUnify(); // returns old Length

	// Binary operators
	/*! \remarks Subtracts a Point3 from a Point3. */
	constexpr Point3 operator-(const Point3& p) const
	{
		return Point3(x - p.x, y - p.y, z - p.z);
	}
	/*! \remarks Adds a Point3 to a Point3. */
	constexpr Point3 operator+(const Point3& p) const
	{
		return Point3(x + p.x, y + p.y, z + p.z);
	}
	/*! \remarks Divides a Point3 by a Point3 element by element. */
	Point3 operator/(const Point3& p) const
	{
		assert(p.x != 0 && p.y != 0 && p.z != 0);
		return Point3(x / p.x, y / p.y, z / p.z);
	}
	/*! \remarks Multiplies a Point3 by a Point3 element by element.\n\n
   <b>(x*x, y*y, z*z)</b>. */
	constexpr Point3 operator*(const Point3& p) const
	{
		return Point3(x * p.x, y * p.y, z * p.z);
	}

	/*! \remarks The dot  product of two Point3's (vectors).*/
	constexpr float DotProd(const Point3& p) const
	{
		return x * p.x + y * p.y + z * p.z;
	}
	constexpr float operator%(const Point3& p) const
	{
		return DotProd(p);
	}

	/*! \remarks The cross product of two Point3's (vectors).
   \return  The cross product of two Point3's. */
	constexpr Point3 CrossProd(const Point3& p) const
	{
		return Point3(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x);
	}
	constexpr Point3 operator^(const Point3& p) const
	{
		return CrossProd(p);
	}

	/*! Converts this Point3 into a Point2 using the X and Y components.
	Useful for passing a Point3 where a Point2 is expected. */
	constexpr Point2 XY() const
	{
		return Point2(x, y);
	}
};

/*! \remarks Returns the component with the maximum abs value. 0=x, 1=y, 2=z. */
inline int MaxComponent(const Point3& p)
{
	return p.MaxComponent();
}
/*! \remarks Returns the component with the minimum abs value. 0=x, 1=y, 2=z. */
inline int MinComponent(const Point3& p)
{
	return p.MinComponent();
}

/*! \remarks Returns the length of the Point3 */
inline float Length(const Point3& p)
{
	return p.Length();
}
/*! \remarks Returns the length of the Point3 */
inline float FLength(const Point3& p)
{
	return p.FLength();
}
/*! \remarks Returns the length squared of the Point3 */
constexpr float LengthSquared(const Point3& p)
{
	return p.LengthSquared();
}

/*! \remarks Returns dot product of two Point3 */
constexpr float DotProd(const Point3& a, const Point3& b)
{
	return a % b;
}

/*! \remarks Returns cross product of two Point3 */
constexpr Point3 CrossProd(const Point3& a, const Point3& b)
{
	return a ^ b;
}

/*! \remarks Returns an Point3 multiplied by a scalar. */
constexpr Point3 operator*(const Point3& p, float f)
{
	return Point3(p.x * f, p.y * f, p.z * f);
}
constexpr Point3 operator*(float f, const Point3& p)
{
	return p * f;
}

/*! \remarks Returns an Point3 offset by (f,f,f). */
constexpr Point3 operator+(const Point3& p, float f)
{
	return Point3(p.x + f, p.y + f, p.z + f);
}
constexpr Point3 operator+(float f, const Point3& p)
{
	return p + f;
}

/*! \remarks Returns an Point3 offset by (-f,-f,-f). */
constexpr Point3 operator-(const Point3& p, float f)
{
	return p + -f;
}

/*! \remarks Returns an Point3 whose members are divided by a scalar.*/
inline Point3 operator/(const Point3& p, float f)
{
	assert(f != 0.f);
	return p * (1.f / f);
}

/*! \remarks Normalizes p using float precision */
inline Point3 FNormalize(const Point3& p)
{
	return p.FNormalize();
}
/*! \remarks Normalizes p using double precision */
inline Point3 Normalize(const Point3& p)
{
	return p.Normalize();
}


// These typedefs must be the same as each other, since
// vertex colors are contained in a MeshMap.
typedef Point3 UVVert;
typedef Point3 VertColor;

// RB: moved this here from object.h
/*! \sa  Class Point3.\n\n
\par Description:
This class describes a vector in space using an origin point <b>p</b>, and a
unit direction vector <b>dir</b>.
\par Data Members:
<b>Point3 p;</b>\n\n
Point of origin.\n\n
<b>Point3 dir;</b>\n\n
Unit direction vector. */
class Ray
{
public:
	constexpr Ray() = default;
	constexpr Ray(const Point3& p_p, const Point3& p_dir) : p(p_p), dir(p_dir) {}

	Point3 p; // point of origin
	Point3 dir; // unit vector
};
