//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//

#pragma once

// Parameter types
//    Several of these types fold to the same underlying value type but provide
//    either a more complete description of the type or imply non-standard dimensions that are
//    automatically applied.

#define TYPE_TAB    0x0800      //!< Flags a parameter type as a Tab<>

/// \name Function Publishing Type Modification Flags
//!@{
#define TYPE_BY_REF 0x1000      //!< Flags a parameter as being delivered by reference (&).
#define TYPE_BY_VAL 0x2000      //!< Flags a parameter as being delivered by value (via a local copy owned by the FPValue).
#define TYPE_BY_PTR 0x4000      //!< Flags a parameter as being delivered by pointer (*)
//!@}

/** Identifies a parameter type.
* These enums identify types of parameters (or properties) that can be stored in parameter block and 
* parameter block2, and can be used as argument types and return value types for function publishing.
 * \see ParamBlockDesc2, ParamType2*/
enum ParamType
{
	TYPE_FLOAT,     //!< Identifies a floating point parameter type.
	TYPE_INT,       //!< Identifies an integer parameter type.
	TYPE_RGBA,      //!< Identifies a Color parameter type. \note Does not contain an alpha channel despite the name.
	TYPE_POINT3,    //!< Identifies a Point3 parameter type.
	TYPE_BOOL,      //!< Identifies a BOOL parameter type.
	TYPE_USER,      //!< Used to define user types.
};

/** Identifies a parameter type. ParamType values are compatible with ParamType2.
 * These enums identify types of parameters (or properties) that can be stored in parameter block2 and
 * can be used as argument types and return value types for function publishing.
 * \li The enumerated types from "TYPE_ENUM" to "TYPE_DOUBLE" are for published function parameter types only, not for pblock2 parameter types.
 * \li The enumerated types from "TYPE_FLOAT_TAB" to "TYPE_FRGBA_TAB" are the tables (Tab<>s) of their base types, and must be in same order as the base types.
 * \li The enumerated types from "TYPE_ENUM_TAB" to "TYPE_DOUBLE_TAB" are for published function parameter types only, not for pblock2 parameter types.
 * \li The enumerated types from "TYPE_FLOAT_BR" to "TYPE_DOUBLE_BR" are pass by-ref types, implies & parameters, int& and float& are passed via .ptr fields, only for FnPub use.
 * \li The enumerated types from "TYPE_FLOAT_TAB_BR" to "TYPE_DOUBLE_TAB_BR" are pass by-ref Tab<> types - tab is passed by ref, data type in tab is TYPE_XXXX.
 * For example: TYPE_FLOAT_TAB_BR corresponds to Tab<float>& and TYPE_INODE_TAB_BR_TYPE corresponds to Tab<INode*>&.
 * \li The enumerated types from "TYPE_RGBA_BV" to "TYPE_CLASS_BV" are pass by-value types, implies dereferencing the (meaningful) pointer-based values, only for FnPub use.
 * \li The enumerated types from "TYPE_FLOAT_TAB_BV" to "TYPE_DOUBLE_TAB_BV" are pass by-val Tab<> types - tab is passed by val, data type in tab is TYPE_XXXX.
 * For example: TYPE_FLOAT_TAB_BV corresponds to Tab<float> and TYPE_INODE_TAB_BV corresponds to Tab<INode*>.
 * \li The enumerated types from "TYPE_FLOAT_BP" to "TYPE_DOUBLE_BP" are pass by-pointer types for int and float types, implies * parameters, int* and float* are passed via .ptr fields, only for FnPub use.
 * \li The enumerated types from "TYPE_KEYARG_MARKER" to "TYPE_UNSPECIFIED" are MAXScript internal types.
 * \see ParamBlockDesc2, ParamType
 */
enum ParamType2 
{
	// Base types
	// -----------------------------------------------------------------------

	//TYPE_FLOAT,
	//TYPE_INT,
	//TYPE_RGBA,
	//TYPE_POINT3,
	//TYPE_BOOL,
	TYPE_ANGLE = TYPE_BOOL + 1, //!< A floating point value with an implied stdAngleDim dimension.
	TYPE_PCNT_FRAC,             //!< A floating point value with an implied stdPercentDim dimension.
	TYPE_WORLD,                 //!< A floating point value that represents world distance units. This implies a parameter dimension of stdWorldDim.
	TYPE_STRING,                //!< A character string. The string has a local copy made and managed by the parameter block.
	TYPE_FILENAME,              //!< Used to identify file names (const MCHAR*). An AssetType should also be specified - see ParamTags.p_assetTypeID and ParamTags.p_assetTypeName.
	TYPE_HSV,                   //!< This type has been deprecated as of 3ds Max 2022. Use TYPE_RGBA instead.
	TYPE_COLOR_CHANNEL,         //!< A single floating point value with an implied stdColor255Dim dimension.
	TYPE_TIMEVALUE,             //!< A single integer value used as a TimeValue. This implies a stdTimeDim dimension.
	TYPE_RADIOBTN_INDEX,        //!< This type is used as an integer for parameters represented as radio buttons in the UI.
	TYPE_MTL,                   //!< A pointer to a Mtl (material) object. This can be one of three types: a reference owned by the parameter block, a reference owned by the block owner, or no reference management (just a copy of the pointer).
	TYPE_TEXMAP,                //!< A pointer to a Texmap (texture map) object. This can be one of three types: a reference owned by the parameter block, a reference owned by the block owner, or no reference management (just a copy of the pointer). 
	TYPE_BITMAP,                //!< A pointer to a PBBitmap (bitmap) object. 
	TYPE_INODE,                 //!< A pointer to an INode. This can be one of three types: a reference owned by the parameter block, a reference owned by the block owner, or no reference management (just a copy of the pointer). 
	TYPE_REFTARG,               //!< A pointer to a ReferenceTarget object. All reference targets in this group can be one of three types: reference owned by parameter block, reference owned by block owner, or no reference management (just a copy of the pointer).

	TYPE_INDEX,                 //!< Used for parameters that are 0-based, but exposed to MAXScript as 1-based. For example, a vertex index.
	TYPE_MATRIX3,               //!< Identifies a Matrix3 parameter type.
	TYPE_PBLOCK2,               //!< A pointer to an IParamBlock2 object. Note that "TYPE_PBLOCK2_TYPE" is not defined.

	TYPE_POINT4,                //!< Identifies a Point4 parameter type.
	TYPE_FRGBA,                 //!< Identifies an AColor parameter type.

	TYPE_RESERVED_1,            //!< Reserved type.
	TYPE_RESERVED_2,            //!< Reserved type.
	TYPE_RESERVED_3,            //!< Reserved type.
	TYPE_RESERVED_4,            //!< Reserved type.
	TYPE_RESERVED_5,            //!< Reserved type.
	TYPE_RESERVED_6,            //!< Reserved type.

	TYPE_POINT2,                //!< Identifies a Point2 parameter type.

	TYPE_RESERVED_7,            //!< Reserved type.
	TYPE_RESERVED_8,            //!< Reserved type.
	TYPE_RESERVED_9,            //!< Reserved type.
	TYPE_RESERVED_10,           //!< Reserved type.
	TYPE_RESERVED_11,           //!< Reserved type.
	TYPE_RESERVED_12,           //!< Reserved type.
	TYPE_RESERVED_13,           //!< Reserved type.
	TYPE_RESERVED_14,           //!< Reserved type.
	TYPE_RESERVED_15,           //!< Reserved type.
	TYPE_RESERVED_16,           //!< Reserved type.
	TYPE_RESERVED_17,           //!< Reserved type.
	TYPE_RESERVED_18,           //!< Reserved type.
	TYPE_RESERVED_19,           //!< Reserved type.
	TYPE_RESERVED_20,           //!< Reserved type.

	// Only for published function parameter types, not for pblock2 parameter types
	TYPE_ENUM,					//!< Identifies an enum argument type or return value.
	TYPE_VOID,					//!< Identifies a void return value.
	TYPE_INTERVAL,				//!< Identifies an Interval argument type or return value.
	TYPE_ANGAXIS,				//!< Identifies an AngAxis argument type or return value.
	TYPE_QUAT,					//!< Identifies an Quat argument type or return value.
	TYPE_RAY,					//!< Identifies a Ray argument type or return value.
	TYPE_BITARRAY,				//!< Identifies a BitArray argument type or return value.
	TYPE_CLASS,					//!< Identifies a ClassDesc argument type or return value.
	TYPE_MESH,					//!< Identifies a Mesh argument type or return value.
	TYPE_OBJECT,				//!< Identifies an Object argument type or return value.
	TYPE_CONTROL,				//!< Identifies a Control argument type or return value.
	TYPE_POINT,					//!< Identifies a Win32 POINT argument type or return value.
	TYPE_TSTR,					//!< Identifies a TSTR argument type or return value.
	TYPE_IOBJECT,				//!< Identifies an IObject argument type or return value.
	TYPE_INTERFACE,				//!< Identifies a FPInterface argument type or return value.
	TYPE_HWND,					//!< Identifies a HWND argument type or return value.
	TYPE_NAME,					//!< Identifies a character string argument type or return value that corresponds to a MAXSCript name value (case insensitive string).
	TYPE_COLOR,					//!< Identifies a Color argument type or return value.
	TYPE_FPVALUE,				//!< Identifies an FPValue argument type or return value.
	TYPE_VALUE,					//!< Identifies a MAXSCript value argument type or return value.
	TYPE_DWORD,					//!< Identifies a DWORD argument type or return value.
	TYPE_bool,					//!< Identifies a bool argument type or return value.
	TYPE_INTPTR,				//!< Identifies an INT_PTR argument type or return value.
	TYPE_INT64,					//!< Identifies an INT64 argument type or return value.
	TYPE_DOUBLE,				//!< Identifies a double argument type or return value.
	TYPE_BOX3,					//!< Identifies a Box3 argument type or return value.
	TYPE_BEZIERSHAPE,			//!< Identifies a BezierShape argument type or return value.

	// Tab<>s
	// -----------------------------------------------------------------------

