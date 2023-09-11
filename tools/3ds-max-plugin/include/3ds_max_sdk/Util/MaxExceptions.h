//**************************************************************************/
// Copyright (c) 1998-2011 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include "../strbasic.h"
#include "../utilexp.h"
#include "../MaxHeap.h"

namespace MaxSDK
{
	namespace Util
	{
		//! \brief Base exception class for 3ds Max.
		/*!	This serves as a base class for other exception classes. 
			Instances of this class own and make their own copies of the error message. */
		class UtilExport Exception : public MaxHeapOperators
		{
		public:
			//! String constructor with a specific message
			explicit Exception(const MCHAR* message);
			//! Copy Constructor
			Exception(const Exception&);
			//! Assignment operator
			Exception& operator=(const Exception&);
			//! Destructor
			virtual ~Exception();
			//! Returns the message of the exception
			virtual const MCHAR* What() const;
		private:
			//! Free's the memory for the string
			void cleanup();
			//! Copies string memory to it's class instance variable mMessage.
			void copy(const MCHAR*);
		protected:
			//! Default constructor
			Exception();
			//! The error message. This memory is owned by an instance of this class.
			MCHAR* mMessage;
		};

		//! \brief Thrown when an out of  bounds index is detected.
		/*! Used when bounds checking indexes passed into arrays.
			For instance if an array has 10 items, valid indexes would
			be 0 thru 9. Therefore an instance of this class could be 
			thrown if an index of -1 or 10 was passed into an array. */
		class UtilExport OutOfRangeException : public Exception
		{
		public:
			OutOfRangeException(const MCHAR* message);
			const MCHAR* What() const;
		};

		//! Thrown when a null pointer is dereferenced
		class UtilExport NullDereferenceException : public Exception
		{
		public:
			NullDereferenceException(const MCHAR* message);
			const MCHAR* What() const;
		};

		//! General multi-purpose exception for runtime errors.
		class UtilExport RunTimeException : public Exception
		{
		public:
			RunTimeException(const MCHAR* message);
			const MCHAR* What() const;
		};
	}
}


