//
// Copyright 2010 Autodesk, Inc.  All rights reserved.  
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//
// 

#pragma once

#include "../GetCOREInterface.h"
#include "../ifnpub.h"

// --- Interface IDs
#define MAIN_THREAD_TASK_MANAGER_INTERFACE Interface_ID(0x79752833, 0x743b438e)

/*! \brief Derive from this class to pass a task to be executed on 3DS Max main thread
*/
class MainThreadTask : public MaxHeapOperators
{
public:
	virtual ~MainThreadTask() {}
	
	/*! \brief This method will be called from the main thread
	*/
	virtual void Execute() = 0;
};  

/*! \brief 
This class allows a plugin to post a Task to be executed on 3DS Max main thread.
It is necessary for example to modify 3DS Max UI controls which can only be done from 3DS Max main thread.
*/
class IMainThreadTaskManager : public FPStaticInterface 
{
public:
	/*! \brief Call this method to execute a task on 3DS Max main thread
	\param [in] task the Task to be executed (see Task), will be freed by the system.
	*/
	virtual void PostTask( MainThreadTask* task ) = 0;

	/*! \brief This will execute all the pending tasks. This should only be called from the main thread, the call is ignored otherwise.
	*/
	virtual void ExecutePendingTasks() = 0;

	/*! \brief Returns true if there are pending tasks
	*/
	virtual bool TasksPending() = 0;

	/*! \brief Retrieves the single instance of the Main thread task Manager
	*/
	static IMainThreadTaskManager* GetInstance()	{
        static IMainThreadTaskManager* s_pIMainThreadTaskManager = static_cast<IMainThreadTaskManager*>(GetCOREInterface(MAIN_THREAD_TASK_MANAGER_INTERFACE));
		return s_pIMainThreadTaskManager;
	}
};