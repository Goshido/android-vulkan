/**********************************************************************
 *<
	FILE: strclass.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "WindowsDefines.h"
#include "maxheap.h"
#include "strbasic.h"
#include "tab.h"
#include "coreexp.h"
#include "maxstring.h"

#include <iostream>

class WStr;
class UTF8Str;
class QString;

/*-------------------------------------------------------------------------------
  SR NOTE64

  "Only" supports 2G strings.

-------------------------------------------------------------------------------*/ 

//-----------------------------------------------------------------------
// CStr: Simple char string class
//-----------------------------------------------------------------------
/*! \sa  Class WStr, UTF8Str and MaxString, ~{ Guidelines for Handling Character Strings }~.\n\n
\par Description:
A simple character string class. This is the standard character string class
used in 3ds Max. Methods and operators are provided for calculating lengths,
concatenation, substring operations, character searching, case conversion,
comparison, and formatted writing.\n\n
This class automatically allocates the proper amount of space for the string.
This is very handy in the case of internationalization / localization. For
example, if you code something like:\n\n
<b>MSTR myString = GetString(IDS_STRING_ID);</b>\n\n
then <b>myString</b>'s constructor will allocate enough space to store the
resource string no matter how long it is. This is much better than doing the
following:\n\n
<b>MCHAR myString[64];</b>\n\n
<b>_tcscpy(myString, GetString(IDS_STRING_ID));</b>\n\n
because the resource string may turn out to be much longer than 64 bytes once
it's translated to German or French (or whatever).\n\n
As another example, if you have the following code:\n\n
<b>MSTR str1 = _M("This is string1.");</b>\n\n
<b>MSTR str2 = _M("This is string2.");</b>\n\n
Then\n\n
<b>MSTR concatStr = str1 + str2;</b>\n\n
will again yield a (concatenated) string will enough space to hold the
concatenated contents of <b>str1</b> and <b>str2</b>, automatically.\n\n
All methods are implemented by the system. <br> 
\note The memory occupied by a CStr object is cannot be larger than 2Gb.

\note By design, this class assume that all the character data that you use
      to instanciate them or manipulate them are using the code page defined
	  by the current active language of the 3dsMax. It should not be used to
	  contains data in UTF-8 or in other code page than the current one.
*/
class UtilExport CStr: public MaxHeapOperators {
	const char *buf;
	public:
		/*! \remarks Constructor. The string is set to NULL. */
		CStr(); 
		/*! \remarks Constructor. The string is initialized to the string passed. */
		CStr(const char *cs);

		/*! \remarks Constructor. The string is initialized to the string passed. */
		CStr(const CStr& ws);
		/*! \remarks Constructor. The string is initialized to the string passed. */
		CStr(const MaxSDK::Util::MaxString& ws);
		/*! \remarks Constructor. The string is initialized to the string passed. */
		CStr(const MaxSDK::Util::MaxStringCast<char>& ws);
        /*! Conversion constructor, from QString. */
        CStr(const QString& qstring);

		/*! \remarks Destructor. The string is deleted. */
		~CStr();

		/*! Returns a pointer to this string which can be written to with up to 'nchars' characters. 
		\remarks The terminating null character is <b>not</b> included in nchars, so technically up to nchars+1 characters may be written.
		\param nchars The minimum number of characters to allocate in the string buffer (excluding the terminating null character). A value of -1
		specifies that the existing buffer should be re-used. */
		char *dataForWrite(size_t nchars = (size_t)-1);

		/*! \remarks Returns a const pointer to the string. If the string is NULL, a pointer to 0 is returned.*/
		const char *data() const;

		/*! \remarks Returns a const pointer to the string. If the string is NULL, a pointer to 0 is returned. */
		operator const char *() const;
        //! Conversion operator to QString
        operator QString() const;

		// realloc to nchars (padding with blanks)
		/*! \remarks Reallocates the string to contain <b>nchars</b>
		characters. If the string is enlarged it is padded with blanks.
		\par Parameters:
		<b>int nchars</b>\n\n
		Specifies the new number of characters for the string. */
		void Resize(int nchars);

		/*! \remarks Returns the number of chars used to store the string in memory, including spaces, but excluding the terminating NULL character. */
		int Length() const { return ByteCount(); }
		/*! \remarks Returns the number of chars used to store the string in memory, including spaces, but excluding the terminating NULL character. */
		int length() const { return Length(); }
		/*! \remarks Returns the number of bytes used to store the string in memory, including spaces, but excluding the terminating NULL character. */
		int ByteCount() const;

		/*! \remarks Returns the number of natural language characters the string is represented on, including spaces, but excluding the terminal NULL character.
		For multi-byte languages, this may be equal or less than the amount of bytes used to store the string in memory. For example, a Chinese character
		might take two bytes. Length() or ByteCount() will count the character as two distinct char. LanguageCharacterCount() will count it as a single char.*/
		int LanguageCharacterCount() const;
		
		/*! \remarks Returns the total number of char allocated to store the string. Including the NULL character and any unused char. */
		size_t AllocatedChars() const;

		/*! \remarks Returns TRUE if the string length is 0; otherwise FALSE.
		*/
		bool isNull() const { return data()[0]==0; }

		/*! \remarks Assignment operator. */
		CStr & operator=(const CStr& cs);
		/*! \remarks Assignment operator. */
		CStr & operator=(const MaxSDK::Util::MaxString& mstr);
		/*! \remarks Assignment operator. */
		CStr & operator=(const MaxSDK::Util::MaxStringCast<char>& mstr);

        /*! \remarks Assignment operator. In release 3.0 and later this method	check for self-assignment. */
		CStr & operator=(const char *cs);

		// Concatenation operators.
		/*! \remarks Concatenation operator. Returns a new string that is this
		string with string <b>cs</b> appended. */
		CStr operator+(const CStr& cs) const;
		/*! \remarks Concatenation. Returns this string with <b>cs</b>	appended. */
		CStr& operator+=(const CStr& cs); 
		/*! \remarks Concatenation. Returns this string with <b>cs</b>	appended. */
		CStr& Append(const CStr& cs)  { return ((*this) += cs); }
		/*! \remarks Concatenation. Returns this string with <b>cs</b>	appended to the end. */
		CStr& append(const CStr& cs)  { return ((*this) += cs); }
		/*! \remarks Returns this string with all characters from <b>pos</b>	to the end removed.
		\par Parameters:
		<b>int pos</b>\n\n
		Specifies the last position in the string. */
		CStr& remove(int pos);	// remove all chars from pos to end
		/*! \remarks Returns this string with N characters removed from <b>pos</b> to the end.
		\par Parameters:
		<b>int pos</b>\n\n
		Specifies the position to begin removing characters.\n\n
		<b>int N</b>\n\n
		Specifies the number of characters to remove. */
		CStr& remove(int pos, int N);	// remove N chars from pos to end

