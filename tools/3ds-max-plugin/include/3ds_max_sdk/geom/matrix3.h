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
// DESCRIPTION: Matrix3 class.
// AUTHOR: Autodesk Inc.
//**************************************************************************/
#pragma once
#include "GeomExport.h"
#include "point3.h"
#include "point4.h"

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


// Flags
#define POS_IDENT 1
#define ROT_IDENT 2
#define SCL_IDENT 4
#define MAT_IDENT (POS_IDENT | ROT_IDENT | SCL_IDENT)

typedef float MRow[3];

class Quat;
class AngAxis;

class Matrix3;
enum IOResult : int;
class ISave;
class ILoad;

/*! This enum can be used to specify axis under a variety of scenarios.  For example, passing kXAxis to GetRow
	returns the vector that represents the X axis of the transform, and accessing a point3 the enum returns
	the X, Y, or Z component of the translation.*/
enum Axis
{
	kXAxis, /// Specifies the X Axis.
	kYAxis, /// Specifies the Y Axis.
	kZAxis, /// Specifies the Z Axis.
};

// We keep kTrans separate, so that functions that don't support a translation
// component can specify kAxis, while functions that support all 4 elements, can
// specify kMatrixComponent
/*! The TransComponent enum simply specifies the translation row index of a Matrix3.
	The enum is used (along with Axis enum) to construct the MatrixComponent pseudo-enum
	via InheritEnum.  This pseudo-enum can be used in places where programmers need to
	access the components of the matrix directly (for example, in the functions GetRow
	or directly in the Matrix3 internals via GetAddr). */
enum TransComponent
{
	kTrans = kZAxis + 1 /// kTrans specifies the row index of the translation portion for a Matrix3
};

/*! \sa  Class Point3, ~{ %Matrix Representations of 3D Transformations }~, Class Quat,  Class AngAxis, Structure
AffineParts, Class BigMatrix.\n\n
\par Description:
This class implements a 4x3 3D transformation matrix object. Methods
are provided to zero the matrix, set it to the identity, compute its inverse, apply incremental translation, rotation
and scaling, and build new X, Y and Z rotation matrices. Operators are provided for matrix addition, subtraction, and
multiplication. All methods are implemented by the system.\n\n
Note: In 3ds Max, all vectors are assumed to be row vectors. Under this
assumption, multiplication of a vector with a matrix can be written either way
(Matrix*Vector or Vector*Matrix), for ease of use, and the result is the same
-- the (row) vector transformed by the matrix.
\par Data Members:
private:\n\n
<b>float m[4][3];</b>\n\n
Matrix storage.\n\n
<b>uint32_t flags;</b>\n\n
Matrix Identity Flags.\n\n
<b>POS_IDENT</b>\n\n
Indicates the translation row of the matrix is the identity.\n\n
<b>ROT_IDENT</b>\n\n
Indicates the rotation elements of the matrix are the identity.\n\n
<b>SCL_IDENT</b>\n\n
Indicates the scale elements of the matrix are the identity.\n\n
<b>MAT_IDENT</b>\n\n
Indicates the matrix is the identity matrix. This is equivalent to
<b>(POS_IDENT|ROT_IDENT|SCL_IDENT)</b>.\n\n */
class Matrix3
{

	// Warning - instances of this class are saved as a binary blob to scene file
	// Adding/removing members will break file i/o

