//
// Copyright 2012 Autodesk, Inc.  All rights reserved. 
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//
//
#pragma once

#include "../Noncopyable.h"

namespace MaxSDK
{
namespace Util
{

//! \brief A pure virtual interface for Poly Object's UI as dialog Showing Status.
/*! class IDlgShowingStatusForPolyObj
If the Poly Objects' UI setting is used as dialog, dialog will replace caddy to show the
parameters of the Poly Objects. If the dialog is showing right now, users change the Poly 
Objects' UI setting as caddy, a tip dialog at the bottom-center of the View panel in Max will
pop up and remind users that the caddy controls will now be used.
*/

class IDlgShowingStatusForPolyObj : public MaxSDK::Util::Noncopyable
{
public:
	//! Destructor
	virtual ~IDlgShowingStatusForPolyObj(){}

	/*! To get whether the Poly Object's UI as dialog is currently showing or not.
		\return true if the dialog is currently showing, false otherwise.
	*/
	virtual bool IsDlgShowingForPolyObj() const = 0;

	/*! To set the showing status of Poly Object's UI as dialog.
		\param[in] bShowing if bShowing is true, the showing status of Poly Object's UI as dialog
				   is on. false otherwise.
	*/
	virtual void SetDlgShowingForPolyObj(bool bShowing) = 0;

	/*! To get whether the Poly Object's UI as dialog is set as on or off.
		\return true if the dialog is set as on, false otherwise.
	*/
	virtual bool GetDlgShowSettingForPolyObj() const = 0;

	/*! To set the showing setting of Poly Object's UI as dialog.
		\param[in] bDlgShow if bDlgShow is true, the showing setting of Poly Object's UI as dialog
				   is on. false otherwise.
	*/
	virtual void SetDlgShowSettingForPolyObj(bool bDlgShow) = 0;
};

UtilExport IDlgShowingStatusForPolyObj* GetDlgShowingStatusForPolyObj();

}
}