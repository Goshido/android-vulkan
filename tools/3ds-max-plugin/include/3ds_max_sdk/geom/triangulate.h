/**********************************************************************
 *<
   FILE: triangulate.h

   DESCRIPTION: Class and method definitions for triangulation related algorithms.

   CREATED BY: Nicholas Frechette

   HISTORY:

 *>   Copyright (c) 2018, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "GeomExport.h"
#include "geom_span.hpp"

#include <cassert>
#include <cstdint>

class Point3;
class DPoint3;

namespace MaxSDK {

/*! \sa  Class BufferProxy.\n\n
\par Description:
This class wraps a raw memory buffer and it allows individual access to elements
each located with a given stride in bytes. e.g. to iterate over every MNMesh vertex position:
MaxSDK::BufferProxy<const Point3> meshVerticesBuffer(mnmeshVertices, numv, sizeof(MNVert));
*/
template <typename ElementType>
class BufferProxy
{
public:
	//! Initializes all members to zero, an invalid empty buffer.
	BufferProxy()
			: mData(nullptr)
			, mNumElements(0)
			, mStride(0)
	{
	}

	/*! \remarks Constructor. mData, mNumElements, and mStride are initialized to the values specified. */
	/*! \param data - initial value for mData
		\param numElements - initial value for mNumElements
		\param stride - initial value for mStride
	*/
	BufferProxy(ElementType* data, size_t numElements, size_t stride = sizeof(ElementType))
			: mData(data)
			, mNumElements(numElements)
			, mStride(stride)
	{
	}

	//! \brief Retrieves the number of elements in the buffer.
	/*! \return The number of elements in the buffer.*/
	size_t Count() const
	{
		return mNumElements;
	}

	//! \brief Retrieves the element stride.
	/*! \return The element stride.*/
	size_t Stride() const
	{
		return mStride;
	}

	//! \brief Retrieves whether the buffer points to actual memory.
	/*! \return The buffer is valid or not.*/
	bool IsValid() const
	{
		return mData != nullptr;
	}

	//! \brief Retrieves whether the buffer is empty or not.
	/*! \return Whether the buffer is empty or not.*/
	bool IsEmpty() const
	{
		return mNumElements != 0;
	}

	/*! \remarks Allows access to an element of the buffer using the subscript operator.
	\return  The element at the provided index.*/
	ElementType& operator[](size_t i)
	{
		assert(IsValid() && i < mNumElements);
		intptr_t rawData = reinterpret_cast<intptr_t>(mData);
		intptr_t elementPtr = rawData + (i * mStride);
		return *reinterpret_cast<ElementType*>(elementPtr);
	}

	/*! \remarks Allows access to an element of the buffer using the subscript operator.
	\return  The element at the provided index.*/
	const ElementType& operator[](size_t i) const
	{
		assert(IsValid() && i < mNumElements);
		intptr_t rawData = reinterpret_cast<intptr_t>(mData);
		intptr_t elementPtr = rawData + (i * mStride);
		return *reinterpret_cast<const ElementType*>(elementPtr);
	}

protected:
	/*! A pointer to the first element in the buffer.*/
	ElementType* mData;

	/*! The number of elements contained in the buffer.*/
	size_t mNumElements;

	/*! Each element in the buffer is mStride bytes apart.*/
	size_t mStride;
};

//////////////////////////////////////////////////////////////////////////

/** Puts diagonals in increase-by-last-index, decrease-by-first order.
	This sorts the diagonals in the following fashion: each diagonal is reordered
	so that its smaller index comes first, then its larger. Then the list of
	diagonals is sorted so that it increases by second index, then decreases by
	first index. Such an ordered list for a 9-gon might be (1,3),(0,3),
	(0,4),(5,7),(4,7),(4,8). (This order is especially convenient for converting
	into triangles - it makes for a linear-time conversion.) SortPolygonDiagonals() uses
	qsort for speed.\n\n

	\param[in,out] diag The diagonals.*/