	TYPE_FLOAT_TAB = TYPE_FLOAT + TYPE_TAB, //!< A table of TYPE_FLOAT values. See TYPE_FLOAT.
	TYPE_INT_TAB,                           //!< A table of TYPE_INT values. See TYPE_INT.
	TYPE_RGBA_TAB,                          //!< A table of TYPE_RGBA values. See TYPE_RGBA.
	TYPE_POINT3_TAB,                        //!< A table of TYPE_POINT3 values. See TYPE_POINT3.
	TYPE_BOOL_TAB,                          //!< A table of TYPE_BOOL values. See TYPE_BOOL.
	TYPE_ANGLE_TAB,                         //!< A table of TYPE_ANGLE values. See TYPE_ANGLE.
	TYPE_PCNT_FRAC_TAB,                     //!< A table of TYPE_PCNT_FRAC values. See TYPE_PCNT_FRAC.
	TYPE_WORLD_TAB,                         //!< A table of TYPE_WORLD values. See TYPE_WORLD.
	TYPE_STRING_TAB,                        //!< A table of TYPE_STRING values. See TYPE_STRING.
	TYPE_FILENAME_TAB,                      //!< A table of TYPE_FILENAME values. See TYPE_FILENAME.
	TYPE_HSV_TAB,                           //!< This type has been deprecated as of 3ds Max 2022. Use TYPE_RGBA_TAB instead.
	TYPE_COLOR_CHANNEL_TAB,                 //!< A table of TYPE_COLOR_CHANNEL values. See TYPE_COLOR_CHANNEL.
	TYPE_TIMEVALUE_TAB,                     //!< A table of TYPE_TIMEVALUE values. See TYPE_TIMEVALUE.
	TYPE_RADIOBTN_INDEX_TAB,                //!< A table of TYPE_RADIOBTN_INDEX_TAB values. See TYPE_RADIOBTN_INDEX_TAB.
	TYPE_MTL_TAB,                           //!< A table of TYPE_MTL values. See TYPE_MTL.
	TYPE_TEXMAP_TAB,                        //!< A table of TYPE_TEXMAP values. See TYPE_TEXMAP.
	TYPE_BITMAP_TAB,                        //!< A table of TYPE_BITMAP values. See TYPE_BITMAP.
	TYPE_INODE_TAB,                         //!< A table of TYPE_INODE values. See TYPE_INODE.
	TYPE_REFTARG_TAB,                       //!< A table of TYPE_REFTARG values. See TYPE_REFTARG.
	TYPE_INDEX_TAB,                         //!< A table of TYPE_INDEX values. See TYPE_INDEX.
	TYPE_MATRIX3_TAB,                       //!< A table of TYPE_MATRIX3 values. See TYPE_MATRIX3.
	TYPE_PBLOCK2_TAB,                       //!< A table of TYPE_BLOCK2 values. See TYPE_BLOCK2.
	TYPE_POINT4_TAB,                        //!< A table of TYPE_POINT4 values. See TYPE_POINT4.
	TYPE_FRGBA_TAB,                         //!< A table of TYPE_FRGBA values. See TYPE_FRGBA.

	TYPE_RESERVED_1_TAB,                    //!< Reserved type.
	TYPE_RESERVED_2_TAB,                    //!< Reserved type.
	TYPE_RESERVED_3_TAB,                    //!< Reserved type.
	TYPE_RESERVED_4_TAB,                    //!< Reserved type.
	TYPE_RESERVED_5_TAB,                    //!< Reserved type.
	TYPE_RESERVED_6_TAB,                    //!< Reserved type.

	TYPE_POINT2_TAB,                        //!< A table of TYPE_POINT2 values.

	// All ParamType values for pblock2 parameter types should be sequential here, so reserving room for future additions
	TYPE_RESERVED_7_TAB,                    //!< Reserved type.
	TYPE_RESERVED_8_TAB,                    //!< Reserved type.
	TYPE_RESERVED_9_TAB,                    //!< Reserved type.
	TYPE_RESERVED_10_TAB,                   //!< Reserved type.
	TYPE_RESERVED_11_TAB,                   //!< Reserved type.
	TYPE_RESERVED_12_TAB,                   //!< Reserved type.
	TYPE_RESERVED_13_TAB,                   //!< Reserved type.
	TYPE_RESERVED_14_TAB,                   //!< Reserved type.
	TYPE_RESERVED_15_TAB,                   //!< Reserved type.
	TYPE_RESERVED_16_TAB,                   //!< Reserved type.
	TYPE_RESERVED_17_TAB,                   //!< Reserved type.
	TYPE_RESERVED_18_TAB,                   //!< Reserved type.
	TYPE_RESERVED_19_TAB,                   //!< Reserved type.
	TYPE_RESERVED_20_TAB,                   //!< Reserved type.

	// Only for published function parameter types, not for pblock2 parameter types.
	TYPE_ENUM_TAB,                          //!< A table of TYPE_ENUM values. See TYPE_ENUM.
	TYPE_VOID_TAB,                          //!< A table of TYPE_VOID values. See TYPE_VOID.
	TYPE_INTERVAL_TAB,                      //!< A table of TYPE_INTERVAL values. See TYPE_INTERVAL.
	TYPE_ANGAXIS_TAB,                       //!< A table of TYPE_ANGAXIS values. See TYPE_ANGAXIS.
	TYPE_QUAT_TAB,                          //!< A table of TYPE_QUAT values. See TYPE_QUAT.
	TYPE_RAY_TAB,                           //!< A table of TYPE_RAY values. See TYPE_RAY.
	TYPE_BITARRAY_TAB,                      //!< A table of TYPE_BITARRAY values. See TYPE_BITARRAY.
	TYPE_CLASS_TAB,                         //!< A table of TYPE_CLASS values. See TYPE_CLASS.
	TYPE_MESH_TAB,                          //!< A table of TYPE_MESH values. See TYPE_MESH.
	TYPE_OBJECT_TAB,                        //!< A table of TYPE_OBJECT values. See TYPE_OBJECT.
	TYPE_CONTROL_TAB,                       //!< A table of TYPE_CONTROL values. See TYPE_CONTROL.
	TYPE_POINT_TAB,                         //!< A table of TYPE_POINT values. See TYPE_POINT.
	TYPE_TSTR_TAB,                          //!< A table of TYPE_TSTR values. See TYPE_TSTR.
	TYPE_IOBJECT_TAB,                       //!< A table of TYPE_IOBJECT values. See TYPE_IOBJECT.
	TYPE_INTERFACE_TAB,                     //!< A table of TYPE_INTERFACE values. See TYPE_INTERFACE.
	TYPE_HWND_TAB,                          //!< A table of TYPE_HWND values. See TYPE_HWND.
	TYPE_NAME_TAB,                          //!< A table of TYPE_NAME values. See TYPE_NAME.
	TYPE_COLOR_TAB,                         //!< A table of TYPE_COLOR values. See TYPE_COLOR.
	TYPE_FPVALUE_TAB,                       //!< A table of TYPE_FPVALUE values. See TYPE_FPVALUE.
	TYPE_VALUE_TAB,                         //!< A table of TYPE_VALUE values. See TYPE_VALUE.
	TYPE_DWORD_TAB,                         //!< A table of TYPE_DWORD values. See TYPE_DWORD.
	TYPE_bool_TAB,                          //!< A table of TYPE_bool values. See TYPE_bool.
	TYPE_INTPTR_TAB,                        //!< A table of TYPE_INTPTR values. See TYPE_INTPTR.
	TYPE_INT64_TAB,                         //!< A table of TYPE_INT64 values. See TYPE_INT64.
	TYPE_DOUBLE_TAB,                        //!< A table of TYPE_DOUBLE values. See TYPE_DOUBLE.
	TYPE_BOX3_TAB,                          //!< A table of TYPE_BOX3 values. See TYPE_BOX3.
	TYPE_BEZIERSHAPE_TAB,                   //!< A table of TYPE_BEZIERSHAPE values. See TYPE_BEZIERSHAPE.

	// Pass by reference types
	// -----------------------------------------------------------------------

	// Note: implies & parameters, int& and float& are passed via .ptr fields, only for FnPub use

