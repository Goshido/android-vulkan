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

#include "../dbgprint.h" // for DebugPrint
#include <typeinfo> // for typeid
#include <string> // for std::string
#include <atomic> // for std::atomic

/*! \brief
This class is derived from in order to automatically count the number of existing instances, and at 
shutdown write to Debug Output the leakage count for the class. The template class Obj should be
set to class deriving from this class. For example:
class ParamBlock2PostLoadInfo : public IParamBlock2PostLoadInfo, private CountedObject<ParamBlock2PostLoadInfo>
On shutdown, with 0 leakage, the output to Debug Output would be:
**Leakage count** class ParamBlock2PostLoadInfo: 0
*/
template <class Obj>
class CountedObject
{
public:
	CountedObject() { GetCounter()++; }
	CountedObject(const CountedObject&) { GetCounter()++; }
	~CountedObject() { GetCounter()--; }

	/*! \brief Returns the current number of class instances.
	*/
	static unsigned __int64 GetObjectCount() { return GetCounter(); }

private:

	//! \brief Prevent Assignments. 
	CountedObject& operator=(const CountedObject&) = delete;

	class CountedObjectCounter
	{
	public:
		CountedObjectCounter(const char* className) : mClassName(className) {}
		~CountedObjectCounter()
		{
			DebugPrint(_T("**Leakage count** %hs: %Iu\n"), mClassName.c_str(), mInstanceCount.load());
		}
		std::atomic<unsigned __int64>& GetCounter()
		{
			return mInstanceCount;
		}

	private:
		std::atomic<unsigned __int64> mInstanceCount{ 0 };
		std::string mClassName;
	};

	std::atomic<unsigned __int64>& GetCounter()
	{
		static CountedObjectCounter mCountedObjectCounter(typeid(Obj).name());
		return mCountedObjectCounter.GetCounter();
	}
};