	friend Matrix3 GEOMEXPORT RotateXMatrix(float angle);
	friend Matrix3 GEOMEXPORT RotateYMatrix(float angle);
	friend Matrix3 GEOMEXPORT RotateZMatrix(float angle);
	friend Matrix3 GEOMEXPORT TransMatrix(const Point3& p);
	friend Matrix3 GEOMEXPORT ScaleMatrix(const Point3& s);
	friend Matrix3 GEOMEXPORT RotateYPRMatrix(float Yaw, float Pitch, float Roll);
	friend Matrix3 GEOMEXPORT RotAngleAxisMatrix(const Point3& axis, float angle);
	friend Matrix3 GEOMEXPORT Inverse(const Matrix3& M);
	friend Matrix3 GEOMEXPORT PseudoInverse(const Matrix3& m);
	friend Matrix3 GEOMEXPORT InverseHighPrecision(const Matrix3& M);
	friend Matrix3 GEOMEXPORT AffineTranspose(const Matrix3& M);
	friend Point3 GEOMEXPORT operator*(const Matrix3& A, const Point3& V);
	friend Point3 GEOMEXPORT operator*(const Point3& V, const Matrix3& A);
	friend Point3 GEOMEXPORT VectorTransform(const Matrix3& M, const Point3& V);
	friend Matrix3 GEOMEXPORT XFormMat(const Matrix3& xm, const Matrix3& m);
	friend Point3 GEOMEXPORT VectorTransform(const Point3& V, const Matrix3& M);
	friend void GEOMEXPORT MatrixMultiply(Matrix3& outMatrix, const Matrix3& matrixA, const Matrix3& matrixB);
	friend void GEOMEXPORT Inverse(Matrix3& outMatrix, const Matrix3& M);
	friend void GEOMEXPORT AffineTranspose(Matrix3& outMatrix, const Matrix3& M);
	friend class Quat;

	/*! Internal: storage for the Matrix3 */
	float m[4][3];

	/*! Internal: Flags to give hints of the state of the Matrix3 */
	uint32_t flags;

	// Access i-th row as Point3 for read or assignment:
	Point3& operator[](int i);

public:
	/*! \remarks Constructor. Initialized to Identity. */
	GEOMEXPORT Matrix3();

	GEOMEXPORT Matrix3(const Matrix3&) = default;
	GEOMEXPORT Matrix3(Matrix3&&) = default;
	GEOMEXPORT Matrix3& operator=(const Matrix3&) = default;
	GEOMEXPORT Matrix3& operator=(Matrix3&&) = default;

	/*! \remarks Returns a reference to the 'i-th' Point3 of the matrix. */
	GEOMEXPORT const Point3& operator[](int i) const;
	/*! \remarks This clears the <b>MAT_IDENT</b> flag to indicate the matrix
	is not the identity. Non-const <b>GetAddr()</b> calls this for you. */
	GEOMEXPORT void SetNotIdent();
	/*! \remarks This sets the specified identity flag(s).
	\param f    Specifies the identity flag bit(s) to set. See Matrix Identity Flags above.
	*/
	GEOMEXPORT void SetIdentFlags(uint32_t f);
	/*! \remarks Returns the identity flags. */
	GEOMEXPORT uint32_t GetIdentFlags() const;
	/*! \remarks Clears the specified identity flag(s). See Matrix Identity
	Flags above.
	\param f    Specifies the identity flag bit(s) to clear. */
	GEOMEXPORT void ClearIdentFlag(uint32_t f);
	/*! \remarks Returns TRUE if the matrix is the identity matrix (based on
	the flags); otherwise FALSE. */
	GEOMEXPORT bool IsIdentity() const;
	/*! \remarks This method may be used to recompute the <b>*_IDENT</b> flags for this
	matrix. For instance, if you call a method, such as
	<b>INode::GetObjTMAfterWSM()</b>, and it returns a matrix, you cannot use
	the <b>IsIdentity()</b> method to check if the matrix is indeed the
	identity. This is because the flags that method checks are not initialized
	by the INode method. What you can do however is call this method first. This
	will validate the flags in the matrix so they accuratly reflect the
	properties of the matrix. If after calling this method, and then calling
	<b>IsIdentity()</b>, the proper result would be returned. */
	GEOMEXPORT void ValidateFlags();

	/*! \remarks Returns the address of this Matrix3.\n\n
	The Matrix3 class keeps flags indicating identity for rotation, scale,
	position, and the matrix as a whole, and thus the direct access via the []
	operator is restricted to prevent developers from modifying the matrix
	without updating the flags. This method, GetAddr(), still lets you get at
	the matrix itself and then you can use the [] operator on the result.\n
	Note: <b>GetAddr()</b> always calls <b>SetNotIdent()</b>, which clears all 
	IDENT flags. If you want to re-enable some optimizations, you can use 
	<b>SetIdentFlags()</b> to mark identity components after this call.\n\n
	Also Note: <b>typedef float MRow[3];</b>
	\return  The address of the Matrix3. */
	GEOMEXPORT MRow* GetAddr();
	/*! \remarks Returns the address of this Matrix3.\n\n
	This provides read-only access to the Matrix3 elements as a
	2 dimentional array.\n\n
	Also Note: <b>typedef float MRow[3];</b> */
	GEOMEXPORT const MRow* GetAddr() const;