	TYPE_FLOAT_BR                   = TYPE_FLOAT + TYPE_BY_REF,          //!< A by-reference TYPE_FLOAT value. See TYPE_FLOAT.
	TYPE_INT_BR                     = TYPE_INT + TYPE_BY_REF,            //!< A by-reference TYPE_INT value. See TYPE_INT.
	TYPE_BOOL_BR                    = TYPE_BOOL + TYPE_BY_REF,           //!< A by-reference TYPE_BOOL value. See TYPE_BOOL.
	TYPE_ANGLE_BR                   = TYPE_ANGLE + TYPE_BY_REF,          //!< A by-reference TYPE_ANGLE value. See TYPE_ANGLE.
	TYPE_PCNT_FRAC_BR               = TYPE_PCNT_FRAC + TYPE_BY_REF,      //!< A by-reference TYPE_PCNT_FRAC value. See TYPE_PCNT_FRAC.
	TYPE_WORLD_BR                   = TYPE_WORLD + TYPE_BY_REF,          //!< A by-reference TYPE_WORLD value. See TYPE_WORLD.
	TYPE_COLOR_CHANNEL_BR           = TYPE_COLOR_CHANNEL + TYPE_BY_REF,  //!< A by-reference TYPE_COLOR_CHANNEL value. See TYPE_COLOR_CHANNEL.
	TYPE_TIMEVALUE_BR               = TYPE_TIMEVALUE + TYPE_BY_REF,      //!< A by-reference TYPE_TIMEVALUE value. See TYPE_TIMEVALUE.
	TYPE_RADIOBTN_INDEX_BR          = TYPE_RADIOBTN_INDEX + TYPE_BY_REF, //!< A by-reference TYPE_RADIOBTN_INDEX value. See TYPE_RADIOBTN_INDEX.
	TYPE_INDEX_BR                   = TYPE_INDEX + TYPE_BY_REF,          //!< A by-reference TYPE_INDEX value. See TYPE_INDEX.
	TYPE_RGBA_BR                    = TYPE_RGBA + TYPE_BY_REF,           //!< A by-reference TYPE_RGBA value. See TYPE_RGBA.
	TYPE_BITMAP_BR                  = TYPE_BITMAP + TYPE_BY_REF,         //!< A by-reference TYPE_BITMAP value. See TYPE_BITMAP.
	TYPE_POINT3_BR                  = TYPE_POINT3 + TYPE_BY_REF,         //!< A by-reference TYPE_POINT3 value. See TYPE_POINT3.
	TYPE_HSV_BR                     = TYPE_HSV + TYPE_BY_REF,            //!< This type has been deprecated as of 3ds Max 2022. Use TYPE_RGBA_BR instead.
	TYPE_REFTARG_BR                 = TYPE_REFTARG + TYPE_BY_REF,        //!< A by-reference TYPE_REFTARG value. See TYPE_REFTARG.
	TYPE_MATRIX3_BR                 = TYPE_MATRIX3 + TYPE_BY_REF,        //!< A by-reference TYPE_MATRIX3 value. See TYPE_MATRIX3.
	TYPE_POINT4_BR                  = TYPE_POINT4 + TYPE_BY_REF,         //!< A by-reference TYPE_POINT4 value. See TYPE_POINT4.
	TYPE_FRGBA_BR                   = TYPE_FRGBA + TYPE_BY_REF,          //!< A by-reference TYPE_FRGBA value. See TYPE_FRGBA.
	TYPE_ENUM_BR                    = TYPE_ENUM + TYPE_BY_REF,           //!< A by-reference TYPE_ENUM value. See TYPE_ENUM.
	TYPE_INTERVAL_BR                = TYPE_INTERVAL + TYPE_BY_REF,       //!< A by-reference TYPE_INTERVAL value. See TYPE_INTERVAL.
	TYPE_ANGAXIS_BR                 = TYPE_ANGAXIS + TYPE_BY_REF,        //!< A by-reference TYPE_ANGAXIS value. See TYPE_ANGAXIS.
	TYPE_QUAT_BR                    = TYPE_QUAT + TYPE_BY_REF,           //!< A by-reference TYPE_QUAT value. See TYPE_QUAT.
	TYPE_RAY_BR                     = TYPE_RAY + TYPE_BY_REF,            //!< A by-reference TYPE_RAY value. See TYPE_RAY.
	TYPE_POINT2_BR                  = TYPE_POINT2 + TYPE_BY_REF,         //!< A by-reference TYPE_POINT2 value. See TYPE_POINT2.
	TYPE_BITARRAY_BR                = TYPE_BITARRAY + TYPE_BY_REF,       //!< A by-reference TYPE_BITARRAY value. See TYPE_BITARRAY.
	TYPE_MESH_BR                    = TYPE_MESH + TYPE_BY_REF,           //!< A by-reference TYPE_MESH value. See TYPE_MESH.
	TYPE_POINT_BR                   = TYPE_POINT + TYPE_BY_REF,          //!< A by-reference TYPE_POINT value. See TYPE_POINT.
	TYPE_TSTR_BR                    = TYPE_TSTR + TYPE_BY_REF,           //!< A by-reference TYPE_TSTR value. See TYPE_TSTR.
	TYPE_COLOR_BR                   = TYPE_COLOR + TYPE_BY_REF,          //!< A by-reference TYPE_COLOR value. See TYPE_COLOR.
	TYPE_FPVALUE_BR                 = TYPE_FPVALUE + TYPE_BY_REF,        //!< A by-reference TYPE_FPVALUE value. See TYPE_FPVALUE.
	TYPE_DWORD_BR                   = TYPE_DWORD + TYPE_BY_REF,          //!< A by-reference TYPE_DWORD value. See TYPE_DWORD.
	TYPE_bool_BR                    = TYPE_bool + TYPE_BY_REF,           //!< A by-reference TYPE_bool value. See TYPE_bool.
	TYPE_INTPTR_BR                  = TYPE_INTPTR + TYPE_BY_REF,         //!< A by-reference TYPE_INTPTR value. See TYPE_INTPTR.
	TYPE_INT64_BR                   = TYPE_INT64 + TYPE_BY_REF,          //!< A by-reference TYPE_INT64 value. See TYPE_INT64.
	TYPE_DOUBLE_BR                  = TYPE_DOUBLE + TYPE_BY_REF,         //!< A by-reference TYPE_DOUBLE value. See TYPE_DOUBLE.
	TYPE_BOX3_BR                    = TYPE_BOX3 + TYPE_BY_REF,           //!< A by-reference TYPE_BOX3 value. See TYPE_BOX3.
	TYPE_BEZIERSHAPE_BR             = TYPE_BEZIERSHAPE + TYPE_BY_REF,    //!< A by-reference TYPE_BEZIERSHAPE value. See TYPE_BEZIERSHAPE.

	// Pass by reference Tab<>s
	// -----------------------------------------------------------------------

	// Note:
	// Tab is passed by ref, data type in tab is TYPE_XXXX. For example:
	// TYPE_FLOAT_TAB_BR      = Tab<float>&
	// TYPE_INODE_TAB_BR_TYPE = Tab<INode*>&

	TYPE_FLOAT_TAB_BR               = TYPE_FLOAT + TYPE_TAB + TYPE_BY_REF,          //!< A by-reference table of TYPE_FLOAT values. See TYPE_FLOAT.
	TYPE_INT_TAB_BR                 = TYPE_INT + TYPE_TAB + TYPE_BY_REF,            //!< A by-reference table of TYPE_INT values. See TYPE_INT.
	TYPE_RGBA_TAB_BR                = TYPE_RGBA + TYPE_TAB + TYPE_BY_REF,           //!< A by-reference table of TYPE_RGBA values. See TYPE_RGBA.
	TYPE_POINT3_TAB_BR              = TYPE_POINT3 + TYPE_TAB + TYPE_BY_REF,         //!< A by-reference table of TYPE_POINT3 values. See TYPE_POINT3.
	TYPE_BOOL_TAB_BR                = TYPE_BOOL + TYPE_TAB + TYPE_BY_REF,           //!< A by-reference table of TYPE_BOOL values. See TYPE_BOOL.
	TYPE_ANGLE_TAB_BR               = TYPE_ANGLE + TYPE_TAB + TYPE_BY_REF,          //!< A by-reference table of TYPE_ANGLE values. See TYPE_ANGLE.
	TYPE_PCNT_FRAC_TAB_BR           = TYPE_PCNT_FRAC + TYPE_TAB + TYPE_BY_REF,      //!< A by-reference table of TYPE_PCNT_FRAC values. See TYPE_PCNT_FRAC.
	TYPE_WORLD_TAB_BR               = TYPE_WORLD + TYPE_TAB + TYPE_BY_REF,          //!< A by-reference table of TYPE_WORLD values. See TYPE_WORLD.
	TYPE_STRING_TAB_BR              = TYPE_STRING + TYPE_TAB + TYPE_BY_REF,         //!< A by-reference table of TYPE_STRING values. See TYPE_STRING.
	TYPE_FILENAME_TAB_BR            = TYPE_FILENAME + TYPE_TAB + TYPE_BY_REF,       //!< A by-reference table of TYPE_FILENAME values. See TYPE_FILENAME.
	TYPE_HSV_TAB_BR                 = TYPE_HSV + TYPE_TAB + TYPE_BY_REF,            //!< This type has been deprecated as of 3ds Max 2022. Use TYPE_RGBA_TAB_BR instead.
	TYPE_COLOR_CHANNEL_TAB_BR       = TYPE_COLOR_CHANNEL + TYPE_TAB + TYPE_BY_REF,  //!< A by-reference table of TYPE_COLOR_CHANNEL values. See TYPE_COLOR_CHANNEL.
	TYPE_TIMEVALUE_TAB_BR           = TYPE_TIMEVALUE + TYPE_TAB + TYPE_BY_REF,      //!< A by-reference table of TYPE_TIMEVALUE values. See TYPE_TIMEVALUE.
	TYPE_RADIOBTN_INDEX_TAB_BR      = TYPE_RADIOBTN_INDEX + TYPE_TAB + TYPE_BY_REF, //!< A by-reference table of TYPE_RADIOBTN_INDEX values. See TYPE_RADIOBTN_INDEX.
	TYPE_MTL_TAB_BR                 = TYPE_MTL + TYPE_TAB + TYPE_BY_REF,            //!< A by-reference table of TYPE_MTL values. See TYPE_MTL.
	TYPE_TEXMAP_TAB_BR              = TYPE_TEXMAP + TYPE_TAB + TYPE_BY_REF,         //!< A by-reference table of TYPE_TEXMAP values. See TYPE_TEXMAP.
	TYPE_BITMAP_TAB_BR              = TYPE_BITMAP + TYPE_TAB + TYPE_BY_REF,         //!< A by-reference table of TYPE_BITMAP values. See TYPE_BITMAP.
	TYPE_INODE_TAB_BR               = TYPE_INODE + TYPE_TAB + TYPE_BY_REF,          //!< A by-reference table of TYPE_INODE values. See TYPE_INODE.
	TYPE_REFTARG_TAB_BR             = TYPE_REFTARG + TYPE_TAB + TYPE_BY_REF,        //!< A by-reference table of TYPE_REFTARG values. See TYPE_REFTARG.
	TYPE_INDEX_TAB_BR               = TYPE_INDEX + TYPE_TAB + TYPE_BY_REF,          //!< A by-reference table of TYPE_INDEX values. See TYPE_INDEX.
	TYPE_MATRIX3_TAB_BR             = TYPE_MATRIX3 + TYPE_TAB + TYPE_BY_REF,        //!< A by-reference table of TYPE_MATRIX3 values. See TYPE_MATRIX3.
	TYPE_POINT4_TAB_BR              = TYPE_POINT4 + TYPE_TAB + TYPE_BY_REF,         //!< A by-reference table of TYPE_POINT4 values. See TYPE_POINT4.
	TYPE_FRGBA_TAB_BR               = TYPE_FRGBA + TYPE_TAB + TYPE_BY_REF,          //!< A by-reference table of TYPE_FRGBA values. See TYPE_FRGBA.
	TYPE_TSTR_TAB_BR                = TYPE_TSTR + TYPE_TAB + TYPE_BY_REF,           //!< A by-reference table of TYPE_TSTR values. See TYPE_TSTR.
	TYPE_ENUM_TAB_BR                = TYPE_ENUM + TYPE_TAB + TYPE_BY_REF,           //!< A by-reference table of TYPE_ENUM values. See TYPE_ENUM.
	TYPE_INTERVAL_TAB_BR            = TYPE_INTERVAL + TYPE_TAB + TYPE_BY_REF,       //!< A by-reference table of TYPE_INTERVAL values. See TYPE_INTERVAL.
	TYPE_ANGAXIS_TAB_BR             = TYPE_ANGAXIS + TYPE_TAB + TYPE_BY_REF,        //!< A by-reference table of TYPE_ANGAXIS values. See TYPE_ANGAXIS.
	TYPE_QUAT_TAB_BR                = TYPE_QUAT + TYPE_TAB + TYPE_BY_REF,           //!< A by-reference table of TYPE_QUAT values. See TYPE_QUAT.
	TYPE_RAY_TAB_BR                 = TYPE_RAY + TYPE_TAB + TYPE_BY_REF,            //!< A by-reference table of TYPE_RAY values. See TYPE_RAY.
	TYPE_POINT2_TAB_BR              = TYPE_POINT2 + TYPE_TAB + TYPE_BY_REF,         //!< A by-reference table of TYPE_POINT2 values. See TYPE_POINT2.
	TYPE_BITARRAY_TAB_BR            = TYPE_BITARRAY + TYPE_TAB + TYPE_BY_REF,       //!< A by-reference table of TYPE_BITARRAY values. See TYPE_BITARRAY.
	TYPE_CLASS_TAB_BR               = TYPE_CLASS + TYPE_TAB + TYPE_BY_REF,          //!< A by-reference table of TYPE_CLASS values. See TYPE_CLASS.
	TYPE_MESH_TAB_BR                = TYPE_MESH + TYPE_TAB + TYPE_BY_REF,           //!< A by-reference table of TYPE_MESH values. See TYPE_MESH.
	TYPE_OBJECT_TAB_BR              = TYPE_OBJECT + TYPE_TAB + TYPE_BY_REF,         //!< A by-reference table of TYPE_OBJECT values. See TYPE_OBJECT.
	TYPE_CONTROL_TAB_BR             = TYPE_CONTROL + TYPE_TAB + TYPE_BY_REF,        //!< A by-reference table of TYPE_CONTROL values. See TYPE_CONTROL.
	TYPE_POINT_TAB_BR               = TYPE_POINT + TYPE_TAB + TYPE_BY_REF,          //!< A by-reference table of TYPE_POINT values. See TYPE_POINT.
	TYPE_IOBJECT_TAB_BR             = TYPE_IOBJECT + TYPE_TAB + TYPE_BY_REF,        //!< A by-reference table of TYPE_IOBJECT values. See TYPE_IOBJECT.
	TYPE_INTERFACE_TAB_BR           = TYPE_INTERFACE + TYPE_TAB + TYPE_BY_REF,      //!< A by-reference table of TYPE_INTERFACE values. See TYPE_INTERFACE.
	TYPE_HWND_TAB_BR                = TYPE_HWND + TYPE_TAB + TYPE_BY_REF,           //!< A by-reference table of TYPE_HWND values. See TYPE_HWND.
	TYPE_NAME_TAB_BR                = TYPE_NAME + TYPE_TAB + TYPE_BY_REF,           //!< A by-reference table of TYPE_NAME values. See TYPE_NAME.
	TYPE_COLOR_TAB_BR               = TYPE_COLOR + TYPE_TAB + TYPE_BY_REF,          //!< A by-reference table of TYPE_COLOR values. See TYPE_COLOR.
	TYPE_FPVALUE_TAB_BR             = TYPE_FPVALUE + TYPE_TAB + TYPE_BY_REF,        //!< A by-reference table of TYPE_FPVALUE values. See TYPE_FPVALUE.
	TYPE_VALUE_TAB_BR               = TYPE_VALUE + TYPE_TAB + TYPE_BY_REF,          //!< A by-reference table of TYPE_VALUE values. See TYPE_VALUE.
	TYPE_DWORD_TAB_BR               = TYPE_DWORD + TYPE_TAB + TYPE_BY_REF,          //!< A by-reference table of TYPE_DWORD values. See TYPE_DWORD.
	TYPE_bool_TAB_BR                = TYPE_bool + TYPE_TAB + TYPE_BY_REF,           //!< A by-reference table of TYPE_bool values. See TYPE_bool.
	TYPE_INTPTR_TAB_BR              = TYPE_INTPTR + TYPE_TAB + TYPE_BY_REF,         //!< A by-reference table of TYPE_INTPTR values. See TYPE_INTPTR.
	TYPE_INT64_TAB_BR               = TYPE_INT64 + TYPE_TAB + TYPE_BY_REF,          //!< A by-reference table of TYPE_INT64 values. See TYPE_INT64.
	TYPE_DOUBLE_TAB_BR              = TYPE_DOUBLE + TYPE_TAB + TYPE_BY_REF,         //!< A by-reference table of TYPE_DOUBLE values. See TYPE_DOUBLE.
	TYPE_BOX3_TAB_BR                = TYPE_BOX3 + TYPE_TAB + TYPE_BY_REF,           //!< A by-reference table of TYPE_BOX3 values. See TYPE_BOX3.
	TYPE_BEZIERSHAPE_TAB_BR         = TYPE_BEZIERSHAPE + TYPE_TAB + TYPE_BY_REF,    //!< A by-reference table of TYPE_BEZIERSHAPE values. See TYPE_BEZIERSHAPE.

