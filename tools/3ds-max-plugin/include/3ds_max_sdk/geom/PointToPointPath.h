//**************************************************************************/
// Copyright (c) 1998-2015 Autodesk, Inc.
// All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once
#include "GeomExport.h"
#include "geom_span.hpp"

#include <memory>

namespace MaxSDK {

/*! \sa  Class EdgeDescr\n\n
\par Description:
This class describes the edge of to another node
edgeIndex is the id of the edge going to the other node
nodeIndex is the id of the other node
*/
class GEOMEXPORT EdgeDescr
{
public:
	EdgeDescr() = default;
	EdgeDescr(int nodeIndex, int edgeIndex, float len);

	int EdgeIndex()
	{
		return mEdgeIndex;
	};
	int NodeIndex()
	{
		return mNodeIndex;
	};
	float Length()
	{
		return mLength;
	};

private:
	int mEdgeIndex = -1;
	int mNodeIndex = -1;
	float mLength = 0.f;
};


/*! \sa  Class PointToPointPath\n\n
\par Description:
This class is used to compute the shortest path between 2 nodes in graph.  You just describe
the graph based on it connection data and then ask for the path between 2 nodes.
ie the Dijkstra's algorithm
*/
class GEOMEXPORT PointToPointPath
{
public:
	PointToPointPath()
	{
	}
	virtual ~PointToPointPath(){};

	/* Creates an instance of the PointToPoint class */
	static PointToPointPath* Create();

	/* sets the number of nodes in the graph */
	virtual void SetNumberNodes(int num) = 0;
	/*! Adds a node to the graph
	/param index is the id of the node you are adding
	/param connectedEdges are all the edges that connect to the node
	*/
	virtual void AddNode(int index, geo::span<const EdgeDescr> connectedEdges) = 0;
	/*!
	Sets the start index of the path
	*/
	virtual void SetStartPoint(int index) = 0;
	/*!
	Sets the end index of the path, these 2 are separated since if you do not change the start path
	there are some optimizations that make it faster
	*/
	virtual void SetEndPoint(int index) = 0;
	/* returns the shortest path between the start and end index and distance*/
	virtual float GetPath(std::unique_ptr<EdgeDescr[]>& connectedEdges, int& numConnectedEdges) = 0;
	/* destroys the instance */
	virtual void DeleteMe() = 0;
};


}; // namespace MaxSDK
