//**************************************************************************/
// Copyright (c) 2020 Autodesk, Inc.
// All rights reserved.
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Double matrix3 class.
// AUTHOR: Autodesk Inc.
//**************************************************************************/
#pragma once
#include "matrix3.h"
#include "DPoint3.h"

#include <cstdint>

/**
 * MAX_SILENCE_DEPRECATED_MATRIX_BOOL_CTOR
 * Silences warning when using the Matrix(bool) constructor.
 * Matrix3 and DMatrix3 are now created as identity by default!
 */
#if defined(MAX_SILENCE_DEPRECATED_MATRIX_BOOL_CTOR)
#define MAX_DEPRECATE_MATRIX_BOOL_CTOR(str)
#else
#define MAX_DEPRECATE_MATRIX_BOOL_CTOR(str) [[deprecated(str)]]
#endif

typedef double DMRow[3];

class Quat;
class AngAxis;

enum IOResult : int;
class ISave;
class ILoad;


/*! \sa Class DPoint3, Class Matrix3, Class Quat,  Class AngAxis, Structure
AffineParts, Class BigMatrix.\n\n
Description:
This class implements a 4x3 3D transformation matrix object. Methods are
provided to zero the matrix, set it to the identity, compute its inverse, apply
incremental translation, rotation and scaling, and build new X, Y and Z
rotation matrices. Operators are provided for matrix addition, subtraction, and
multiplication. All methods are implemented by the system.\n\n
Note: In 3ds Max, all vectors are assumed to be row vectors. Under this
assumption, multiplication of a vector with a matrix can be written either way
(Matrix*Vector or Vector*Matrix), for ease of use, and the result is the same
-- the (row) vector transformed by the matrix.
Data Members:
private:\n\n
double m[4][3];\n\n
Matrix storage.\n\n
uint32_t flags;\n\n
Matrix Identity Flags.\n\n
POS_IDENT\n\n
Indicates the translation row of the matrix is the identity.\n\n
ROT_IDENT\n\n
Indicates the rotation elements of the matrix are the identity.\n\n
SCL_IDENT\n\n
Indicates the scale elements of the matrix are the identity.\n\n
MAT_IDENT\n\n
Indicates the matrix is the identity matrix. This is equivalent to
(POS_IDENT|ROT_IDENT|SCL_IDENT).\n\n
*/
class DMatrix3
{

	// Warning - instances of this class are saved as a binary blob to scene file
	// Adding/removing members will break file i/o

	friend DMatrix3 GEOMEXPORT RotateXMatrix(double angle);
	friend DMatrix3 GEOMEXPORT RotateYMatrix(double angle);
	friend DMatrix3 GEOMEXPORT RotateZMatrix(double angle);
	friend DMatrix3 GEOMEXPORT TransMatrix(const DPoint3& p);
	friend DMatrix3 GEOMEXPORT ScaleMatrix(const DPoint3& s);
	friend DMatrix3 GEOMEXPORT RotateYPRMatrix(double yaw, double pitch, double roll);
	friend DMatrix3 GEOMEXPORT RotAngleAxisMatrix(const DPoint3& axis, double angle);
	friend DMatrix3 GEOMEXPORT Inverse(const DMatrix3& m);
	friend DPoint3 GEOMEXPORT operator*(const DMatrix3& a, const DPoint3& v);
	friend DPoint3 GEOMEXPORT operator*(const DPoint3& v, const DMatrix3& a);
	friend DPoint3 GEOMEXPORT VectorTransform(const DMatrix3& m, const DPoint3& v);
	friend DMatrix3 GEOMEXPORT XFormMat(const DMatrix3& xm, const DMatrix3& m);
	friend DPoint3 GEOMEXPORT VectorTransform(const DPoint3& v, const DMatrix3& m);
	friend void GEOMEXPORT MatrixMultiply(DMatrix3& outMatrix, const DMatrix3& matrixA, const DMatrix3& matrixB);
	friend void GEOMEXPORT Inverse(DMatrix3& outMatrix, const DMatrix3& m);
	friend class Quat;

	/*! Internal: storage for the DMatrix3 */
	double m[4][3];