	// Pass by value types
	// -----------------------------------------------------------------------

	// Note: implies dereferencing the (meaningful) pointer-based values, only for FnPub use

	TYPE_RGBA_BV                    = TYPE_RGBA + TYPE_BY_VAL,            //!< A by-value TYPE_RGBA value. See TYPE_RGBA.
	TYPE_POINT3_BV                  = TYPE_POINT3 + TYPE_BY_VAL,          //!< A by-value TYPE_POINT3 value. See TYPE_POINT3.
	TYPE_HSV_BV                     = TYPE_HSV + TYPE_BY_VAL,             //!< This type has been deprecated as of 3ds Max 2022. Use TYPE_RGBA_BV instead.
	TYPE_INTERVAL_BV                = TYPE_INTERVAL + TYPE_BY_VAL,        //!< A by-value TYPE_INTERVAL value. See TYPE_INTERVAL.
	TYPE_BITMAP_BV                  = TYPE_BITMAP + TYPE_BY_VAL,          //!< A by-value TYPE_BITMAP value. See TYPE_BITMAP.
	TYPE_MATRIX3_BV                 = TYPE_MATRIX3 + TYPE_BY_VAL,         //!< A by-value TYPE_MATRIX3 value. See TYPE_MATRIX3.
	TYPE_POINT4_BV                  = TYPE_POINT4 + TYPE_BY_VAL,          //!< A by-value TYPE_POINT4 value. See TYPE_POINT4.
	TYPE_FRGBA_BV                   = TYPE_FRGBA + TYPE_BY_VAL,           //!< A by-value TYPE_FRGBA value. See TYPE_FRGBA.
	TYPE_ANGAXIS_BV                 = TYPE_ANGAXIS + TYPE_BY_VAL,         //!< A by-value TYPE_ANGAXIS value. See TYPE_ANGAXIS.
	TYPE_QUAT_BV                    = TYPE_QUAT + TYPE_BY_VAL,            //!< A by-value TYPE_QUAT value. See TYPE_QUAT.
	TYPE_RAY_BV                     = TYPE_RAY + TYPE_BY_VAL,             //!< A by-value TYPE_RAY value. See TYPE_RAY.
	TYPE_POINT2_BV                  = TYPE_POINT2 + TYPE_BY_VAL,          //!< A by-value TYPE_POINT2 value. See TYPE_POINT2.
	TYPE_BITARRAY_BV                = TYPE_BITARRAY + TYPE_BY_VAL,        //!< A by-value TYPE_BITARRAY value. See TYPE_BITARRAY.
	TYPE_MESH_BV                    = TYPE_MESH + TYPE_BY_VAL,            //!< A by-value TYPE_MESH value. See TYPE_MESH.
	TYPE_POINT_BV                   = TYPE_POINT + TYPE_BY_VAL,           //!< A by-value TYPE_POINT value. See TYPE_POINT.
	TYPE_TSTR_BV                    = TYPE_TSTR + TYPE_BY_VAL,            //!< A by-value TYPE_TSTR value. See TYPE_TSTR.
	TYPE_COLOR_BV                   = TYPE_COLOR + TYPE_BY_VAL,           //!< A by-value TYPE_COLOR value. See TYPE_COLOR.
	TYPE_FPVALUE_BV                 = TYPE_FPVALUE + TYPE_BY_VAL,         //!< A by-value TYPE_FPVALUE value. See TYPE_FPVALUE.
	TYPE_CLASS_BV                   = TYPE_CLASS + TYPE_BY_VAL,           //!< A by-value TYPE_CLASS value. See TYPE_CLASS.
	TYPE_BOX3_BV                    = TYPE_BOX3 + TYPE_BY_VAL,            //!< A by-value TYPE_BOX3 value. See TYPE_BOX3.
	TYPE_BEZIERSHAPE_BV             = TYPE_BEZIERSHAPE + TYPE_BY_VAL,     //!< A by-value TYPE_BEZIERSHAPE value. See TYPE_BEZIERSHAPE.

	// Pass by value Tab<>s
	// -----------------------------------------------------------------------

	// Note:
	// Tab is passed by val, data type in tab is TYPE_XXXX. For example:
	// TYPE_FLOAT_TAB_BV = Tab<float>
	// TYPE_INODE_TAB_BV = Tab<INode*>

