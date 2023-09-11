//**************************************************************************/
// Copyright (c) 1998-2011 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Universal Max String
// AUTHOR:      Richard Houle
// DATE:        Jan 27 2011
//***************************************************************************/

#ifndef __MAXSDK_MAXSTRING_H__
#define __MAXSDK_MAXSTRING_H__

#include "WindowsDefines.h"
#include <tchar.h>
#include "maxheap.h"
#include "strbasic.h"

namespace MaxSDK { namespace Util {

// forward declarations
class MaxStringData;
class MaxStringInternal;
class MaxStringCastUTF8;
class MaxStringCastCP;
template<typename ChType> class MaxStringCast;

/*! String encodings used in the MaxString class.
	In addition to the standard Windows constants like CP_ACP, CP_UTF8, etc... The following
	constants can also be used as encoding. */
enum MaxStringDataEncoding {
	MSDE_CP_UTF16            = 65012, //!< Encoding for UTF16. This is 2 - 4 bytes per character.
	MSDE_CP_ASCII            = 65013, //!< Encoding for windows code pages with 1 byte per character. This encoding only recognizes characters up to 127.
	MSDE_CP_UCS4             = 65014, //!< Encoding up to 4 bytes per character.
	MSDE_CP_MASK             = 0x0fffffff, //!< Mask for use in testing encoding.
};

/*! Stores all the strings inside CStr and WStr.
	CStr and WStr use this class to perform their internal operations and hold
	different encodings for a string.
	It has several goals:
	\li Limit the number of conversions between codepage and string format. (ACP -> UTF8 -> UTF16)
	This is done by storing several version of the same string.
	\li Limit the number of string copies. This class uses a lazy copy algorithm.
	Several MaxString objects will use the same memory blocks, and are copied prior
	to modification.
	\li Prevent loss of information. String operations done with this object will use
	the most-optimized way of performing a string operation. ie. Concatenating an
	UTF-16 string with a ACP string will give a UTF-16 string.
	Note: ACP: Active Code Page is abbreviated as ACP. Depending on the Windows active language, the active code
	page can be different. ACP always refer to the code page that Windows API functions
	expect. */
class UtilExport MaxString : public MaxHeapOperators
{
	friend class MaxStringInternal;

protected:
	//! The data member that holds all encoding and string information.
	MaxStringData* p;

public:
	//! \defgroup Constructors Constructors
	//!@{
	//! Default Constructor
	MaxString();

	/*! Copy Constructor
		The implementation of this function can be considered cheap in processor 
		and memory usage. As long as both MaxString objects aren't changed, they will share
		the same reference. All MaxString's members that modify this object will
		perform a copy before modifying the data if it's referenced several times. */
	MaxString(const MaxString&);
	//! Destructor
	~MaxString();
	//!@}

protected:
	//! Alternate protected constructor.
	MaxString(MaxStringData* data);
	//! protected assignment operator for an internal data type.
	MaxString& operator = (MaxStringData* data);

public:
	//! \defgroup Operators Concatenation Operators
	//!@{
	//! Assignment operator for a MaxString.
	MaxString& operator = (const MaxString&);

	//! Create a new string that is the concatenation of this string and another.
	MaxString operator + (const MaxString&) const;

	//! Appends a string at the end of this string.
	MaxString& operator += (const MaxString&);
	//!@}

	//! \defgroup Comparators Comparison Operators and methods
	//!@{
	/*! Check if the strings are equal. 
		A common code page is determined for the 2 MaxStrings, strings with those encodings are created if necessary, and then strcmp / wcscmp is used to compare those strings.
		\return Values are -1, 0, or 1. */
	int Compare(const MaxString&) const;

	//! Compare this string with another while ignoring the case.
	int CompareIgnoreCase(const MaxString&) const;

	/*! Equality operator.
		\return  true if the strings are equal; otherwise false. */
	bool operator == (const MaxString&) const;
	/*! Inequality operator.
		\return  false if the strings are equal; otherwise true. */
	bool operator != (const MaxString&) const;
	/*! Returns true if this string is less than the other; otherwise false. */
	bool operator <  (const MaxString&) const;
	/*! Returns true if this string is less than or equal to the other; otherwise false. */
	bool operator <= (const MaxString&) const;
	/*! Returns true if this string is greater than the other; otherwise false. */
	bool operator >  (const MaxString&) const;
	/*! Returns true if this string is greater than or equal to the other; otherwise false. */
	bool operator >= (const MaxString&) const;
	//!@}
public:

