//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//
//

#pragma once
#include "GeomExport.h"
#include "point2.h"
#include "point3.h"
#include "geom_span.hpp"

#include <cstdint>

// forward declarations
class Box3;
class Matrix3;

/**
 * Calculates the barycentric coordinates of the point p in 3-dimensional space
 * according to a reference triangle indicated by its three vertices p0, p1 and p2.
 * \param p0,p1,p2 The three vertices of the reference triangle in 3-dimensional space.
 * \param p The point for which the barycentric coordinates should be calculated.
 * \return Barycentric coordinates of point p.
 * \see BaryCoords(Point2, Point2, Point2, Point2)
 */
GEOMEXPORT Point3 BaryCoords(Point3 p0, Point3 p1, Point3 p2, Point3 p);

/**
 * Calculates the barycentric coordinates of the point p in 2-dimensional space
 * according to a reference triangle indicated by its three vertices p0, p1 and p2.
 * \param p0,p1,p2 The three vertices of the reference triangle in 2-dimensional space.
 * \param p The point for which the barycentric coordinates should be calculated.
 * \return Barycentric coordinates of point p.
 * \see BaryCoords(Point3, Point3, Point3, Point3)
 */
GEOMEXPORT Point3 BaryCoords(Point2 p0, Point2 p1, Point2 p2, Point2 p);

/**
 * Checks if the given ray intersects a three-dimensional box.
 * \param ray The ray
 * \param b The box
 * \return \c true if the ray intersects the box, \c false otherwise.
 */
GEOMEXPORT bool RayHitsBox(Ray& ray, Box3& b);

/**
 * Calculates the distance of a given point from a line in two dimensions.
 * \param p0,p1 Pointers to the two 2-dimensional points with which the line is defined.
 * \param q Pointer to the 2-dimensional point whose distance from the line should be calculated.
 * \return The distance of the point from the line.
 */
GEOMEXPORT float DistPtToLine(Point2* p0, Point2* p1, Point2* q);

/**
 * Calculates the distance of a given point from a line in three dimensions.
 * \param p0,p1 Pointers to the two 3-dimensional points with which the line is defined.
 * \param q Pointer to the 3-dimensional point whose distance from the line should be calculated.
 * \return The distance of the point from the line.
 */
GEOMEXPORT float Dist3DPtToLine(Point3* p0, Point3* p1, Point3* q);

/**
 * Compute the point of intersection, or a least squares approximation to it, for an arbitrary number N of lines in
 * three dimensions. The ith of the N lines is presumed to be defined in terms of the parameter s_i according to:
 *
 *     x = fixedCoeffs[i] + s_i * (directionVecs[i])
 *
 * A least squares problem is solved for the vector s' of parameters representing (the approximations to) the point of
 * intersection, and intersectionPoint is computed by averaging estimates from each line, as:
 *
 *     intersectionPoint = (1 / N) * (SUM_OVER_I(fixedCoeffs[i] + s'_i * (directionVecs[i])))
 *
 * \param [out] intersectionPoint: least squares approximation to intersection point
 * \param [in] fixedCoeffs: fixed coefficients of line representation
 * \param [in] directionVecs: line direction vectors
 * \param [in] doNormalize: when true, normalized forms of the direction vectors are used in the least squares system
 * \return Flag indicating whether solution of least squares problem was successful. If it was not, intersectionPoint
 * will be returned as a zero vector.
 */
GEOMEXPORT bool ComputeIntersectionPoint(Point3& intersectionPoint, geo::span<const Point3> fixedCoeffs,
		geo::span<const Point3> directionVecs, bool doNormalize = true);

/**
 * \deprecated Do not use this function. Use ComputeBumpVec2D() instead.
 * This is here for compatibility only.
 *
 * Computes the 3 bump basis vectors from the UVW coordinates of the triangle.
 *
 * \param tv Texture coordinates at 3 triangle vertices
 * \param v Coordinates of triangle vertices (usually in camera space).
 * \param[out] bvec The 3 bump basis vectors (normalized) corresponding to the U,V,and W axes.
 */
GEOMEXPORT void ComputeBumpVectors(const Point3 tv[3], const Point3 v[3], Point3 bvec[3]);


/**
 * Computes the 2 Bump basis vectors from the UV, VW, or WU  at a triangle.
 *
 * \param axis Either AXIS_UV, AXIS_VW, or AXIS_WU .
 * \param tv Texture coordinates at 3 triangle vertices.
 * \param v Coordinates of triangle vertices (usually in camera space)
 * \param[out] bvec The 2 normalized bump basis vectors corresponding to the specified axes.
 *
 * \note This is the recommended way to compute bump vectors instead of ComputeBumpVectors() which
 *			can give erroneous results.
 */
GEOMEXPORT void ComputeBumpVec2D(int axis, const Point3 tv[3], const Point3 v[3], Point3 bvec[2]);

/**
 * returns the bump basis vector for the U texture channel (called the tangent)
 * \param tv Texture coordinates at 3 triangle vertices.
 * \param v Coordinates of triangle vertices (usually in camera space)
 * \return The 2 normalized bump basis vectors corresponding to the specified axes.
 */