	/*! Internal: Flags to give hints of the state of the DMatrix3 */
	uint32_t flags;

	// Access i-th row as DPoint3 for read or assignment:
	DPoint3& operator[](int i);

public:
	/*!  Constructor. Matrix is initialized to Identity. */
	GEOMEXPORT DMatrix3();

	GEOMEXPORT DMatrix3(const DMatrix3&) = default;
	GEOMEXPORT DMatrix3(DMatrix3&&) = default;
	GEOMEXPORT DMatrix3& operator=(const DMatrix3&) = default;
	GEOMEXPORT DMatrix3& operator=(DMatrix3&&) = default;

	/*!  Returns a reference to the 'i-th' DPoint3 of the matrix. */
	GEOMEXPORT const DPoint3& operator[](int i) const;

	/*!  This clears the MAT_IDENT flag to indicate the matrix
	is not the identity. Non-const <b>GetAddr()</b> calls this for you. */
	GEOMEXPORT void SetNotIdent();
	/*!  This sets the specified identity flag(s).
	\param f    Specifies the identity flag bit(s) to set. See Matrix Identity Flags above.
	*/
	GEOMEXPORT void SetIdentFlags(uint32_t f);
	/*!  Returns the identity flags. */
	GEOMEXPORT uint32_t GetIdentFlags() const;
	/*!  Clears the specified identity flag(s). See Matrix Identity
	Flags above.
	\param f    Specifies the identity flag bit(s) to clear. */
	GEOMEXPORT void ClearIdentFlag(uint32_t f);
	/*!  Returns TRUE if the matrix is the identity matrix (based on
	the flags); otherwise FALSE. */
	GEOMEXPORT bool IsIdentity() const;
	/*!  This method may be used to recompute the *_IDENT flags for this
	matrix. For instance, if you call a method, such as
	INode::GetObjTMAfterWSM(), and it returns a matrix, you cannot use
	the IsIdentity() method to check if the matrix is indeed the
	identity. This is because the flags that method checks are not initialized
	by the INode method. What you can do however is call this method first. This
	will validate the flags in the matrix so they accuratly reflect the
	properties of the matrix. If after calling this method, and then calling
	IsIdentity(), the proper result would be returned. */
	GEOMEXPORT void ValidateFlags();

	/*! \remarks Returns the address of this DMatrix3.\n\n
	The DMatrix3 class keeps flags indicating identity for rotation, scale,
	position, and the matrix as a whole, and thus the direct access via the []
	operator is restricted to prevent developers from modifying the matrix
	without updating the flags. This method, GetAddr(), still lets you get at
	the matrix itself and then you can use the [] operator on the result.\n
	Note: <b>GetAddr()</b> always calls <b>SetNotIdent()</b>, which clears all 
	IDENT flags. If you want to re-enable some optimizations, you can use 
	<b>SetIdentFlags()</b> to mark identity components after this call.\n\n
	Also Note: typedef double DMRow[3];
	\return  The address of the DMatrix3. */
	GEOMEXPORT DMRow* GetAddr();
	/*!  Returns the address of this DMatrix3.\n\n
	This provides read-only access to the DMatrix3 elements as a
	2 dimentional array.\n\n
	Also Note: typedef double DMRow[3]; */
	GEOMEXPORT const DMRow* GetAddr() const;

	/*!  Constructor, Unused.
	The default constructor now initializes the matrix to identity.
	Kept for backward compatibility.
	*/
	MAX_DEPRECATE_MATRIX_BOOL_CTOR(
		"Matrix3 and DMatrix3 are now initialized to identity by default.\n"
		"No need to use the DMatrix3(bool) constructor anymore.\n"
		"Define MAX_SILENCE_DEPRECATED_MATRIX_BOOL_CTOR to silence this warning."
	)
	GEOMEXPORT DMatrix3(bool);

	/*!  Constructor. Initializes the matrix with the row data passed and validates
	the matrix flags.
	\param  row0     The data for row 0.
	\param  row1     The data for row 1.
	\param  row2     The data for row 2.
	\param  row3     The data for row 3. */
	GEOMEXPORT DMatrix3(const DPoint3& row0, const DPoint3& row1, const DPoint3& row2, const DPoint3& row3);