	/*! \remarks Constructor, Unused.
	Default constructor now initializes to identity.
	Kept for backward compatibility.
	*/
	MAX_DEPRECATE_MATRIX_BOOL_CTOR(
		"Matrix3 and DMatrix3 are now initialized to identity by default.\n"
		"No need to use the Matrix3(bool) constructor anymore.\n"
		"Define MAX_SILENCE_DEPRECATED_MATRIX_BOOL_CTOR to silence this warning."
	)
	GEOMEXPORT explicit Matrix3(bool);

	/*! \remarks Constructor. The matrix is initialized to <b>fp</b>.
	\param fp Specifies the initial values for the matrix. */
	GEOMEXPORT explicit Matrix3(float (*fp)[3]);
	/*! \remarks Constructor. Initializes the matrix with the row data passed and validates
	the matrix flags.
	\param  U     The data for row 0.
	\param  V     The data for row 1.
	\param  N     The data for row 2.
	\param  T     The data for row 3. */
	GEOMEXPORT Matrix3(const Point3& U, const Point3& V, const Point3& N, const Point3& T);

	/*! \remarks Initializes the matrix with the row data passed and validates the matrix
	flags.
	\param  U     The data for row 0.
	\param  V     The data for row 1.
	\param  N     The data for row 2.
	\param  T     The data for row 3.
	\return  A reference to this matrix. */
	GEOMEXPORT Matrix3& Set(const Point3& U, const Point3& V, const Point3& N, const Point3& T);

	/*! An global instance of Matrix3 set to the identity.  An identity
		matrix has no rotation, scale or translation on it.  In other words,
		it is a matrix that has no effect when multiplied with another matrix.\n
		the structure of the Matrix is as follows: \n\n
		[1, 0, 0]\n
		[0, 1, 0]\n
		[0, 0, 1]\n
		[0, 0, 0] */
	GEOMEXPORT static const Matrix3 Identity;

	/*! \remarks Compares the elements of this matrix and the one specified element by
	element for exact equality. Returns nonzero if they are equal; otherwise
	zero.
	\param  M     The matrix to compare against. */
	GEOMEXPORT int operator==(const Matrix3& M) const;

	/*! \remarks Compares the elements of this matrix and the one specified element by
		element for inequality. Returns false iff they are unequal.
	\param  M     The matrix to compare against. */
	GEOMEXPORT bool operator!=(const Matrix3& M) const;

	/*! \remarks	 Compares the elements of this matrix and the one specified element by element
	for equality within the specified tolerance <b>epsilon</b>. Returns nonzero if
	they are 'equal'; otherwise zero.
	\param  M The matrix to compare against.
	\param epsilon The tolerance for comparison. If the values in the matrix are within this value
	(<b>+ epsilon</b> or <b>- epsilon</b>) they are considered equal. */
	GEOMEXPORT int Equals(const Matrix3& M, float epsilon = 1E-6f) const;

	/*! \remarks Subtracts a <b>Matrix3</b> from this <b>Matrix3</b>. */
	GEOMEXPORT Matrix3& operator-=(const Matrix3& M);
	/*! \remarks Adds a <b>Matrix3</b> to this <b>Matrix3</b>. */
	GEOMEXPORT Matrix3& operator+=(const Matrix3& M);
	/*! \remarks Multiplies this Matrix3 by the specified Matrix3 (<b>*this =
	(*this)*M;</b>). */
	/*! Multiply this matrix on the right by Matrix m.
		\code
			Matrix3 tm1, tm2;
			Matrix3 tm3 = tm1 * tm2;	// Is equivalent to the below
			tm1 *= tm2;					// tm1 now equals tm3
		\endcode
		\param M The matrix multiplied to the right of this matrix
		\return A reference to this matrix */
	GEOMEXPORT Matrix3& operator*=(const Matrix3& M);
	/*! \remarks Multiplies each element of this Matrix3 by a float.
		 \code
			 Matrix3 tm1(1), tm2(2);
			 tm1 *= 0.7f;	// This multiplies all elements by 0.7f
			 tm2.Scale(Point3(0.7f, 0.7f, 0.7f), TRUE); // This multiplies all elements by 0.7f
			 tm1 == tm2; // is true
		 \endcode
		 \param a The scale to apply to each element of this matrix
		 \return a reference to this matrix */
	GEOMEXPORT Matrix3& operator*=(float a);

