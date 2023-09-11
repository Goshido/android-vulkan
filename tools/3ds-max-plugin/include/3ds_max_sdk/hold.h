//*****************************************************************************
// Copyright 1994, 2009 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise accompanies 
// this software in either electronic or hard copy form.   
//
//*****************************************************************************
//
//	FILE: hold.h
//	DESCRIPTION:	Describes the hold system objects 
//	CREATED BY:	Dan Silva
//
//*****************************************************************************

#pragma once

#include "maxheap.h"
#include "baseinterface.h"
#include "strclass.h"

#define RESTOBJ_DOES_REFERENCE_POINTER		1 // cmd ID for RestoreObj::Execute
#define RESTOBJ_GOING_TO_DELETE_POINTER		2 // cmd ID for RestoreObj::Execute
#define REFTARG_AUTO_DELETED				10 // cmd ID for HoldStore::Execute

#define HOLD_SYSTEM_EMPTY					1 // cmd ID for Hold::Execute - return 1 if hold system is definitely empty
#define HOLD_POINTER_IS_MANAGER				2 // cmd ID for Hold::Execute - return 1 if arg1 is a pointer to the undo manager
#define HOLD_SUPERLEVEL						3 // cmd ID for Hold::Execute - return Hold.superLevel

class HoldStore;
class OperationDesc;
/*! \sa  Class Hold, ~{ The Undo and Redo System }~.\n\n
\par Description:
This class is the restore object used in the undo / redo system of 3ds Max.
 */
class RestoreObj : public InterfaceServer {
	friend class HoldStore;
	friend class GenUndoObject;
	friend class CheckForFlush;
	friend class UndoMgrBody;
		RestoreObj *next,*prev;
	public:
		RestoreObj() { next = prev = NULL; }
		virtual ~RestoreObj() {};
		/*! \remarks The developer implements this method to restore the state of the
		database to as it was when <b>theHold.Put()</b> was called with this
		restore object.\n\n
		Regardless of the value of the isUndo parameter, the method must
		restore the state of the scene to the one it was in when theHold.Put()
		was called with this restore object.\n\n
		RestoreObj::Restore and RestoreObj::Redo may be called several times in
		a row: these methods must protect against  any problems that may arise
		because of this, such as trying to undo multiple times in a row what
		has been undone already.
		\param isUndo Nonzero if <b>Restore()</b> is being called in response to the Undo
		command; otherwise zero. If <b>isUndo</b> is nonzero, the developer
		needs to save whatever data they need to allow the user to redo the
		operation. */
		virtual void Restore(int isUndo)=0;
		/*! \remarks This method is called when the user selects the Redo command. The
		developer should restore the database to the state prior to the last
		Undo command. */
		virtual void Redo()=0;
		/*! \remarks Returns the size of the restore object in bytes. This size does not
		need to be exact but should be close. This is used to make sure all the
		accumulated restore objects do not grow beyond a manageable size.
		\par Default Implementation:
		<b>{ return 1; }</b>
		\return  The size of the restore object in bytes. */
		virtual int Size() { return sizeof(RestoreObj); }
		/*! \remarks This method is called when <b>theHold.Accept()</b> or
		<b>theHold.Cancel()</b> is called. This means the restore object is no
		longer held, it was either tossed out or sent to the undo system. The
		developer may then call <b>ClearAFlag(A_HELD)</b> to indicate the
		restore object is no longer being held. */
		virtual void EndHold() { }
		/*! \remarks This method is used internally to 3ds Max in debugging only. It is used
		to display a symbolic name for the restore object.
		\return  The name of the restore object.
		\par Default Implementation:
		<b>{ return MSTR(_M("---")); }</b> */
		virtual MSTR Description() { return MSTR(_M("---")); }

		/*! \remarks This method is used to specify whether putting this restore object
		to the hold system should mark the scene dirty in terms of requiring a scene file save.
		\return  Whether to make the scene dirty in terms of requiring a scene file save.*/
		virtual bool MarkSceneDirtyForSave() const { return true; }

		/*! \remarks This is a general purpose function that allows the API to be extended
		in the future. The 3ds Max development team can assign new <b>cmd</b>
		numbers and continue to add functionality to this class without having
		to 'break' the API.\n\n
		This is reserved for future use.
		\par Default Implementation:
		<b>{return -1;}</b> */
		virtual INT_PTR Execute(int cmd, ULONG_PTR arg1 = 0, ULONG_PTR arg2 = 0, ULONG_PTR arg3 = 0)
		{
			UNUSED_PARAM(cmd);
			UNUSED_PARAM(arg1);
			UNUSED_PARAM(arg2);
			UNUSED_PARAM(arg3);
			return -1;
		}
	};

