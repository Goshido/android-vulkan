//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2017 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "..\utilexp.h"
#include "..\noncopyable.h"
#include <WTypes.h>
#include "..\strbasic.h"

#pragma warning(push)
#pragma warning(disable: 4275)      // Disable warning about base class not dll-exported

namespace MaxSDK
{
	namespace Util
	{
		const unsigned long INFINITE_TIME = (unsigned long)(0xffffffff);
		/*! \brief
		This class creates and attempts to acquire a named Mutex based on a file name in its constructor, and releases
		and closes the Mutex in its destructor. Instances of this class are typically used to control access to a file across 
		multiple 3ds Max sessions.
		*/
		class UtilExport FileMutexObject : public Noncopyable
		{
		public:
			/*! \brief Create a named Mutex using the lower case filename with back slashes converted to
			forward slashes, and attempt to acquire the Mutex for the specified period of time.
			\param [in] lpFileName the filename used to form the named Mutex's name.
			Note that if the filename is null, a mutex isn't created or acquired.
			\param [in] timeOut the length of time in milliseconds to attempt to acquire the Mutex.
			*/
			FileMutexObject(const MCHAR* lpFileName, unsigned long timeOut = INFINITE_TIME);
			/*! \brief Releases and closes the Mutex.
			*/
			~FileMutexObject();

			/*! \brief Returns true if the Mutex was acquired.
			*/
			bool MutexAquired();

			/*! \brief Releases the lock if the Mutex was acquired.
			*/
			void ReleaseMutex();

		private:
			HANDLE mMutex;
			bool mMutexAquired;
		};

	}// namespace Util

} // namespace MaxSDK

#pragma warning(pop)