	/*! \remarks Set this matrix to the Identity Matrix. */
	GEOMEXPORT void IdentityMatrix();
	/*! \remarks This method sets all elements of the matrix to 0.0f */
	GEOMEXPORT void Zero();

	/*! \remarks Returns the specified row of this matrix.
	\param i    Specifies the row to retrieve. */
	GEOMEXPORT Point3 GetRow(int i) const;

	/*! \remarks Sets the specified row of this matrix to the specified values.
	\param i    Specifies the row to set.
	\param p    The values to set. */
	GEOMEXPORT void SetRow(int i, Point3 p);
	/*! \remarks Returns the 'i-th' column of this matrix.
	\param i    Specifies the column to get (0-2). */
	GEOMEXPORT Point4 GetColumn(int i) const;
	/*! \remarks Sets the 'i-th' column of this matrix to the specified values.
	Note : Clears the matrix flags.
	\param i    Specifies the column to set (0-2).
	\param col    The values to set. */
	GEOMEXPORT void SetColumn(int i, Point4 col);
	/*! \remarks Returns the upper three entries in the specified column.
	\param i    Specifies the partial column to get (0-2). */
	GEOMEXPORT Point3 GetColumn3(int i) const;

	/*! \remarks This method zeros the translation portion of this matrix. */
	GEOMEXPORT void NoTrans();
	/*! \remarks This method zeros the rotation and scale portion of this matrix. */
	GEOMEXPORT void NoRot();
	/*! \remarks This method zeros the scale portion of this matrix without
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

	/*! \remarks Sets the translation row of this matrix to the specified
	values. The <b>POS_IDENT</b> flag is cleared.
	\param p    Specifies the values for the translation row. */
	GEOMEXPORT void SetTrans(const Point3& p);
	/*! \remarks Sets the specified component of the translation row of this
	matrix to the specified value. The <b>POS_IDENT</b> flag is cleared.
	\param i    Specifies the component of the translation row of this matrix to set.
	\param v    The value to set. */
	GEOMEXPORT void SetTrans(int i, float v);
	/*! \remarks Returns the translation row of this matrix.
	\return  The translation row of this matrix. */
	GEOMEXPORT const Point3& GetTrans() const;

	/*! \remarks Apply an incremental translation transformation to this
	matrix. This is equivalent to multiplying on the RIGHT by the transform.
	\param  p    Specifies the translation. */
	GEOMEXPORT void Translate(const Point3& p);
	/*! \remarks Apply an incremental X rotation transformation to this matrix.
	This is equivalent to multiplying on the RIGHT by the transform.
	\param angle    Specifies the X rotation in radians. */
	GEOMEXPORT void RotateX(float angle);
	/*! \remarks Apply an incremental Y rotation transformation to this matrix.
	This is equivalent to multiplying on the RIGHT by the transform.
	\param angle    Specifies the Y rotation in radians. */
	GEOMEXPORT void RotateY(float angle);
	/*! \remarks Apply an incremental Z rotation transformation to this matrix.
	This is equivalent to multiplying on the RIGHT by the transform.
	\param angle    Specifies the Z rotation in radians. */
	GEOMEXPORT void RotateZ(float angle);
	/*! \remarks Apply an incremental scaling transformation to this matrix.
	This is equivalent to multiplying on the RIGHT by the transform.
	\param s The scale values.
	\param trans If set to TRUE, the translation component is scaled. If <b>trans</b>
	= <b>FALSE</b> the translation component is unaffected. When 3ds Max was
	originally written there was a bug in the code for this method where the
	translation portion of the matrix was not being scaled. This meant that when
	a matrix was scaled the bottom row was not scaled. Thus it would always
	scale about the local origin of the object, but it would scale the world
	axes. When this bug was discovered, dependencies existed in the code upon
	this bug. Thus it could not simply be fixed because it would break the
	existing code that depended upon it working the incorrect way. To correct
	this the <b>trans</b> parameter was added. If this is set to <b>TRUE</b>,
	the translation component will be scaled correctly. The existing plug-ins
	don't use this parameter, it defaults to <b>FALSE</b>, and the code behaves
	the old way. */
	GEOMEXPORT void Scale(const Point3& s, bool trans = false);

