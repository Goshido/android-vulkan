// Copyright (c) 2014 Autodesk, Inc.
// All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.

#pragma once

#include "GeomExport.h"
#include "point3.h"

#include <iosfwd>
#include <cassert>

/*! class DPoint3
 Description:
This class describes a 3D point using double precision x, y and z coordinates.
Methods are provided to add and subtract points, multiply and divide by
scalars, and element by element multiply and divide two points. All methods are
implemented by the system.
 Data Members:
double x,y,z;  */
class GEOMEXPORT DPoint3
{
public:
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;

	//! Initializes all vector components to zero.
	DPoint3() = default;
	constexpr DPoint3(const DPoint3&) = default;
	constexpr DPoint3(DPoint3&&) = default;
	DPoint3& operator=(const DPoint3&) = default;
	DPoint3& operator=(DPoint3&&) = default;

	/*!  Constructor. x, y, and z are initialized to the values specified. */
	constexpr DPoint3(double X, double Y, double Z) : x(X), y(Y), z(Z) {}
	/*!  Constructor. x, y, and z are initialized to. af[0], af[1],
	and af[2] respectively. */
	constexpr DPoint3(const double af[3]) : x(af[0]), y(af[1]), z(af[2]) {}

	constexpr DPoint3(const Point3& p) : x(p.x), y(p.y), z(p.z) {}
	constexpr DPoint3& operator=(const Point3& p)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		return *this;
	}

	static const DPoint3 Origin; /*!  const for (0,0,0) */
	static const DPoint3 XAxis; /*!  const for (1,0,0) */
	static const DPoint3 YAxis; /*!  const for (0,1,0) */
	static const DPoint3 ZAxis; /*!  const for (0,0,1) */

	// Access operators
	/*!  Allows access to x, y and z using the subscript operator.
	\return  An index of 0 will return x, 1 will return y, 2 will return z. */
	double& operator[](int i)
	{
		assert((i >= 0) && (i <= 2));
		return (&x)[i];
	}
	/*!  Allows access to x, y and z using the subscript operator.
	\return  An index of 0 will return x, 1 will return y, 2 will return z. */
	const double& operator[](int i) const
	{
		assert((i >= 0) && (i <= 2));
		return (&x)[i];
	}

	// Conversion function
	/*!  Conversion function. Returns the address of the DPoint3.x */
	constexpr operator double*()
	{
		return &x;
	}
	constexpr operator const double*() const
	{
		return &x;
	}
	/*! Convert DPoint3 to Point3 */
	constexpr operator Point3() const
	{
		return Point3(x, y, z);
	}

	// Unary operators
	/*!  Unary - operator. Negates both x, y and z. */
	constexpr DPoint3 operator-() const
	{
		return DPoint3(-x, -y, -z);
	}
	/*!  Unary +. Returns the point unaltered. */
	constexpr DPoint3 operator+() const
	{
		return *this;
	}

	// Assignment operators
	/*!  Subtracts a DPoint3 from this DPoint3. */
	constexpr DPoint3& operator-=(const DPoint3& p)
	{
		x -= p.x;
		y -= p.y;
		z -= p.z;
		return *this;
	}
	/*!  Adds a DPoint3 to this DPoint3. */
	constexpr DPoint3& operator+=(const DPoint3& p)
	{
		x += p.x;
		y += p.y;
		z += p.z;
		return *this;
	}
	//! Member-wise multiplication of two vectors
	constexpr DPoint3& operator*=(const DPoint3& p)
	{
		x *= p.x;
		y *= p.y;
		z *= p.z;
		return *this;
	}
	//! Member-wise, in-place division of this vector
	DPoint3& operator/=(const DPoint3& p)
	{
		assert(p.x != 0 && p.y != 0 && p.z != 0);
		x /= p.x;
		y /= p.y;
		z /= p.z;
		return *this;
	}

	//! \remarks Adds floating point value to this DPoint3.
	constexpr DPoint3& operator+=(double f)
	{
		x += f;
		y += f;
		z += f;
		return *this;
	}
	//! \remarks Subtracts floating point value from this DPoint3.
	constexpr DPoint3& operator-=(double f)
	{
		return *this += -f;
	}
	/*!  Each element of this DPoint3 is multiplied by the specified
	double. */
	constexpr DPoint3& operator*=(double f)
	{
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}
	/*!  Each element of this DPoint3 is divided by the specified
	double. */
	DPoint3& operator/=(double f)
	{
		assert(f != 0);
		return *this *= (1.0 / f);
	}

	// Binary operators
	/*!  Subtracts a DPoint3 from a DPoint3. */
	constexpr DPoint3 operator-(const DPoint3& p) const
	{
		return DPoint3(x - p.x, y - p.y, z - p.z);
	}
	/*!  Adds a DPoint3 to a DPoint3. */
	constexpr DPoint3 operator+(const DPoint3& p) const
	{
		return Point3(x + p.x, y + p.y, z + p.z);
	}
	/*! \remarks Divides a DPoint3 by a DPoint3 element by element. */
	DPoint3 operator/(const DPoint3& p) const
	{
		assert(p.x != 0 && p.y != 0 && p.z != 0);
		return DPoint3(x / p.x, y / p.y, z / p.z);
	}
	//! Member-wise multiplication of two vectors: (x*x, y*y, z*z)
	DPoint3 operator*(const DPoint3& p) const
	{
		return DPoint3(x * p.x, y * p.y, z * p.z);
	}

	/*!  The 'Length' squared of this point. */
	constexpr double LengthSquared() const
	{
		return x * x + y * y + z * z;
	}
	/*!  Returns the 'Length' of this point (vector)*/
	double Length() const
	{
		return sqrt(LengthSquared());
	}

	/*! Returns unit vector in the same direction as this point*/
	DPoint3 Normalize() const;
	/*!  in place normalize */
	DPoint3& Unify();
	double LengthUnify(); // returns old Length

	/*! \return The largest axis */
	int MaxComponent() const;
	/*! \return The smallest axis */
	int MinComponent() const;


	// Test for equality
	/*!  Equality operator. Test for equality between two DPoint3's.
	\return  true if the DPoint3's are equal. */
	constexpr bool operator==(const DPoint3& p) const
	{
		return (p.x == x) && (p.y == y) && (p.z == z);
	}
	/*!  Equality operator. Test for nonequality between two DPoint3's.
	\return  true if the DPoint3's are not equal. */
	constexpr bool operator!=(const DPoint3& p) const
	{
		return !(*this == p);
	}
	/*! \remarks Returns true if the absolute difference between point components
	is less or equal to the given epsilon for each component*/
	bool Equals(const DPoint3& p, double epsilon = 1E-6) const;

	/*! \remarks The dot product of two DPoint3's (vectors).*/
	constexpr double DotProd(const DPoint3& p) const
	{
		return x * p.x + y * p.y + z * p.z;
	}
	constexpr double operator%(const DPoint3& p) const
	{
		return DotProd(p);
	}

	/*!  Computes the cross product of this DPoint3 and the specified DPoint3. */
	constexpr DPoint3 CrossProd(const DPoint3& p) const
	{
		return DPoint3(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x);
	}
	constexpr DPoint3 operator^(const DPoint3& p) const
	{
		return CrossProd(p);
	}
};