/*! \sa  Class RestoreObj, ~{ The Undo and Redo System }~.\n\n
\par Description:
The undo / redo system of 3ds Max uses a global instance of this class called
<b>theHold</b>. Developers call methods of <b>theHold</b> object to participate
in the undo / redo system.  */
class Hold : public BaseInterfaceServer
{
private:
	HoldStore* holdStore;
	HoldStore* holdList;
	int undoEnabled;
	int superLevel;
	int putCount;
	HoldStore* ResetStore();
	void Init();
	void Accept(const OperationDesc& cmdDesc, bool legacyInterface);
	void AddOpToUndoMgr(const OperationDesc& cmdDesc, bool legacyInterface);
public:
	//! constructor
	CoreExport Hold();
	//! destructor
	CoreExport ~Hold();

	/*! \brief The developer calls this method to register a new restore object with the system.
	\param obj The restore object to register.
	\return - true if successfully added, false if not. It will return false if it's not holding.
	\code
	if ( theHold.Holding() ) {
	theHold.Put(new MeshSelectRestore(meshData,this));
	}
	\endcode 	*/
	CoreExport bool Put(RestoreObj* obj);
	
	/*! \brief Starts an undoable operation.
        When a plugin is about to start a user-triggered operation that should be undoable (normally as a result of pressing a button or 
        activating a UI control in some way), this method must be called to initiate the undoable operation. 
        This method turns the undo system on - placing it in the "holding" state, such that any subsequent scene modifications will
        get registered as undoable operations. For example, any change in ParamBlock2 parameter values
        will automatically be registered with the undo system if and only if Begin() has been called. Failure to call Begin()
        before performing scene changes will render those changes permanent and impossible to undo.

        Following the completion of the user-triggered operation, Accept() must be called to signal completion of the undo operation.

        Internally, a counter is used to support overlapping undoable operations. The undoable operation is registered by the system
        only once the counter returns to 0 - i.e. once there have been an equal number of Accept() and Begin(). 

        \remark For every call to Begin(), there should be a matching call to either Cancel() or Accept().*/
	CoreExport void Begin();
	
	//! Temporarily suspends holding.
	CoreExport void Suspend();
	
	//! Reports whether the hold system is suspended or not.
	CoreExport int IsSuspended();
	
	//!  It resumes holding if it was suspended.
	CoreExport void Resume();
	
	/*! Indicates if theHold.Begin() has been called. 
	Any operation that modifies the database checks to see if theHold is currently
	in a holding state. If the undo system is 'holding' it is ready to
	accept restore objects. For more details see the Advanced Topics
	section on ~{ The Undo and Redo System }~.
	\return  Nonzero if theHold is holding; otherwise 0.
	\code	
	if ( theHold.Holding() ) {
	...
	} 
	\endcode */
	CoreExport int	Holding();  // are we holding?
	
	/*! Returns nonzero if the system is restoring and zero if not.
	\param isUndo This parameter is updated to indicate if the restore is coming from an
	undo. It's assigned nonzero if it is; zero if not. */
	CoreExport int  Restoring(int& isUndo);
	
	/*! Returns nonzero if the system is redoing and zero if not. */
	CoreExport int  Redoing();
	
	/*! Returns nonzero if the system is restoring or redoing and zero if not. 	*/
	CoreExport int  RestoreOrRedoing();

	/*! This is used internally. Plug-In developers should not call this
	method. Prevents Undo when Accept() is called. */
	CoreExport void DisableUndo();  // prevents Undo when Accept is called.
	
	/*! This is used internally. Plug-In developers should not call this
	method. Allows Undo when Accept() is called. */
	CoreExport void EnableUndo();
	
	/*! This is used internally. Plug-In developers should not call this
	method. Returns state set by DisableUndo/EnableUndo. */
	CoreExport BOOL IsUndoDisabled();

	/*! Returns current nesting level of Begin() calls. */
	CoreExport int GetBeginDepth();

	/*! This method will call Restore() on all the restore objects registered
	since the last Begin(). This restores the database to the state
	it was in when Begin() was called. The restore objects are NOT
	deleted. */
	CoreExport void Restore();
	
	/*! This tosses out the restore objects since the last Begin() but
	still continues holding. Group several Begin-End lists into a single Super-group. */
	CoreExport void Release();

	// 3 ways to terminate the Begin()...

	/*! This method is used internally to 3ds Max and should not be called by a
	plug-in developer. It leaves the database in its modified state but
	throws out the restore object. */
	CoreExport void End();
	
    //! For internal use only.
	CoreExport void Accept(int commandResId);

	//! \brief Signals the completion of an undoable operation.
    /*! Once a user-triggered, undoable operation has been completed, this method must be called to signal that completion to the system.
        The undoable operation is registered and published using the provided as its title.
        \remark For every call to Begin(), there should be a matching call to either Cancel() or Accept().
        \param commandName The string which is to be displayed to the user, as the name of the undo operation (e.g. to appear in the Edit menu).
            The system will prepend "Undo " to the string. */
	CoreExport void Accept(const MCHAR *commandName);

	//! \brief Signals the completion of an undoable operation.
	/*! Once a user-triggered, undoable operation has been completed, this method must be called to signal that completion to the system.
		The undoable operation is registered and published using the information in its operation descriptor.
		\remark For every call to Begin(), there should be a matching call to either Cancel() or Accept().
		\remark This method overload shall be used when the operation is also reported to ADP "MAX.MODEL" way point.
		\param opDesc The command description with all Adp reportable information together with string to be displayed to the user, 
			as the name of the undo operation (e.g. to appear in the Edit menu).*/
	CoreExport void Accept(const OperationDesc& opDesc);