	/*! \remarks Apply an incremental translation transformation to this
	matrix. This is equivalent to multiplying on the LEFT by the transform.
	param  p    Specifies the translation distance. */
	GEOMEXPORT void PreTranslate(const Point3& p);
	/*! \remarks Apply an incremental X rotation transformation to this matrix.
	This is equivalent to multiplying on the LEFT by the transform.
	\param angle    Specifies the X rotation in radians. */
	GEOMEXPORT void PreRotateX(float angle);
	/*! \remarks Apply an incremental Y rotation transformation to this matrix.
	This is equivalent to multiplying on the LEFT by the transform.
	\param angle    Specifies the Y rotation in radians. */
	GEOMEXPORT void PreRotateY(float angle);
	/*! \remarks Apply an incremental Z rotation transformation to this matrix.
	This is equivalent to multiplying on the LEFT by the transform.
	\param angle    Specifies the Z rotation in radians. */
	GEOMEXPORT void PreRotateZ(float angle);
	/*! \remarks Apply an incremental scaling transformation to this matrix.
	This is equivalent to multiplying on the LEFT by the transform.
	\param s The scale values.
	The translation component is unaffected. */
	GEOMEXPORT void PreScale(const Point3& s);
	/*! \remarks Sets this matrix to the identity and the translation components to the
	specified values.
	\param  p     The translation values to store. */
	GEOMEXPORT void SetTranslate(const Point3& p); // makes translation matrix
	/*! \remarks Sets this matrix to the identity and the rotation components to the
	specified X rotation.
	\param angle     The angle for X rotation (in radians). */
	GEOMEXPORT void SetRotateX(float angle); // makes rotation matrix
	/*! \remarks Sets this matrix to the identity and the rotation components to the
	specified Y rotation.
	\param angle     The angle for Y rotation (in radians). */
	GEOMEXPORT void SetRotateY(float angle);
	/*! \remarks Sets this matrix to the identity and the rotation components to the
	specified Z rotation.
	\param angle     The angle for Z rotation (in radians). */
	GEOMEXPORT void SetRotateZ(float angle);
	/*! \remarks Sets the rotation components of the matrix as specified by the quaternion.
	The translation and scale components will match the identity matrix.
	\param  q     Specifies the rotation to use for the matrix. */
	GEOMEXPORT void SetRotate(const Quat& q);
	/*! \remarks Sets the rotation components of the matrix as specified by the AngAxis. The
	translation and scale components will match the identity matrix.
	\param  aa     Specifies the rotation to use for the matrix. */
	GEOMEXPORT void SetRotate(const AngAxis& aa);
	/*! \remarks Sets the rotation components of this matrix using yaw, pitch and roll
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
	GEOMEXPORT void SetRotate(float yaw, float pitch, float roll);
	/*! \remarks	 Sets the rotation portion of the matrix to the rotation specified by the angle
	and axis and sets the translation portion to zeros.
	\param  axis 	 The axis of rotation.
	\param angle 	 The angle of rotation about the axis in radians. */
	GEOMEXPORT void SetAngleAxis(const Point3& axis, float angle);
	/*! \remarks Sets the scale components of this matrix to the specified values. The other
	components to this matrix will match the identity.
	\param  s     The scale factors for the matrix. */
	GEOMEXPORT void SetScale(const Point3& s); // makes scale matrix
	/*! \remarks This creates a matrix describing a viewpoint which is at the 'from'
	location, looking toward the 'to' location; the viewpoint is tilted so that
	the 'up' vector points to the top of the view.
	\param  from     This specifies the viewpoint source location.
	\param  to     This vector specifies the direction of view.
	\param  up     This vector points to the top of the view. */
	GEOMEXPORT void SetFromToUp(const Point3& from, const Point3& to, const Point3& up);
	/*! \remarks This method performs an in-place inversion on this matrix.
	An inverted matrix, when multiplied by the original, yields the identity. */
	GEOMEXPORT void Invert();