	TYPE_FLOAT_TAB_BV               = TYPE_FLOAT + TYPE_TAB + TYPE_BY_VAL,          //!< A by-value table of TYPE_FLOAT values. See TYPE_FLOAT.
	TYPE_INT_TAB_BV                 = TYPE_INT + TYPE_TAB + TYPE_BY_VAL,            //!< A by-value table of TYPE_INT values. See TYPE_INT.
	TYPE_RGBA_TAB_BV                = TYPE_RGBA + TYPE_TAB + TYPE_BY_VAL,           //!< A by-value table of TYPE_RGBA values. See TYPE_RGBA.
	TYPE_POINT3_TAB_BV              = TYPE_POINT3 + TYPE_TAB + TYPE_BY_VAL,         //!< A by-value table of TYPE_POINT3 values. See TYPE_POINT3.
	TYPE_BOOL_TAB_BV                = TYPE_BOOL + TYPE_TAB + TYPE_BY_VAL,           //!< A by-value table of TYPE_BOOL values. See TYPE_BOOL.
	TYPE_ANGLE_TAB_BV               = TYPE_ANGLE + TYPE_TAB + TYPE_BY_VAL,          //!< A by-value table of TYPE_ANGLE values. See TYPE_ANGLE.
	TYPE_PCNT_FRAC_TAB_BV           = TYPE_PCNT_FRAC + TYPE_TAB + TYPE_BY_VAL,      //!< A by-value table of TYPE_PCNT_FRAC values. See TYPE_PCNT_FRAC.
	TYPE_WORLD_TAB_BV               = TYPE_WORLD + TYPE_TAB + TYPE_BY_VAL,          //!< A by-value table of TYPE_WORLD values. See TYPE_WORLD.
	TYPE_STRING_TAB_BV              = TYPE_STRING + TYPE_TAB + TYPE_BY_VAL,         //!< A by-value table of TYPE_STRING values. See TYPE_STRING.
	TYPE_FILENAME_TAB_BV            = TYPE_FILENAME + TYPE_TAB + TYPE_BY_VAL,       //!< A by-value table of TYPE_FILENAME values. See TYPE_FILENAME.
	TYPE_HSV_TAB_BV                 = TYPE_HSV + TYPE_TAB + TYPE_BY_VAL,            //!< This type has been deprecated as of 3ds Max 2022. Use TYPE_RGBA_TAB_BV instead.
	TYPE_COLOR_CHANNEL_TAB_BV       = TYPE_COLOR_CHANNEL + TYPE_TAB + TYPE_BY_VAL,  //!< A by-value table of TYPE_COLOR_CHANNEL values. See TYPE_COLOR_CHANNEL.
	TYPE_TIMEVALUE_TAB_BV           = TYPE_TIMEVALUE + TYPE_TAB + TYPE_BY_VAL,      //!< A by-value table of TYPE_TIMEVALUE values. See TYPE_TIMEVALUE.
	TYPE_RADIOBTN_INDEX_TAB_BV      = TYPE_RADIOBTN_INDEX + TYPE_TAB + TYPE_BY_VAL, //!< A by-value table of TYPE_RADIOBTN_INDEX values. See TYPE_RADIOBTN_INDEX.
	TYPE_MTL_TAB_BV                 = TYPE_MTL + TYPE_TAB + TYPE_BY_VAL,            //!< A by-value table of TYPE_MTL values. See TYPE_MTL.
	TYPE_TEXMAP_TAB_BV              = TYPE_TEXMAP + TYPE_TAB + TYPE_BY_VAL,         //!< A by-value table of TYPE_TEXMAP values. See TYPE_TEXMAP.
	TYPE_BITMAP_TAB_BV              = TYPE_BITMAP + TYPE_TAB + TYPE_BY_VAL,         //!< A by-value table of TYPE_BITMAP values. See TYPE_BITMAP.
	TYPE_INODE_TAB_BV               = TYPE_INODE + TYPE_TAB + TYPE_BY_VAL,          //!< A by-value table of TYPE_INODE values. See TYPE_INODE.
	TYPE_REFTARG_TAB_BV             = TYPE_REFTARG + TYPE_TAB + TYPE_BY_VAL,        //!< A by-value table of TYPE_REFTARG values. See TYPE_REFTARG.
	TYPE_INDEX_TAB_BV               = TYPE_INDEX + TYPE_TAB + TYPE_BY_VAL,          //!< A by-value table of TYPE_INDEX values. See TYPE_INDEX.
	TYPE_MATRIX3_TAB_BV             = TYPE_MATRIX3 + TYPE_TAB + TYPE_BY_VAL,        //!< A by-value table of TYPE_MATRIX3 values. See TYPE_MATRIX3.
	TYPE_POINT4_TAB_BV              = TYPE_POINT4 + TYPE_TAB + TYPE_BY_VAL,         //!< A by-value table of TYPE_POINT4 values. See TYPE_POINT4.
	TYPE_FRGBA_TAB_BV               = TYPE_FRGBA + TYPE_TAB + TYPE_BY_VAL,          //!< A by-value table of TYPE_FRGBA values. See TYPE_FRGBA.
	TYPE_PBLOCK2_TAB_BV             = TYPE_PBLOCK2 + TYPE_TAB + TYPE_BY_VAL,        //!< A by-value table of TYPE_PBLOCK2 values. See TYPE_PBLOCK2.
	TYPE_VOID_TAB_BV                = TYPE_VOID + TYPE_TAB + TYPE_BY_VAL,           //!< A by-value table of TYPE_VOID values. See TYPE_VOID.
	TYPE_TSTR_TAB_BV                = TYPE_TSTR + TYPE_TAB + TYPE_BY_VAL,           //!< A by-value table of TYPE_TSTR values. See TYPE_TSTR.
	TYPE_ENUM_TAB_BV                = TYPE_ENUM + TYPE_TAB + TYPE_BY_VAL,           //!< A by-value table of TYPE_ENUM values. See TYPE_ENUM.
	TYPE_INTERVAL_TAB_BV            = TYPE_INTERVAL + TYPE_TAB + TYPE_BY_VAL,       //!< A by-value table of TYPE_INTERVAL values. See TYPE_INTERVAL.
	TYPE_ANGAXIS_TAB_BV             = TYPE_ANGAXIS + TYPE_TAB + TYPE_BY_VAL,        //!< A by-value table of TYPE_ANGAXIS values. See TYPE_ANGAXIS.
	TYPE_QUAT_TAB_BV                = TYPE_QUAT + TYPE_TAB + TYPE_BY_VAL,           //!< A by-value table of TYPE_QUAT values. See TYPE_QUAT.
	TYPE_RAY_TAB_BV                 = TYPE_RAY + TYPE_TAB + TYPE_BY_VAL,            //!< A by-value table of TYPE_RAY values. See TYPE_RAY.
	TYPE_POINT2_TAB_BV              = TYPE_POINT2 + TYPE_TAB + TYPE_BY_VAL,         //!< A by-value table of TYPE_POINT2 values. See TYPE_POINT2.
	TYPE_BITARRAY_TAB_BV            = TYPE_BITARRAY + TYPE_TAB + TYPE_BY_VAL,       //!< A by-value table of TYPE_BITARRAY values. See TYPE_BITARRAY.
	TYPE_CLASS_TAB_BV               = TYPE_CLASS + TYPE_TAB + TYPE_BY_VAL,          //!< A by-value table of TYPE_CLASS values. See TYPE_CLASS.
	TYPE_MESH_TAB_BV                = TYPE_MESH + TYPE_TAB + TYPE_BY_VAL,           //!< A by-value table of TYPE_MESH values. See TYPE_MESH.
	TYPE_OBJECT_TAB_BV              = TYPE_OBJECT + TYPE_TAB + TYPE_BY_VAL,         //!< A by-value table of TYPE_OBJECT values. See TYPE_OBJECT.
	TYPE_CONTROL_TAB_BV             = TYPE_CONTROL + TYPE_TAB + TYPE_BY_VAL,        //!< A by-value table of TYPE_CONTROL values. See TYPE_CONTROL.
	TYPE_POINT_TAB_BV               = TYPE_POINT + TYPE_TAB + TYPE_BY_VAL,          //!< A by-value table of TYPE_POINT values. See TYPE_POINT.
	TYPE_IOBJECT_TAB_BV             = TYPE_IOBJECT + TYPE_TAB + TYPE_BY_VAL,        //!< A by-value table of TYPE_IOBJECT values. See TYPE_IOBJECT.
	TYPE_INTERFACE_TAB_BV           = TYPE_INTERFACE + TYPE_TAB + TYPE_BY_VAL,      //!< A by-value table of TYPE_INTERFACE values. See TYPE_INTERFACE.
	TYPE_HWND_TAB_BV                = TYPE_HWND + TYPE_TAB + TYPE_BY_VAL,           //!< A by-value table of TYPE_HWND values. See TYPE_HWND.
	TYPE_NAME_TAB_BV                = TYPE_NAME + TYPE_TAB + TYPE_BY_VAL,           //!< A by-value table of TYPE_NAME values. See TYPE_NAME.
	TYPE_COLOR_TAB_BV               = TYPE_COLOR + TYPE_TAB + TYPE_BY_VAL,          //!< A by-value table of TYPE_COLOR values. See TYPE_COLOR.
	TYPE_FPVALUE_TAB_BV             = TYPE_FPVALUE + TYPE_TAB + TYPE_BY_VAL,        //!< A by-value table of TYPE_FPVALUE values. See TYPE_FPVALUE.
	TYPE_VALUE_TAB_BV               = TYPE_VALUE + TYPE_TAB + TYPE_BY_VAL,          //!< A by-value table of TYPE_VALUE values. See TYPE_VALUE.
	TYPE_DWORD_TAB_BV               = TYPE_DWORD + TYPE_TAB + TYPE_BY_VAL,          //!< A by-value table of TYPE_DWORD values. See TYPE_DWORD.
	TYPE_bool_TAB_BV                = TYPE_bool + TYPE_TAB + TYPE_BY_VAL,           //!< A by-value table of TYPE_bool values. See TYPE_bool.
	TYPE_INTPTR_TAB_BV              = TYPE_INTPTR + TYPE_TAB + TYPE_BY_VAL,         //!< A by-value table of TYPE_INTPTR values. See TYPE_INTPTR.
	TYPE_INT64_TAB_BV               = TYPE_INT64 + TYPE_TAB + TYPE_BY_VAL,          //!< A by-value table of TYPE_INT64 values. See TYPE_INT64.
	TYPE_DOUBLE_TAB_BV              = TYPE_DOUBLE + TYPE_TAB + TYPE_BY_VAL,         //!< A by-value table of TYPE_DOUBLE values. See TYPE_DOUBLE.
	TYPE_BOX3_TAB_BV                = TYPE_BOX3 + TYPE_TAB + TYPE_BY_VAL,           //!< A by-value table of TYPE_BOX3 values. See TYPE_BOX3.
	TYPE_BEZIERSHAPE_TAB_BV         = TYPE_BEZIERSHAPE + TYPE_TAB + TYPE_BY_VAL,    //!< A by-value table of TYPE_BEZIERSHAPE values. See TYPE_BEZIERSHAPE.

	// Pass by pointer types for int and float types
	// -----------------------------------------------------------------------

	// Note: implies * parameters, int* and float* are passed via .ptr fields, only for FnPub use

	TYPE_FLOAT_BP                   = TYPE_FLOAT + TYPE_BY_PTR,          //!< A by-pointer TYPE_FLOAT value. See TYPE_FLOAT.
	TYPE_INT_BP                     = TYPE_INT + TYPE_BY_PTR,            //!< A by-pointer TYPE_INT value. See TYPE_INT.
	TYPE_BOOL_BP                    = TYPE_BOOL + TYPE_BY_PTR,           //!< A by-pointer TYPE_BOOL value. See TYPE_BOOL.
	TYPE_ANGLE_BP                   = TYPE_ANGLE + TYPE_BY_PTR,          //!< A by-pointer TYPE_ANGLE value. See TYPE_ANGLE.
	TYPE_PCNT_FRAC_BP               = TYPE_PCNT_FRAC + TYPE_BY_PTR,      //!< A by-pointer TYPE_PCNT_FRAC value. See TYPE_PCNT_FRAC.
	TYPE_WORLD_BP                   = TYPE_WORLD + TYPE_BY_PTR,          //!< A by-pointer TYPE_WORLD value. See TYPE_WORLD.
	TYPE_COLOR_CHANNEL_BP           = TYPE_COLOR_CHANNEL + TYPE_BY_PTR,  //!< A by-pointer TYPE_COLOR_CHANNEL value. See TYPE_COLOR_CHANNEL.
	TYPE_TIMEVALUE_BP               = TYPE_TIMEVALUE + TYPE_BY_PTR,      //!< A by-pointer TYPE_TIMEVALUE value. See TYPE_TIMEVALUE.
	TYPE_RADIOBTN_INDEX_BP          = TYPE_RADIOBTN_INDEX + TYPE_BY_PTR, //!< A by-pointer TYPE_RADIOBTN_INDEX value. See TYPE_RADIOBTN_INDEX.
	TYPE_INDEX_BP                   = TYPE_INDEX + TYPE_BY_PTR,          //!< A by-pointer TYPE_INDEX value. See TYPE_INDEX.
	TYPE_ENUM_BP                    = TYPE_ENUM + TYPE_BY_PTR,           //!< A by-pointer TYPE_ENUM value. See TYPE_ENUM.
	TYPE_DWORD_BP                   = TYPE_DWORD + TYPE_BY_PTR,          //!< A by-pointer TYPE_DWORD value. See TYPE_DWORD.
	TYPE_bool_BP                    = TYPE_bool + TYPE_BY_PTR,           //!< A by-pointer TYPE_bool value. See TYPE_bool.
	TYPE_INTPTR_BP                  = TYPE_INTPTR + TYPE_BY_PTR,         //!< A by-pointer TYPE_INTPTR value. See TYPE_INTPTR.
	TYPE_INT64_BP                   = TYPE_INT64 + TYPE_BY_PTR,          //!< A by-pointer TYPE_INT64 value. See TYPE_INT64.
	TYPE_DOUBLE_BP                  = TYPE_DOUBLE + TYPE_BY_PTR,         //!< A by-pointer TYPE_DOUBLE value. See TYPE_DOUBLE.

