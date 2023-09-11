//**************************************************************************/
// Copyright (c) 1998-2010 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once
#define ADSK_MAXSDK_NO_POD_TYPE_CHECK
#ifndef ADSK_MAXSDK_NO_POD_TYPE_CHECK
/** Template class that allows for compile time testing whether 
	the template's type argument is a plain old data (POD).
	A POD is a class or Union with no user defined constructor or
	destructor. With no private or protected non-static members,
	no base classes, and no virtual functions. 
	See the C++ standard, sections 8.5.1/1, 9/4, and 3.9/10 
	for more information on PODs.
	The implementation of this compile time constraint uses 
	the fact that non-POD types cannot be members of a union.
	Instantiating MaxSDK::Util::IsPlainOldDataType with a non-POD type will yield a 
	compiler error.
	\code
		MaxSDK::Util::IsPlainOldDataType<bool>(); // no compiler time error, bool is a POD
		MaxSDK::Util::IsPlainOldDataType<Point2>(); // error C2621: member 'MaxSDK::Util::IsPlainOldDataType<T>::t' 
			//of union 'MaxSDK::Util::IsPlainOldDataType<T>::<unnamed-tag>' has copy constructor with [T=Point2]
	\endcode
	MaxSDK::Util::IsPlainOldDataType can be instantiated at global, namespace, class or function scope. */
namespace MaxSDK
{
	namespace Util
	{
		template<typename T> struct IsPlainOldDataType
		{ 
			union 
			{
				T v; // T must be a 'Plain Old Data' (i.e. POD) type
			} u;
		};
	}
}

/** The following 3ds Max SDK types are considered to 
	behave like plain old data although they aren't PODs. */
class Point2;
class IPoint2;
class Point3;
class Point4;
class Matrix3;
class Quat;
class ScaleValue;
class UVVertSet;
class Interval;
class TrackHitRecord;
struct Edge;
struct Vedge;
// possibly take out at a later time
class ParamRef;
class TVFace;
class FaceRemap;
class VertMove;
class AdjFace;
class FaceCreate;
class FaceChange;
class FaceSmooth;
class ExtrudeData;
struct DepIterData;
class SortableID;
// private internal classes
class TCBQuatKey;
template <class T,int ELS> class HybridKey;
template <class T,int ELS> class TCBKey;
template <class T,int ELS, class K> class TCBKeyTable;
class HPoint2Key;
class HPoint3Key;
class HPoint4Key;
class IKPosKey;
template <class T,int ELS, class K> class HybridKeyTable;
template <class T,int ELS> class LinearKey;
template <class T,int ELS, class K> class LinearKeyTable;
class KeyCallbackParams;
class SegCallbackParams;

template<> struct MaxSDK::Util::IsPlainOldDataType< Point2> { };
template<> struct MaxSDK::Util::IsPlainOldDataType< IPoint2> { };
template<> struct MaxSDK::Util::IsPlainOldDataType< Point3> { };
template<> struct MaxSDK::Util::IsPlainOldDataType< Point4> { };
template<> struct MaxSDK::Util::IsPlainOldDataType< Matrix3> { };
template<> struct MaxSDK::Util::IsPlainOldDataType< Quat> { };
template<> struct MaxSDK::Util::IsPlainOldDataType< ScaleValue> { };
template<> struct MaxSDK::Util::IsPlainOldDataType< const char> { };
template<> struct MaxSDK::Util::IsPlainOldDataType< UVVertSet> { };
template<> struct MaxSDK::Util::IsPlainOldDataType< Interval> { };
template<> struct MaxSDK::Util::IsPlainOldDataType< TrackHitRecord> { };
template<> struct MaxSDK::Util::IsPlainOldDataType< Edge> { };
template<> struct MaxSDK::Util::IsPlainOldDataType< Vedge> { };
// possibly take out at a later time
template<> struct MaxSDK::Util::IsPlainOldDataType< ParamRef    > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< TVFace      > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< FaceRemap   > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< VertMove    > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< AdjFace     > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< FaceCreate  > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< FaceChange  > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< FaceSmooth  > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< ExtrudeData > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< DepIterData > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< SortableID > { };
// private internal class
template<> struct MaxSDK::Util::IsPlainOldDataType< TCBQuatKey                                     > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< KeyCallbackParams                              > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< SegCallbackParams                              > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< HybridKey<Quat,4>                              > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< HybridKey<Point3,3>                            > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< HybridKey<float,1>                             > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< HybridKey<ScaleValue,3>                        > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< TCBKey<ScaleValue,3>                           > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< HPoint3Key                                     > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< HybridKeyTable<Point3,3,HPoint3Key>            > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< LinearKey<float,1>                             > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< LinearKeyTable<float,1,LinearKey<float,1>>     > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< LinearKey<Point3,3>                            > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< LinearKeyTable<Point3,3,LinearKey<Point3,3>>   > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< LinearKey<Quat,4>                              > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< LinearKeyTable<Quat,4,LinearKey<Quat,4>>       > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< LinearKey<ScaleValue,3>                        > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< LinearKeyTable<ScaleValue,3,LinearKey<ScaleValue,3>>   > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< HPoint4Key                                     > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< HybridKeyTable<Point4,4,HPoint4Key>            > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< HPoint2Key                                     > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< HybridKeyTable<Point2,2,HPoint2Key>            > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< TCBKey<float,1>                                > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< TCBKey<Point3,3>                               > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< TCBKey<Point4,4>                               > { };
template<> struct MaxSDK::Util::IsPlainOldDataType< IKPosKey                                       > { };

#endif

#ifndef ADSK_MAXSDK_NO_POD_TYPE_CHECK
	#define ADSK_MAXSDK_IS_PLAIN_OLD_DATA_TYPE(_Typename) MaxSDK::Util::IsPlainOldDataType<_Typename>()
#else
	#define ADSK_MAXSDK_IS_PLAIN_OLD_DATA_TYPE(_Typename)
#endif