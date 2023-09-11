//**************************************************************************/
// Copyright (c) 1994-2011 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

//  3ds Max 2013 and later are always Unicode

#if defined(_UNICODE) && defined(_MBCS)
	#error Cannot compile with both _UNICODE and _MBCS enabled!
#endif

#ifndef UNICODE
#define    UNICODE
#endif

#define    STRCONST L

// Bring in the generic international text header file
#include <tchar.h>

#define MCHAR_IS_WCHAR

/*! MBCS/Unicode helper defines
std::wofstream doesn't mix well with Unicode. If you try to output a character that is not representable 
in the current (window user) code page, the stream is flagged as 'in error', and no further 
output to that stream occurs. 
Plus, file output encoding should be that specified by the user, accessible via Interface14::DefaultTextSaveCodePage
Thus, it is recommended that M_STD_FOSTREAM not be used. It is much better to use the 
TextFile::Reader/Writer classes and use the codepage accessed via Interface14::DefaultTextSaveCodePage. 
Likewise, the use std::wifstream (and _tfopen/fopen in text mode) is not recommended when reading string data as the
current (window user) code page will be used. File input encoding should that specified by the user, accessible via 
Interface14::DefaultTextLoadCodePage
Thus, it is recommended that M_STD_FISTREAM (and _tfopen/fopen in text mode) not be used. It is much better to use the 
TextFile::Reader/Writer classes and use the codepage accessed via Interface14::DefaultTextSaveCodePage. 
!*/
#define MCHAR					wchar_t
#define LPCMSTR					const MCHAR*
#define LPMSTR					MCHAR*
#define _M_TEXT(x)				L ## x
#define M_STD_STRING			std::wstring
#define M_LOADSTRING			::LoadStringW
#define M_STD_OSTRINGSTREAM		std::wostringstream
#define M_STD_ISTRINGSTREAM		std::wistringstream
#define M_STD_CERR				std::wcerr
#define M_STD_CIN				std::wcin
#define M_STD_OSTREAM			std::wostream
#define M_STD_ISTREAM			std::wistream
#define M_STD_COUT				std::wcout
#define M_STD_FOSTREAM			std::wofstream
#define M_STD_FISTREAM			std::wifstream
#define M_STD_ISTRSTREAM		std::wistringstream

/**
* The 'EOF' value is unicode/mbcs build dependent for _puttc, _puttch, 
* _ungettc, _ungettch, _ungettch_nolock, _fputts, _fputtc, _gettc, and 
* _fgettc (and maybe other methods)
*/
#define M_EOF					WEOF

//! Used to wrap string literals
#define _M(x)		_M_TEXT(x)

// These macros work in conjunction with the VC conversion macros (A2W, A2T, W2T, etc)
// See the MSDN documentation for details.  If you get compile errors such as
// 
// error C2065: '_lpa' : undeclared identifier
// error C2065: '_convert' : undeclared identifier
// error C2065: '_acp' : undeclared identifier
// 
// when trying to use any of these macros, it's probably because there's a
// 
// USES_CONVERSION
// 
// missing in your function.

// M2A: Build-specific to ASCII
// A2M: ASCII to build-specific
// M2W: Build-specific to wide char
// W2M: Wide char to build-specific
// M2T: Max definition of MCHAR to current project definition of TCHAR
// T2M: current project definition of TCHAR to Max definition of MCHAR 

// NOTE: converted strings are stored in a stack-based buffer. so don't say
// something like:
//   return A2M(msg.data());

// NOTE: The CStr/WStr/MaxString conversion methods should be used instead of
// the VC conversion macros. In some (unusual) cases, the VC conversion macros
// return incorrect results.

#define M2A(p)					CW2A(p)
#define A2M(p)					CA2W(p)
#define M2W(p)					(p)
#define W2M(p)					(p)
#define M2W_UTF(p)				(p)
#define W2M_UTF(p)				(p)
#define M2T(p)					(p)
#define T2M(p)					(p)
#define A2CT_UTF(lpa)			CA2W(lpa, CP_UTF8)
#define T2CA_UTF(lpw)			CW2A(lpw, CP_UTF8)
#define A2CM_CP(lpa)			CA2W(lpa, CP_UTF8) //These two macros are for when you are not sure the encoding, may change later
#define M2CA_CP(lpw)			CW2A(lpw, CP_UTF8)