	/*!  Initializes the matrix with the row data passed and validates the matrix
	flags.
	\param  row0     The data for row 0.
	\param  row1     The data for row 1.
	\param  row2     The data for row 2.
	\param  row3     The data for row 3.
	\return  A reference to this matrix. */
	GEOMEXPORT DMatrix3& Set(const DPoint3& row0, const DPoint3& row1, const DPoint3& row2, const DPoint3& row3);

	/*! An global instance of DMatrix3 set to the identity.  An identity
		matrix has no rotation, scale or translation on it.  In other words,
		it is a matrix that has no effect when multiplied with another matrix.\n
		the structure of the Matrix is as follows: \n\n
		[1, 0, 0]\n
		[0, 1, 0]\n
		[0, 0, 1]\n
		[0, 0, 0] */
	GEOMEXPORT static const DMatrix3 Identity;

	/*!  Compares the elements of this matrix and the one specified element by
	element for exact equality. Returns nonzero if they are equal; otherwise
	zero.
	\param  m     The matrix to compare against. */
	GEOMEXPORT int operator==(const DMatrix3& m) const;

	/*!  Compares the elements of this matrix and the one specified element by
		element for inequality. Returns false iff they are unequal.
	\param  mat     The matrix to compare against. */
	GEOMEXPORT bool operator!=(const DMatrix3& mat) const;

	/*! 	 Compares the elements of this matrix and the one specified element by element
	for equality within the specified tolerance epsilon. Returns nonzero if
	they are 'equal'; otherwise zero.
	\param  m The matrix to compare against.
	\param epsilon The tolerance for comparison. If the values in the matrix are within this value
	(+ epsilon or - epsilon) they are considered equal. */
	GEOMEXPORT int Equals(const DMatrix3& m, double epsilon = 1E-12) const;

	/*!  Subtracts a DMatrix3 from this DMatrix3. */
	GEOMEXPORT DMatrix3& operator-=(const DMatrix3& m);
	/*!  Adds a DMatrix3 to this DMatrix3. */
	GEOMEXPORT DMatrix3& operator+=(const DMatrix3& m);
	/*!  Multiplies this DMatrix3 by the specified DMatrix3 (*this =
	(*this)*M;). */
	/*! Multiply this matrix on the right by Matrix m.
		\code
			DMatrix3 tm1, tm2;
			DMatrix3 tm3 = tm1 * tm2;	// Is equivalent to the below
			tm1 *= tm2;					// tm1 now equals tm3
		\endcode
		\param m The matrix multiplied to the right of this matrix
		\return A reference to this matrix */
	GEOMEXPORT DMatrix3& operator*=(const DMatrix3& m);
	/*!  Multiplies each element of this DMatrix3 by a double.
		 \code
			 DMatrix3 tm1(1), tm2(2);
			 tm1 *= 0.7f;	// This multiplies all elements by 0.7f
			 tm2.Scale(DPoint3(0.7f, 0.7f, 0.7f), TRUE); // This multiplies all elements by 0.7f
			 tm1 == tm2; // is true
		 \endcode
		 \param a The scale to apply to each element of this matrix
		 \return a reference to this matrix */
	GEOMEXPORT DMatrix3& operator*=(double a);

	/*!  Set this matrix to the Identity Matrix. */
	GEOMEXPORT void IdentityMatrix();
	/*!  This method sets all elements of the matrix to 0.0f */
	GEOMEXPORT void Zero();

	/*!  Returns the specified row of this matrix.
	\param i    Specifies the row to retrieve. */
	GEOMEXPORT DPoint3 GetRow(int i) const;

	/*!  Sets the specified row of this matrix to the specified values.
	\param i    Specifies the row to set.
	\param p    The values to set. */
	GEOMEXPORT void SetRow(int i, DPoint3 p);
	/*!  Returns the 'i-th' column of this matrix.
	\param i    Specifies the column to get (0-2). */
	GEOMEXPORT Point4 GetColumn(int i) const;
	/*!  Sets the 'i-th' column of this matrix to the specified values.
	Note : Clears the matrix flags.
	\param i    Specifies the column to set (0-2).
	\param col    The values to set. */
	GEOMEXPORT void SetColumn(int i, Point4 col);
	/*!  Returns the upper three entries in the specified column.
	\param i    Specifies the partial column to get (0-2). */
	GEOMEXPORT DPoint3 GetColumn3(int i) const;

