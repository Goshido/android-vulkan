//**************************************************************************/
// Copyright (c) 1998-2021 Autodesk, Inc.
// All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// FILE:        maxcolors.h
// DESCRIPTION: Typedefs for general jaguar types.
// AUTHOR:      Philippe Groarke
// HISTORY:     created 20 August 2021
//**************************************************************************/
#pragma once
#include <cstdint>

/*! \defgroup bitmapStorageTypes Bitmap Storage Types */
//!@{
//! \brief 8 bits for each of the Red, Green and Blue components.
/*! Structure Color48, Structure Color64. */
struct Color24
{
	constexpr Color24(uint8_t vr = 0, uint8_t vg = 0, uint8_t vb = 0)
			: r(vr)
			, g(vg)
			, b(vb)
	{
	}
	uint8_t r, g, b;
};

//! \brief 16 bits for each of the Red, Green and Blue components.
/*! \sa Structure Color24, Structure Color64. */
struct Color48
{
	constexpr Color48(uint16_t vr = 0, uint16_t vg = 0, uint16_t vb = 0)
			: r(vr)
			, g(vg)
			, b(vb)
	{
	}
	uint16_t r, g, b;
};

//! \brief 16 bits for each of the Red, Green, Blue, and Alpha components.
/*! \sa Structure Color24, Structure Color48. */
struct Color64
{
	constexpr Color64(uint16_t vr = 0, uint16_t vg = 0, uint16_t vb = 0, uint16_t va = 0)
			: r(vr)
			, g(vg)
			, b(vb)
			, a(va)
	{
	}
	uint16_t r, g, b, a;
};

//-- Pixel storage classes used by BitmapManager ----------------------------------------------------

//! \brief 24 bit color: 8 bits each for Red, Green, and Blue.
/*! \sa Class Bitmap, Class BitmapStorage, Class BitmapManager. */
struct BMM_Color_24
{
	constexpr BMM_Color_24(uint8_t vr = 0, uint8_t vg = 0, uint8_t vb = 0)
			: r(vr)
			, g(vg)
			, b(vb)
	{
	}
	uint8_t r, g, b;
};

//! \brief 32 bit color: 8 bits each for Red, Green, Blue, and Alpha.
/*! \sa Class Bitmap, Class BitmapStorage, Class BitmapManager. */
struct BMM_Color_32
{
	constexpr BMM_Color_32(uint8_t vr = 0, uint8_t vg = 0, uint8_t vb = 0, uint8_t va = 0)
			: r(vr)
			, g(vg)
			, b(vb)
			, a(va)
	{
	}
	uint8_t r, g, b, a;
};

//! \brief 48 bit color: 16 bits each for Red, Green, and Blue.
/*! \sa Class Bitmap, Class BitmapStorage, Class BitmapManager. */
struct BMM_Color_48
{
	constexpr BMM_Color_48(uint16_t vr = 0, uint16_t vg = 0, uint16_t vb = 0)
			: r(vr)
			, g(vg)
			, b(vb)
	{
	}
	uint16_t r, g, b;
};

//! \brief 64 bit color: 16 bits each for Red, Green, Blue, and Alpha.
/*! \sa Class Bitmap, Class BitmapStorage, Class BitmapManager. */
struct BMM_Color_64
{
	constexpr BMM_Color_64(uint16_t vr = 0, uint16_t vg = 0, uint16_t vb = 0, uint16_t va = 0)
			: r(vr)
			, g(vg)
			, b(vb)
			, a(va)
	{
	}
	uint16_t r, g, b, a;
};

//! \brief High Dynamic Range bitmaps make use of this class to store color information using floating point values.
/*! \sa Class Bitmap, Class BitmapStorage, Class BitmapManager, ~{ Working with Bitmaps }~. */
struct BMM_Color_fl
{
	constexpr BMM_Color_fl(float vr = 0.0f, float vg = 0.0f, float vb = 0.0f, float va = 0.0f)
			: r(vr)
			, g(vg)
			, b(vb)
			, a(va)
	{
	}

	/*! Storage for the floating point color information. */
	float r, g, b, a;

	/*! \remarks Returns the address of the floating point values. */
	operator float*()
	{
		return &r;
	}
	/*! \remarks Returns the address of the floating point values. */
	operator const float*() const
	{
		return &r;
	}

	/*! \remarks Returns the specified color c clipped (limited to) the range 0 to 65535. */
	static uint16_t clipColor(float c)
	{
		return c <= 0.0f ? 0 : c >= 1.0f ? 65535 : static_cast<uint16_t>(c * 65535.0);
	}
};
//!@}