	//! Determine if this string is null. Note: an empty string is not null.
	bool IsNull() const;
	//! Clears the data of this object.
	void SetNull();

public:
	/**
	 * Allocate a new string inside this object and returns
	 * a modifiable pointer to it. You can modify this string
	 * as long as you don't perform any other operation with
	 * this object's instance. */
	char* AllocBufferCP(UINT codepage, size_t bytes);
	
	//! \see MaxString::AllocBufferCP(UINT,size_t)
	char* AllocBufferACP(size_t bytes);

	//! \see MaxString::AllocBufferCP(UINT,size_t)
	char* AllocBufferUTF8(size_t bytes);

	//! \see MaxString::AllocBufferCP(UINT,size_t)
	wchar_t* AllocBufferUTF16(size_t wchars);

	//! \see MaxString::AllocBufferCP(UINT,size_t)
	unsigned int* AllocBufferUTF32(size_t uint);


public:

	//! \defgroup Encoding Setter Methods. These set the MaxString's content based on the string passed in.
	//!@{
	/*!	Reinitializes the instance with the specified string and code page
		\param codepage - The encoding of the string passed in.
		\param string - The new text to replace this string with
		\param length - Length in characters of the string parameter. 
			Use this only when you already know the size of the string getting passed in.
			Otherwise leave the parameter out, and by default the function will compute the 
			length for you. */
	void SetCP      (UINT codepage, const char* string, size_t length = (size_t)-1);

	/*!Reinitializes the instance with the specified Active Code Page (ACP) string.
		\param string - The new text to replace this string with
		\param length - Length in characters of the string parameter. 
			Use this only when you already know the size of the string getting passed in.
			Otherwise leave the parameter out, and by default the function will compute the 
			length for you. */
	void SetACP     (const char* string,  size_t length = (size_t)-1);

	/*! Reinitializes the instance with the specified UTF8 string.
		\param string - The new text to replace this string with
		\param length - Length in characters of the string parameter. 
			Use this only when you already know the size of the string getting passed in.
			Otherwise leave the parameter out, and by default the function will compute the 
			length for you. */
	void SetUTF8    (const char* string,  size_t length = (size_t)-1);

	/*! Reinitializes the instance with the specified UTF16 string.
		\param string - The new text to replace this string with
		\param length - Length in characters of the string parameter. 
			Use this only when you already know the size of the string getting passed in.
			Otherwise leave the parameter out, and by default the function will compute the 
			length for you. */
	void SetUTF16   (const wchar_t* string, size_t length = (size_t)-1);
	//!@}

public:

	//! \defgroup Conversion methods Instance methods for converting to other encodings.
	//!@{
	/*! Returns a MaxStringCastCP which in turn enables casting to a char array encoded with a specific code page. 
		The char array held by the MaxStringCastCP instance is valid as long as the instance is valid.
	\param codepage - The code page to convert to
	\param length [out] - If length is not null, the size of the resulting string is stored to the length pointer.
	\return - A MaxStringCastCP instance that can be used to cast to a char* array. */
	MaxStringCastCP ToCP(UINT codepage, size_t* length = NULL) const;

	/*! Cast this object to a char array using Active Code Page encoding
		\param length [out] - If length is not null, the size of the resulting string is stored to the length pointer.
		\return A MaxStringCast<char> instance that can be used to cast to a char* array.
		\see MaxString::ToCP(UINT,size_t*) */
	MaxStringCast<char> ToACP(size_t* length = NULL) const; 

	/*! Cast this object to a UTF8 encoded string. 
		\param length [out] - If length is not null, the size of the resulting string is stored to the length pointer.
		\return A MaxStringCastUTF8 instance that can be used to cast to a char* array.
		\see MaxString::ToCP(UINT,size_t*) */
	MaxStringCastUTF8 ToUTF8(size_t* length = NULL) const;

	/*! Cast this object to a UTF16 or UNICODE encoded string. 
		\param length [out] - If length is not null, the size of the resulting string is stored to the length pointer.
		\return A MaxStringCast<wchar_t> instance that can be used to cast to a wchar_t* array.
		\see MaxString::ToCP(UINT,size_t*) */
	MaxStringCast<wchar_t> ToUTF16(size_t* length = NULL) const;

	/*! Cast this object to a UTF32 encoded string. 
		\param length [out] - If length is not null, the size of the resulting string is stored to the length pointer.
		\return A MaxStringCast<unsigned int> instance that can be used to cast to an unsigned int* array.
		\see MaxString::ToCP(UINT,size_t*) */
	MaxStringCast<unsigned int> ToUTF32(size_t* length = NULL) const;
	//!@}

	//! Get the number of characters in the string, based on the encoding. 
	size_t Length(UINT encoding) const;

	//! Returns the number of times this string has been converted to other encodings.
	int ConversionCount() const;

