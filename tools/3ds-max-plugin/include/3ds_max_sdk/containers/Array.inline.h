//*****************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//*****************************************************************************/
/*==============================================================================

  file:     Array.inline.h
  author:   Daniel Levesque
  created:  27 March 2006
  description:
    Array container.

==============================================================================*/
#pragma once
namespace MaxSDK {

template<class T> inline T* Array<T>::ArrayAllocate(size_t len)
{
	DbgAssert(len < 0x40000000);  // 1G sanity check
	T* p = (T*) UtilAllocateMemory(len * sizeof(T));
	return p;
}
#pragma warning(push)
// Disable the warnings for conditional expression is constant as a result of the compiler support for Type Traits
#pragma warning(disable:4127)
#pragma push_macro("new")
#undef new
// This uses placement new, so disable any new macros here

template <class T> inline void Array<T>::ArrayConstruct(T* arrayBegin, size_t len, const T& defaultVal)
{
	for(size_t i = 0; i < len; ++i)
	{
		new(&(arrayBegin[i])) T(defaultVal);
	}
}
#pragma pop_macro("new")

template <class T> inline void Array<T>::ArrayDeAllocate(T* arrayBegin)
{
	UtilDeallocateMemory(arrayBegin);
}

#pragma warning(push)
#pragma warning(disable:4100)
template <class T> inline void Array<T>::ArrayDestruct(T* arrayBegin, size_t len)
{
	if (!std::is_trivially_destructible<T>::value)
	{
		for (size_t i = 0; i < len; ++i)
		{
			arrayBegin[i].~T();
		}
	}
}
#pragma warning(pop) // 4100

template <class T> void Array<T>::ArrayCopy(T* pCopy, size_t nMaxCount, const T* pSource, size_t nCount)
{
	// Auto-detect whether it's safe to use memcpy() or whether we need
	// to call the copy operator. We're counting on the fact that this condition,
	// being resolvable at compile-time, will be removed by the optimizer.
	if (std::is_copy_assignable<T>::value)
	{
		// Type has an assignment operator; use it.
		for (size_t i = 0; i < nCount; ++i)
		{
			pCopy[i] = (pSource[i]);
		}
	}
	else
	{
		// Type does not have an assignment operator; use memcpy() as it's usually faster.
		if (nCount > 0)
		{
			memcpy_s(pCopy, nMaxCount * sizeof(T), pSource, nCount * sizeof(T));
		}
	}
}

template <class T> void Array<T>::ArrayCopyOverlap(T* pCopy, size_t nMaxCount, const T* pSource, size_t nCount)
{
	// Auto-detect whether it's safe to use memcpy() or whether we need
	// to call the copy operator. We're counting on the fact that this condition,
	// being resolvable at compile-time, will be removed by the optimizer.
	if (std::is_copy_assignable<T>::value)
	{
		// Type has an assignment operator; use it.
		if (pCopy == pSource)
		{
			// nothing to do here, bail early
			return;
		}

		if (pCopy < pSource)
		{
			// forward iteration
			for (size_t i = 0; i < nCount; i++)
			{
				pCopy[i] = pSource[i];
			}
		}
		else
		{
			// backward iteration
			for (size_t i = nCount - 1; i != (size_t)-1; --i)
			{
				pCopy[i] = pSource[i];
			}
		}
	}
	else
	{
		// Type does not have an assignment operator; use memmove() as it's usually faster.
		if (nCount > 0)
		{
			memmove_s(pCopy, nMaxCount * sizeof(T), pSource, nCount * sizeof(T));
		}
	}
}
#pragma push_macro("new")
#undef new
// This uses placement new, so disable any new macros here
template <class T> void Array<T>::ArrayCopyConstruct(T* pCopy, size_t nMaxCount, const T* pSource, size_t nCount)
{
	// Auto-detect whether it's safe to use memcpy() or whether we need
	// to call the copy operator. We're counting on the fact that this condition,
	// being resolvable at compile-time, will be removed by the optimizer.
	if (std::is_copy_constructible<T>::value)
	{
		// Type has an assignment operator; use it.
		for (size_t i = 0; i < nCount; ++i)
		{
			new(&(pCopy[i])) T(pSource[i]); // using placement new.
		}
	}
	else
	{
		// Type does not have an assignment operator; use memcpy() as it's usually faster.
		if (nCount > 0)
		{
			memcpy_s(pCopy, nMaxCount * sizeof(T), pSource, nCount * sizeof(T));
		}
	}
}
#pragma pop_macro("new")
#pragma warning(pop) // 4127

// Inline methods.
template <class T> inline bool Array<T>::contains(const T& value, size_t start) const
{ 
	return this->findFrom(value, start) != -1;
}

template <class T> inline size_t Array<T>::length() const
{ 
	return mUsedLen;
}

template <class T> inline bool Array<T>::isEmpty() const
{ 
	return mUsedLen == 0; 
}

template <class T> inline size_t Array<T>::lengthUsed() const
{ 
	return mUsedLen; 
}

template <class T> inline size_t Array<T>::lengthReserved() const
{ 
	return mReservedLen; 
}

template <class T> inline size_t Array<T>::growLength() const
{ 
	return mGrowLen;
}

template <class T> inline const T* Array<T>::asArrayPtr() const
{ 
	return mpArray;
}

template <class T> inline T* Array<T>::asArrayPtr()
{ 
	return mpArray; 
}

template <class T> inline bool Array<T>::isValidIndex(size_t i) const
{ 
	// We should prohibit index's that are the maximum size_t value
	return (i != (size_t)-1) && i < mUsedLen;
}

template <class T> inline T& Array<T>::operator [] (size_t i)
{
	if (!isValidIndex(i))
	{
		DbgAssert(false);
		throw MaxSDK::Util::OutOfRangeException(_M("Argument index out of bounds, passed into a MaxSDK::Array::operator[]"));
	}
	return mpArray[i];
}

template <class T> inline const T& Array<T>::operator [] (size_t i) const
{ 
	if (!isValidIndex(i))
	{
		DbgAssert(false);
		throw MaxSDK::Util::OutOfRangeException(_M("Argument index out of bounds, passed into a MaxSDK::Array::operator[]"));
	}
	return mpArray[i];
}

template <class T> inline T& Array<T>::at(size_t i)
{ 
	if (!isValidIndex(i))
	{
		DbgAssert(false);
		throw MaxSDK::Util::OutOfRangeException(_M("Argument index out of bounds, passed into a MaxSDK::Array::at()"));
	}
	return mpArray[i];
}

template <class T> inline const T& Array<T>::at(size_t i) const
{ 
	if (!isValidIndex(i))
	{
		DbgAssert(false);
		throw MaxSDK::Util::OutOfRangeException(_M("Argument index out of bounds, passed into a MaxSDK::Array::at()"));
	}
	return mpArray[i];
}

template <class T> inline Array<T>& Array<T>::setAt(size_t i, const T& value)
{ 
	if (!isValidIndex(i))
	{
		DbgAssert(false);
		throw MaxSDK::Util::OutOfRangeException(_M("Argument index out of bounds, passed into a MaxSDK::Array::setAt()"));
	}
	mpArray[i] = value;
	return *this;
}

template <class T> inline T& Array<T>::first()
{ 
	DbgAssert(!this->isEmpty());
	DbgAssert(mpArray != nullptr);
	if (this->isEmpty() || (nullptr == mpArray))
	{
		throw MaxSDK::Util::RunTimeException(_M("Attempting to call MaxSDK::Array::first() on an empty array"));
	}
	return mpArray[0]; 
}

template <class T> inline const T& Array<T>::first() const
{ 
	DbgAssert(!this->isEmpty());
	DbgAssert(mpArray != nullptr);
	if (this->isEmpty() || (nullptr == mpArray))
	{
		throw MaxSDK::Util::RunTimeException(_M("Attempting to call MaxSDK::Array::first() on an empty array"));
	}
	return mpArray[0];
}

template <class T>
T* Array<T>::begin() {
	return mpArray;
}
template <class T>
const T* Array<T>::begin() const {
	return mpArray;
}

template <class T> inline T& Array<T>::last()
{
	DbgAssert(!this->isEmpty());
	DbgAssert(mpArray != nullptr);
	if (this->isEmpty() || (nullptr == mpArray))
	{
		throw MaxSDK::Util::RunTimeException(_M("Attempting to call MaxSDK::Array::last() on an empty array"));
	}
	return mpArray[mUsedLen-1];
}

template <class T> inline const T& Array<T>::last() const
{ 
	DbgAssert(!this->isEmpty()); 
	DbgAssert(mpArray != nullptr);
	if (this->isEmpty() || (nullptr == mpArray))
	{
		throw MaxSDK::Util::RunTimeException(_M("Attempting to call MaxSDK::Array::last() on an empty array"));
	}
	return mpArray[mUsedLen-1];
}
template <class T>
T* Array<T>::end() {
	return mpArray + mUsedLen;
}
template <class T>
const T* Array<T>::end() const {
	return mpArray + mUsedLen;
}

template <class T> inline size_t Array<T>::append(const T& value)
{ 
	insertAt(mUsedLen, value);
	return mUsedLen-1; 
}

template <class T> Array<T>& Array<T>::append(const T* values, size_t count)
{
	return insertAt(mUsedLen, values, count);
}

template <class T> inline Array<T>& Array<T>::removeFirst()
{ 
	DbgAssert(!isEmpty()); 
	return removeAt(0); 
}

template <class T> inline Array<T>& Array<T>::removeLast()
{ 
	DbgAssert(!isEmpty());
	return removeAt(mUsedLen - 1); 
}

template <class T> inline Array<T>& Array<T>::removeAll()
{ 
	if(mUsedLen > 0) {
		ArrayDestruct(mpArray, mUsedLen);
		mUsedLen = 0;
	}
	return *this; 
}

template <class T> inline Array<T>& Array<T>::setGrowLength(size_t glen)
{ 
	DbgAssert(glen > 0);
	if(glen > 0) {
		mGrowLen = glen;
	}
	else {
		DbgAssert(false);
		// Growth length needs to be at least 1.
		mGrowLen = 1; 
	}
	return *this; 
}


template <class T> inline void Array<T>::handleOutOfMemory() {

	DbgAssert(false);
	UtilOutOfMemoryException();
}

} // namespace MaxSDK 
