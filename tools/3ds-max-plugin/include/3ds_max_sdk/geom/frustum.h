/**********************************************************************
*<
	FILE: frustum.h

	DESCRIPTION: Frustum and scene management class

	CREATED BY: Wenle Wang

	HISTORY:

		Feb 2007 - Created

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/

#pragma once
#include "GeomExport.h"
#include "Point3.h"
#include "Point2.h"
#include "Box3.h"
#include "Matrix3.h"

// Plane equation in 3D
struct FPlane
{
	Point3 Normal;
	float Distance;

	float CalcDistance(Point3& inP) const
	{
		return inP % Normal + Distance;
	}
};

struct FSphere
{
	Point3 Center;
	float Radius;
};

enum IntersectionCode
{
	IC_FullyIn = 1,
	IC_FullyOut = 2,
	IC_InOut = 3,
};


/*
===================================================
	Frustum
===================================================
*/

struct GEOMEXPORT Frustum
{
	enum FrustumPlane
	{
		FP_Near = 0,
		FP_Far,
		FP_Left,
		FP_Right,
		FP_Top,
		FP_Bottom,

		FP_Num,
	};

	Point3 m_posEye;
	Point3 m_dirEye;
	Point3 m_dirUp;

	Point2 m_centerBias;
	Point2 m_centerScale;

	float m_fNearClip;
	float m_fFarClip;

	bool m_bPerspective;
	bool m_bEnaBiasScale;

	// For perspective projection
	float m_fFovX;
	float m_fFovY;
	float m_fAspect;

	// For ortho projection
	float m_fWidth, m_fHeight;


	//
	// In order:
	//	Near -	UpperLeft
	//			UpperRight
	//			LowerLeft
	//			LowerRight

	//	Far  -	UpperLeft
	//			UpperRight
	//			LowerLeft
	//			LowerRight
	//
	Point3 m_Vertex[8];

	// In order of FrustumPlane
	// Normal pointing to inside of frustum
	FPlane m_Plane[6];


	// Dummy constructor
	Frustum()
	{
		m_bPerspective = true;
	}

	// Create frustum from viewport settings
	Frustum(bool bPerspective, float fNear, float fFar, Matrix3& matAffine, float fFOV, float fAspect, float fWidth,
			float fHeight);

	// Create a ortho frustum
	Frustum(Point3& Origin, Point3& Direction, Point3& Up, float Width, float Height, float Length);

	// Create a frustum to tightly encapsulate the given AABB
	Frustum(Point3& Origin, Point3& Direction, Point3& Up, Box3& AABB);

	// Create a frustum to tightly encapsulate the given FRUSTUM
	Frustum(Point3& Origin, Point3& Direction, Point3& Up, Frustum& Fru);

	// Shrink the given frustum, to make it as tight as possible, according to the scene AABB info
	Frustum(Frustum& Fru, Box3& AABB);

	// Given camera settings and an array of vertices, calculate the tight bounding frustum.
	Frustum(Point3& Origin, Point3& Direction, Point3& Up, Point3 Vertices[], int nVertex);

	// Given camera position and an array of vertices, calculate the tight bounding frustum.
	Frustum(Point3& Origin, Point3 Vertices[], int nVertex, bool x_major);

	// Create a frustum by setting fovx/fovy/near/far
	Frustum(Point3& Origin, Point3& Direction, Point3& Up, float FovX, float FovY, float NearClip, float FarClip);


	// When all the other parameters are ready, get Vertex & Plane
	void UpdateDerivedData();


	// Both the matrices are row-majored
	//
	// Max projection transformation convention is looking at -Z, up is +Y, right is +X, right-handed,
	// After projection transformation, Z value is clamped into [0,1], where (0,0,-f) maps to (0,0,1), (0,0,-n) maps to
	// (0,0,0)
	void GetViewMatrix(float Matrix[4][4]) const;
	void GetProjMatrix(float Matrix[4][4]) const;

	// Check whether frustum intersects with a sphere
	// return IntersectionCode
	int IntersectSphere(FSphere& Sphere) const;

	// Check whether frustum intersects with an OBB
	// return IntersectionCode
	int IntersectOBB(Box3& LocalB, const Matrix3& LocalToWorld) const;

	// Check whether frustum intersects with an AABB
	// return IntersectionCode
	int IntersectAABB(Box3& AABB) const;


	// Fast versions, return binary result: TRUE for (potential) intersection
	bool IntersectSphereFast(FSphere& Sphere) const;
	bool IntersectOBBFast(Box3& LocalB, const Matrix3& LocalToWorld) const;
	bool IntersectAABBFast(Box3& AABB) const;
};