	/*!  This method zeros the translation portion of this matrix. */
	GEOMEXPORT void NoTrans();
	/*!  This method zeros the rotation and scale portion of this matrix. */
	GEOMEXPORT void NoRot();
	/*!  This method zeros the scale portion of this matrix without
	orthogonalization. If the matrix was sheared (skewed) then this method is not
	able to remove the scale component completely. In that case, use the Orthogonalize()
	method instead to remove the scale component entirely. Read the SCL_IDENT flag
	to check whether the NoScale() method was enough to make the matrix to be orthogonal
	(with perpendicular axes of unit length). */
	GEOMEXPORT void NoScale();

	/*! Ortho-normalize the matrix.  This ensures that each axis of the basis is
	of length 1 and at right angles to the other. This is an "unbiased"
	orthogonalization, which means that no single axis is used as the basis
	for the other axis, and all axis will be modified equally.\n
	\note This is an iterative process, and should not be used in high-performance
	situations.  It seems to take a maximum of 4 iterations to converge. */
	GEOMEXPORT void Orthogonalize();

	/*!  Sets the translation row of this matrix to the specified
	values. The POS_IDENT flag is cleared.
	\param p    Specifies the values for the translation row. */
	GEOMEXPORT void SetTrans(const DPoint3& p);
	/*!  Sets the specified component of the translation row of this
	matrix to the specified value. The POS_IDENT flag is cleared.
	\param i    Specifies the component of the translation row of this matrix to set.
	\param v    The value to set. */
	GEOMEXPORT void SetTrans(int i, double v);
	/*!  Returns the translation row of this matrix.
	\return  The translation row of this matrix. */
	GEOMEXPORT const DPoint3& GetTrans() const;

	/*!  Apply an incremental translation transformation to this
	matrix. This is equivalent to multiplying on the RIGHT by the transform.
	\param  p    Specifies the translation. */
	GEOMEXPORT void Translate(const DPoint3& p);
	/*!  Apply an incremental X rotation transformation to this matrix.
	This is equivalent to multiplying on the RIGHT by the transform.
	\param angle    Specifies the X rotation in radians. */
	GEOMEXPORT void RotateX(double angle);
	/*!  Apply an incremental Y rotation transformation to this matrix.
	This is equivalent to multiplying on the RIGHT by the transform.
	\param angle    Specifies the Y rotation in radians. */
	GEOMEXPORT void RotateY(double angle);
	/*!  Apply an incremental Z rotation transformation to this matrix.
	This is equivalent to multiplying on the RIGHT by the transform.
	\param angle    Specifies the Z rotation in radians. */
	GEOMEXPORT void RotateZ(double angle);
	/*!  Apply an incremental scaling transformation to this matrix.
	This is equivalent to multiplying on the RIGHT by the transform.
	\param s The scale values.
	\param trans If set to TRUE, the translation component is scaled. If trans
	= FALSE the translation component is unaffected. When 3ds Max was
	originally written there was a bug in the code for this method where the
	translation portion of the matrix was not being scaled. This meant that when
	a matrix was scaled the bottom row was not scaled. Thus it would always
	scale about the local origin of the object, but it would scale the world
	axes. When this bug was discovered, dependencies existed in the code upon
	this bug. Thus it could not simply be fixed because it would break the
	existing code that depended upon it working the incorrect way. To correct
	this the trans parameter was added. If this is set to TRUE,
	the translation component will be scaled correctly. The existing plug-ins
	don't use this parameter, it defaults to FALSE, and the code behaves
	the old way. */
	GEOMEXPORT void Scale(const DPoint3& s, bool trans = false);