		// Substring operator
		/*! \remarks Returns a substring of this string, beginning at position
		<b>start</b>, of length <b>nchars</b>. */
		CStr Substr(int start, int nchars) const;
		/*! \remarks Returns a substring of this string, beginning at multi-byte character position
		<b>firstCharacterIndex</b>, of length <b>numberOfMBCharacters</b>. */
		CStr MultiByteCharacterSubString(int firstCharacterIndex, int numberOfMBCharacters) const;
		/*! \remarks Returns the first byte index value of the multi-byte character position
		<b>characterIndex</b>. */
		int FindMultiByteCharacterFirstByteIndex(int characterIndex) const;
		/*! \remarks Returns the last byte index value of the multi-byte character position
		<b>characterIndex</b>. */
		int FindMultiByteCharacterLastByteIndex(int characterIndex) const;

		/*!
		 * Returns the nth character of this string.
		 *
		 * Warning: In some environment, this function may return a partial
		 * character, especially for asiatic languages. If you intend to do
		 * any advance transformation or interpretation of the returned data,
		 * you should consider calling CStr::MultiByteCharacterSubString
		 * instead.
		 *
		 * \param i Index of that character.
		 */
		char operator[](int i) const;	

		// Char search:(return -1 if not found)
		/*! \remarks Returns the index of the first occurrence of character
		<b>c</b> in this string. Returns -1 if not found. */
		int first(char c) const;
		/*! \remarks Returns the index of the last occurrence of character
		<b>c</b> in this string. Returns -1 if not found. */
		int last(char c) const;

		// Comparison
		/*! \remarks Equality operator.
		\return  true if the strings are equal; otherwise false. */
		bool operator==(const CStr &cs) const;
		/*! \remarks Returns true if this string contain only the character <b>c</b>.*/
		bool operator==(char c) const;
		/*! \remarks Inequality operator.
		\return  false if the strings are equal; otherwise true. */
		bool operator!=(const CStr &cs) const;
		/*! \remarks Returns true if this string does not contain only the character <b>c</b>.*/
		bool operator!=(char c) const;
		/*! \remarks Returns true if this string is less than <b>cs</b>; otherwise false. */
		bool operator<(const CStr &cs) const;
		/*! \remarks Returns true if this string is less than or equal to <b>ws</b>; otherwise false. */
		bool operator<=(const CStr &ws) const;
		/*! \remarks Returns true if this string is greater than <b>ws</b>; otherwise false. */
		bool operator>(const CStr &ws) const;
		/*! \remarks Returns true if this string is greater than or equal to <b>ws</b>; otherwise false. */
		bool operator>=(const CStr &ws) const;

		/*! \remarks Converts all character of this string to uppercase. */
		void toUpper();
		/*! \remarks Converts all character of this string to lowercase. */
		void toLower();

		/*! \brief Write a formatted string into this CStr.
		 * Writes the format string, filled in by the optional arguments into
		 * this CStr.  See the ISO C++ documentation for more information on 
		 * printf and format strings.
		 * \param format Specifies how to format the destination string.
		 * \param ... optional arguments to format into the destination string.
		 * \pre format is not null.
		 * \pre There are the correct number of elliptical arguments to fill the 
		 * format string.
		 * \post This string is replaced with the formatted string.
		 * \return The number of characters written to this string, not including
		 * the null terminating character, or a negative value if an error occurs.
		 */
		int printf(const char *format, ...);

		/*! \brief Write a formatted string into this CStr using specified locale.
		 * Writes the format string, filled in by the optional arguments into
		 * this CStr.  See the ISO C++ documentation for more information on
		 * printf_l and format strings.
		 * \param format Specifies how to format the destination string.
		 * \param theLocale The locale to use.
		 * \param ... optional arguments to format into the destination string.
		 * \pre format is not null.
		 * \pre There are the correct number of elliptical arguments to fill the
		 * format string.
		 * \post This string is replaced with the formatted string.
		 * \return The number of characters written to this string, not including
		 * the null terminating character, or a negative value if an error occurs.
		 */
		int printf_l(const char *format, const _locale_t theLocale, ...);

		/*! \brief Write a formatted string into this CStr.
		This method is similar to CStr::printf. Instead of taking a variable list of 
		arguments as parameter, it takes a structure representing a variable list of 
		arguments. This allows CStr objects to be used to build strings based on a 
		format string and a variable number of arguments:
		\code
		void LogMessage(const char* format, ...) {
			va_list args;
			va_start(args, format);
			CStr buf;
			buf.vprintf(format, args);
			va_end(args);
			// log the message contained by buf
		}
		\endcode
		*/
		int vprintf(const char *format, va_list args);