	/*! \remarks Perform matrix multiplication. */
	GEOMEXPORT Matrix3 operator*(const Matrix3&)const;
	/*! \remarks Perform matrix addition. */
	GEOMEXPORT Matrix3 operator+(const Matrix3&) const;
	/*! \remarks Perform matrix subtraction. */
	GEOMEXPORT Matrix3 operator-(const Matrix3&) const;

	/*! \remarks Returns the specified point transformed by this matrix.
	\param  p     The point to transform by this matrix. */
	GEOMEXPORT Point3 PointTransform(const Point3& p) const;

	/*! \remarks Returns the specified vector transformed by this matrix.
	\param  p     The vector to transform by this matrix. */
	GEOMEXPORT Point3 VectorTransform(const Point3& p) const;

	/*! \remarks	 Transforms the specified list of points with this matrix.
	 \param array The array of points to transform with this matrix.
	 \param n 	 The number of points in the array.
	 \param stride The size of the increment used when moving to the next point. If you wish to
	 transform an array of data objects which contain x, y, and z coordinates in
	 order (such as a <b>Point4</b>, or a structure containing a <b>Point3</b> as a
	 member) you can specify a 'stride' value (for instance
	 <b>sizeof(data_object)</b>). */
	GEOMEXPORT void TransformPoints(Point3* array, int n, int stride = sizeof(Point3));

	/*! \remarks	 Transforms the specified list of points with this matrix and stores the
	 resulting transformed points in the storage passed.
	 \param array 	 The array of points to transform (the source).
	 \param to The array to store the transformed points (the destination).
	 \param n 	 The number of points in the source array.
	 \param stride The size increment used when moving to the next source location.
	 \param strideTo The size increment used when moving to the next storage location. */
	GEOMEXPORT void TransformPoints(
			const Point3* array, Point3* to, int n, int stride = sizeof(Point3), int strideTo = sizeof(Point3));

	/*! \remarks	 Transforms the specified list of vectors with this matrix.
	 \param array The array of vectors to transform with this matrix.
	 \param n The number of vectors in the array.
	 \param stride The size of the increment used when moving to the next vector. If you wish to
	 transform an array of data objects which contain x, y, and z coordinates in
	 order (such as a <b>Point4</b>, or a structure containing a <b>Point3</b> as a
	 member) you can specify a 'stride' value (for instance
	 <b>sizeof(data_object)</b>). */
	GEOMEXPORT void TransformVectors(Point3* array, int n, int stride = sizeof(Point3));

	/*! \remarks	 Transforms the specified list of vectors with this matrix and stores the
	 resulting transformed vectors in the storage passed.
	 \param array 	 The array of vectors to transform (the source).
	 \param to The array to store the transformed vectors (the destination).
	 \param n 	 The number of vectors in the source array.
	 \param stride The size increment used when moving to the next source location.
	 \param strideTo  The size increment used when moving to the next storage location. */
	GEOMEXPORT void TransformVectors(
			const Point3* array, Point3* to, int n, int stride = sizeof(Point3), int strideTo = sizeof(Point3));

	/*! \remarks Retrieves the yaw, pitch and roll angles represented by the rotation in
	this matrix.
	\param yaw     The yaw rotation angle is stored here (in radians).
	\param pitch     The pitch rotation angle is stored here (in radians).
	\param roll     The roll rotation angle is stored here (in radians).    */
	GEOMEXPORT void GetYawPitchRoll(float* yaw, float* pitch, float* roll);

	/*! Save this Matrix3 to disk
	\param isave The interface responsible for actually saving the data
	\return IO_OK on success, or a failure code
	\remarks Save and Load are only forward declared in this header!
	If you need to use Save or Load, you must link with maxutil, which
	is where these methods are implemented.
	This way, geom.dll is independent from maxsdk. */
	UTILGEOMEXPORT IOResult Save(ISave* isave);

	/*! Load the data for this Matrix3.
	\param iload The interface responsible for actually loading the data
	\return IO_OK on success, or a failure code
	\remarks Save and Load are only forward declared in this header!
	If you need to use Save or Load, you must link with maxutil, which
	is where these methods are implemented.
	This way, geom.dll is independent from maxsdk. */
	UTILGEOMEXPORT IOResult Load(ILoad* iload);