	/*!  Apply an incremental translation transformation to this
	matrix. This is equivalent to multiplying on the LEFT by the transform.
	param  p    Specifies the translation distance. */
	GEOMEXPORT void PreTranslate(const DPoint3& p);
	/*!  Apply an incremental X rotation transformation to this matrix.
	This is equivalent to multiplying on the LEFT by the transform.
	\param angle    Specifies the X rotation in radians. */
	GEOMEXPORT void PreRotateX(double angle);
	/*!  Apply an incremental Y rotation transformation to this matrix.
	This is equivalent to multiplying on the LEFT by the transform.
	\param angle    Specifies the Y rotation in radians. */
	GEOMEXPORT void PreRotateY(double angle);
	/*!  Apply an incremental Z rotation transformation to this matrix.
	This is equivalent to multiplying on the LEFT by the transform.
	\param angle    Specifies the Z rotation in radians. */
	GEOMEXPORT void PreRotateZ(double angle);
	/*!  Apply an incremental scaling transformation to this matrix.
	This is equivalent to multiplying on the LEFT by the transform.
	\param s The scale values.
	The translation component is unaffected. */
	GEOMEXPORT void PreScale(const DPoint3& s);
	/*!  Sets this matrix to the identity and the translation components to the
	specified values.
	\param  p     The translation values to store. */
	GEOMEXPORT void SetTranslate(const DPoint3& p); // makes translation matrix
	/*!  Sets this matrix to the identity and the rotation components to the
	specified X rotation.
	\param angle     The angle for X rotation (in radians). */
	GEOMEXPORT void SetRotateX(double angle); // makes rotation matrix
	/*!  Sets this matrix to the identity and the rotation components to the
	specified Y rotation.
	\param angle     The angle for Y rotation (in radians). */
	GEOMEXPORT void SetRotateY(double angle);
	/*!  Sets this matrix to the identity and the rotation components to the
	specified Z rotation.
	\param angle     The angle for Z rotation (in radians). */
	GEOMEXPORT void SetRotateZ(double angle);
	/*!  Sets the rotation components of the matrix as specified by the quaternion.
	The translation and scale components will match the identity matrix.
	\param  q     Specifies the rotation to use for the matrix. */
	GEOMEXPORT void SetRotate(const Quat& q);
	/*!  Sets the rotation components of the matrix as specified by the AngAxis. The
	translation and scale components will match the identity matrix.
	\param  aa     Specifies the rotation to use for the matrix. */
	GEOMEXPORT void SetRotate(const AngAxis& aa);
	/*!  Sets the rotation components of this matrix using yaw, pitch and roll
	angles. There are many different conventions for specifying a rotation by
	means of three Euler angles. This function uses the convention of rotating
	around the world Z axis, then the X axis, then the Y axis; the three
	arguments are given in the order Y, X, Z.\n\n
	This one is equivalent to:\n\n
	 M.IdentityMatrix();\n\n
	 M.RotateZ(roll);\n\n
	 M.RotateX(pitch);\n\n
	 M.RotateY(yaw);\n\n
	--Which presupposes Y is vertical, X is sideways, Z is forward
	\param yaw     The yaw angle in radians.
	\param pitch     The pitch angle in radians.
	\param roll     The roll angle in radians. */
	GEOMEXPORT void SetRotate(double yaw, double pitch, double roll);
	/*! 	 Sets the rotation portion of the matrix to the rotation specified by the angle
	and axis and sets the translation portion to zeros.
	\param  axis 	 The axis of rotation.
	\param angle 	 The angle of rotation about the axis in radians. */
	GEOMEXPORT void SetAngleAxis(const DPoint3& axis, double angle);
	/*!  Sets the scale components of this matrix to the specified values. The other
	components to this matrix will match the identity.
	\param  s     The scale factors for the matrix. */
	GEOMEXPORT void SetScale(const DPoint3& s); // makes scale matrix
	/*!  This creates a matrix describing a viewpoint which is at the 'from'
	location, looking toward the 'to' location; the viewpoint is tilted so that
	the 'up' vector points to the top of the view.
	\param  from     This specifies the viewpoint source location.
	\param  to     This vector specifies the direction of view.
	\param  up     This vector points to the top of the view. */
	GEOMEXPORT void SetFromToUp(const DPoint3& from, const DPoint3& to, const DPoint3& up);
	/*!  This method performs an in-place inversion on this matrix.
	An inverted matrix, when multiplied by the original, yields the identity. */
	GEOMEXPORT void Invert();

