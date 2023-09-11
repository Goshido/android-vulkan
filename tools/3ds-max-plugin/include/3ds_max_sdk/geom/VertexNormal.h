//**************************************************************************/
// Copyright (c) 1998-2011 Autodesk, Inc.
// All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once
#include "point3.h"

#include <cstdint>

namespace MaxSDK {
/**
	The class used to compute vertex normals considering smoothing.
*/
class GEOMEXPORT VertexNormal
{
public:
	Point3 norm; ///< The normal.
	uint32_t smooth; ///< The smoothing groups.
	VertexNormal* next; ///< A pointer to the next normal -- this class is a linked list.
	bool init; ///< The init variable is used as a flag to indicate if the first normal in the list has been
			   ///< initialized.

	/** Default Constructor */
	VertexNormal();

	/** Constructor.
	\param n - The vertex normal vector
	\param s - The ID of the smoothing group
	*/
	VertexNormal(Point3& n, uint32_t s);

	/** Copy Constructor */
	VertexNormal(const VertexNormal& copyVNormal);

	/** Assignment operator */
	VertexNormal& operator=(const VertexNormal& from);

	/** Destructor */
	virtual ~VertexNormal();

	/** Add a normal to the list if the smoothing group bits overlap, otherwise create a new vertex normal in the list.
	It is used when a face is going to add its normal to a vertex.
	This method is passed the normal and the smoothing information for that face.
	It checks if the normal passed shares smoothing information with the existing normal.
	If it does, the normal is added in, and the smoothing bits are bitwise OR-ed in.
	If it does not, a new vertex normal is created.
	\param n The new normal to be added.
	\param s The smoothing group information.
	*/
	void AddNormal(Point3& n, uint32_t s);

	/** Retrieves a normal if the smoothing groups overlap or there is
		only one in the list
	\param s The smoothing group information.
	\return The Normal
	*/
	Point3& GetNormal(uint32_t s);

	/** Normalize each normal in the list
	 */
	void Normalize();

private:
	void internalCopy(const VertexNormal& from);
};

} // namespace MaxSDK
