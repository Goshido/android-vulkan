//**************************************************************************/
// Copyright (c) 1998-2011 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: INI File I/O
// AUTHOR:      Christian Roy
// DATE:        June 2011
//***************************************************************************/

#pragma once

#include "../utilexp.h"
#include <WTypes.h>
#include "../containers/Array.h"

#ifndef WritePrivateProfileString
#define WritePrivateProfileString WritePrivateProfileStringW
#endif
#ifndef WritePrivateProfileStruct
#define WritePrivateProfileStruct WritePrivateProfileStructW
#endif
#ifndef WritePrivateProfileSection
#define WritePrivateProfileSection WritePrivateProfileSectionW
#endif
#ifndef GetPrivateProfileString
#define GetPrivateProfileString GetPrivateProfileStringW
#endif
#ifndef GetPrivateProfileStruct
#define GetPrivateProfileStruct GetPrivateProfileStructW
#endif
#ifndef GetPrivateProfileSectionNames
#define GetPrivateProfileSectionNames GetPrivateProfileSectionNamesW
#endif
#ifndef GetPrivateProfileSection
#define GetPrivateProfileSection GetPrivateProfileSectionW
#endif
#ifndef GetPrivateProfileInt
#define GetPrivateProfileInt GetPrivateProfileIntW
#endif

namespace MaxSDK
{
    namespace Util
    {
        /** 
         * These service should replace every occurrence of the standard WIN32 implementation. 
         * The WIN32 implementation specifies that when the file name contains UNICODE character, 
         * the file will be created with a UNICODE BOM.  This is ABSOLUTLY INCORRECT!!!
         * In order to properly save UNICODE INI file, we need to enforce that every INI file
         * has a BOM token.
         **/

        /**
         * Copies a string into the specified section of an initialization file.
         * This service will create the new INI file with a UTF-16LE
         * and delegate to WIN32 implementation to actually write the new key value pair.
         * If the INI file exist and is not a UNICODE file, it will be 
         * re-created as a UTF-16LE and the key|value pair will be written in the proper 
         * section.
         * This service uses a FileMutexObject instance to ensure only one 3ds Max session is accessing 
         * the .ini at a time.
         * @param pSection      The name of the section to which the string will be copied. 
         *                      If the section does not exist, it is created. The name of the section is case-independent; 
         *                      the string can be any combination of uppercase and lowercase letters.
         * @param pKeyName      The name of the key to be associated with a string. 
         *                      If the key does not exist in the specified section, it is created. 
         *                      If this parameter is NULL, the entire section, including all 
         *                      entries within the section, is deleted.
         * @param pString       A null-terminated string to be written to the file. If this parameter 
         *                      is NULL, the key pointed to by the pKeyName parameter is deleted.
         * @param pFileName     The name of the initialization file.
         * @return              If the function successfully copies the string to the initialization 
         *                      file, the return value is nonzero.
         *                      If the file was created using Unicode characters, the function writes 
         *                      Unicode characters to the file. Otherwise, the function writes ANSI characters.
        */
        UtilExport BOOL WritePrivateProfileString(
            LPCMSTR pSection,
            LPCMSTR pKeyName,
            LPCMSTR pString,
            LPCMSTR pFileName
        );

        /** 
         * Copies data into a key in the specified section of an initialization file. 
         * As it copies the data, the function calculates a checksum and appends it to the end of the data. 
         * This service uses a FileMutexObject instance to ensure only one 3ds Max session is accessing
         * the .ini at a time.
         * @param pSection      The name of the section to which the string will be copied. If the section 
         *                      does not exist, it is created. The name of the section is case independent, 
         *                      the string can be any combination of uppercase and lowercase letters.
         * @param pKeyName      The name of the key to be associated with a string. If the key 
         *                      does not exist in the specified section, it is created. If this parameter is 
         *                      NULL, the entire section, including all keys and entries within the section, 
         *                      is deleted.
         * @param pStruct       The data to be copied. If this parameter is NULL, the key is deleted.
         * @param uSizeStruct   The size of the buffer pointed to by the pStruct parameter, in bytes.
         * @param pFileName     The name of the initialization file.
         * @return              If the function successfully copies the string to the initialization file, 
         *                      the return value is nonzero.
         *                      If the function fails, or if it flushes the cached version of the most 
         *                      recently accessed initialization file, the return value is zero. 
         *                      To get extended error information, call GetLastError.
         */
        UtilExport BOOL WritePrivateProfileStruct(
            LPCMSTR pSection,
            LPCMSTR pKeyName,
            LPVOID pStruct,
            UINT uSizeStruct,
            LPCMSTR pFileName
        );