	/*!  Perform matrix multiplication. */
	GEOMEXPORT DMatrix3 operator*(const DMatrix3&)const;
	/*!  Perform matrix addition. */
	GEOMEXPORT DMatrix3 operator+(const DMatrix3&) const;
	/*!  Perform matrix subtraction. */
	GEOMEXPORT DMatrix3 operator-(const DMatrix3&) const;

	/*!  Returns the specified point transformed by this matrix.
	\param  p     The point to transform by this matrix. */
	GEOMEXPORT DPoint3 PointTransform(const DPoint3& p) const;

	/*!  Returns the specified vector transformed by this matrix.
	\param  p     The vector to transform by this matrix. */
	GEOMEXPORT DPoint3 VectorTransform(const DPoint3& p) const;

	/*! 	 Transforms the specified list of points with this matrix.
	 \param array The array of points to transform with this matrix.
	 \param n 	 The number of points in the array.
	 \param stride The size of the increment used when moving to the next point. If you wish to
	 transform an array of data objects which contain x, y, and z coordinates in
	 order (such as a Point4, or a structure containing a DPoint3 as a
	 member) you can specify a 'stride' value (for instance
	 sizeof(data_object)). */
	GEOMEXPORT void TransformPoints(DPoint3* array, int n, int stride = sizeof(DPoint3));

	/*! 	 Transforms the specified list of points with this matrix and stores the
	 resulting transformed points in the storage passed.
	 \param array 	 The array of points to transform (the source).
	 \param to The array to store the transformed points (the destination).
	 \param n 	 The number of points in the source array.
	 \param stride The size increment used when moving to the next source location.
	 \param strideTo The size increment used when moving to the next storage location. */
	GEOMEXPORT void TransformPoints(
			const DPoint3* array, DPoint3* to, int n, int stride = sizeof(DPoint3), int strideTo = sizeof(DPoint3));

	/*! 	 Transforms the specified list of vectors with this matrix.
	 \param array The array of vectors to transform with this matrix.
	 \param n The number of vectors in the array.
	 \param stride The size of the increment used when moving to the next vector. If you wish to
	 transform an array of data objects which contain x, y, and z coordinates in
	 order (such as a Point4, or a structure containing a DPoint3 as a
	 member) you can specify a 'stride' value (for instance
	 sizeof(data_object)). */
	GEOMEXPORT void TransformVectors(DPoint3* array, int n, int stride = sizeof(DPoint3));

	/*! 	 Transforms the specified list of vectors with this matrix and stores the
	 resulting transformed vectors in the storage passed.
	 \param *array 	 The array of vectors to transform (the source).
	 \param to The array to store the transformed vectors (the destination).
	 \param n 	 The number of vectors in the source array.
	 \param stride The size increment used when moving to the next source location.
	 \param strideTo  The size increment used when moving to the next storage location. */
	GEOMEXPORT void TransformVectors(
			const DPoint3* array, DPoint3* to, int n, int stride = sizeof(DPoint3), int strideTo = sizeof(DPoint3));

	/*!  Retrieves the yaw, pitch and roll angles represented by the rotation in
	this matrix.
	\param yaw     The yaw rotation angle is stored here (in radians).
	\param pitch     The pitch rotation angle is stored here (in radians).
	\param roll     The roll rotation angle is stored here (in radians).    */
	GEOMEXPORT void GetYawPitchRoll(double* yaw, double* pitch, double* roll);

	/*! Save this DMatrix3 to disk
	\param isave The interface responsible for actually saving the data
	\return IO_OK on success, or a failure code
	\remarks Save and Load are only forward declared in this header!
	If you need to use Save or Load, you must link with maxutil, which
	is where these methods are implemented.
	This way, geom.dll is independent from maxsdk. */
	UTILGEOMEXPORT IOResult Save(ISave* isave);

	/*! Load the data for this DMatrix3.
	\param iload The interface responsible for actually loading the data
	\return IO_OK on success, or a failure code
	\remarks Save and Load are only forward declared in this header!
	If you need to use Save or Load, you must link with maxutil, which
	is where these methods are implemented.
	This way, geom.dll is independent from maxsdk. */
	UTILGEOMEXPORT IOResult Load(ILoad* iload);