	/*! \remarks Returns the 'parity' of the matrix. Scaling one axis of the matrix
	  negatively switches the 'parity'. However if you scale two axis the parity will flip back.
	  Scaling three axis switches the parity again.\n\n
	When rendering a mesh, if you scale something along one axis, it turns
	'inside out'. That is the direction when the normals are reversed. This
	method may be used to detect that case and then reverse the normals. The 3ds
	Max renderer does this -- if this method returns TRUE it flips all the
	normals so it won't turn inside out.
	  */
	GEOMEXPORT bool Parity() const;
};

/*! \remarks Builds a new matrix for use as a X rotation transformation.
\param angle Specifies the angle of rotation in radians.
\return  A new X rotation <b>Matrix3</b>. */
GEOMEXPORT Matrix3 RotateXMatrix(float angle);

/*! \remarks Builds a new matrix for use as a Y rotation transformation.
\param angle Specifies the angle of rotation in radians.
\return  A new Y rotation <b>Matrix3</b>. */
GEOMEXPORT Matrix3 RotateYMatrix(float angle);

/*! \remarks Builds a new matrix for use as a Z rotation transformation.
\param angle Specifies the angle of rotation in radians.
\return  A new Z rotation <b>Matrix3</b>. */
GEOMEXPORT Matrix3 RotateZMatrix(float angle);

/*! \remarks Builds a new matrix for use as a translation transformation.
\param  p Specifies the translation values.
\return  A new translation <b>Matrix3</b>. */
GEOMEXPORT Matrix3 TransMatrix(const Point3& p);

/*! \remarks Builds a new matrix for use as a scale transformation.
\param  s Specifies the scale values.
\return  A new scale <b>Matrix3</b>. */
GEOMEXPORT Matrix3 ScaleMatrix(const Point3& s);

/*! \remarks Builds a new matrix for use as a rotation transformation by
specifying yaw, pitch and roll angles.\n\n
This definition will depend on what our coordinate system is. This one is
equivalent to:\n\n
<b>M.IdentityMatrix();</b>\n\n
<b>M.RotateZ(roll);</b>\n\n
<b>M.RotateX(pitch);</b>\n\n
<b>M.RotateY(yaw);</b>\n\n
Which presupposes Y is vertical, X is sideways, Z is forward
\param Yaw Specifies the yaw angle in radians.
\param Pitch Specifies the pitch angle in radians.
\param Roll Specifies the roll angle in radians.
\return  A new rotation <b>Matrix3</b>. */
GEOMEXPORT Matrix3 RotateYPRMatrix(float Yaw, float Pitch, float Roll);

/*! \remarks Builds a new matrix for use as a rotation transformation by
specifying an angle and axis.
\param axis Specifies the axis of rotation. Note that this angle is expected to be
normalized.
\param angle Specifies the angle of rotation. Note: The direction of the angle in this
method is opposite of that in <b>AngAxisFromQ()</b>.
\return  A new rotation Matrix3. */
GEOMEXPORT Matrix3 RotAngleAxisMatrix(const Point3& axis, float angle);

/*! \remarks Return the inverse of the matrix
\param  M The matrix to compute the inverse of. */
GEOMEXPORT Matrix3 Inverse(const Matrix3& M); // return Inverse of matrix

/*! \remarks Return the inverse of the matrix using doubles for the intermediary results
	\param  M The matrix to compute the inverse of. */
GEOMEXPORT Matrix3 InverseHighPrecision(const Matrix3& M);

/*! \remarks Return the pseudoinverse of an affine symmetric matrix, meaning that if the
matrix is degenerate (projects into a plane, line or point; eg. has both row 1 and column 1 zero)
this will still produce a matrix that inverts the nonzero parts of the transform.
	\param m The matrix to compute the pseudoinverse of. */
GEOMEXPORT Matrix3 PseudoInverse(const Matrix3& m);

/*! \remarks Transposes the affine portion of the matrix. Since Matrix3 does not have
  a 4th column you cannot transpose the entire 4x4 matrix, so this method only transposes
  the 3x3 matrix representing rotations, scale and skew.
  \param  M The matrix to compute the affine transpose of. */
GEOMEXPORT Matrix3 AffineTranspose(const Matrix3& M); // return affine transpose of matrix