	/*! Returns the number of characters allocated in memory, in which the string is stored. (Including the NULL character).
		This is different from the number of characters actually stored for the string. Thus this can be bigger
		than the length of the string. 
		It might actually be a little bit bigger than the length of
		the string if this object was manipulated with AllocBufferCP()
		or AllocBufferUTF16(). */
	size_t AllocatedChars() const;

public:

	//! \defgroup Factory methods 
	//! Methods for Creating MaxString instances.
	//!@{
	/*! Construct a MaxString that will contain a copy of a string passed in.
		\param codepage - The code page of the string that is passed in
		\param string - The string to copy
		\param length - The length of the string that is passed in. Use this when the string length is already known. Doing so will increase performance.
		\return - A new MaxString instance */
	static MaxString FromCP(UINT codepage, const char* string, size_t length = (size_t)-1);

	/*! Construct a MaxString from an Active Code Page encoded string
		\param string - The string to copy
		\param length - The length of the string that is passed in. Use this when the string length is already known. Doing so will increase performance.
		\return - A new MaxString instance */
	static MaxString FromACP(const char* string,  size_t length = (size_t)-1);

	/*! Construct a MaxString from a UTF8 encoded string
		\param string - The string to copy
		\param length - The length of the string that is passed in. Use this when the string length is already known. Doing so will increase performance.
		\return - A new MaxString instance */
	static MaxString FromUTF8(const char* string,  size_t length = (size_t)-1);

	/*! Construct a MaxString from a UTF16 encoded wide string
		\param string - The string to copy
		\param length - The length of the string that is passed in. Use this when the string length is already known. Doing so will increase performance.
		\return - A new MaxString instance */
	static MaxString FromUTF16(const wchar_t* string, size_t length = (size_t)-1);

	/*! Construct a MaxString from a UTF32 encoded wide string
		\param string - The string to copy
		\param length - The length of the string that is passed in. Use this when the string length is already known. Doing so will increase performance.
		\return - A new MaxString instance */
	static MaxString FromUTF32(const unsigned int* string, size_t length = (size_t)-1);

	//! Generate a MaxString out of Win32 error code.
	static MaxString FromWin32Error(DWORD err);

	/*! Generate a MaxString out of an ANSI error code.
		ANSI error codes are used principally when using
		the standard C libraries functions.
		Implemented using the strerror function. */
	static MaxString FromAnsiError(int err);
	//!@}

public:

	//! \defgroup Code Page Query Methods
	//!@{
	/*! Normalize the code page.
		Will resolve encoding aliases and returns their meaning.
		\param code - The code page value to convert.
		CP_ACP        (Active Code Page), returns GetACP().
		CP_OEMCP      (BIOS's Code Page), returns GetOEMCP().
		CP_MACCP      
		CP_THREAD_ACP (Current thread's Code Page). Calls GetLocaleInfo().
		\return The new code page value */
	static UINT NormalizeCP(UINT code);

	/*! Gets a common code page.
		Determine the ideal intermediate codepage between two others to perform operation
		like string compare, string concatenate, etc. */
	static UINT CommonLosslessCP(UINT codepage1, UINT codepage2);
	
	/*! Gets a common code page.
		Determine the ideal intermediate codepage between this MaxString and another one.
		\see CommonLosslessCP(UINT, UINT) */
	UINT CommonLosslessCP(MaxString&);
	//!@}

public:

#ifdef MCHAR_IS_WCHAR
	/**
	 * \see AllocBufferUTF16
	 * \see AllocBufferACP
	 */
	inline wchar_t* AllocBuffer(size_t s) { return AllocBufferUTF16(s); }

	/**
	 * \see ToUTF16
	 * \see ToACP
	 */
	inline const MaxStringCast<wchar_t> ToMCHAR(size_t* length = NULL) const;

	/**
	 * \see FromUTF16
	 * \see FromACP
	 */
	static inline MaxString FromMCHAR(const wchar_t* string, size_t length = (size_t)-1) { return FromUTF16(string, length); }
#else
	inline        char*               AllocBuffer(size_t s)                                        { return AllocBufferACP(s); }
	inline const  MaxStringCast<char> ToMCHAR    (size_t* length = NULL) const;
	static inline MaxString           FromMCHAR  (const char* string, size_t length = (size_t)-1)  { return FromACP(string, length); }
#endif

	//! \defgroup BSTR conversion methods
	//!@{

	/*! Return a BSTR of this string. The string is allocated with SysAllocString,
	 * and the caller is responsible to free it using SysFreeString. */
	wchar_t* ToBSTR() const;

	/*! Construct a MaxString out of a BSTR.
		\see FromUTF16 */
	static inline MaxString FromBSTR(const wchar_t* string, size_t length = (size_t)-1) { return FromUTF16(string, length); }