GEOMEXPORT Point3 ComputeTangent(const Point3 tv[3], const Point3 v[3]);

/**
 * Computes the bump basis vector for the U texture channel (called the tangent),
 * and the cross product of the surface normal with the tangent (called the binormal).
 * These along with the surface normal are used as the basis vectors for normal mapping.
 * \param tv Texture coordinates at 3 triangle vertices.
 * \param v Coordinates of triangle vertices (usually in camera space)
 * \param[out] bvec The 2 normalized bump basis vectors corresponding to the specified axes.
 */
GEOMEXPORT void ComputeTangentAndBinormal(const Point3 tv[3], const Point3 v[3], Point3 bvec[2]);

/**
* This class is a callback for computing bump basis vectors on a given set of triangle/quad faces.
* The callback could be used as an input parameter of the method ComputeMikkTangents to compute the tangent and
binormal(bitangent).
* The Plug-ins need to implement GetNumberOfFaces, GetNumberVerticesOfFace, GetVertex, GetNormal and GetTexture
* methods to input all required data to be computed, and SetTangent method to output the computation results.
* Usage: The following code demostrates how to compute the MikkT tangents for a given triangle face.
* \code
	// Derive from ITangentsComputationCallback to fill the input data and receive the output results
	class TangentsComputationCallback : public ITangentsComputationCallback
	{
	public:
		// Return 1 for the case with only one face
		virtual int GetNumberOfFaces() const override
		{
			return 1;
		}
		// Return the vertex number for the given face, 3 for triangle face, 4 for quad face
		virtual int GetNumberVerticesOfFace(const int faceIdx) const override
		{
			return 3;
		}
		// Input the position coordinates for the given vertex specified by the index of face and vertex
		virtual void GetVertex(Point3& position, const int faceIdx, const int vertIdx) override
		{
			if (vertIdx ==0) {
				position = { 0.0f, 0.0f, 0.0f };
			}
			else if (vertIdx==1) {
				position = { 0.0f, 1.0f, 0.0f };
			}
			else if (vertIdx==2) {
				position = { 1.0f, 0.0f, 0.0f };
			}
		}
		// Input the normal coordinates for the given vertex specified by the index of face and vertex
		virtual void GetNormal(Point3& normal, const int faceIdx, const int vertIdx) override
		{
			// Use the same normal coordinates { 0.0f, 0.0f, 1.0f } for all three vertices for simplicity
			if (vertIdx == 0) {
				normal = { 0.0f, 0.0f, 1.0f };
			}
			else if (vertIdx == 1) {
				normal = { 0.0f, 0.0f, 1.0f };
			}
			else if (vertIdx == 2) {
				normal = { 0.0f, 0.0f, 1.0f };
			}
		}
		// Input the texture coordinates for the given vertex specified by the index of face and vertex
		virtual void GetTexture(Point2& texture, const int faceIdx, const int vertIdx) override
		{
			if (vertIdx == 0) {
				texture = { 0.0f, 0.0f };
			}
			else if (vertIdx == 1) {
				texture = { 0.0f, 1.0f };
			}
			else if (vertIdx == 2) {
				texture = { 1.0f, 0.0f };
			}
		}
		// Ouput the tangent, bitangent and handedness for the given vertex specified by the index of face and vertex
		virtual void SetTangent(const Point3& tangent, const Point3& binormal,
			const float handedness, const int faceIdx, const int vertIdx) override
		{
			// Set tangent
			Point3 tan = tangent;
			outputTangents.Append(1, &tan);

			// Set binormal
			Point3 binml = binormal;
			outputBinormals.Append(1, &binml);

			// Set handedness
			float hand = handedness;
			outputHandednesses.Append(1, &hand);
		}

		// Return the computation result of Tangents
		Tab<Point3> GetTangentsResult() {
			return outputTangents;
		}

		// Return the computation result of Binormals
		Tab<Point3> GetBinormalsResult() {
			return outputBinormals;
		}

		// Return the computation result of Handednesses
		Tab<float> GetHandednessesResult() {
			return outputHandednesses;
		}

		private:
			// Output tangent/binormal data
			Tab<Point3> outputTangents;
			Tab<Point3> outputBinormals;
			Tab<float> outputHandednesses;
	};

	// Compute tangent basis
	TangentsComputationCallback cb;
	ComputeMikkTangents(&cb);
	// Get the computation results
	Tab<Point3> tangents = cb.GetTangentsResult();
	Tab<Point3> binormals = cb.GetBinormalsResult();
	Tab<float> handednesses = cb.GetHandednessesResult();
* \endcode
*/
class ITangentsComputationCallback
{
public:
	/**
	 * Get the number of faces to be computed.
	 * \return The the number of faces to be computed.
	 */
	virtual int GetNumberOfFaces() const = 0;
	/**
	 * Get the number of vertices for the given index of face.
	 * \param faceIndex The index of the face.
	 * \return The number of vertices for the given index of face.
	 */
	virtual int GetNumberVerticesOfFace(const int faceIndex) const = 0;
	/**
	 * Get the vertex coordinates for the given index of face and index of vertex.
	 * \param[out] vertex The vertex coordinates for the given index of face and index of vertex.
	 * \param faceIndex The index of the face.
	 * \param vertexIndex The index of the vertex based on the face.
	 */
	virtual void GetVertex(Point3& vertex, const int faceIndex, const int vertexIndex) = 0;
	/**
	 * Get the normal coordinates for the given index of face and index of vertex.
	 * \param[out] normal The normal coordinates for the given index of face and index of vertex.
	 * \param faceIndex The index of the face.
	 * \param vertexIndex The index of the vertex based on the face.
	 */
	virtual void GetNormal(Point3& normal, const int faceIndex, const int vertexIndex) = 0;
	/**
	 * Get the texture coordinates for the given index of face and index of vertex.
	 * \param[out] texture The texture coordinates for the given index of face and index of vertex.
	 * \param faceIndex The index of the face.
	 * \param vertexIndex The index of the vertex based on the face.
	 */
	virtual void GetTexture(Point2& texture, const int faceIndex, const int vertexIndex) = 0;
	/**
	 * Set the tangent and binormal(bitangent) coordinates for the given index of face and index of vertex.
	 * \param tangent The tangent coordinates for the given index of face and index of vertex.
	 * \param binormal The binormal(bitangent) coordinates for the given index of face and index of vertex.
	 * \param handedness The handedness of tangent basis, either 1.0f or -1.0f.
	 * the handedness can be used to determine the direction of the binormal(bitangent), callers can use normal, tangent
	 * and handedness to re-calculate the binormal(bitangent) in pixel level,
	 * binormal(bitangent) = handedness * CrossProd(normal, tangent);
	 * \param faceIndex The index of the face.
	 * \param vertexIndex The index of the vertex based on the face.
	 */
	virtual void SetTangent(const Point3& tangent, const Point3& binormal, const float handedness, const int faceIndex,
			const int vertexIndex) = 0;

protected:
	virtual ~ITangentsComputationCallback()
	{
	}
};

