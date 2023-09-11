//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once
#include "MaxHeap.h"
#include "CoreExport.h"
#include "strbasic.h"

// This must be updated if a new entry is added to DimType!
#define NUM_BUILTIN_DIMS	10

/*! \defgroup dimType Dimension Types */
//!@{
enum DimType {
	DIM_WORLD,		//!< For spatial value
	DIM_ANGLE,		//!< For Angular values
	DIM_COLOR,	 	//!< For colors with a range of 0-1
	DIM_COLOR255,	//!< For colors with a range of 0-255
	DIM_PERCENT,  	//!< For Percentage with a range of 0-100
	DIM_NORMALIZED,	//!< For normalized values with a range of 0-1
	DIM_SEGMENTS,	//!< For segments
	DIM_TIME,		//!< For time units
	DIM_CUSTOM,		//!< For custom units
	DIM_NONE
};
//!@}

// These two classes describe the dimension of a parameter (sub-anim).
// The dimension type and possibly the dimension scale (if the type is
// custom) are used to determine a scale factor for the parameter.
// When a controller is drawing a function curve, it only needs to
// use the Convert() function - the scale factor is rolled into the single
// 'vzoom' parameter passed to PaintFCurves.
// So, for a controller to plot a value 'val' at time t it would do the
// following:
// int x = TimeToScreen(t,tzoom,tscroll);
// int y = ValueToScreen(dim->Convert(val),rect.h()-1,vzoom,vscroll);
//
/*! \sa  Class ParamDimension.
\par Description:
This class (along with ParamDimension) describes the dimension of a parameter.
This dimension can be considered a unit of measure. It describes the
parameter's type and order of magnitude.\n\n
The dimension type and possibly the dimension scale (if the type is custom) are
used to determine a scale factor for the parameter. When a controller is
drawing a function curve, it only needs to use the Convert() function - the
scale factor is rolled into the single 'vzoom' parameter passed to
<b>Animatable::PaintFCurves()</b>. So, for a controller to plot a value 'val'
at time t it would do the following:\n\n
<b>int x = TimeToScreen(t,tzoom,tscroll);</b>\n\n
<b>int y = ValueToScreen(dim-\>Convert(val),rect.h()-1,vzoom,vscroll);</b> 
*/
class ParamDimensionBase: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~ParamDimensionBase() {}
		/*! \remarks Returns the dimension type of the parameter. See \ref dimType */
		virtual DimType DimensionType()=0;
		/*! \remarks When a controller needs to display the parameter values (for example in
		the function curve editor) it converts the value using this method.
		\param value The value to convert.
		\return  The converted value. */
		virtual float Convert(float value)=0;
		/*! \remarks This method is used to un-convert from a displayed value to the actual value
		\param value The value to un-convert.
		\return The un-converted value. */
		virtual float UnConvert(float value)=0;

		/*! \brief Converts integer values based on the parameter dimensions.
		In order to ensure that integers larger than 2 ^ 24 - 1 are converted properly, use this method instead of 
		ParamDimensionBase::Convert(float value)
		\remarks This method converts the integer parameter value to a float value and calls Convert on that float value. 
		If the return value from that call is unchanged, the original integer value is returned, otherwise the 
		returned float value cast to an integer is returned. This is to handle the case where a large integer 
		value (>2^24 - 1) that cannot be represented as a float value is passed. 
		This would result in a value change even though the Convert implementation itself did not change the value.
		\param value The value to convert.
		\return The converted value. */
		CoreExport int Convert(int value);

		/*! \brief Unconverts integer values based on the parameter dimensions.
		In order to ensure that integers larger than 2 ^ 24 - 1 are converted properly, use this method instead of
		ParamDimensionBase::UnConvert(float value)
		\remarks This method converts the integer parameter value to a float value and calls UnConvert on that float value.
		If the return value from that call is unchanged, the original integer value is returned, otherwise the
		returned float value cast to an integer is returned. This is to handle the case where a large integer
		value (>2^24 - 1) that cannot be represented as a float value is passed.
		This would result in a value change even though the UnConvert implementation itself did not change the value.
		\param value The value to convert.
		\return The converted value. */
		CoreExport int UnConvert(int value);
}; 