	/*! Construct a OLE string. 
		\return A MaxStringCast<wchar_t> which can be cast to a wchar_t for converting to an OLESTR */
	inline const MaxStringCast<wchar_t> ToOLESTR(size_t* length = NULL) const;

	/*! Construct a MaxString out of a OLESTR.
		\see FromUTF16 */
	static inline MaxString FromOLESTR(const wchar_t* string, size_t length = (size_t)-1) { return FromUTF16(string, length); }
	//!@}

};




//! \defgroup Casting Conversion Classes
//!@{

/*!	A MaxStringCast is used when casting a MaxString to a type. 
It holds
1. A MaxString, which increments the ref count on the MaxStringData (so the MaxStringData pointer won't be deleted)
2. A pointer to the appropriate string buffer in the MaxStringData.

So, the string pointer is guaranteed to be valid as along as the MaxStringCast instance is alive. 
Warning: If you cast directly to a, for example, char*, then something else could operate on the MaxString 
causing it to delete the string buffer in the MaxStringData, and you would be left holding a 
dangling pointer. 

Note, data held by this class should be treated as read only and not modifiable. 
Thus its operator methods only convert to constant types. 
For instance if used with a char type, its operators will only return a const char*. 
If you want a char* type need to create a copy instead. Therefore do not use const_cast<> with this class. */
template <typename ChType> 
class MaxStringCast : public MaxHeapOperators 
{
protected:
	MaxString     p;
	const ChType* buf;

public:
	inline MaxStringCast() 
		: buf(NULL)
	{
	}

	inline MaxStringCast(const MaxString& string, const ChType* w) 
		: p(string)
		, buf(w)
	{
	}

public:
	/** Returns a const pointer to the string. If the string is NULL, a pointer to 0 is returned. */
	inline operator const ChType*() const
	{
		return (buf) ? buf: null_data();
	}

	/** Returns a const pointer to the string. If the string is NULL, a pointer to 0 is returned. */
	inline const ChType* data() const
	{
		return (buf) ? buf: null_data();
	}

	/** Returns a const pointer pointer to 0. */
	inline const ChType* null_data() const
	{
		static ChType null_char = 0; 
		return &null_char;
	}

	/**
	 * returns true if this string is null or empty, false otherwise
	 */
	bool isNull() const
	{
		return 0 == data()[0];
	}

public:
#ifdef MCHAR_IS_WCHAR
	/**
	 * Cast this object to a MCHAR. The string returned
	 * is valid as long as the MaxStringCast exists.
	 */
	inline const MaxStringCast<wchar_t> ToMCHAR(size_t* length = NULL) const
	{
		return p.ToUTF16(length);
	}
#else
	/**
	 * Cast this object to a MCHAR. The string returned
	 * is valid as long as the MaxStringCast exists.
	 */
	inline const MaxStringCast<char> ToMCHAR(size_t* length = NULL) const
	{
		return p.ToACP(length);
	}
#endif

	/**
	 * Cast this object to a wchar_t. The string returned
	 * is valid as long as the MaxStringCast exists.
	 */
	inline const MaxStringCast<wchar_t> ToOLESTR(size_t* length = NULL) const
	{
		return p.ToUTF16(length);
	}

	/** Returns a const ref to the owning MaxString. */
	inline const MaxString& owner() const
	{
		return p;
	}

};

//! A Utility class for helping to convert to UTF8.
//! \see MaxStringCast
class MaxStringCastUTF8 : public MaxStringCast<char>
{
public:
	inline MaxStringCastUTF8() 
		: MaxStringCast<char>()
	{
	}

	inline MaxStringCastUTF8(const MaxString& string, const char* w) 
		: MaxStringCast<char>(string, w)
	{
	}
};

//! A Utility class for helping to convert to a specified code page.
//! \see MaxStringCast
class MaxStringCastCP : public MaxStringCast<char>
{
public:
	inline MaxStringCastCP() 
		: MaxStringCast<char>()
	{
	}

	inline MaxStringCastCP(const MaxString& string, const char* w) 
		: MaxStringCast<char>(string, w)
	{
	}
};

//!@}

#ifdef MCHAR_IS_WCHAR
inline const MaxStringCast<wchar_t> MaxString::ToMCHAR(size_t* length) const
{
	return ToUTF16(length);
}
#else
inline const MaxStringCast<char> MaxString::ToMCHAR(size_t* length) const
{
	return ToACP(length);
}
#endif

inline const MaxStringCast<wchar_t> MaxString::ToOLESTR(size_t* length) const
{
	return ToUTF16(length);
}

}}

#endif