/*! \remarks These transform a <b>Point3</b> with a <b>Matrix3</b>. These two
versions of transforming a point with a matrix do the same thing, regardless of
the order of operands (linear algebra rules notwithstanding).
\param  A The matrix to transform the point with.
\param  V The point to transform.
\return  The transformed Point3. */
GEOMEXPORT Point3 operator*(const Matrix3& A, const Point3& V); // Transform Point with matrix

/*! \remarks These transform a <b>Point3</b> with a <b>Matrix3</b>. These two
versions of transforming a point with a matrix do the same thing, regardless of
the order of operands (linear algebra rules notwithstanding).
\param  V The point to transform.
\param  A The matrix to transform the point with.
\return  The transformed <b>Point3</b>. */
GEOMEXPORT Point3 operator*(const Point3& V, const Matrix3& A); // Transform Point with matrix

/*! \remarks Transform the vector (Point3) with the specified matrix.
\param  M The matrix to transform the vector with.
\param  V The vector to transform.
\return  The transformed vector (as a Point3). */
GEOMEXPORT Point3 VectorTransform(const Matrix3& M, const Point3& V);
GEOMEXPORT Point3 VectorTransform(const Point3& V, const Matrix3& M);

/*! transform a plane.
	\note this only works if M is orthogonal
	\param M The transformation to apply to the plain
	\param plane the plane to be transformed */
GEOMEXPORT Point4 TransformPlane(const Matrix3& M, const Point4& plane);

/*! \remarks This method is used to build a matrix that constructs a
transformation in a particular space. For example, say you have a rotation you
want to apply, but you want to perform the rotation in another coordinate
system. To do this, you typically transform into the space of the coordinate
system, then apply the transformation, and then transform out of that
coordinate system. This method constructs a matrix that does just this. It
transforms matrix <b>m</b> so it is applied in the space of matrix <b>xm</b>.
It returns a <b>Matrix3</b> that is <b>xm*m*Inverse(xm)</b>.
\param  xm Specifies the coordinate system you want to work in.
\param  m Specifies the transformation matrix.
\return  Returns a <b>Matrix3</b> that is <b>xm*m*Inverse(xm)</b>. */
GEOMEXPORT Matrix3 XFormMat(const Matrix3& xm, const Matrix3& m);

/*! \remarks Mirrors the input matrix against the X, Y, or Z axis.
	This function mirrors the input matrix such that if it transformed a Point3 p,
	the transform applied by the mirrored transform would place p in an equivalent
	position relative but with one of it's elements negated.
	\code
	Point3 p = ...; // Some arbitrary point;
	Matrix3 tm = ...; // Some matrix
	Matrix3 tmMirror = tm;
	MirrorMatrix(tm, kXAxis); // Mirror this matrix around the X Axis;
	Point3 p3Result = tm.PointTransform(p);			// Apply the original transformation
	Point3 p3Mirrored = tmMirror.PointTransform(p);	// Apply the mirrored transformation
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
GEOMEXPORT void MirrorMatrix(Matrix3& tm, Axis axis, bool scaleMatrix = false);


/*! \remarks Same as Maxtrix3::operator[]. Perform matrix multiplication without additional matrix copy
	\param  outMatrix The result of matrixA * matrixB
	\param  matrixA First matrix to multiply
	\param  matrixB Second matrix to multiply*/
GEOMEXPORT void MatrixMultiply(Matrix3& outMatrix, const Matrix3& matrixA, const Matrix3& matrixB);

/*! \remarks Same as Maxtrix3::Inverse. Compute the inverse of the matrix without additional matrix copy
	\param  outMatrix The inversed matrix.
	\param  M The matrix to compute the inverse of. */
GEOMEXPORT void Inverse(Matrix3& outMatrix, const Matrix3& M);

/*! \remarks Same as Matrix3::AffineTranspose. Computes the affine transpose without additional matrix copy
  \param  outMatrix The inversed matrix.
  \param  M The matrix to compute the affine transpose of. */
GEOMEXPORT void AffineTranspose(Matrix3& outMatrix, const Matrix3& M);

/*! \remarks Create a rotational matrix oriented to the provided normal.
  \param  normal The normal vector, must be normalized.
  \param  mat The output orientation matrix. */
GEOMEXPORT void SetMatrixFromNormal(const Point3& normal, Matrix3& mat);
