//
// Copyright 2012 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "../maxapi.h"
#include "TipSystemExport.h"

namespace MaxSDK { namespace Util{

/*! An interface of giving users a tip .
*/

class ITipSystem : public MaxHeapOperators
{
public:

	//! \brief Destructor
	virtual ~ITipSystem(){}

	//! \brief Give users a tip
	/* \param elapsedTime - The elapsed time for the tip window showing
	 * \param tipContent - The content in the tip window
	 * \param xRatio - the normalized position in x (0 = right | 1 = left)
	 * \param yRatio - the normalized position in y (0 = top | 1 = bottom)
	 * \param hParent - The parent window for the tip window to dock on
	 */
	virtual bool ShowTip(UINT elapsedTime, const MSTR& tipContent, float xRAtio, float yRatio, HWND hParent) = 0;

};

TipSystemExport ITipSystem* GetTipSystem();

}}