/*! \sa  Class ParamDimensionBase,  \ref dimType.
\par Description:
Any parameter that can be controlled by a controller has a dimension. This
dimension can be considered a unit of measure. It describes its type and its
order of magnitude. When a controller needs to display the parameter values
(for example in the function curve editor) it converts the value using its
parameter dimension Convert() function. It can also convert back using the
Unconvert() function.\n\n
Some parameters are stored over one range of values and displayed in another.
For example parameter that use <b>stdAngleDim</b> store their parameters in
radians but display them in degrees. By using <b>stdAngleDim</b> the value is
converted to the proper format for display. Some parameter dimensions do not
perform any conversion, for example <b>stdWorldDim</b>.\n\n
There are several default parameter dims implemented. Listed with each one is
the type, convert functions and range of values usually stored (these are not
enforced in any way).\n\n
If the type of parameter for your plug-in does not fit within any of the
dimensions listed here you may simply use <b>defaultDim</b>. This performs no
conversions and has no range.\n\n
<b>ParamDimension *defaultDim;</b>\n\n
Convert() is original value.\n\n
UnConvert() is original value.\n\n
Range: None.\n\n
<b>ParamDimension *stdWorldDim;</b>\n\n
The DimType is <b>DIM_WORLD</b>\n\n
Convert() return original value.\n\n
UnConvert() returns original value.\n\n
<b>ParamDimension *stdAngleDim;</b>\n\n
The DimType is <b>DIM_ANGLE.</b>\n\n
Convert() is RadToDeg()\n\n
UnConvert() is DegToRad()\n\n
<b>ParamDimension *stdColorDim;</b>\n\n
The DimType is <b>DIM_COLOR</b>\n\n
Convert() return original value.\n\n
UnConvert() returns original value.\n\n
Range: 0-1\n\n
<b>ParamDimension *stdColor255Dim;</b>\n\n
The DimType is <b>DIM_COLOR255</b>\n\n
Convert() is value * 255.0f\n\n
UnConvert() is value / 255.0f\n\n
Range: 0-255\n\n
<b>ParamDimension *stdPercentDim;</b>\n\n
The DimType is <b>DIM_PERCENT</b>\n\n
Convert() is value * 100.0f\n\n
UnConvert() is value / 100.0f\n\n
Range: 0-100\n\n
<b>ParamDimension *stdNormalizedDim;</b>\n\n
The DimType is <b>DIM_NORMALIZED</b>\n\n
Convert() is original value.\n\n
UnConvert() is original value.\n\n
Range: 0-1\n\n
<b>ParamDimension *stdSegmentsDim;</b>\n\n
The DimType is <b>DIM_SEGMENTS</b>\n\n
Convert() is original value.\n\n
UnConvert() is original value.\n\n
<b>ParamDimension *stdTimeDim;</b>\n\n
The DimType is <b>DIM_TIME</b>\n\n
Convert() is value/GetTicksPerFrame()\n\n
UnConvert()is value*GetTicksPerFrame() (see the section
~{ Time Functions }~ for
details). \n  If the DimType is custom then the methods below must be
implemented. Note: This class is derived from ParamDimensionBase which provides
methods <b>DimensionType()</b>, <b>Convert()</b> and <b>Unconvert()</b>. */
class ParamDimension : public ParamDimensionBase {
	public:
		// If the DimType is custom than these must be implemented.
		/*! \remarks Returns the dimension scale. */
		virtual float GetDimScale() {return 1.0f;}
		/*! \remarks Sets the dimension scale. */
		virtual void SetDimScale() {}
		/*! \remarks Returns the name of the dimension. */
		virtual MCHAR* DimensionName() {return const_cast<MCHAR*>(_M(""));}
};

// These point to default implementations for the standard DIM types.
CoreExport extern ParamDimension *defaultDim;
CoreExport extern ParamDimension *stdWorldDim;
CoreExport extern ParamDimension *stdAngleDim;
CoreExport extern ParamDimension *stdColorDim;
CoreExport extern ParamDimension *stdColor255Dim;
CoreExport extern ParamDimension *stdPercentDim;
CoreExport extern ParamDimension *stdNormalizedDim;
CoreExport extern ParamDimension *stdSegmentsDim;
CoreExport extern ParamDimension *stdTimeDim;