		/*! \brief Write a formatted string into this CStr using specified locale.
		This method is similar to CStr::printf_l. Instead of taking a variable list of
		arguments as parameter, it takes a structure representing a variable list of
		arguments. This allows CStr objects to be used to build strings based on a
		format string, a locale, and a variable number of arguments:
		\code
		void LogMessage_l(const char* format, const _locale_t theLocale, ...) {
			va_list args;
			va_start(args, format);
			CStr buf;
			buf.vprintf_l(format, theLocale, args);
			va_end(args);
			// log the message contained by buf
		}
		\endcode
		*/
		int vprintf_l(const char *format, const _locale_t theLocale, va_list args);

public:
	/*! \remarks Returns the string in memory allocated by SysAllocString. Caller is responsible for freeing memory using SysFreeString. */
	wchar_t*                           ToBSTR() const;
	static CStr                        FromBSTR(const wchar_t* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCastCP      ToCP   (UINT cp, size_t* length = NULL) const;
	static CStr                        FromCP (UINT cp, const char* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCast<char>  ToACP  (size_t* length = NULL) const;
	static CStr                        FromACP(const char* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCastUTF8    ToUTF8  (size_t* length = NULL) const;
	static CStr                        FromUTF8(const char* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCast<WCHAR> ToOLESTR   (size_t* length = NULL) const;
	static CStr                        FromOLESTR(const wchar_t* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCast<WCHAR> ToUTF16  (size_t* length = NULL) const;
	static CStr                        FromUTF16(const wchar_t* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCast<unsigned int>  ToUTF32  (size_t* length = NULL) const;
	static CStr                                FromUTF32(const unsigned int* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxString            ToMaxString() const;
	void                               ToMaxString(MaxSDK::Util::MaxString&) const;
	static CStr                        FromMaxString(MaxSDK::Util::MaxString& string) { return CStr(string); }

	CStr                               ToCStr() const { return *this; }
	static CStr                        FromCStr(const CStr& string) { return CStr(string); }

	WStr                               ToWStr() const;
	static CStr                        FromWStr(const WStr& string);

#ifdef MCHAR_IS_WCHAR
	inline MaxSDK::Util::MaxStringCast<wchar_t> ToMCHAR  (size_t* length = NULL) const { return ToUTF16(length); }
	static inline CStr                          FromMCHAR(const wchar_t* string, size_t length = (size_t)-1) { return FromUTF16(string, length); }

	WStr                               ToMSTR() const;
	static CStr                        FromMSTR(const WStr& string);
#else
	MaxSDK::Util::MaxStringCast<char>  ToMCHAR(size_t* length = NULL) const { return ToACP(length); }
	static inline CStr                 FromMCHAR(const char* string, size_t length = (size_t)-1) { return FromACP(string, length); }

	CStr                               ToMSTR() const { return *this; }
	static CStr                        FromMSTR(const CStr& string) { return CStr(string); }
#endif

	/**
	 * Returns true if the string ends with s; otherwise returns false.
	 *
	 * \param s                   The string to be searched.
	 * \param caseSensitive       If it is true(default), the search is case sensitive; otherwise the search is case insensitive.
	 */
	bool EndsWith(const CStr& s, bool caseSensitive = true);
	
	/**
	 * Returns true if the string ends with c; otherwise returns false. This function overloads EndsWith().
	 *
	 * \param c                   The character to be searched.
	 * \param caseSensitive       If it is true(default), the search is case sensitive; otherwise the search is case insensitive.
	 */
	bool EndsWith(char c, bool caseSensitive = true);

	/**
	 * Returns true if the string starts with s; otherwise returns false.
	 *
	 * \param s                   The string to be searched.
	 * \param caseSensitive       If it is true(default), the search is case sensitive; otherwise the search is case insensitive.
	 */
	bool StartsWith(const CStr& s, bool caseSensitive = true);

	/**
	 * Returns true if the string starts with c; otherwise returns false.
	 *
	 * \param c                   The character to be searched.
	 * \param caseSensitive       If it is true(default), the search is case sensitive; otherwise the search is case insensitive.
	 */
	bool StartsWith(char c, bool caseSensitive = true);

	/**
	 * Returns the number of line feeds inside a string.
	 */
	size_t NumberOfLines() const;

	/**
	 *  Replaces one substring with another in this string.
	 *
	 * \param pFind          The substring to be replaced.
	 * \param pReplaceBy     The string the substring is to be replaced with.
	 * \param firstOnly      If true, only first occurrence of 'from' is replaced, otherwise all occurrences are replaced
	 * \param startPosition  The character position to start searching from
	 * \return               Returns the number of replacements made.
	 */
	size_t Replace(const char* pFind, const char* pReplaceBy, bool firstOnly = false, size_t startPosition = 0);

private:
	/**
	 * NOTE. These methods don't have any implementation. They are only
	 * here to trigger compile-time errors if you're trying to assign non-ACP
	 * data inside this object.
	 */

	/* Disable CStr str(maxStr.ToUTF8()) */
	CStr(const MaxSDK::Util::MaxStringCastUTF8&);

	/* Disable CStr str; str = maxStr.ToUTF8(xxx) */
	CStr& operator = (const MaxSDK::Util::MaxStringCastUTF8&);

	/* Disable CStr str(maxStr.ToCP(xxx)) */
	CStr(const MaxSDK::Util::MaxStringCastCP&);

	/* Disable CStr str; str = maxStr.ToCP(xxx) */
	CStr& operator = (const MaxSDK::Util::MaxStringCastCP&);

	/* Disable UTF8Str utf8str; CStr str(utf8str) */
	CStr(const UTF8Str&);

	/* Disable CStr str; UTF8Str utf8str; str = utf8str */
	CStr& operator = (const UTF8Str&);
};

//-----------------------------------------------------------------------
// UTF8Str: Simple UTF8 encoded char string class
//-----------------------------------------------------------------------
/*! \sa  Class CStr, WStr and MaxString, ~{ Guidelines for Handling Character Strings }~.\n\n
\par Description:
A UTF8 encoded character string class. This class uses UTF8 encode to hold each character.
This UTF8Str class provide very basic functionality
Methods and operators are provided for calculating lengths, concatenation,
case conversion, comparison. 
\note The memory occupied by a UTF8Str object is cannot be larger than 2Gb.
*/
class UtilExport UTF8Str: public MaxHeapOperators {
	const char *buf;
	public:
		/*! \remarks Constructor. The string is set to NULL. */
		UTF8Str(); 
		/*! \remarks Constructor. The string is initialized to the string passed. */
		UTF8Str(const char *cs);

		/*! \remarks Constructor. The string is initialized to the string passed. */
		UTF8Str(const UTF8Str& ws);
		/*! \remarks Constructor. The string is initialized to the string passed. */
		UTF8Str(const MaxSDK::Util::MaxString& ws);
		/*! \remarks Constructor. The string is initialized to the string passed. */
		UTF8Str(const MaxSDK::Util::MaxStringCastUTF8&);
        /*! Conversion constructor, from QString. */
        UTF8Str(const QString& qstring);

		/*! \remarks Destructor. The string is deleted. */
		~UTF8Str();

		/*! Returns a pointer to this string which can be written to with up to 'nchars' characters. 
		\remarks The terminating null character is <b>not</b> included in nchars, so technically up to nchars+1 characters may be written.
		\param nchars The minimum number of characters to allocate in the string buffer (excluding the terminating null character). A value of -1
		specifies that the existing buffer should be re-used. */
		char *dataForWrite(size_t nchars = (size_t)-1);

		/*! \remarks Returns a const pointer to the string. If the string is NULL, a pointer to 0 is returned.*/
		const char *data() const;

		/*! \remarks Returns a const pointer to the string. If the string is NULL, a pointer to 0 is returned. */
		operator const char *() const;
        //! Conversion operator to QString
        operator QString() const;

		// realloc to nchars (padding with blanks)
		/*! \remarks Reallocates the string to contain <b>nchars</b>
		characters. If the string is enlarged it is padded with blanks.
		\par Parameters:
		<b>int nchars</b>\n\n
		Specifies the new number of characters for the string. */
		void Resize(int nchars);

		/*! \remarks Returns the number of chars used to store the string in memory, including spaces, but excluding the terminating NULL character. */
		int Length() const { return ByteCount(); }
		/*! \remarks Returns the number of chars used to store the string in memory, including spaces, but excluding the terminating NULL character. */
		int length() const { return Length(); }
		/*! \remarks Returns the number of bytes used to store the string in memory, including spaces, but excluding the terminating NULL character. */
		int ByteCount() const;

		/*! \remarks Returns the number of natural language characters the string is represented on, including spaces, but excluding the terminal NULL character.
		This may be equal or less than the amount of bytes used to store the string in memory. For example, a Chinese character
		might take up to 4 bytes. Length() or ByteCount() will count the character as up to 4 distinct chars. LanguageCharacterCount() will count it as a single char.*/
		int LanguageCharacterCount() const;
		
		/*! \remarks Returns the total number of char allocated to store the string. Including the NULL character and any unused char. */
		size_t AllocatedChars() const;

		/*! \remarks Returns TRUE if the string length is 0; otherwise FALSE.
		*/
		bool isNull() const { return data()[0]==0; }

		/*! \remarks Assignment operator. */
		UTF8Str & operator=(const UTF8Str& cs);
		/*! \remarks Assignment operator. */
		UTF8Str & operator=(const MaxSDK::Util::MaxString& mstr);
		/*! \remarks Assignment operator. */
		UTF8Str& operator = (const MaxSDK::Util::MaxStringCastUTF8&);

		/*! \remarks Assignment operator. In release 3.0 and later this method	check for self-assignment. */
		UTF8Str & operator=(const char *cs);

		// Concatenation operators.
		/*! \remarks Concatenation operator. Returns a new string that is this
		string with string <b>cs</b> appended. */
		UTF8Str operator+(const UTF8Str& cs) const;
		/*! \remarks Concatenation. Returns this string with <b>cs</b>	appended. */
		UTF8Str& operator+=(const UTF8Str& cs); 
		/*! \remarks Concatenation. Returns this string with <b>cs</b>	appended. */
		UTF8Str& Append(const UTF8Str& cs)  { return ((*this) += cs); }
		/*! \remarks Concatenation. Returns this string with <b>cs</b>	appended to the end. */
		UTF8Str& append(const UTF8Str& cs)  { return ((*this) += cs); }
		/*! \remarks Returns a substring of this string, beginning at multi-byte character position
		<b>firstCharacterIndex</b>, of length <b>numberOfMBCharacters</b>. */
		UTF8Str MultiByteCharacterSubString(int firstCharacterIndex, int numberOfMBCharacters) const;
		/*! \remarks Returns the first byte index value of the multi-byte character position
		<b>characterIndex</b>. */
		int FindMultiByteCharacterFirstByteIndex(int characterIndex) const;
		/*! \remarks Returns the last byte index value of the multi-byte character position
		<b>characterIndex</b>. */
		int FindMultiByteCharacterLastByteIndex(int characterIndex) const;
private:
		/*!
		* return next character, updating a byte-index variable 
		*
		 * \param i Index of that character.
		 */
		//should return Class Char instead in the future, 
		unsigned int utf8_nextchar(int *i) const; //For private use only

public:
		// Comparison
		/*! \remarks Equality operator.
		\return  true if the strings are equal; otherwise false. */
		bool operator==(const UTF8Str &cs) const;
		/*! \remarks Inequality operator.
		\return  false if the strings are equal; otherwise true. */
		bool operator!=(const UTF8Str &cs) const;
		/*! \remarks Returns true if this string is less than <b>cs</b>; otherwise false. */
		bool operator<(const UTF8Str &cs) const;
		/*! \remarks Returns true if this string is less than or equal to <b>ws</b>; otherwise false. */
		bool operator<=(const UTF8Str &ws) const;
		/*! \remarks Returns true if this string is greater than <b>ws</b>; otherwise false. */
		bool operator>(const UTF8Str &ws) const;
		/*! \remarks Returns true if this string is greater than or equal to <b>ws</b>; otherwise false. */
		bool operator>=(const UTF8Str &ws) const;
		/*! \remarks Converts all character of this string to uppercase. */
		void toUpper();
		/*! \remarks Converts all character of this string to lowercase. */
		void toLower();

		/*! \brief Write a formatted string into this UTF8Str.
		 * Writes the format string, filled in by the optional arguments into
		 * this UTF8Str.  See the ISO C++ documentation for more information on 
		 * printf and format strings.
		 * Note: do not use %ls formatting since conversion from wide string to 
		 * narrow string will occur using active code page encoding rather than 
		 * utf8 encoding
		 * \param format Specifies how to format the destination string.
		 * \param ... optional arguments to format into the destination string.
		 * \pre format is not null.
		 * \pre There are the correct number of elliptical arguments to fill the 
		 * format string.
		 * \post This string is replaced with the formatted string.
		 * \return The number of characters written to this string, not including
		 * the null terminating character, or a negative value if an error occurs.
		 */
		int printf(const char *format, ...);

		/*! \brief Write a formatted string into this UTF8Str using specified locale.
		 * Writes the format string, filled in by the optional arguments into
		 * this UTF8Str.  See the ISO C++ documentation for more information on
		 * printf_l and format strings.
		 * Note: do not use %ls formatting since conversion from wide string to
		 * narrow string will occur using active code page encoding rather than
		 * utf8 encoding
		 * \param format Specifies how to format the destination string.
		 * \param theLocale The locale to use.
		 * \param ... optional arguments to format into the destination string.
		 * \pre format is not null.
		 * \pre There are the correct number of elliptical arguments to fill the
		 * format string.
		 * \post This string is replaced with the formatted string.
		 * \return The number of characters written to this string, not including
		 * the null terminating character, or a negative value if an error occurs.
		 */
		int printf_l(const char *format, const _locale_t theLocale, ...);

		/*! \brief Write a formatted string into this UTF8Str.
		This method is similar to UTF8Str::printf. Instead of taking a variable list of 
		arguments as parameter, it takes a structure representing a variable list of 
		arguments. This allows UTF8Str objects to be used to build strings based on a 
		format string and a variable number of arguments.
		* Note: do not use %ls formatting since conversion from wide string to 
		* narrow string will occur using active code page encoding rather than 
		* utf8 encoding
		*/
		int vprintf(const char *format, va_list args);

		/*! \brief Write a formatted string into this UTF8Str using specified locale.
		This method is similar to UTF8Str::printf_l. Instead of taking a variable list of
		arguments as parameter, it takes a structure representing a variable list of
		arguments. This allows UTF8Str objects to be used to build strings based on a
		format string, a locale, and a variable number of arguments.
		* Note: do not use %ls formatting since conversion from wide string to
		* narrow string will occur using active code page encoding rather than
		* utf8 encoding
		*/
		int vprintf_l(const char *format, const _locale_t theLocale, va_list args);
public:
	/*! \remarks Returns the string in memory allocated by SysAllocString. Caller is responsible for freeing memory using SysFreeString. */
	wchar_t*                           ToBSTR() const;
	static UTF8Str                        FromBSTR(const wchar_t* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCastCP      ToCP   (UINT cp, size_t* length = NULL) const;
	static UTF8Str                        FromCP (UINT cp, const char* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCast<char>  ToACP  (size_t* length = NULL) const;
	static UTF8Str                        FromACP(const char* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCastUTF8    ToUTF8  (size_t* length = NULL) const;
	static UTF8Str                        FromUTF8(const char* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCast<WCHAR> ToOLESTR   (size_t* length = NULL) const;
	static UTF8Str                        FromOLESTR(const wchar_t* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCast<WCHAR> ToUTF16  (size_t* length = NULL) const;
	static UTF8Str                        FromUTF16(const wchar_t* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCast<unsigned int>  ToUTF32  (size_t* length = NULL) const;
	static UTF8Str                                FromUTF32(const unsigned int* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxString            ToMaxString() const;
	void                               ToMaxString(MaxSDK::Util::MaxString&) const;
	static UTF8Str                        FromMaxString(MaxSDK::Util::MaxString& string) { return UTF8Str(string); }

	UTF8Str                               ToUTF8Str() const { return *this; }
	static UTF8Str                        FromUTF8Str(const UTF8Str& string) { return UTF8Str(string); }

	WStr                               ToWStr() const;
	static UTF8Str                        FromWStr(const WStr& string);

#ifdef MCHAR_IS_WCHAR
	inline MaxSDK::Util::MaxStringCast<wchar_t> ToMCHAR  (size_t* length = NULL) const { return ToUTF16(length); }
	static inline UTF8Str                          FromMCHAR(const wchar_t* string, size_t length = (size_t)-1) { return FromUTF16(string, length); }

	WStr                               ToMSTR() const;
	static UTF8Str                        FromMSTR(const WStr& string);
#else
	MaxSDK::Util::MaxStringCast<char>  ToMCHAR(size_t* length = NULL) const { return ToACP(length); }
	static inline UTF8Str                 FromMCHAR(const char* string, size_t length = (size_t)-1) { return FromACP(string, length); }

	UTF8Str                               ToMSTR() const { return *this; }
	static UTF8Str                        FromMSTR(const UTF8Str& string) { return UTF8Str(string); }
#endif

	/**
	 * Returns true if the string ends with s; otherwise returns false.
	 *
	 * \param s                   The string to be searched.
	 * \param caseSensitive       If it is true(default), the search is case sensitive; otherwise the search is case insensitive.
	 */
	bool EndsWith(const UTF8Str& s, bool caseSensitive = true);
	
	
	/**
	 * Returns true if the string starts with s; otherwise returns false.
	 *
	 * \param s                   The string to be searched.
	 * \param caseSensitive       If it is true(default), the search is case sensitive; otherwise the search is case insensitive.
	 */
	bool StartsWith(const UTF8Str& s, bool caseSensitive = true);


	/**
	 * Returns the number of line feeds inside a string.
	 */
	size_t NumberOfLines() const;

private:
	/**
	 * NOTE. These methods don't have any implementation. They are only
	 * here to trigger compile-time errors if you're trying to assign non-ACP
	 * data inside this object.
	 */

	/* Disable UTF8Str str(maxStr.ToCP(xxx)) */
	UTF8Str(const MaxSDK::Util::MaxStringCastCP&);

	/* Disable UTF8Str str; str = maxStr.ToCP(xxx) */
	UTF8Str& operator = (const MaxSDK::Util::MaxStringCastCP&);

	/* Disable UTF8Str str(maxStr.ToACP(xxx)) */
	UTF8Str(const MaxSDK::Util::MaxStringCast<char>&);

	/* Disable UTF8Str str; str = maxStr.ToACP(xxx) */
	UTF8Str& operator = (const MaxSDK::Util::MaxStringCast<char>&);

	/* Disable CStr str; UTF8Str utf8str(str) */
	UTF8Str(const CStr&);

	/* Disable CStr str; UTF8Str utf8str; utf8str = str */
	UTF8Str& operator = (const CStr&);

	/*Disable compare with char*/
	bool operator==(char c) const;

	/*Disable compare with char*/
	bool operator!=(char c) const;

	/* Disable remove character from index pos to end. string cannot be modified */
	UTF8Str& remove(int pos);

	/* Disable remove N characters from pos to end. string cannot be modified */
	UTF8Str& remove(int pos, int N);

	/* Disable Substring operator */
	UTF8Str Substr(int start, int nchars) const;

	/*Disable Return the char at byte index i of this string.*/
	char operator[](int i) const;

	/*Disable search a char in the UTF8 string*/
	bool EndsWith(char c, bool caseSensitive = true);

	/*Disable search a char in the UTF8 string*/
	int first(char c) const;
	
	/*Disable search a char in the UTF8 string*/
	int last(char c) const;
};


//-----------------------------------------------------------------------
// WStr: Simple Wide char string class
//-----------------------------------------------------------------------
/*! \sa  Class CStr, UTF8Str and MaxString, ~{ Guidelines for Handling Character Strings }~.\n\n
\par Description:
A wide character string class. This class uses 16 bits to hold each character.
Methods and operators are provided for calculating lengths, concatenation,
substring operations, character searching, case conversion, comparison, and
formatted writing. All methods are implemented by the system.\n\n
OLE file IO requires the wide characters of WStr. <br> 
\note The memory occupied by a WStr object is cannot be larger than 2Gb.
*/
class UtilExport WStr: public MaxHeapOperators {
	const wchar_t *buf;
	public:
		/*! \remarks Constructor. The string is set to NULL. */
		WStr();

		/*! \remarks Constructor. The string is initialized to the string passed. */
		WStr(const wchar_t *wcstr);
		/*! \remarks Constructor. The string is initialized to the string passed. */
		WStr(const WStr& ws);
		/*! \remarks Constructor. The string is initialized to the string passed. */
		WStr(const MaxSDK::Util::MaxString& ws);
		/*! \remarks Constructor. The string is initialized to the string passed. */
		WStr(const MaxSDK::Util::MaxStringCast<WCHAR>& ws);
        /*! Conversion constructor, from QString. */
        WStr(const QString& qstring);

		/*! \remarks Destructor. The string is deleted. */
		~WStr();
		/*! Returns a pointer to this string which can be written to with up to 'nchars' characters. 
		\remarks The terminating null character is <b>not</b> included in nchars, so technically up to nchars+1 characters may be written.
		\param nchars The minimum number of characters to allocate in the string buffer (excluding the terminating null character). A value of -1
		specifies that the existing buffer should be re-used. */
		wchar_t *dataForWrite(size_t nchars = (size_t)-1);

		/*! \remarks		Returns a const pointer to the string. If the string is NULL, a pointer to 0 is returned. */
		const wchar_t *data() const;

		/*! \remarks Returns a const pointer to the string. If the string is NULL, a pointer to 0 is returned. */
		operator const wchar_t *() const;
        //! Conversion operator to QString
        operator QString() const;

		// realloc to nchars (padding with blanks)
		/*! \remarks Reallocates the string to contain <b>nchars</b>	characters. 
		If the string is enlarged it is padded with blanks.
		\par Parameters: 
		<b>int nchars</b>\n\n
		Specifies the new number of characters for the string. */
		void Resize(int nchars);
		/*! \remarks Returns the number of WCHAR that are used to represents this string, including spaces, but excluding the terminal NULL character. */
		int Length() const;
		/*! \remarks Returns the number of WCHAR that are used to represents this string, including spaces, but excluding the terminal NULL character. */
		int length() const { return Length(); }
		/*! \remarks Returns the number of bytes used to store the string in memory, including spaces, but excluding the terminating NULL character. */
		int ByteCount() const;
		
		/*! \remarks Returns the number of natural language characters the string is represented on, including spaces, but excluding the terminal NULL character.		
		
		This may be equal or less than the amount of WCHAR used to store the string in memory. For example, a chinese character
		might take two WCHARs. Length() will count the character as two distinct WCHARs. LanguageCharacterCount() will count it as
		a single character. */
		int LanguageCharacterCount() const;
		
		/*! \remarks Returns the total number of WCHAR allocated to store the string. Including the NULL character and any unused WCHAR. */
		size_t AllocatedChars() const;
		
		/*! \remarks Returns TRUE if the string length is 0; otherwise FALSE.*/
		bool isNull() const { return data()[0]==0; }

		/*! \remarks Assignment operator. In release 3.0 and later this
		operator checks for self-assignment. */
		WStr & operator=(const WStr& ws);
		/*! \remarks Assignment operator. */
		WStr & operator=(const MaxSDK::Util::MaxString& mstr);
		/*! \remarks Assignment operator. */
		WStr & operator=(const wchar_t *wcstr);
		/*! \remarks Assignment operator. */
		WStr & operator=(const MaxSDK::Util::MaxStringCast<WCHAR>& mstr);

		// Concatenation operators.
		/*! \remarks Concatenation operator. Returns a new string that is this
		string with string <b>ws</b> appended. */
		WStr operator+(const WStr& ws) const; 
		/*! \remarks Concatenation. Returns this string with <b>ws</b>	appended. */
		WStr & operator+=(const WStr& ws); 
		/*! \remarks Concatenation. Returns this string with <b>ws</b> appended. */
		WStr& Append(const WStr& ws) { return ((*this) += ws); }
		/*! \remarks Concatenation. Returns this string with <b>ws</b> appended. */
		WStr& append(const WStr& ws)  { return ((*this) += ws); }
		/*! \remarks Returns this string with N characters removed from <b>pos</b> to the end.
		\par Parameters:
		<b>int pos</b>\n\n
		Specifies the position to begin removing characters.\n\n
		<b>int N</b>\n\n
		Specifies the number of characters to remove. */
		WStr& remove(int pos);	// remove chars from pos to end
		WStr& remove(int pos, int N);	// remove N chars from pos to end

		// Substring operator
		/*! \remarks Returns a substring of this string, beginning at position
		<b>start</b>, of length <b>nchars</b>. */
		WStr Substr(int start, int nchars) const;

		/*!
		 * Returns the nth character of this string.
		 *
		 * Warning: In some environment, this function may return a partial
		 * character, especially for Asiatic languages. If you intend to do
		 * any advance transformation or interpretation of the returned data,
		 * you should consider calling WStr::MultiByteCharacterSubString
		 * instead.
		 *
		 * \param i Index of that character.
		 */
		wchar_t operator[](int i) const;

		// Char search:(return -1 if not found)
		/*! \remarks Returns the index of the first occurrence of character <b>c</b>
		in this string. Returns -1 if not found. */
		int first(wchar_t c) const;
		/*! \remarks Returns the index of the last occurrence of character <b>c</b> in
		this string. Returns -1 if not found. */
		int last(wchar_t c) const;

		// Comparison
		/*! \remarks Equality operator.
		\return  true if the strings are equal; otherwise false. */
		bool operator==(const WStr &ws) const;
		/*! \remarks Returns true if this string contain only the character <b>c</b>.*/
		bool operator==(wchar_t c) const;
		/*! \remarks Inequality operator.
		\return  False if the strings are equal; otherwise true. */
		bool operator!=(const WStr &ws) const;
		/*! \remarks Returns true if this string does not contain only the character <b>c</b>.*/
		bool operator!=(wchar_t c) const;
		/*! \remarks Returns true if this string is less than <b>ws</b>; otherwise false. */
		bool operator<(const WStr &ws) const;
		/*! \remarks Returns true if this string is less than or equal to <b>ws</b>; otherwise false. */
		bool operator<=(const WStr &ws) const;
		/*! \remarks Returns true if this string is greater than <b>ws</b>;	otherwise false. */
		bool operator>(const WStr &ws) const;
		/*! \remarks Returns true if this string is greater than or equal to <b>ws</b>; otherwise false. */
		bool operator>=(const WStr &ws) const;

		/*! \remarks Converts all character of this string to uppercase. */
		void toUpper();
		/*! \remarks Converts all character of this string to lowercase. */
		void toLower();

		/**
		 * \brief Write a formatted string into this WStr.
		 *
		 * Writes the format string, filled in by the optional arguments into
		 * this WStr.  See the ISO C++ documentation for more information on 
		 * printf and format strings.
		 *
		 * \param format Specifies how to format the destination string.
		 * \param ... optional arguments to format into the destination string.
		 * \pre format is not null.
		 * \pre There are the correct number of elliptical arguments to fill the 
		 * format string.
		 * \post This string is replaced with the formatted string.
		 * \return The number of characters written to this string, not including
		 * the null terminating character, or a negative value if an error occurs.
		 */
		int printf(const wchar_t *format, ...);

		/**
		 * \brief Write a formatted string into this WStr using specified locale.
		 *
		 * Writes the format string, filled in by the optional arguments into
		 * this WStr.  See the ISO C++ documentation for more information on
		 * printf_l and format strings.
		 *
		 * \param format Specifies how to format the destination string.
		 * \param theLocale The locale to use.
		 * \param ... optional arguments to format into the destination string.
		 * \pre format is not null.
		 * \pre There are the correct number of elliptical arguments to fill the
		 * format string.
		 * \post This string is replaced with the formatted string.
		 * \return The number of characters written to this string, not including
		 * the null terminating character, or a negative value if an error occurs.
		 */
		int printf_l(const wchar_t *format, const _locale_t theLocale, ...);

		/*! \brief Write a formatted string into this WStr.
		This method is similar to WStr::printf. Instead of taking a variable list of 
		arguments as parameter, it takes a structure representing a variable list of 
		arguments. This allows WStr objects to be used to build strings based on a 
		format string and a variable number of arguments:
		\code
		void LogMessage(const wchar_t* format, ...) {
		va_list args;
		va_start(args, format);
		WStr buf;
		buf.vprintf(format, args);
		va_end(args);
		// log the message contained by buf
		}
		\endcode
		*/
		int vprintf(const wchar_t *format, va_list args);

		/*! \brief Write a formatted string into this WStr specified locale.
		This method is similar to WStr::printf_l. Instead of taking a variable list of
		arguments as parameter, it takes a structure representing a variable list of
		arguments. This allows WStr objects to be used to build strings based on a
		format string, a locale, and a variable number of arguments:
		\code
		void LogMessage_l(const wchar_t* format, const _locale_t theLocale, ...) {
		va_list args;
		va_start(args, format);
		WStr buf;
		buf.vprintf(format, theLocale, args);
		va_end(args);
		// log the message contained by buf
		}
		\endcode
		*/
		int vprintf_l(const wchar_t *format, const _locale_t theLocale, va_list args);

public:
		WStr MultiByteCharacterSubString(int firstCharacterIndex, int numberOfMBCharacters) const;

public:
	/*! \remarks Returns the string in memory allocated by SysAllocString. Caller is responsible for freeing memory using SysFreeString. */
	wchar_t*                           ToBSTR() const;
	static WStr                        FromBSTR(const wchar_t* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCastCP      ToCP  (UINT cp, size_t* length = NULL) const;
	static WStr                        FromCP(UINT cp, const char* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCast<char>  ToACP  (size_t* length = NULL) const;
	static WStr                        FromACP(const char* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCastUTF8    ToUTF8  (size_t* length = NULL) const;
	static WStr                        FromUTF8(const char* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCast<WCHAR> ToOLESTR  (size_t* length = NULL) const;
	static WStr                        FromOLESTR(const wchar_t* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCast<WCHAR> ToUTF16  (size_t* length = NULL) const;
	static WStr                        FromUTF16(const wchar_t* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxStringCast<unsigned int>	ToUTF32  (size_t* length = NULL) const;
	static WStr									FromUTF32(const unsigned int* string, size_t length = (size_t)-1);

	MaxSDK::Util::MaxString            ToMaxString() const;
	void                               ToMaxString(MaxSDK::Util::MaxString&) const;
	static WStr                        FromMaxString(MaxSDK::Util::MaxString& string) { return WStr(string); }

	CStr                               ToCStr() const;
	static WStr                        FromCStr(const CStr& string);

	WStr                               ToWStr() const { return *this; }
	static WStr                        FromWStr(const WStr& string) { return WStr(string); }

#ifdef MCHAR_IS_WCHAR
	inline const wchar_t*              ToMCHAR  (size_t* length = NULL) const { return ToUTF16(length); }
	static inline WStr                 FromMCHAR(const wchar_t* string, size_t length = (size_t)-1) { return FromUTF16(string, length); }

	WStr                               ToMSTR() const { return *this; }
	static WStr                        FromMSTR(const WStr& string) { return WStr(string); }
#else
	inline MaxSDK::Util::MaxStringCast<char> ToMCHAR  (size_t* length = NULL) const { return ToACP(length); }
	static inline WStr                       FromMCHAR(const char* string, size_t length = (size_t)-1) { return FromACP(string, length); }

	CStr                               ToMSTR() const;
	static WStr                        FromMSTR(const CStr& string);
#endif

	/**
	 * Returns true if the string ends with s; otherwise returns false.
	 *
	 * \param s                   The string to be searched.
	 * \param caseSensitive       If it is true(default), the search is case sensitive; otherwise the search is case insensitive.
	 */
	bool EndsWith(const WStr& s, bool caseSensitive = true);

	/**
	 * Returns true if the string ends with c; otherwise returns false. This function overloads EndsWith().
	 *
	 * \param c                   The character to be searched.
	 * \param caseSensitive       If it is true(default), the search is case sensitive; otherwise the search is case insensitive.
	 */
	bool EndsWith(wchar_t c, bool caseSensitive = true);

	/**
	 * Returns true if the string starts with s; otherwise returns false.
	 *
	 * \param s                   The string to be searched.
	 * \param caseSensitive       If it is true(default), the search is case sensitive; otherwise the search is case insensitive.
	 */
	bool StartsWith(const WStr& s, bool caseSensitive = true);

	/**
	 * Returns true if the string starts with c; otherwise returns false. This function overloads StartsWith().
	 *
	 * \param c                   The character to be searched.
	 * \param caseSensitive       If it is true(default), the search is case sensitive; otherwise the search is case insensitive.
	 */
	bool StartsWith(wchar_t c, bool caseSensitive = true);

	/**
	 * Returns the number of line feeds inside a string.
	 */
	size_t NumberOfLines() const;

	/**
	 *  Replaces one substring with another in this string.
	 *
	 * \param pFind          The substring to be replaced.
	 * \param pReplaceBy     The string the substring is to be replaced with.
	 * \param firstOnly      If true, only first occurrence of 'from' is replaced, otherwise all occurrences are replaced
	 * \param startPosition  The character position to start searching from
	 * \return               Returns the number of replacements made.
	 */
	size_t Replace(const wchar_t* pFind, const wchar_t* pReplaceBy, bool firstOnly = false, size_t startPosition = 0);

};

#ifdef _UNICODE
#define TSTR WStr
#else
#define TSTR CStr
#endif

#ifdef MCHAR_IS_WCHAR
	typedef WStr	MSTR;
#else
	typedef CStr	MSTR;
#endif

//--FilterList----------------------------------------------------------------------
// A class whose sole purpose is for building up a  filter list to passing to
// GetSaveFileName and GetOpenFileName.  It automatically puts in the embedded nulls
// and two terminating nulls.
//	 Example:
//
//	FilterList filterList;
//	filterList.Append( _M("Max files(*.max)"));
//	filterList.Append( _M("*.max"));
//	ofn.lpstrFilter  = filterList;
//	GetSaveFileName(&ofn)
//----------------------------------------------------------------------------------

/*! class FilterList
\par Description:
A class whose sole purpose is for building up a filter list to pass to the
Windows API functions <b>GetSaveFileName()</b> and <b>GetOpenFileName()</b>. It
automatically puts in the embedded nulls and two terminating nulls. All methods
are implemented by the system.\n\n
Example usage:\n\n
<b>FilterList filterList;</b>\n\n
<b>filterList.Append(_M("MAX files(*.max)"));</b>\n\n
<b>filterList.Append(_M("*.max"));</b>\n\n
<b>ofn.lpstrFilter = filterList;</b>\n\n
<b>GetSaveFileName(\&ofn)</b> \n\n
persistent filter support is supported by this class.  The index can be used by OPENFILENAME structure.  
The default value is 1 and both DoMaxSaveAsDialog and DoMaxOpenDialog have
been updated to honour these settings.  The developer has to simply store the new index and set it
as the actual index the next time these functions are used.\n
CAUTION: The resulting filter list string is used as-is in the Windows file selector, and
expects pairs of entries (a description entry and a wildcard entry, as shown in the example usage above).
Providing an odd number of Appended strings can result in a crash.\n
An example is shown below, and assumes filterIndex is a global variable.
\code

	FilterList filterList;
	filterList.Append(_M("MAX files(*.max)"));
	filterList.Append(_M("*.max"));

	filters.SetFilterIndex(filterIndex);


	if (GetCOREInterface8()->DoMaxOpenDialog(hWnd, title, filename, initDir, filterList))
	{
		filterIndex = filters.GetNewFilterIndex();
	}

\endcode

*/
class FilterList: public MaxHeapOperators {
    Tab<const MCHAR> buf;
	int m_filterIndex;
	int m_newFilterIndex;
	public:
		//! \brief Constructor
		UtilExport FilterList();
		/*! \remarks Appends the string passed to <b>buf</b>. */
		UtilExport void Append(const MCHAR *name);
		UtilExport void Append(FilterList& filters);
		/*! \remarks Returns the address of <b>buf</b>. */
		UtilExport operator const MCHAR *(); 

		/*! Returns the current filter index that will be used in OPENFILENAME structure
		\return The current index
		*/
		UtilExport int GetFilterIndex();

		/*! Sets the filter index to be used by OPENFILENAME structure
		\param filterIndex The index to use
		*/
		UtilExport void SetFilterIndex(int filterIndex);

		/*! Returns the new filter index reflecting the users choice of filter.  Currently 
		DoMaxSaveAsDialog and DoMaxOpenDialog store this value when they successfully run.  
		It allows a developer to set the new index correctly using SetFilterIndex()
		\return The new index as chosen by the user.
		*/
		UtilExport int GetNewFilterIndex();

		/*! Sets the new filter index as returned by OPENFILENAME. 
		DoMaxSaveAsDialog and DoMaxOpenDialog store this value for the developer automatically
		\param filterIndex The new index to set reflecting the users input.
		*/
		UtilExport void SetNewFilterIndex(int filterIndex);

		/*! \brief Returns the file description string identified by an index into the filter list
		 * \param filterIndex - 1 based index into the filter list
		 * \return Pointer to the string representing the file filter description
		*/
		UtilExport const MCHAR* GetFilterDescriptor(unsigned int filterIndex);

		/*! \brief Returns the file extension string identified by an index into the filter list
		 * \param filterIndex - 1 based index into the filter list
		 * \return Pointer to the string representing the file filter extension. 
		 * The returned extension is exactly in the form in which it's been recorded in the FilterList instance.
		*/
		UtilExport const MCHAR* GetFilterExtension(unsigned int filterIndex);
	};

UtilExport bool operator == (const char*, const CStr&);
UtilExport bool operator != (const char*, const CStr&);
UtilExport bool operator == (const CStr&, const char*);
UtilExport bool operator != (const CStr&, const char*);

UtilExport bool operator == (const wchar_t*, const WStr&);
UtilExport bool operator != (const wchar_t*, const WStr&);
UtilExport bool operator == (const WStr&, const wchar_t*);
UtilExport bool operator != (const WStr&, const wchar_t*);

// For NULL comparison.
UtilExport bool operator == (const int, const CStr&);
UtilExport bool operator != (const int, const CStr&);
UtilExport bool operator == (const CStr&, const int);
UtilExport bool operator != (const CStr&, const int);

UtilExport bool operator == (const int, const WStr&);
UtilExport bool operator != (const int, const WStr&);
UtilExport bool operator == (const WStr&, const int);
UtilExport bool operator != (const WStr&, const int);

UtilExport bool operator==(const CStr& cstr, const QString& qstr);
UtilExport bool operator!=(const CStr& cstr, const QString& qstr);
UtilExport bool operator==(const WStr& wstr, const QString& qstr);
UtilExport bool operator!=(const WStr& wstr, const QString& qstr);
UtilExport bool operator==(const UTF8Str& ustr, const QString& qstr);
UtilExport bool operator!=(const UTF8Str& ustr, const QString& qstr);
UtilExport bool operator==(const QString& qstr, const CStr& cstr);
UtilExport bool operator!=(const QString& qstr, const CStr& cstr);
UtilExport bool operator==(const QString& qstr, const WStr& wstr);
UtilExport bool operator!=(const QString& qstr, const WStr& wstr);
UtilExport bool operator==(const QString& qstr, const UTF8Str& ustr);
UtilExport bool operator!=(const QString& qstr, const UTF8Str& ustr);

/*! CStr/WStr output to std output streams.
Note: it is highly recommended that std file streams not be used. The file
is written using the Window User active code page, and if a character is 
written that cannot be represented in that code page, the stream is flagged
as 'in error' and no further output occurs.
Instead, use MaxSDK::TextFile::Writer, preferably with UTF8 encoding and a UTF8 BOM.
!*/
UtilExport std::ostream &	operator << (std::ostream &s, const WStr &v);
UtilExport std::wostream &	operator << (std::wostream &s, const WStr &v);
UtilExport std::ostream &	operator << (std::ostream &s, const CStr &v);
UtilExport std::wostream &	operator << (std::wostream &s, const CStr &v);


/*! \defgroup SplitFilename SplitFilename - Split a filename into its components 
These function is used to extract the path, filename and/or extension out of a fully-qualified path name.

Pass in NULL for components which you do not require.

Note that SplitFilename will also support being passed a GUID string representing an asset.
Example: "{B5B12976-3C00-475A-A619-CE0C76BD8483}"
If the IAssetManager can find an asset corresponding to this string, SplitFilename will use the filename of this asset.

	\code
	{
		CStr p, f, e;
		CStr name("c:\\mydir\\myfile.txt");
	
		SplitFilename(name, &p, &f, &e);
	
		// p now contains "c:\\mydir"
		// f now contains "myfile"
		// e now contains ".txt"
	}

	{
		WStr p;
		WStr name(L"c:\\mydir\\myfile.txt");

		SplitFilename(name, &p, NULL, NULL);	

		// p now contains L"c:\\mydir"
	}

	{
		wchar_t file[MAX_PATH];

		SplitFilename(L"c:\\mydir\\myfile.txt", NULL, file, NULL);

		// file now contains L"myfile"
	}
	\endcode

	\param[in] full_path Full pathname, cannot be NULL.
	\param[out] directory	Extracted path name, with directory.  Pass NULL if you do not need it.
	\param[in] dbuflen      Applies only to the char* (wchar_t*) versions of SplitFilename(): size of the \p directory string buffer, in char (wchar_t) units, not bytes.
	\param[out] filename	Extracted filename, without extension.  Pass NULL if you do not need it.
	\param[in] fbuflen      Applies only to the char* (wchar_t*) versions of SplitFilename(): size of the \p filename string buffer, in char (wchar_t) units, not bytes.
	\param[out] extension	Extracted extension.  Pass NULL if you do not need it.
	\param[in] ebuflen      Applies only to the char* (wchar_t*) versions of SplitFilename(): size of the \p extension string buffer, in char (wchar_t) units, not bytes.
*/
/*! \brief SplitFilename (CStr* version)
\ingroup SplitFilename */
CoreExport void SplitFilename(const CStr& full_path, CStr* directory, CStr* filename, CStr* extension);
/*! \brief SplitFilename (char* version) 
\ingroup SplitFilename */
CoreExport void SplitFilename(LPCSTR full_path, LPSTR directory, size_t dbuflen, LPSTR filename, size_t fbuflen, LPSTR extension, size_t ebuflen);
/*! \brief SplitFilename (WStr* version) 
\ingroup SplitFilename */
CoreExport void SplitFilename(const WStr& full_path, WStr* directory, WStr* filename, WStr* extension);
/*! \brief SplitFilename (wchar_t* version) 
\ingroup SplitFilename */
CoreExport void SplitFilename(LPCWSTR full_path, LPWSTR directory, size_t dbuflen, LPWSTR filename, size_t fbuflen, LPWSTR extension, size_t ebuflen);

/*! \defgroup SplitFilenameOld SplitFilenameOld - Split a filename into its components 
  SplitFilenameOld only handles actual file names passed, not AssetId strings. */
/*! \brief SplitFilenameOld (CStr* version)
\ingroup SplitFilenameOld */
UtilExport void SplitFilenameOld(LPCSTR full_path, LPSTR directory, size_t dbuflen, LPSTR filename, size_t fbuflen, LPSTR extension, size_t ebuflen);
/*! \brief SplitFilenameOld (char* version) 
\ingroup SplitFilenameOld */
UtilExport void SplitFilenameOld(const CStr& full_path, CStr* directory, CStr* filename, CStr* extension);
/*! \brief SplitFilenameOld (WStr* version) 
\ingroup SplitFilenameOld */
UtilExport void SplitFilenameOld(const WStr& full_path, WStr* directory, WStr* filename, WStr* extension);
/*! \brief SplitFilenameOld (wchar_t* version) 
\ingroup SplitFilenameOld */
UtilExport void SplitFilenameOld(LPCWSTR full_path, LPWSTR directory, size_t dbuflen, LPWSTR filename, size_t fbuflen, LPWSTR extension, size_t ebuflen);
/*--------------------------------------------------
Split filename "name" into 
p  path
f  filename.ext
-------------------------------------------------*/
CoreExport void SplitPathFile(const MSTR& full_path, MSTR* directory, MSTR* filename);

/* /brief SplitPathFileOld only handles actual file names passed, not AssetId strings. */
UtilExport void SplitPathFileOld(const MSTR& full_path,MSTR* directory, MSTR* filename);

//! Check to see if the string matches the pattern in pattern
UtilExport BOOL MatchPattern(const MSTR& pString, const MSTR& pattern, BOOL ignoreCase=TRUE);


//-------------------------------------------------------------------------
// A Case Sensitive "smart" alphanumeric compare that sorts things so that
// numerical suffices come out in numerical order.
//-------------------------------------------------------------------------
UtilExport int MaxAlphaNumComp(const MCHAR *a, const MCHAR *b);

//-------------------------------------------------------------------------
// A Case Insensitive "smart" alphanumeric compare that sorts things so that
// numerical suffices come out in numerical order.
//-------------------------------------------------------------------------
UtilExport int MaxAlphaNumCompI(const MCHAR *a, const MCHAR *b);