	/*!  Returns the 'parity' of the matrix. Scaling one axis of the matrix
	  negatively switches the 'parity'. However if you scale two axis the parity will flip back.
	  Scaling three axis switches the parity again.\n\n
	When rendering a mesh, if you scale something along one axis, it turns
	'inside out'. That is the direction when the normals are reversed. This
	method may be used to detect that case and then reverse the normals. The 3ds
	Max renderer does this -- if this method returns TRUE it flips all the
	normals so it won't turn inside out.
	  */
	GEOMEXPORT bool Parity() const;


	/*! Converts to a Matrix3
	\return the concersion to Matrix3 */
	GEOMEXPORT Matrix3 ToMatrix3();

	/*! Copies a matrix3 into the DMatrix3	*/
	GEOMEXPORT void FromMatrix3(const Matrix3& tm);
};

/*!  Builds a new matrix for use as a X rotation transformation.
\param angle Specifies the angle of rotation in radians.
\return  A new X rotation DMatrix3. */
GEOMEXPORT DMatrix3 RotateXMatrix(double angle);

/*!  Builds a new matrix for use as a Y rotation transformation.
\param angle Specifies the angle of rotation in radians.
\return  A new Y rotation DMatrix3. */
GEOMEXPORT DMatrix3 RotateYMatrix(double angle);

/*!  Builds a new matrix for use as a Z rotation transformation.
\param angle Specifies the angle of rotation in radians.
\return  A new Z rotation DMatrix3. */
GEOMEXPORT DMatrix3 RotateZMatrix(double angle);

/*!  Builds a new matrix for use as a translation transformation.
\param  p Specifies the translation values.
\return  A new translation DMatrix3. */
GEOMEXPORT DMatrix3 TransMatrix(const DPoint3& p);

/*!  Builds a new matrix for use as a scale transformation.
\param  s Specifies the scale values.
\return  A new scale DMatrix3. */
GEOMEXPORT DMatrix3 ScaleMatrix(const DPoint3& s);

/*!  Builds a new matrix for use as a rotation transformation by
specifying yaw, pitch and roll angles.\n\n
This definition will depend on what our coordinate system is. This one is
equivalent to:
\code
M.IdentityMatrix();
M.RotateZ(roll);
M.RotateX(pitch);
M.RotateY(yaw);
\endcode
Which presupposes Y is vertical, X is sideways, Z is forward
\param yaw Specifies the yaw angle in radians.
\param pitch Specifies the pitch angle in radians.
\param roll Specifies the roll angle in radians.
\return  A new rotation DMatrix3. */
GEOMEXPORT DMatrix3 RotateYPRMatrix(double yaw, double pitch, double roll);

/*!  Builds a new matrix for use as a rotation transformation by
specifying an angle and axis.
\param axis Specifies the axis of rotation. Note that this angle is expected to be
normalized.
\param angle Specifies the angle of rotation. Note: The direction of the angle in this
method is opposite of that in AngAxisFromQ().
\return  A new rotation DMatrix3. */
GEOMEXPORT DMatrix3 RotAngleAxisMatrix(const DPoint3& axis, double angle);

/*!  Return the inverse of the matrix
\param  m The matrix to compute the inverse of. */
GEOMEXPORT DMatrix3 Inverse(const DMatrix3& m); // return Inverse of matrix

/*!  These transform a DPoint3 with a DMatrix3. These two
versions of transforming a point with a matrix do the same thing, regardless of
the order of operands (linear algebra rules notwithstanding).
\param  a The matrix to transform the point with.
\param  v The point to transform.
\return  The transformed DPoint3. */
GEOMEXPORT DPoint3 operator*(const DMatrix3& a, const DPoint3& v); // Transform Point with matrix

/*!  These transform a DPoint3 with a DMatrix3. These two
versions of transforming a point with a matrix do the same thing, regardless of
the order of operands (linear algebra rules notwithstanding).
\param  v The point to transform.
\param  a The matrix to transform the point with.
\return  The transformed DPoint3. */
GEOMEXPORT DPoint3 operator*(const DPoint3& v, const DMatrix3& a); // Transform Point with matrix