	//! \brief Aborts an undoable operation previously initiated with Begin().
    /*! If an undo operation is to be aborted before it was completed (through Accept()), then this method should be called.
        All scene changes that have been registered since Begin() was called are undone by this method. */
	CoreExport void Cancel();

	//		
	// Group several Begin-End lists into a single Super-group.
	/*! \remarks Implemented by the System.\n\n
	Normally this is NOT needed but in special cases this can be useful.
	This allows a developer to group a set of Begin()/Accept() sequences to
	be undone in one operation.\n\n
	Consider the case of the user using the Shift-Move command to create a
	new node in the scene. There are two parts to this process. First the
	node must be cloned and second the position must be tracked as the user
	moves the mouse to place the new node in the scene. Naturally if the
	user wanted to Undo this operation, they would expect a single
	selection of the Undo command would accomplish it. However the process
	was not a single operation. There was the initial cloning of the node,
	and then the iterative process of moving the node in the scene,
	restoring its position, moving it again, restoring it again, etc. Cases
	like this are handled using methods of <b>theHold</b> named
	<b>SuperBegin(), SuperAccept()</b> and <b>SuperCancel()</b>. These
	allow the developer to group several restore objects together so that
	they may be undone via a single selection of Undo. Note that in this
	example it is only necessary to use <b>SuperBegin()</b> and
	<b>SuperAccept()</b> because the move was restoring interactively.
	Normally a developer does NOT need to use these methods even if they
	have several operations that modify the database. The undo system will
	automatically register all the restore objects needed as part of the
	undo object when <b>theHold.Accept()</b> is called and these may be
	undone using a single UNDO command.
	\par Sample Code:
	See the sample program <b>/MAXSDK/SAMPLES/OBJECTS/BONES.CPP</b>. */
	CoreExport void SuperBegin();
	/*! \remarks Implemented by the System.\n\n
	When a developer has used <b>SuperBegin()</b>, this method is used to
	Accept. This leaves the database in its modified state and registers
	the restore object with the undo system. This will allow the user to
	undo the operation.
	\param nameID The resource ID of the string to appear in the Edit menu next to Undo or Redo.
	\par Sample Code:
	See the sample program <b>/MAXSDK/SAMPLES/OBJECTS/BONES.CPP</b>. */
	CoreExport void SuperAccept(int nameID);
	/*! \remarks Implemented by the System.\n\n
	When a developer has used <b>SuperBegin()</b>, this method is used to
	Accept. This leaves the database in its modified state and registers
	the restore object with the undo system. This will allow the user to
	undo the operation.
	\param name The string to appear in the Edit menu next to Undo or Redo. */
	CoreExport void SuperAccept(const MCHAR *name);
	/*! \remarks Implemented by the System.\n\n
	When a developer has used <b>SuperBegin()</b>, this method is used to
	Cancel. This restores the database to its previous state and throws out
	the restore object. This cancels the operation.
	\par Sample Code:
	See the sample program <b>/MAXSDK/SAMPLES/OBJECTS/BONES.CPP</b>. */
	CoreExport void SuperCancel();
	/*! \remarks Implemented by the System.\n\n
	Returns current nesting level of SuperBegin() calls. */
	CoreExport int GetSuperBeginDepth();


	// Get the number of times Put() has been called in the current session of MAX
	/*! \remarks Returns the number of times <b>Put()</b> has been called in the current
	session of 3ds Max. */
	CoreExport int GetGlobalPutCount();

	CoreExport INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0);

	/*! \brief Returns the (approximate) memory usage of this hold.

	The memory returned is the sum of all RestoreObj's Size() methods, so the accuracy
	of this method depends on the accuracy of the restore objects.

	\see RestoreObj::Size
	\return Size, in bytes, of the restore objects held by this hold.
	!*/
	CoreExport DWORD_PTR Size() const;

protected:
	friend class HoldStore;
	unsigned flags;
	enum {
		kInRestore,
		kInUndo,
		kInRedo
	};
};

extern CoreExport Hold theHold;

CoreExport void EnableUndoDebugPrintout(BOOL onoff);

// A class to help control the hold system. Create an instance of this class, and when it is
// destructed any hold suspends are resumed. Makes it safe in case a throw occurs after the suspend,
// but before the resume
class HoldSuspend: public MaxHeapOperators {
public:
	HoldSuspend(BOOL suspendNow = TRUE) : suspendCount(0) {
		if (suspendNow) {
			this->Suspend();
		}
	}
	~HoldSuspend()	{
		while(suspendCount > 0) {
			this->Resume();
		}
	}
	void Suspend() {
		if (suspendCount == 0) 
			theHold.Suspend ();
		suspendCount++;
	}
	void Resume() {
		if (suspendCount == 1) 
			theHold.Resume();
		if (suspendCount > 0) 
			suspendCount--;
	}

private:
	int	suspendCount;

};					  