	// There are no specific by-pointer Tab<> types, all Tab<> types are by-pointer by default

	TYPE_MAX_TYPE, //!< End of enum marker.

	// MAXScript internal types
	// -----------------------------------------------------------------------

	TYPE_KEYARG_MARKER = 253, //!< MAXScript internal type.
	TYPE_MSFLOAT,             //!< MAXScript internal type.
	TYPE_UNSPECIFIED,         //!< MAXScript internal type.
};

#pragma deprecated(TYPE_HSV)
#pragma deprecated(TYPE_HSV_TAB)
#pragma deprecated(TYPE_HSV_BR)
#pragma deprecated(TYPE_HSV_TAB_BR)
#pragma deprecated(TYPE_HSV_BV)
#pragma deprecated(TYPE_HSV_TAB_BV)

//! \defgroup ControlTypeList List of ControlType Choices
//!@{
/** Used to associate automatically-generated UI controls with parameters in a ParamBlockDesc2.
    \see ControlType2, ParamBlockDesc2 
    ~{ Parameter UI Control Types }~
*/ 
enum ControlType {
	/** Identifies a spinner control. The spinner control is used to provide input of values limited to a fixed numeric type. For example, the control may be 
        limited to the input of only positive integers. The input options are integer, float, universe (world space units), 
        positive integer, positive float, positive universe, and time. This control allows the user to increment or decrement a 
        value by clicking on the up or down arrows. The user may also click and drag on the arrows to interactively adjust the value. 
        The Ctrl key may be held to accelerate the value changing speed, while the Alt key may be held to decrease the value changing speed. 

        The value can be one of the EditSpinnerType enum values.

        The list of dialog item IDs depends on the ParamType of the parameter. For TYPE_POINT3 and TYPE_RGBA, you supply 3 pairs of IDs, one for each coordinate, each pair specifying the editbox and spinner IDs. For the others you specify one editbox/spinner pair of IDs. The display scale can be a floating point value or the special value SPIN_AUTOSCALE. Eg: 
        \code p_ui, TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_RADIUS, IDC_RADSPINNER, SPIN_AUTOSCALE, \endcode 

        If the parameter is a table type then, as with all other control types, you must specify a fixed table size and supply a list of dialog item IDs, one set for each element in the tab. Eg, for a 3 element table: 
        \code p_ui, TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_RADIUS1, IDC_RADSPINNER1, IDC_RADIUS2, IDC_RADSPINNER2, IDC_RADIUS3, IDC_RADSPINNER3, SPIN_AUTOSCALE, \endcode

        This control type can be used with any of the following ParamTypes: TYPE_ANGLE, TYPE_PCNT_FRAC, TYPE_COLOR_CHANNEL, TYPE_FLOAT, TYPE_TIMEVALUE, TYPE_INT, TYPE_POINT3, TYPE_RGBA

        ~{ Spinner and Slider Control Types }~ */
    TYPE_SPINNER, 

    /** Identifies a radio button control. Radio buttons are used to provide the user with a single boolean choice, or when used in groups, to select 
        among several options. These are the standard Win32 radio button controls. Following the TYPE_RADIO, you must supply 
        an integer count of the number of radio buttons in this group and then a list of dialog item IDs for each button. A radio 
        button can be used with TYPE_INT or TYPE_RADIOBTN_INDEX parameters. The value of the parameter defaults to the ordinal number 
        of the radio button, starting at 0. You can optionally supply a p_vals tag immediately following the p_ui section, which 
        would be followed by a list of numbers, one for each radio button. These numbers will become the (non-ordinal) parameter 
        value corresponding to which button is set. */
	TYPE_RADIO,         

    /** Identifies a single checkbox control with a parameter. Check boxes are used to provide the user with a single boolean choice. This is 
        the standard Win32 check box control. The TYPE_SINGLECHECKBOX must be followed dialog item ID. A 
        TYPE_SINGLECHECKBOX can only be used with TYPE_INT or TYPE_BOOL parameters. */
	TYPE_SINGLECHECKBOX, 
	TYPE_SINGLECHEKBOX = TYPE_SINGLECHECKBOX, // For fix of legacy issue with misspelling

    /** Identifies a multiple checkbox control. This control type is not currently supported. */
	TYPE_MULTICHECKBOX,  
	TYPE_MULTICHEKBOX = TYPE_MULTICHECKBOX,   // For fix of legacy issue with misspelling
	
    /** Identifies a color swatch color control. The color swatch control presents the user with the 
        standard 3ds Max  modeless color selector when the user clicks on the control. The color swatch control displays 
        the currently selected color. The TYPE_COLORSWATCH with the dialog item ID of the swatch custom control. This 
        can only be used with TYPE_POINT3 or TYPE_RGBA parameter. */
    TYPE_COLORSWATCH,   
	};

/** Used to associated automatically generated UI controls with parameters in a ParamBlockDesc2. 
    Wherever ControlType2 is used, a ControlType can be used as well. 
    \see ControlType, ParamBlockDesc2,
    ~{ Parameter UI Control Types }~
*/ 
enum ControlType2
{
	/** Identifies a 3ds Max custom edit box control. The TYPE_EDITBOX must be followed by the dialog item ID of the edit box 
	custom control. This can only be used with TYPE_STRING and TYPE_FILENAME parameters. */
	TYPE_EDITBOX = TYPE_COLORSWATCH + 1,

	/** Idenfies a two-state (pressed or not-pressed) check button control. Check boxes are used to provide the user 
	with a single boolean choice. This is the standard Win32 check box control. The TYPE_SINGLECHECKBOX must 
	be followed by a dialog item ID. A TYPE_SINGLECHECKBOX can only be used with TYPE_INT or TYPE_BOOL parameters. */
	TYPE_CHECKBUTTON,

	/** Identifies a scene node (INode) picker button. This is a 3ds Max ICustButton control used in a CBT_CHECK mode. 
	Follow the control type with the dialog item ID of the button control. When the user presses this button a 
	PickModeCallback command mode is entered and the user can pick a scene node under the filtering of any validation 
	supplied (see tags p_validator, p_classID and p_sclassID). This can only be used with TYPE_INODE parameters. 
	Use the p_prompt tag to supply a status line prompt. */
	TYPE_PICKNODEBUTTON,

	/** A texture map selector button that supports drag and drop of texture maps. This is an ICustButton control used in 
	CBT_PUSH mode. The control type must be followed by the dialog item ID of the button control. This button opens up 
	a map selector dialog when pressed. This can only be used with TYPE_TEXMAP parameters. Use the p_prompt tag to 
	supply a status line prompt. */
	TYPE_TEXMAPBUTTON,

	/** Identifies a material selector button that supports drag and drop of materials. This is an ICustButton control used in CBT_PUSH 
	mode. The control type must be followed by the dialog item ID of the button control. This button displays a material 
	selector dialog when pressed. This can only be used with TYPE_MTL parameters. Use the p_prompt tag to supply a status 
	line prompt. */
	TYPE_MTLBUTTON,

	/** Identifies a bitmap browser button. This is a 3ds Max CustButton control used in a CBT_PUSH mode. Follow the control type with 
	the dialog item ID of the CustButton custom control. This button displays a standard 3ds Max Bitmap browser when 
	pressed and is Bitmap drag-and-drop sensitive. Can only be used with TYPE_BITMAP parameters. Use the p_prompt tag 
	to supply a status line prompt. */
	TYPE_BITMAPBUTTON,

	/** Identifies a file open dialog button. This is an ICustButton control used in CBT_PUSH mode. The control type with the 
	dialog item ID of the button control. This button open a standard Windows Open File dialog for selecting a file name. This 
	type can only be used with TYPE_STRING and TYPE_FILENAME parameters. Use any of the p_caption, p_init_file, and p_file_types 
	tags to further control the dialog. */
	TYPE_FILEOPENBUTTON,

	/** Identifies a file save dialog button. This is an ICustButton control used in CBT_PUSH mode. The control type with the 
	dialog item ID of the button control. This button open a standard Windows Save File dialog for selecting a file name. This 
	type can only be used with TYPE_STRING and TYPE_FILENAME parameters. Use any of the p_caption, p_init_file, and p_file_types 
	tags to further control the dialog. */
	TYPE_FILESAVEBUTTON,

	/** Identifies a list box control for managing a Tab<int>. 
	The setup consists of a ListBox control, 3 buttons for adding, replacing and deleting items in the list and a 3ds Max spinner to supply
	source values for Add & Replace. After the control type, you supply 4 dialog item IDs. The first is the ListBox control, 
	then dialog item IDs for 3 CustButton controls for an Add, Replace and Delete button, respectively. Follow these with a spinner type, 
	editbox/spinner dialog item ID pair and a display scale, exactly as for TYPE_SPINNER. You can supply the value 0 for any of the 
	Add, Replace, or Delete buttons if you don't need them in the dialog. 
	This can only be used with the following parameter types: TYPE_TIMEVALUE_TAB, TYPE_INT, TYPE_INT_TAB 	

	The Add/Replace/Delete buttons automatically keep the Tab<> parameter in step with the list. 
	The TYPE_INT parameter type and TYPE_INTLISTBOX control type combination is recognized specially and is used to allow a dropdown 
	list to be associated with an int parameter such that the selection index in the dropdown becomes the integer parameter value. 
	In this mode, after the control type you supply the ListBox control ID, then a count followed by that many string resource IDs. 
	These strings are used to populate the dropdown.
	Eg: 
	\code p_ui, TYPE_INTLISTBOX, <list_ctrl_id>, <num_items>, [ <item1_str_id>, <item2_str_id>, ... ] \endcode
	These are the list control res ID, followed by a list of initial string items to load up given as a count 
	(which can be 0 – you can load them up dynamically in the dialog proc), and then a list of string resource IDs. 
	See the std2_shader_type parameter in \\maxsdk\\samples\\materials\\stdmtl2.cpp for an example.

	\sa ~{ List Box Control Types }~. */