/*! \remarks Returns the component with the maximum abs value. 0=x, 1=y, 2=z. */
inline int MaxComponent(const DPoint3& p)
{
	return p.MaxComponent();
}
/*! \remarks Returns the component with the minimum abs value. 0=x, 1=y, 2=z. */
inline int MinComponent(const DPoint3& p)
{
	return p.MinComponent();
}

/*! \remarks Returns the length of the DPoint3 */
inline double Length(const DPoint3& p)
{
	return p.Length();
}
/*! \remarks Returns the length squared of the DPoint3 */
constexpr double LengthSquared(const DPoint3& p)
{
	return p.LengthSquared();
}

/*! \remarks Returns dot product of two DPoint3 */
constexpr double DotProd(const DPoint3& a, const DPoint3& b)
{
	return a % b;
}

/*! \remarks Returns cross product of two DPoint3 */
constexpr DPoint3 CrossProd(const DPoint3& a, const DPoint3& b)
{
	return a ^ b;
}

/*! \remarks Returns a DPoint3 multiplied by a scalar. */
constexpr DPoint3 operator*(const DPoint3& p, double f)
{
	return DPoint3(p.x * f, p.y * f, p.z * f);
}
constexpr DPoint3 operator*(double f, const DPoint3& p)
{
	return p * f;
}

/*! \remarks Returns a DPoint3 offset by (f,f,f). */
constexpr DPoint3 operator+(const DPoint3& p, double f)
{
	return DPoint3(p.x + f, p.y + f, p.z + f);
}
constexpr DPoint3 operator+(double f, const DPoint3& p)
{
	return p + f;
}

/*! \remarks Returns a DPoint3 offset by (-f,-f,-f). */
constexpr DPoint3 operator-(const DPoint3& p, double f)
{
	return p + -f;
}

/*! \remarks Returns a DPoint3 whose members are divided by a scalar.*/
inline DPoint3 operator/(const DPoint3& p, double f)
{
	assert(f != 0.0);
	return p * (1.0 / f);
}

/*! \remarks Normalizes p using double precision */
inline DPoint3 Normalize(const DPoint3& p)
{
	return p.Normalize();
}

// Helper for converting DPoint3 to Point3
constexpr Point3 Point3FromDPoint3(const DPoint3& from)
{
	return Point3(from.x, from.y, from.z);
}

/*! \sa  Class DRay.\n\n
 Description:
This class describes a vector in space using an origin point p, and a
unit direction vector in double precisiondir.
 Data Members:
DPoint3 p;\n\n
Point of origin.\n\n
DPoint3 dir;\n\n
Unit direction vector. */
class DRay
{
public:
	DPoint3 p; // point of origin
	DPoint3 dir; // unit vector
};
