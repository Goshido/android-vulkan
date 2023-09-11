//
// Copyright 2012 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "../utilexp.h"
#include "../maxapi.h"

namespace MaxSDK { namespace Util{

/*! An interface of bailing out the busy processing functions by pressing 
	the 'Esc' key.
*/
class IBailOutBusyProcessManager : public MaxHeapOperators
{
public:
	/*!	Plug-ins may have functions of long busy process which can't be accepted by users.
		In this condition, users can use this interface to escape the long busy process by pressing ESC key.
		\code
		void LongBusyProcess()
		{
			...
			MaxSDK::Util::IBailOutBusyProcessManager* pBailOutManager = MaxSDK::Util::GetBailOutManager();
			if(pBailOutManager)
			{
				pBailOutManager->EmbedBailOut();
			}
			
			for(int i=0; i<100000; i++)
			{
			   if(pBailOutManager && pBailOutManager->ShouldBail())
			   {
				   MemoryClear(); // here users need to clear the memory manually.
			       break;
			   }
			   ...
			   Process()
			   ...
			}
			if(pBailOutManager)
			{
				pBailOutManager->BailOutEnd();
			}
		}
		\endcode

        Note that, for safety when the bailout manager is used inside multiple nested function calls,
        the state of the instance is set and cleared only by the outermost calls to EmbedbailOut and
        BailOutEnd. For instance, if EmbedBailOut and BailOutEnd were called in Process() above, the
        state flags representing embedded state and bail-out condition would be initialized and
        cleared by the call in LengBusyProcess(), while the calls in Process() would serve to track the
        number of nested levels.
	*/
	//! Destructor
	virtual ~IBailOutBusyProcessManager(){}

	/*! At the beginning of the busy process, a bail-out can be embed, so that
		you can trigger the bail-out in the busy process to jump out the busy process.
		For example, you can embed bail-out at the front of long time loop.
	*/
	virtual void EmbedBailOut() = 0;

	/*!	After bail-out operation is done, you need to end the bail-out.
	*/
	virtual void BailOutEnd() = 0;

	/*!	To detect whether the bail-out should jump out the busy process or not.
		\return If true bail-out should jump out the busy process.
		If false bail-out should not jump out the busy process.
	*/
	virtual bool ShouldBail() = 0;

	/*! To detect whether the bail-out is done or not.
		\return If true bail-out is done. If false bail-out is missed.
	*/
	virtual bool IsBailOut() const = 0;

	/*! To detect whether the bail-out is embed or not.
		\return If true bail-out is embed. If false bail-out is not embed.
	*/
	virtual bool IsBailOutEmbeded() const = 0;

};

UtilExport IBailOutBusyProcessManager* GetBailOutManager();

/*! A wrapper around the underlying IBailOutProcessManager instance. Ensures proper cleanup of the state of the
    instance even if we happen to throw out of the function in which it is used.
*/
class UtilExport BailOutManager: public MaxHeapOperators
{
public:
	/*!	Plug-ins may have functions of long busy process which can't be accepted by users.
		In this condition, users can use this interface to escape the long busy process by pressing ESC key.
		\code
		void LongBusyProcess()
		{
			using MaxSDK::Util;
            BailOutManager bailoutManager;
			...
			for(int i=0; i<100000; i++)
			{
			   if(bailoutManager.ShouldBail())
			   {
				   MemoryClear(); // here users need to clear the memory manually.
			       break;
			   }
			   ...
			   Process()
			   ...
			}
		}
		\endcode
	*/
//      Note: The implementation of this class, and the underlying BailOutBusyProcessManagerImp singleton that
//      it wraps, ensure "proper" behaviour for a nested sequence of scopes that each instantiate a separate
//      BailOutManager on the stack (hence, dynamic allocation is prevented). Consider a definition of Process
//      in the above snippet:
//
//      void Process()
//      {
//          using MaxSDK::Util;
//          BailOutManager bailoutManager;
//          for(int i=0; i<100000; i++)
//          {
//              ...
//              if(bailoutManager.ShouldBail())
//              {
//                  MemoryClear(); // here users need to clear the memory manually.
//                  break;
//              }
//              ...
//          }
//      }
//
//      Assume the BailOutBusyProcessManagerImp singleton is in an "unembedded" state before the call to
//      LongBusyProcess. The instantiation in LongBusyProcess sets the level counter in the singleton to
//      1, and the instantiation in Process sets it to 2. If we exit Process for any reason, whether
//      through any return statement or by throwing an exception, the call to BailOutEnd in ~BailOutManager
//      will decrement the level counter back to 1, but will preserve the state of the singleton, so that
//      it is still in an embedded state (i.e., it will continue to check for escapes). Further, if
//      Process was exited due to an escape being caught, the escaped state will still be set, so that
//      LongBusyProcess will exit as well. On any form of exit from LongBusyProcess, the level count will
//      be decremented to 0, and the state of the singleton will be fully reset. Note that this would be
//      equally true if Process were simply a nested scope, rather than a separate function.

	//! Constructor
    BailOutManager()
    {
        m_manager = GetBailOutManager();
        m_manager->EmbedBailOut();
    }

	//! Destructor
    ~BailOutManager()
    {
        m_manager->BailOutEnd();
    }

	/*!	To detect whether the bail-out should jump out the busy process or not.
		\return If true bail-out should jump out the busy process.
		If false bail-out should not jump out the busy process.
	*/
	bool ShouldBail()
    {
        return m_manager->ShouldBail();
    }

	/*! To detect whether the bail-out is done or not.
		\return If true bail-out is done. If false bail-out is missed.
	*/
	bool IsBailOut()
    {
        return m_manager->IsBailOut();
    }

	/*! To detect whether the bail-out is embed or not.
		\return If true bail-out is embed. If false bail-out is not embed.
	*/
	bool IsBailOutEmbeded()
    {
        return m_manager->IsBailOutEmbeded();
    }

	/*! Copying and dynamic allocation are not supported for this class.
	*/
    static void* operator new(size_t size)
    {
        UNUSED_PARAM(size);
        throw std::bad_alloc();
        return NULL;
    }

private:

	/*! Copying and dynamic allocation are not supported for this class.
	*/
    BailOutManager(const BailOutManager& bailoutManager);
    BailOutManager& operator=(const BailOutManager& bailoutManager);
    static void* operator new[](size_t size);

	/*! Underlying IBailOutBusyProcessManager instance.
	*/
    IBailOutBusyProcessManager* m_manager;
};
}}