/*!  Transform the vector (DPoint3) with the specified matrix.
\param  m The matrix to transform the vector with.
\param  v The vector to transform.
\return  The transformed vector (as a DPoint3). */
GEOMEXPORT DPoint3 VectorTransform(const DMatrix3& m, const DPoint3& v);
GEOMEXPORT DPoint3 VectorTransform(const DPoint3& v, const DMatrix3& m);

/*! transform a plane.
	\note this only works if M is orthogonal
	\param m The transformation to apply to the plain
	\param plane the plane to be transformed */
GEOMEXPORT Point4 TransformPlane(const DMatrix3& m, const Point4& plane);

/*!  This method is used to build a matrix that constructs a
transformation in a particular space. For example, say you have a rotation you
want to apply, but you want to perform the rotation in another coordinate
system. To do this, you typically transform into the space of the coordinate
system, then apply the transformation, and then transform out of that
coordinate system. This method constructs a matrix that does just this. It
transforms matrix m so it is applied in the space of matrix xm.
It returns a DMatrix3 that is xm*m*Inverse(xm).
\param  xm Specifies the coordinate system you want to work in.
\param  m Specifies the transformation matrix.
\return  Returns a DMatrix3 that is xm*m*Inverse(xm). */
GEOMEXPORT DMatrix3 XFormMat(const DMatrix3& xm, const DMatrix3& m);

/*!  Mirrors the input matrix against the X, Y, or Z axis.
	This function mirrors the input matrix such that if it transformed a DPoint3 p,
	the transform applied by the mirrored transform would place p in an equivalent
	position relative but with one of it's elements negated.
	\code
	DPoint3 p = ...; // Some arbitrary point;
	DMatrix3 tm = ...; // Some matrix
	DMatrix3 tmMirror = tm;
	MirrorMatrix(tm, kXAxis); // Mirror this matrix around the X Axis;
	DPoint3 p3Result = tm.PointTransform(p);			// Apply the original transformation
	DPoint3 p3Mirrored = tmMirror.PointTransform(p);	// Apply the mirrored transformation
	p3Result[kXAxis] == -p3Mirrored[kXAxis];		// The point is mirrored around X
	p3Result[kYAxis] == p3Mirrored[kYAxis];			// The point is unchanged around Y
	p3Result[kZAxis] == p3Mirrored[kZAxis];			// The point is unchanged around Z
	\endcode
	\param[in,out] tm The Matrix to mirror
	\param axis The world plane to mirror around.  The axis will be reflected
			against the plane formed by the point (0, 0, 0) and the normal where
			normal[axis] = 1;
	\param scaleMatrix The matrix can be mirrored either by pure rotation, or by pure scaling.
			If true, the matrix is mirrored by flipping one of its axis, changing the handedness
			of the matrix.  This had the advantage that the mirroring will be passed onto
			all it's children.  This is preferential when the system can handle it, but can cause
			issues in systems that do not compensate for scale, eg game pipelines and skinning.
			if false, the mirroring will be achieved using rotation only and will not affect child local positions.
			It is better to use non-scaling mirroring when dealing with tools or processes that
			do not well support scale, or when the matrix will be blended with non-scaled matrices.
			(eg, Point/Quat based pipelines and skinning) */
GEOMEXPORT void MirrorMatrix(DMatrix3& tm, Axis axis, bool scaleMatrix = false);


/*!  Same as Maxtrix3::operator[]. Perform matrix multiplication without additional matrix copy
	\param  outMatrix The result of matrixA * matrixB
	\param  matrixA First matrix to multiply
	\param  matrixB Second matrix to multiply*/
GEOMEXPORT void MatrixMultiply(DMatrix3& outMatrix, const DMatrix3& matrixA, const DMatrix3& matrixB);

/*!  Same as Maxtrix3::Inverse. Compute the inverse of the matrix without additional matrix copy
	\param  outMatrix The inversed matrix.
	\param  m The matrix to compute the inverse of. */
GEOMEXPORT void Inverse(DMatrix3& outMatrix, const DMatrix3& m);
