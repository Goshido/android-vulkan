/**********************************************************************
 *<
	FILE: gfloat

	DESCRIPTION: Single Precision Floating Point Routines

	CREATED BY: Don Brittain & Dan Silva

	HISTORY:
		12/11/2020 - MK: Moved to single precision implementations, removed ASM references no longer applicable to x64 

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include <math.h>

//-------------------------------------------------------------------------------
// Single precision floating point stuff...
//

/*! \remarks Returns both the sine and cosine of the specified angle as floats. */ 
inline void SinCos (float angle, float *sine, float *cosine) 
{
	*sine = sinf(angle);
	*cosine = cosf(angle);
}

/*! \remarks Returns the sine of the specified angle as a float. */ 
inline float Sin(float angle)
{
	return sinf(angle);
}

/*! \remarks Returns the cosine of the specified angle as a float. */
inline float Cos(float angle)
{
	return cosf(angle);
}

/*! \remarks Returns the square root of the specified argument as a float. */ 
inline float Sqrt(float arg)
{
	return sqrtf(arg);
}