        /** 
         * Replaces the keys and values for the specified section in an initialization file.
         * This service uses a FileMutexObject instance to ensure only one 3ds Max session is accessing
         * the .ini at a time.
         * @param pSection     The name of the section in which data is written. 
         * @param pString      The new key names and associated values that are to be written 
         *                     to the named section. This string is limited to 65,535 bytes.
         * @param pFileName    The name of the initialization file. If this parameter does not contain 
         *                     a full path for the file, the function searches the Windows directory for 
         *                     the file. If the file does not exist and pFileName does not contain a full path, 
         *                     the function creates the file in the Windows directory.
         *                     If the file exists and was created using Unicode characters, the function 
         *                     writes Unicode characters to the file. Otherwise, the function creates
         *                     a file using ANSI characters.
         * @return             If the function succeeds, the return value is nonzero.
         *                     If the function fails, the return value is zero. 
         *                     To get extended error information, call GetLastError.
         */
        UtilExport BOOL WritePrivateProfileSection(
            LPCMSTR pSection,
            LPCMSTR pString,
            LPCMSTR pFileName
        );

        /**
         * Gets a string from the specified section of an initialization file.
         * This service uses a FileMutexObject instance to ensure only one 3ds Max session is accessing the .ini at a time.
         * @param pSection          The name of the section containing the key name. If this parameter is NULL, the 
		 *                          GetPrivateProfileString function copies all section names in the file to the supplied buffer.
         * @param pKeyName          The name of the key whose associated string is to be retrieved. If this parameter is NULL, 
		 *                          all key names in the section specified by the pSection parameter are copied to the buffer 
		 *                          specified by the pReturnedString parameter.
         * @param pDefault	        A default string. If the pKeyName key cannot be found in the initialization file, 
         *                          GetPrivateProfileString copies the default string to the pReturnedString buffer. 
         *                          If this parameter is NULL, the default is an empty string, "".
         * @param pReturnedString   A pointer to the buffer that receives the retrieved string. 
         * @param nSize             The size of the buffer pointed to by the pReturnedString parameter, in characters.
         * @param pFileName         The name of the initialization file.
         * @return                  The return value is the number of characters copied to the buffer, not including the 
         *                          terminating null character. 
         *                          If neither pSection nor pKeyName is NULL and the supplied destination buffer is too small to 
         *                          hold the requested string, the string is truncated and followed by a null character, and 
         *                          the return value is equal to nSize minus one.
         *                          If either pSection or pKeyName is NULL and the supplied destination buffer is too small to 
         *                          hold all the strings, the last string is truncated and followed by two null characters. In this 
         *                          case, the return value is equal to nSize minus two.
        */
        UtilExport DWORD GetPrivateProfileString(
            LPCMSTR pSection,
            LPCMSTR pKeyName,
            LPCMSTR pDefault,
            LPMSTR pReturnedString,
            DWORD nSize,
            LPCMSTR pFileName
        );

        /**
         * Gets a string from the specified section of an initialization file.
         * This service uses a FileMutexObject instance to ensure only one 3ds Max session is accessing the .ini at a time.
		 * @param pSection          The name of the section containing the key name. If this parameter is NULL, the
		 *                          GetPrivateProfileString function copies all section names in the file to the supplied buffer.
		 * @param pKeyName          The name of the key whose associated string is to be retrieved. If this parameter is NULL,
		 *                          all key names in the section specified by the pSection parameter are copied to the buffer
		 *                          specified by the pReturnedString parameter.
		 * @param pDefault	        A default string. If the pKeyName key cannot be found in the initialization file,
		 *                          GetPrivateProfileString copies the default string to the pReturnedString buffer.
		 *                          If this parameter is NULL, the default is an empty string, "".
         * @param returnedString    A reference to the MaxSDK::Array<MCHAR> buffer that receives the retrieved string. This service
		 *                          will adjust the size as necessary to hold the string.
         * @param pFileName         The name of the initialization file.
         * @return                  The return value is the number of characters copied to the buffer, not including the 
         *                          terminating null character. 
        */
        UtilExport DWORD GetPrivateProfileString(
            LPCMSTR pSection,
            LPCMSTR pKeyName,
            LPCMSTR pDefault,
            MaxSDK::Array<MCHAR> &returnedString,
            LPCMSTR pFileName
        );

        /**
         * Retrieves the data associated with a key in the specified section of an initialization file. As it retrieves the data, 
		 * the function calculates a checksum and compares it with the checksum calculated by the WritePrivateProfileStruct function 
		 * when the data was added to the file.
         * This service uses a FileMutexObject instance to ensure only one 3ds Max session is accessing the .ini at a time.
         * @param pSection       The name of the section in the initialization file.
         * @param pKeyName       The name of the key whose data is to be retrieved..
         * @param pStruct        A pointer to the buffer that receives the data associated with the file, section, and key names.
         * @param uSizeStruct    The size of the buffer pointed to by the pStruct parameter, in bytes.
         * @param pFileName      The name of the initialization file.
		 * @return               If the function succeeds, the return value is nonzero. If the function fails, the return value is zero.
        */
		UtilExport BOOL GetPrivateProfileStruct(
			LPCMSTR pSection,
			LPCMSTR pKeyName,
			LPVOID  pStruct,
			UINT    uSizeStruct,
			LPCMSTR pFileName
		);