	TYPE_INTLISTBOX,

	/** Identifies a list box control for managing a Tab<float>. 
	The setup is identical to TYPE_INTLISTBOX.
	This can only be used with the following parameter types: TYPE_ANGLE_TAB, TYPE_PCNT_FRAC_TAB, TYPE_COLOR_CHANNEL_TAB, TYPE_FLOAT_TAB. 

	sa ~{ List Box Control Types }~. */
	TYPE_FLOATLISTBOX,

	/** Identifies a series of controls for displaying and managing a list box control containing a Tab<MSTR> parameter. 
	The setup consists of a ListBox control, 3 buttons for adding, replacing and deleting items in the list and a 3ds Max CustEdit box 
	to supply source strings for Add & Replace. After the control type, you supply 4 dialog item IDs. 
	The first is the ListBox control, then dialog item IDs for 3 CustButton controls for an Add, Replace and Delete button, respectively. 
	Follow these with the CustEdit control dialog item ID. You can supply the value 0 for any of the Add, Replace, Delete buttons 
	or CustEdit control IDs if you don't need them in the dialog. Can only be used with TYPE_STRING_TAB parameters. 
	The Add/Replace/Delete buttons automatically keep the Tab<> parameter in step with the list.
	 */
	TYPE_STRINGLISTBOX,

	/** Identifies a series of controls for displaying and managing a list box control containing a Tab<INode> parameter. 
	The setup consists of a ListBox control, 3 buttons for picking, replacing and deleting items in the list. 
	After the control type, you supply 4 dialog item IDs. The first is the ListBox control, then dialog item IDs for 3 
	CustButton controls for a Pick, Replace and Delete button, respectively. THe Pick and Replace buttons act exactly as 
	TYPE_PICKNODEBUTTONS to get nodes to add-to or replace-in the list. You can supply the value 0 for any of the Add, Replace, 
	Delete buttons IDs if you don't need them in the dialog. Can only use with TYPE_INODE_TAB parameters. 
	The Pick/Replace/Delete buttons automatically keep the Tab<> parameter in step with the list, including managing References as needed.
	*/
	TYPE_NODELISTBOX,

	/** Identifies a standard 3ds Max slider control.   
	It requires a type, list of dialog item resource IDs and a number of ticks. The slider type should be one of the EditSpinnerType values.*/
	TYPE_SLIDER,

	/** \internal */
	TYPE_BUTTON,

	/** Identifies a series of controls for displaying and managing a list box control containing a Tab<Point3> parameter. 
	The setup consists of a ListBox control, 3 buttons for adding, replacing and deleting items in the list, a source spinner type,
	three pairs of editboxes and spinners, and a display scale. After the control type, you supply 4 dialog item IDs. 
	The first is the ListBox control, then dialog item IDs for 3 CustButton controls for an Add, Replace and Delete button, respectively. 
	Follow this with the source spinner type, the editbox and spinner for the first item, the editbox and spinner for the second item, 
	the editbox and spinner for the third item, and finally the display scale. Note that you can only use this with TYPE_POINT3_TAB parameters.
	*/
	TYPE_POINT3LISTBOX, 

	/** Parameters with a UI type of TYPE_SHADERSUBPARAMETERBLOCK indicate PB2 parameters
	for which param maps must be created, while all parameters should be edited
	in the same rollup.
	\see mrShaderParamMap2 */
	TYPE_SHADERSUBPARAMETERBLOCK,

	/** Identifies a series a controls for displaying and managing a list box control containing a Tab<Point4> parameter. */
	TYPE_POINT4LISTBOX,

	/** Identifies a color swatch color control for colors with an alpha channel. 
	The color swatch control presents the user with the 
	standard 3ds Max  modeless color selector when the user clicks on the control. The color swatch control displays 
	the currently selected color. The TYPE_COLORSWATCH with the dialog item ID of the swatch custom control. This 
	can only be used with TYPE_POINT4 or TYPE_FRGBA parameter. */
	TYPE_COLORSWATCH_FRGBA, 

	/** A combo box control that can be used with parameters of type TYPE_INT and TYPE_INT_TAB. 
	\sa ~{ Combo Box Control Types }~. 
	When used with parameters of type TYPE_INT, it needs to be followed by a series of controls for displaying and managing a combo-box control, 
	as shown here: 
	\code p_ui, TYPE_INT_COMBOBOX, <ctrl_id>, <num_items>, [ <item1_str_id>, <item2_str_id>, ... ] \endcode

	where: 
	- \<ctrl_id\>: is the resource id of the combo-box control 
	- \<num_items\>: is the number of items in the combo-box. If zero, no resource string ids need to be provided 
	- \<item1_str_id\>, \<item2_str_id\>, ...: optional string resource ids for the user visible names of the combo-box items.

	Example: 
	\code 
	pb_int_combobox1, _T("TYPE_INT_COMBOBOX1"), TYPE_INT, // parameter type 
	P_RESET_DEFAULT+P_ANIMATABLE, IDS_INT_COMBOBOX, p_ui, TYPE_INT_COMBOBOX, IDC_INT_COMBOBOX, // resource id of combo box ui widget 
	4, // number of items in the combobox 
	IDS_COMBOBOX_ITEM1, IDS_COMBOBOX_ITEM2, IDS_COMBOBOX_ITEM3, IDS_COMBOBOX_ITEM4, // string resource ids representing the "human readable" names of the combobox items 
	p_vals, 10, 50, 30, 20, p_default, 10, p_tooltip, IDS_INT_COMBOBOX, end 
	\endcode 

	When TYPE_INT_COMBOBOX is used with parameters of type TYPE_INT_TAB, it needs to be followed by a series of controls for displaying 
	and managing one combo-box control per tab element, as in: 
	\code p_ui, TYPE_INT_COMBOBOX, <ctrl_id1>,<ctrl_id2>,... <ctrl_idn>, <num_items>, [ <item1_str_id>, <item2_str_id>, ... ] \endcode

	where: 
	- \<ctrl_idn\>: resource ids of the combo-box control, 1 \<= n \<= number of tab elements 
	- \<num_items\>: is the number of items in each combo-box. If zero, no resource string ids need to be provided 
	- \<item1_str_id\>, \<item2_str_id\>, ...: optional string resource ids for the user visible names of the combo-box items.

	Example: 
	\code pb_int_combobox2, _T("TYPE_INT_COMBOBOX2"), TYPE_INT_TAB, 2, // parameter type and number of elements in the tab
	P_RESET_DEFAULT+P_ANIMATABLE, IDS_LISTBOX, p_ui, TYPE_INT_COMBOBOX, IDC_INT_COMBOBOX2_1, IDC_INT_COMBOBOX2_2, // one combo-box resource id per tab element 
	4, IDS_COMBOBOX_ITEM1, IDS_COMBOBOX_ITEM2, IDS_COMBOBOX_ITEM3, IDS_COMBOBOX_ITEM4, // string resource ids representing the "human readable" names of the combobox items 
	p_vals, 10, 50, 30, 20, p_default, 10, p_tooltip, IDS_INT_COMBOBOX, end
	\endcode 

	The p_vals flag may follow a combo-box ui specification: p_vals, \<list_of_integers\>
	If p_vals is specified, the list of integer values that follows represents the values associated with each combo-box item. 
	These values don't have to be consecutive or in order (increasing or decreasing). See TYPE_RADIO. 
	If p_vals is not following p_ui, TYPE_INT_COMBOBOX, each combo-box item is mapped to an integer value starting with zero 
	and up to the number of items in the combo-box.

	*/
	TYPE_INT_COMBOBOX,

	/** Identifies a series of controls for displaying and managing a list box control containing a Tab<Point4> parameter. 
	The setup consists of a ListBox control, 3 buttons for adding, replacing and deleting items in the list, a source spinner type,
	three pairs of editboxes and spinners, and a display scale. After the control type, you supply 4 dialog item IDs. 
	The first is the ListBox control, then dialog item IDs for 3 CustButton controls for an Add, Replace and Delete button, respectively. 
	Follow this with the source spinner type, the editbox and spinner for the first item, the editbox and spinner for the second item, 
	and finally the display scale. Note that you can only use this with TYPE_POINT2_TAB parameters.
	*/
	TYPE_POINT2LISTBOX, 

	//The float editable combobox contains a combobox and a editbox. 
	//The Editbox will show the related float value for each combobox item. 
	//The float combobox has an item named "custom". 
	//If the "custom" is selected, the user can input a arbitrary value in the Editbox(This control also has a min and a max value). 
	//If  the current selected item is not "custom", the combobox will go to the "custom" item when the user input a value into the Editbox which is not related to any item.
	//The example for adding it to the ParamBlockDesc2 is below:
	// 	//// dlgIds[0] is the id of COMBOBOX, dlgIds[1] is the id of editBox
	//	pbd->AddParam(mUiId++, uiElement->GetParameterName(),	TYPE_FLOAT,	P_COMPUTED_NAME, 	0, 	
	//  p_default, 		value, 
	//  p_range,        min, max,
	//  p_ui, 			mapID, TYPE_FLOAT_EDITABLE_COMBOBOX, dlgIds[0], dlgIds[1], 0,  
	//  p_accessor,		&s_proteinParamAccessor,
	//  end
	//  );
	TYPE_FLOAT_EDITABLE_COMBOBOX 

};
//!@}

//!\defgroup ParamTagsList List of ParamTags Choices
//!@{
/** Optional parameter definition tags. These tags are used in the ParamBlockDesc2 constructor as 
    part of the "optional_tagged_param_specs". The typical format in the var args list is:
    "tag", "optional_param_spec", */
enum ParamTags
{	
    /** Specifies the default value used when a parameter block is first created (or reset to defaults). 
    	It must be of the correct type to match the ParamType of the parameter (for example float for 
    	TYPE_FLOATs, int for TYPE_INTs, Color(x,y,z) for TYPE_RGBA, Point3(x,y,z) for TYPE_POINT3s, etc.)

        Valid for the following types: TYPE_ANGLE, TYPE_PCNT_FRAC, TYPE_COLOR_CHANNEL, TYPE_FLOAT, 
        TYPE_TIMEVALUE, TYPE_INT, TYPE_BOOL, TYPE_RADIOBTN_INDEX, TYPE_POINT3, TYPE_RGBA, TYPE_STRING, 
        TYPE_FILENAME, TYPE_MATRIX3. 

        Examples:
        \code p_default, FALSE,p_default, 1,p_default, Point3(0,0,0),p_default, 25.0, \endcode
        */
    p_default = -(1<<30),