/**
 * Computes the bump basis vector in MikkT way for the U texture channel (called the tangent),
 * and the cross product of the surface normal with the tangent (called the binormal).
 * These along with the surface normal are used as the basis vectors for normal mapping.
 * \param tangentsComputationCallback The callback will be called during the
 * computation to fill all required data and receive the results of computation.
 */
GEOMEXPORT void ComputeMikkTangents(ITangentsComputationCallback* tangentsComputationCallback);

//! Return value of ComputeUVTangents().
struct UVTangentVectors
{
	UVTangentVectors()
	{
	}
	UVTangentVectors(const Point3& u, const Point3& v)
			: tangentU(u)
			, tangentV(v)
	{
	}
	Point3 tangentU;
	Point3 tangentV;
};

/**
 * Computes the bump basis vectors for the U and V components of the texture coordinates channel (also called the
 * <b>tangent</b> and <b>bitangent</b>).
 * \remark This function differs from ComputeTangentAndBinormal() as the latter
 * computes the binormal - the cross product of U tangent vector and normal - rather than the V tangent (or bitangent).
 * The binormal and bitangent will differ if the U and V coordinates are not perpendicular to each other.
 * \param p0 The XYZ coordinate of the first triangle vertex.
 * \param p1 The XYZ coordinate of the second triangle vertex.
 * \param p2 The XYZ coordinate of the third triangle vertex.
 * \param uv0 The UV coordinate of the first triangle vertex.
 * \param uv1 The UV coordinate of the second triangle vertex.
 * \param uv2 The UV coordinate of the third triangle vertex.
 * \return Two unit vectors in XYZ space, tangent to the surface of the given triangle, aligned with the U and V
 * directions of the given texture coordinates.
 */
GEOMEXPORT UVTangentVectors ComputeUVTangents(
		const Point3& p0, const Point3& p1, const Point3& p2, const Point2& uv0, const Point2& uv1, const Point2& uv2);

/**
 * Low precision compression of a vector from 12 bytes to 4 bytes.
 * Only accurate to 1 part in 512. This is commonly used to compress normals.
 * The vector has to be <= 1.0 in length.
 * \param p The decompressed vector.
 * \return The 4-byte long compressed value of p
 * \see DeCompressNormal(), BMM_CHAN_NORMAL
 */
GEOMEXPORT uint32_t CompressNormal(Point3 p);

/**
 * Decompresses a vector that was compressed using CompressNormal().
 * This function may also be used to decompress a surface normal from the G-Buffer
 * using the BMM_CHAN_NORMAL channel. The decompressed vector has absolute error
 * \<.001 in each component.
 * \param n A vector compressed using CompressNormal()
 * \return A normalized decompressed normal vector
 * \see CompressNormal()
 */
GEOMEXPORT Point3 DeCompressNormal(uint32_t n);

/**
 * Creates an arbitrary axis system given an up vector that conforms to the AutoCAD algorithm.
 * \param zAxis The up vector.
 * \param[out] matrix The new axis system.
 */
GEOMEXPORT void ArbAxis(const Point3& zAxis, Matrix3& matrix);