        /**
         * Retrieves the names of all sections in an initialization file.
         * This service uses a FileMutexObject instance to ensure only one 3ds Max session is accessing the .ini at a time.
         * @param pReturnedString   A pointer to a buffer that receives the section names associated with the named file. 
         *                          The buffer is filled with one or more null-terminated strings; the last string is followed 
         *                          by a second null character.
         * @param nSize             The size of the buffer pointed to by the pReturnedString parameter, in characters.
         * @param pFileName         The name of the initialization file.
         * @return                  The return value is the number of characters copied to the buffer, not including the 
         *                          terminating null character. If the buffer is not large enough to contain all the section 
         *                          names associated with the specified initialization file, the return value is equal to the 
         *                          size specified by nSize minus two.
        */
        UtilExport DWORD GetPrivateProfileSectionNames(
            LPMSTR pReturnedString,
            DWORD nSize, 
            LPCMSTR pFileName
        );

        /**
         * Retrieves the names of all sections in an initialization file.
         * This service uses a FileMutexObject instance to ensure only one 3ds Max session is accessing the .ini at a time.
         * @param returnedString    A reference to the MaxSDK::Array<MCHAR> buffer that receives the section names associated with the named file.
         *                          The buffer is filled with one or more null-terminated strings; the last string is followed
         *                          by a second null character. This service will adjust the size as necessary to hold the string.
         * @param pFileName         The name of the initialization file.
         * @return                  The return value is the number of characters copied to the buffer, not including the 
         *                          terminating null character. 
        */
        UtilExport DWORD GetPrivateProfileSectionNames(
            MaxSDK::Array<MCHAR> &returnedString,
            LPCMSTR pFileName
        );

        /**
         * Retrieves all the keys and values for the specified section of an initialization file.
         * This service uses a FileMutexObject instance to ensure only one 3ds Max session is accessing the .ini at a time.
		 * @param pSection          The name of the section in the initialization file.
         * @param pReturnedString   A pointer to a buffer that receives the key name and value pairs associated with the named section. 
         *                          The buffer is filled with one or more null-terminated strings; the last string is followed 
         *                          by a second null character.
         * @param nSize             The size of the buffer pointed to by the pReturnedString parameter, in characters.
         * @param pFileName         The name of the initialization file.
         * @return                  The return value is the number of characters copied to the buffer, not including the 
         *                          terminating null character. If the buffer is not large enough to contain all the key name and value 
		 *                          pairs associated with the named section, the return value is equal to the 
         *                          size specified by nSize minus two.
        */
		UtilExport DWORD GetPrivateProfileSection(
			LPCMSTR pSection,
			LPMSTR  pReturnedString,
			DWORD   nSize,
			LPCMSTR pFileName
		);

        /**
         * Retrieves all the keys and values for the specified section of an initialization file.
         * This service uses a FileMutexObject instance to ensure only one 3ds Max session is accessing the .ini at a time.
		 * @param pSection          The name of the section in the initialization file.
		 * @param returnedString    A reference to the MaxSDK::Array<MCHAR> buffer that receives the key name and value pairs associated with the named file.
		 *                          The buffer is filled with one or more null-terminated strings; the last string is followed
		 *                          by a second null character. This service will adjust the size as necessary to hold the string.
         * @param pFileName         The name of the initialization file.
         * @return                  The return value is the number of characters copied to the buffer, not including the 
         *                          terminating null character. 
        */
		UtilExport DWORD GetPrivateProfileSection(
			LPCMSTR pSection,
			MaxSDK::Array<MCHAR> &returnedString,
			LPCMSTR pFileName
		);

        /**
         * Retrieves an integer associated with a key in the specified section of an initialization file.
         * This service uses a FileMutexObject instance to ensure only one 3ds Max session is accessing the .ini at a time.
         * @param pSection          The name of the section containing the key name.
         * @param pKeyName          The name of the key whose value is to be retrieved. 
         * @param nDefault	        The default value to return if the key name cannot be found in the initialization file.
         * @param pFileName         The name of the initialization file.
         * @return                  The return value is the integer equivalent of the string following the specified key name in the 
         *                          specified initialization file. If the key is not found, the return value is the specified default value. 
        */
		UtilExport DWORD GetPrivateProfileInt(
			LPCMSTR pSection,
			LPCMSTR pKeyName,
			INT nDefault,
			LPCMSTR pFileName
		);
	}
}

// if you need to use the global namespace version of these methods, define 'NO_INIUTIL_USING' before including this header
#ifndef NO_INIUTIL_USING

using MaxSDK::Util::WritePrivateProfileString;
using MaxSDK::Util::WritePrivateProfileStruct;
using MaxSDK::Util::WritePrivateProfileSection;

using MaxSDK::Util::GetPrivateProfileString;
using MaxSDK::Util::GetPrivateProfileStruct;
using MaxSDK::Util::GetPrivateProfileSectionNames;
using MaxSDK::Util::GetPrivateProfileSection;
using MaxSDK::Util::GetPrivateProfileInt;

#endif