    /** Specifies default value when constructed by scripts. For example the MAXScript command sphere.radius defaults to 
        25.0 when created by the scripter, but p_default is set to 0.0 so interactive creation starts out with a point-sized 
        sphere when the first mouse click is made.

        Example:
        \code p_ms_default, 25.0, \endcode
        */
	p_ms_default,

    /** Specifies legal range, used in spinner/slider setup.
        Supplied as two values of the correct type (as described in p_default, above).
        Ranges are used for the following control types: TYPE_ANGLE, TYPE_PCNT_FRAC, 
        TYPE_COLOR_CHANNEL, TYPE_FLOAT, TYPE_TIMEVALUE, TYPE_INT, TYPE_RADIOBTN_INDEX, TYPE_POINT3, TYPE_RGBA, TYPE_FRGBA. 
		  Note that the range is not used for MaxScript validation of parameters.
        Example:
        \code p_range, -99999999.0, 99999999.0, \endcode
        */
	p_range,

    /** Specifies associated UI rollout control IDs, etc.
        An optional tag that precedes a variable list of arguments depending on the type of UI control specified. 
        This sequence of arguments is similar in form to the ParamDescBlock2 class constructors.
        Following the p_ui tag, one of the control types in the ControlType or ControlType2 enums should be specified. 
        
        \note If a p_ui is supplied for a Tab<> parameter type and the control is *not* of on the ListBox types, 
            the table size should be fixed and supplied in the "required_param_specs" table_size field, and you 
            should supply a set of dialog item ID's for each element in the table. See the example in ControlType.TYPE_SPINNER.
        */
	p_ui,
	
    /** Specifies a validator object. You supply a pointer to an instance of a class derived from PBValidator. 
        This class has a PBValidator::Validate() method which can return TRUE if the PB2Value passed to it is valid and 
        FALSE otherwise. This can be used for instance by a node pick button to check if a choosen node is acceptable. 

        Example:
        \code p_validator, &fOutValidator, \endcode
        */
    p_validator,

    /** Specifies an accessor object. You supply a pointer to an instance of a class derived 
        from PBAccessor. This class is used to provide a parameter Get/SetValue callback. The callback 
        can be used to monitor parameter value changes, or to implement dynamically-computed virtual parameters. The 
        class has two virtual methods, PBAccessor::Get() and PBAccessor::Set(), each given a PB2Value&, parameter ID, 
        etc. In the case of a Get() you can modify the value in the PB2Value& to implement a virtual get. 

        Example:
        \code p_accessor, &cmap_accessor, \endcode

        */
	p_accessor,

    /** This defines radio button values in button order if button settings need to correspond to non-ordinal numbers. 
        The value of the parameter defaults to the ordinal number of the radio button, starting at 0. You can optionally 
        supply this tag immediately following the TYPE_RADIO p_ui. This tag should be followed by a list of numbers, one 
        for each radio button. These numbers will become the (non-ordinal) parameter value corresponding to which button is set. 
        
        Example:
        \code p_vals, 0,1,2,4,3 , \endcode
        */
	p_vals,

    //	p_subanim_order,			//!< Overrides sequential subanim order. Unused.
	
    /** Used if the flag value includes P_OWNERS_REF. For these reftarg parameters this specifies the reference number in 
        the block's owner for this reference. If the parameter is a Tab<>, then the reference number supplied is the base 
        reference number of the 0'th element, with sequential reference numbers for the following elements. 

        Example:
        \code p_refno, UVGEN_REF, \endcode
        */
    p_refno,

    /** For Texmap items in Mtls this defines the integer SubTexmap index for this Texmap in the owner Mtl. This is used 
        by TYPE_TEXMAPBUTTON ParamMap2 control types to give to the Material Edtor for automatic button and drag And 
        drop handling. 

        Example:
        \code p_subtexno, 0, \endcode
        */
	p_subtexno,

    /** Defines the sub-material integer index for this Mtl in an owner Mtl. This is used by TYPE_MTLBUTTON ParamMap2 control 
        types to give to the Material Edtor for automatic button and Drag And Drop handling. 

        Example:
        \code p_submtlno, 1, \endcode
        */
	p_submtlno,

    /** This allows you to supply a dimension for this parameter. You specify a ParamDimension* as the argument. Certain 
        parameter types have an implied dimension. Defaults to defaultDim. 

        Example:
        \code p_dim, stdWorldDim, \endcode
        */
	p_dim,

    /** This specifies the class ID used as a validator for the various reftarg parameters. This is used by the scripter, 
        picknode filter, etc. For example, if you supply this in a TYPE_INODE parameter and use a TYPE_PICKNODE parammap 
        control, the picker uses this class ID in the picking filter. If you set P_CAN_CONVERT in the parameter's flag, it 
        applies an Object::CanConvertTo() test to the node's world state using the Class_ID supplied to perform the filter test. */
	p_classID,

    /** Specifies the super class ID used as a validator for the various reftarg parameters. For example, if you supply this 
        in a TYPE_INODE parameter and use a TYPE_PICKNODE parammap control, the picker uses this super class ID in the 
        picking filter. 

        Example:
        \code p_sclassID, SHAPE_CLASS_ID, \endcode
        */
	p_sclassID,
	
    /** The associated UI controls are enabled by default. Supply TRUE or FALSE to indicate whether associated UI controls 
        are enabled or disabled when the rollout dialog is first opened. If not supplied, defaults to enabled. Can be 
        overridden by p_enable_ctrls. 
        
        Example:
        \code p_enabled, FALSE, \endcode
        
        */
    p_enabled,

    /** For TYPE_BOOL parameters, lists which other params should be automatically UI enabled/disabled by changes in state 
        of this parameter. This allows you to easily set up conditional enabling of other UI controls based on the state of 
        the controlling parameter. This tag is to be followed by the number of controls as an integer, and then the list of 
        individual parameter IDs (ParamID). 

        Example:
        \code 
        p_enable_ctrls, 3, sel_falloff, sel_pinch, sel_bubble
        \endcode
        */
	p_enable_ctrls,
	
    /** This sets the status line prompt string resource ID for various picker buttons. 

    Example:
    \code p_prompt, IDS_PICK_CAM_PROMPT, \endcode
    */
    p_prompt,

    /** This is a caption string resource ID for TYPE_FILEOPENBUTTON or TYPE_FILESAVEBUTTON open/save file dialogs */
	p_caption,

    /** This establishes the initial filename for open/save file dlgs. Use a direct string for the argument, not 
        a resource ID. The filename can be changed at runtime; do this by setting the init_file member of the 
        ParamDef for the parameter. For example:
        
        \code
        pbdesc->GetParamDef(file_param).init_file = new_file_name; 
        \endcode 
    */
	p_init_file,

    /** Defines file patterns used by open/save file dialogs. The argument is a string resource ID. This 
        string sets up the file type drop-down in the open/save dialog. It is in the following form:

        \code
        "<description1>|<pattern1>|<description2>|<pattern2>|...|"
        \endcode

        In other words, it is a sequence of file type descriptions and file type patterns each separated by a '|' 
        vertical bar and terminated by a '|' vertical bar. For example:

        \code
        "Data(*.dat)|*.dat|Excel(*.csv)|*.csv|All|*.*|"
        \endcode 

        This example specifies 3 types in the file type dropdown, the first reading "Data(*.dat)" and 
        matching *.dat and the second reading "Excel(*.csv)" and matching *.csv and the third reading "All" 
        and matching any file. */
	p_file_types,

    /** \internal Sets the ctrl_id array for the parameter. */
	p_ctrl_ids,
	
    /** Specifies which additional rollup/map the parameter is supposed to appear in. A parameter may have 
        multiple instance of p_uix. */
    p_uix,

    /** Default can be taken from .ini file */
	p_configurable_default,

    /** Specifies a tooltip string by resource ID to be displayed for the user interface control associated with a 
        parameter described in a parameter block. The p_tooltip flag needs to be followed by the resource id of the 
        string representing the tooltip. 

        Example:
        \code p_tooltip, <tooltip_str_id> \endcode
        */
    p_tooltip,

    /** Specfies asset type ID.  The value must be a MaxSDK::AssetManagement::AssetType enum value.  This creates 
    an asset that is managed by the asset manager.

    Only one of p_assetTypeID and p_assetTypeName may be specified.  If neither is specified, the asset pointed to by 
    TYPE_FILENAME is stored as a string. */ 
	p_assetTypeID,

    /** Specifies asset type name. The value must be a MaxSDK::AssetManagement::AssetType enum name.  If the name is not found,
    AssetType.kOtherAsset is used. 

    Only one of p_assetTypeID and p_assetTypeName may be specified.  If neither is specified, the asset pointed to by 
    TYPE_FILENAME is stored as a string. */
	p_assetTypeName,

    /** When this tag is present, all values specified by p_default, p_ms_default, p_range, and the spinner scale are treated as being in meters - 
        as opposed to normally being in system units, when this tag is not present. Meters were chosen, over having configurable units, for 
        simplicity and to avoid breaking SDK compatibility. Valid for parameters of types TYPE_FLOAT and TYPE_WORLD. **/
    p_defaults_and_ranges_in_meters,

    /** This allows you to supply a non-localized parameter name for this parameter. This name should be the parameter name in English, and
		should be specified if the specified parameter name ("radialStrength" in the example below) is not the same as the localized name of 
		the parameter acquired using the parameter's resource ID (IDS_EP_RDSTR in the example below).

		Example:
		\code 
			PB_RADSTR, _T("radialStrength"), TYPE_FLOAT, P_ANIMATABLE + P_RESET_DEFAULT, IDS_EP_RDSTR,
				p_default,          0.5f,
				p_range,            0.0f,1000.0f,
				p_ui,               TYPE_SPINNER, EDITTYPE_FLOAT, IDC_VORTEX_RD, IDC_VORTEX_RDSPIN, SPIN_AUTOSCALE, 
				p_nonLocalizedName, _T("Radial Pull Strength"), 
				p_end, 
		\endcode
		where resource string IDS_EP_RDSTR is "Radial Pull Strength" in English language pack.
		*/
	p_nonLocalizedName,

    /** Signals the end of the "required_param_specs" entry in a parameter block descriptor. */
	p_end = p_default + 1024,

    /** \internal */
	properties,

    /** \internal */
	enums,
};
//!@}