GEOMEXPORT void SortPolygonDiagonals(geo::span<int> diag);
GEOMEXPORT void SortPolygonDiagonals(geo::span<uint64_t> diag);

/** Uses a triangulation scheme optimized for convex polygons to find a set of
	diagonals for this sequence of vertices, creating a triangulation for the
	polygon they form. Face vertices are optional. If there is no index buffer,
	we assume they are trivial: 0, 1, 2, 3, 4, etc.

	\param[in] faceVertexIndices The face vertices in the sequence.\n\n
	\param[in] meshVertices The array of vertices.\n\n
	\param[out] diag A pointer to an array of size (deg-3)*2 where the diagonals can be put. */
GEOMEXPORT void BestConvexDiagonals(
		const BufferProxy<const int>& faceVertexIndices, const BufferProxy<const Point3>& meshVertices, int* diag);

/** Uses a triangulation scheme optimized for convex polygons to find a set of
	diagonals for this sequence of vertices, creating a triangulation for the
	polygon they form. Face vertices are optional. If there is no index buffer,
	we assume they are trivial: 0, 1, 2, 3, 4, etc.

	\param[in] faceVertexIndices The face vertices in the sequence.\n\n
	\param[in] meshVertices The array of vertices.\n\n
	\param[out] diag A pointer to an array of size (deg-3)*2 where the diagonals can be put. */
GEOMEXPORT void BestConvexDiagonals(
		const BufferProxy<const int>& faceVertexIndices, const BufferProxy<const DPoint3>& meshVertices, int* diag);

/** This method finds diagonals for this sequence of vertices, creating a
	triangulation for the polygon they form.

	\param[in] faceVertexIndices The face vertices in the sequence.\n\n
	\param[in] meshVertices The array of vertices.\n\n
	\param[out] diag A pointer to an array of size (deg-3)*2 where the diagonals can be put. */
GEOMEXPORT void FindDiagonals(
		const BufferProxy<const int>& faceVertexIndices, const BufferProxy<const Point3>& meshVertices, int* diag);

/** This method finds diagonals for this sequence of vertices, creating a
	triangulation for the polygon they form.

	\param[in] faceVertexIndices The face vertices in the sequence.\n\n
	\param[in] meshVertices The array of vertices.\n\n
	\param[out] diag A pointer to an array of size (deg-3)*2 where the diagonals can be put. */
GEOMEXPORT void FindDiagonals(
		const BufferProxy<const int>& faceVertexIndices, const BufferProxy<const DPoint3>& meshVertices, int* diag);

/** This method fills in the buffer with the full triangulation for the face,
	based on the internal diagonal list.
	The buffer must be of size (deg-2)*3.
	\param[in] deg Face degree (number of vertices).\n\n
	\param[in,out] diag The diagonals.\n\n
	\param[out] out_tri The output triangle indices, 3 indices per triangle. */
GEOMEXPORT void GetTriangles(int deg, geo::span<int> diag, geo::span<int> out_tri);
GEOMEXPORT void GetTriangles(uint16_t deg, geo::span<uint16_t> diag, geo::span<uint16_t> out_tri);
GEOMEXPORT void GetTriangles(uint64_t deg, geo::span<uint64_t> diag, geo::span<uint64_t> out_tri);

/** This method fills in the buffer with the full triangulation for the face,
	based on the internal diagonal list.
	The buffer must be of size (deg-2)*3.
	Diags are not reordered.
	\param[in] deg Face degree (number of vertices).\n\n
	\param[in] diag The diagonals.\n\n
	\param[out] out_tri The output triangle indices, 3 indices per triangle. */
GEOMEXPORT void GetTriangles(int deg, geo::span<const int> diag, geo::span<int> out_tri);
GEOMEXPORT void GetTriangles(uint16_t deg, geo::span<const uint16_t> diag, geo::span<uint16_t> out_tri);
GEOMEXPORT void GetTriangles(uint64_t deg, geo::span<const uint64_t> diag, geo::span<uint64_t> out_tri);
} // namespace MaxSDK
