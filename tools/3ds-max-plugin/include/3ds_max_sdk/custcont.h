//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// FILE:        custcont.h
// DESCRIPTION: Custom Controls for Max
// AUTHOR:      Rolf Berteig
// HISTORY:     created 17 November, 1994
//**************************************************************************/

#pragma once

#include "maxheap.h"
#include "winutil.h"
#include "buildver.h"
#include "strclass.h"
#include "actiontableTypedefs.h"
#include "geom/acolor.h"
#include "baseinterface.h"
#include "ifnpub.h"
#include "Path.h"

// Standard Category for rollups
#include "RollupConstants.h"
#pragma warning(push)
#pragma warning(disable:4100)

//! Pass this to CustButton::Execute() to turnoff borders, like in the toolbar
#define I_EXEC_CB_NO_BORDER					0xA000
//! Pass this to CustStatus::Execute() to turnoff borders, like in the toolbar
#define I_EXEC_CS_NO_BORDER					0xA001 
//! Set a spinner back to its Reset value
#define I_EXEC_SPINNER_RESET				0xA002 

//! \brief When called during a CC_SPINNER_xxx messages, it will return true if 
//! the msg was triggered by a right-click reset
#define I_EXEC_SPINNER_IS_RESET_CHANGE  	0xA003	

//! disable the alt key spinner behaviour 
#define I_EXEC_SPINNER_ALT_DISABLE			0xA004
//! enable the alt key spinner behaviour 
#define I_EXEC_SPINNER_ALT_ENABLE			0xA005
//! disable the one click spinner behaviour with alt or ctrl down
#define I_EXEC_SPINNER_ONE_CLICK_DISABLE	0xA006
//! enable the one click spinner behaviour with alt or ctrl down
#define I_EXEC_SPINNER_ONE_CLICK_ENABLE		0xA007
//! enable or disable button drag & drop within a toolbar
#define I_EXEC_BUTTON_DAD_ENABLE			0xA008 
//! return the hwnd for the tooltip 
#define I_EXEC_GET_TOOLTIP_HWND				0xA009

// Values returned by DADMgr::SlotOwner()
#define OWNER_MEDIT_SAMPLE 		0
#define OWNER_NODE 				1
#define OWNER_MTL_TEX			2  //  button in mtl or texture
#define OWNER_SCENE				3  //  button in light, modifier, atmospheric, etc
#define OWNER_BROWSE_NEW		4
#define OWNER_BROWSE_LIB		5
#define OWNER_BROWSE_MEDIT		6
#define OWNER_BROWSE_SCENE		7
#define OWNER_MATERIAL_EXPLORER	8

class CUIFrameMgrPrivate;
class ReferenceTarget;
class IParamBlock;
class IParamBlock2;
class FPInterface;
class QWidget;

namespace MaxSDK
{
	class QmaxDockingWinHostDelegate;
};


/*! \sa  Class ReferenceTarget
\par Description:
Drag and drop functionality has been expanded to include all map and material
buttons--including those in the non-standard materials, plus most cases of
bitmap buttons. As a result, whenever you see a button representing a material
or map you can drag the button over a like button to display the
Swap/Copy/Cancel dialog. Likewise, you can drag any materials or maps from the
modeless version of the Materials/Maps Browser.\n\n
The drag-and-drop functions distinguish between material maps and bitmaps. A
bitmap is an image file, such as a .tga, or .jpg. A map is an image used by the
Materials Editor. It might consist of an image file, but could just as easily
be a parametric image, such as Checkers or Noise, or it could be a map tree
consisting of several different types of maps or bitmaps. Users can drag any
map slot or button to any other map slot or button--including the sample slots.
Users can drag the Bitmap button in the Bitmap Parameters rollout to the Bitmap
button in the Image area of the Displace modifier, and vice-versa.\n\n
Users can drag <b>from</b>:\n\n
*  Sample slots\n\n
*  Browser lists (text or iconic)\n\n
*  The sample-sphere preview window in the Browser.\n\n
*  Material map buttons, including:\n\n
*  The buttons in the Maps rollout\n\n
*  The shortcut map buttons\n\n
*  Any map buttons at any level\n\n
*  Submaterial buttons, such as those found in the Multi/Subobject
material\n\n
*  Projector light map button\n\n
*  Environment background map button\n\n
*  Fog Color and Opacity maps buttons\n\n
Users can drag <b>to</b>:\n\n
*  Objects in the viewports\n\n
*  The Type button in the Materials Editor from the Browser.\n\n
*  All of the items in the FROM list, with this exception: You can only
drag to the Browser when it displays the material library.\n\n
All methods of this class are virtual. For developers of plug-in textures and
materials see Class TexDADMgr, Class MtlDADMgr. These classes provide
implementations of these methods and the objects can simply be used.  */
class DADMgr : public InterfaceServer {
	public:
		/*! \remarks This method is called on the item that supports drag and
		drop to see what (if anything) can be dragged from the point <b>p</b>.
		This method returns a super class id to indicate the type of item that
		can be dragged away. If it does not support anything being dragged from
		the specified point a SClass_ID of <b>0</b> should be returned.
		\par Parameters:
		<b>HWND hwnd</b>\n\n
		The source window handle\n\n
		<b>POINT p</b>\n\n
		The screen point (relative to the window upper left as 0,0). */
		virtual SClass_ID GetDragType(HWND hwnd, POINT p)=0;

		/*! \remarks If the method <b>GetInstance()</b> creates a new instance
		every time it is called, then the this method should return TRUE.
		Otherwise it should return FALSE. This prevents <b>GetInstance()</b>
		from being called repeatedly as the drag progresses.
		\par Parameters:
		<b>HWND hwnd</b>\n\n
		The source window handle.\n\n
		<b>POINT p</b>\n\n
		The point to drag from.\n\n
		<b>SClass_ID type</b>\n\n
		The super class ID to create.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL IsNew(HWND hwnd, POINT p, SClass_ID type) { return FALSE; } 

		/*! \remarks This method is called on potential dropee to see if can
		accept the specified type at the specified point.
		\par Parameters:
		<b>ReferenceTarget *dropThis</b>\n\n
		A pointer to the item to check.\n\n
		<b>HWND hfrom</b>\n\n
		The window handle of the source.\n\n
		<b>HWND hto</b>\n\n
		The window handle of the destination.\n\n
		<b>POINT p</b>\n\n
		The point to check.\n\n
		<b>SClass_ID type</b>\n\n
		The super class ID of <b>dropThis</b>.\n\n
		<b>BOOL isNew = FALSE</b>\n\n
		TRUE if the item is a new instance; otherwise FALSE.
		\return  TRUE if the specified item can be dropped; otherwise FALSE. */
		virtual BOOL OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew = FALSE)=0;

		/*! \remarks This method is called on a potential target to allow it
		to substitute custom cursors. It returns the handle for the custom
		cursor to use (or NULL to ignore).
		\par Parameters:
		<b>ReferenceTarget *dropThis</b>\n\n
		The pointer to the item to check.\n\n
		<b>HWND hfrom</b>\n\n
		The window handle of the source.\n\n
		<b>HWND hto</b>\n\n
		The window handle of the destination.\n\n
		<b>POINT p</b>\n\n
		The point to check.\n\n
		<b>SClass_ID type</b>\n\n
		The super class ID of <b>dropThis</b>.\n\n
		<b>BOOL isNew = FALSE</b>\n\n
		TRUE if the item is a new instance; otherwise FALSE.
		\par Default Implementation:
		<b>{ return NULL;}</b> */
		virtual HCURSOR DropCursor(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew = FALSE){ return NULL;}

		/*! \remarks Returns a predefined value to indicate the source of the
		drag.
		\return  One of the following values:\n\n
		<b>OWNER_MEDIT_SAMPLE</b>\n\n
		From a materials editor sample slot.\n\n
		<b>OWNER_NODE</b>\n\n
		From a node in the scene.\n\n
		<b>OWNER_MTL_TEX</b>\n\n
		From a button in a material or texture.\n\n
		<b>OWNER_SCENE</b>\n\n
		From a button in a light, modifier, atmospheric effect, etc.\n\n
		<b>OWNER_BROWSE_NEW</b>\n\n
		From the browser in the new category.\n\n
		<b>OWNER_BROWSE_LIB</b>\n\n
		From the browser in the library category.\n\n
		<b>OWNER_BROWSE_MEDIT</b>\n\n
		From the browser in the materials editor category.\n\n
		<b>OWNER_BROWSE_SCENE</b>\n\n
		From the browser in the scene category.
		<b>OWNER_MATERIAL_EXPLORER</b>\n\n
		From the material explorer.
		\par Default Implementation:
		<b>{ return OWNER_MTL_TEX; }</b> */
		virtual int SlotOwner() { return OWNER_MTL_TEX; } 
		 
		/*! \remarks Return a pointer to the drag source.
		\par Parameters:
		<b>HWND hwnd</b>\n\n
		The source window where the mouse down occurred.\n\n
		<b>POINT p</b>\n\n
		The point to drag from (position within <b>hwnd</b>).\n\n
		<b>SClass_ID type</b>\n\n
		The super class ID of the item to create. */
		virtual ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type)=0;

		/*! \remarks Return a pointer to the drag destination. For window which 
		won't scroll automatically during drag and drop, just use the default implementation.
		\par Parameters:
		<b>HWND hwnd</b>\n\n
		The destination window where the mouse up occurred.\n\n
		<b>POINT p</b>\n\n
		The point to drop to (position within <b>hwnd</b>).\n\n
		<b>SClass_ID type</b>\n\n
		The super class ID of the item. 
		\par Default Implementation:
		<b>{ return GetInstance(hwnd, p, type); }</b> */
		virtual ReferenceTarget *GetDestinationInstance(HWND hwnd, POINT p, SClass_ID type)
		{ return GetInstance(hwnd, p, type);}

		/*! \remarks This is the method called to actually process the drop
		operation. This routine is called on the target with the pointer
		returned by the source's <b>GetInstance()</b>, or possibly a clone of
		it as the <b>dropThis</b>.
		\par Parameters:
		<b>ReferenceTarget *dropThis</b>\n\n
		A pointer to the item to drop.\n\n
		<b>HWND hwnd</b>\n\n
		The destination window handle (where the mouse was released).\n\n
		<b>POINT p</b>\n\n
		The destination point (within <b>hwnd</b>).\n\n
		<b>SClass_ID type</b>\n\n
		The type of object being dropped -- the super class ID of <b>dropThis</b>.\n\n
		<b>DADMgr* srcMgr</b>\n\n
		The source DADMgr pointer. NULL by default.\n\n
		<b>BOOL bSrcClone</b>\n\n
		TRUE if the <b>dropThis</b> is a clone of the drag source object, 
		FALSE otherwise. FALSE by default*/
		virtual void  Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, DADMgr* srcMgr = NULL, BOOL bSrcClone = FALSE)=0;

		/*! \remarks This method is called when the source and target WINDOW
		are the same.
		\par Parameters:
		<b>HWND h1</b>\n\n
		The source/target window handle.\n\n
		<b>POINT p1</b>\n\n
		The source point.\n\n
		<b>POINT p2</b>\n\n
		The target point.
		\par Default Implementation:
		<b>{}</b> */
		virtual void  SameWinDragAndDrop(HWND h1, POINT p1, POINT p2) {}

		/*! \remarks This lets the manager know whether to call
		<b>LocalDragAndDrop()</b> if the same DADMgr is handling both the
		source and target windows, or just ignore this condition. Return TRUE
		if <b>LocalDragAndDrop()</b> should be called; otherwise FALSE.
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual BOOL  LetMeHandleLocalDAD() { return 0; }

		/*! \remarks This is called if the same <b>DADMgr</b> is handling both
		the source and target windows, if <b>LetMeHandleLocalDAD()</b> returned
		TRUE.
		\par Parameters:
		<b>HWND h1</b>\n\n
		The window handle.\n\n
		<b>HWND h2</b>\n\n
		The window handle.\n\n
		<b>POINT p1</b>\n\n
		The drag source point.\n\n
		<b>POINT p2</b>\n\n
		The drop destination point.
		\par Default Implementation:
		<b>{}</b> */
		virtual void  LocalDragAndDrop(HWND h1, HWND h2, POINT p1, POINT p2){}

		/*! \remarks If this method returns TRUE, then Custom Buttons that use
		this DAD Manager will automatically support a tooltip that matches the
		button text. Note that this method will only show a tooltip when the
		button text is too long and thus exceeds the button size.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL AutoTooltip(){ return FALSE; }

		/*! \remarks If a drag source doesn't want any references being made
		to the instance returned, then this method should return TRUE: it will
		force a copy to be made; otherwise return FALSE.
		\par Parameters:
		<b>HWND hwnd</b>\n\n
		The source window handle.\n\n
		<b>POINT p</b>\n\n
		The source point (within <b>hwnd</b>).\n\n
		<b>SClass_ID type</b>\n\n
		The type of object being dragged.\n\n

		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL CopyOnly(HWND hwnd, POINT p, SClass_ID type) { return FALSE; } 

		/*! \remarks Normally the mouse down and mouse up messages are not
		sent to the source window when doing drag and drop, but if you need
		them, return TRUE.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL AlwaysSendButtonMsgsOnDrop(){ return FALSE; }

		/*! \remarks		This is a general purpose function that allows the API to be extended in the
		future. The 3ds Max development team can assign new <b>cmd</b> numbers and
		continue to add functionality to this class without having to 'break' the
		API.\n\n
		This is reserved for future use.
		\par Parameters:
		<b>int cmd</b>\n\n
		The command to execute.\n\n
		<b>ULONG arg1=0</b>\n\n
		Optional argument 1 (defined uniquely for each <b>cmd</b>).\n\n
		<b>ULONG arg2=0</b>\n\n
		Optional argument 2.\n\n
		<b>ULONG arg3=0</b>\n\n
		Optional argument 3.
		\return  An integer return value (defined uniquely for each <b>cmd</b>).
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 

		/*! \remarks This method is called on potential target to see if can instance
		"dropThis" at the specified point. Returns TRUE if it is okay to drop
		the specified item and FALSE if not.
		\par Parameters:
		<b>ReferenceTarget *dropThis</b>\n\n
		The pointer to the item to check.\n\n
		<b>HWND hfrom</b>\n\n
		The window handle of the source.\n\n
		<b>HWND hto</b>\n\n
		The window handle of the destination.\n\n
		<b>POINT p</b>\n\n
		The point to check.\n\n
		<b>SClass_ID type</b>\n\n
		The super class ID of <b>dropThis</b>.
		\par Default Implementation:
		<b>{ return TRUE; }</b> */
		virtual BOOL OkToDropInstance(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type) { return TRUE; }
	};


/*! \sa  ~{ Custom User Interface Controls }~.\n\n
\par Description:
This is the base class from which the 3ds Max custom controls are derived. All
methods of this class are implemented by the system.  */
class ICustomControl : public InterfaceServer {
	public:
		/*! \remarks Returns the handle of the control. */
		virtual HWND GetHwnd()=0;
		/*! \remarks This method is used to enable the control so it may be
		operated by the user.
		\param onOff TRUE to enable; FALSE to disable. */
		virtual void Enable(BOOL onOff=TRUE)=0;
		/*! \remarks This method is used to disable the control so it may not
		be selected or used. When disabled, the control usually appears grayed
		out. */
		virtual void Disable()=0;
		/*! \remarks This returns TRUE if the control is enabled and FALSE if
		it is disabled. */
		virtual BOOL IsEnabled()=0;
		// this second enable function is used to disable and enable custom controls
		// when the associated parameter has a non-keyframable parameter.
		// The effective enable state is the AND of these two enable bits.
		/*! \remarks This method is used internally and should not be called by plug-in
		developers. This second enable function is used to disable and enable
		custom controls when the associated parameter has a non-keyframable
		parameter. The effective enable state is the AND of these two enable
		bits.\n\n
		For example, when a parameter has a controller plugged into it, and the
		controller is not keyframable, any spinner control associated with it
		won't be effective. That's because the controller doesn't take input --
		it only outputs values. To prevent the user from being confused by the
		ineffectiveness of the spinner the control it's automatically disabled
		by the system using this method.
		\param onOff TRUE to enable; FALSE to disable. */
		virtual void Enable2(BOOL onOff=TRUE)=0;
		// Generic expansion function
		/*! \remarks		This is a general purpose function that allows the API to be extended in the
		future. The 3ds Max development team can assign new <b>cmd</b> numbers and
		continue to add functionality to this class without having to 'break' the
		API.\n\n
		This is reserved for future use.
		\param cmd The command to execute.
		\param arg1 Optional argument 1 (defined uniquely for each <b>cmd</b>).
		\param arg2 Optional argument 2.
		\param arg3 Optional argument 3.
		\return  An integer return value (defined uniquely for each <b>cmd</b>).
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; }
		//! \brief Sets a tooltip for the custom control
		/*! This method allows for turning on or off the tooltip for a custom control.
		\param bEnable - if true, the tooltip is enabled, otherwise disabled. 
		When disabled, the tooltip won't be displayed when the the mouse hovers on top of the control.
		Enabling the tooltip repeatedly will have the same effect as enabling it once.
		\param text - pointer to a string representing the text to be displayed in the tooltip window.
		This parameter is ignored when the tooltip is being disabled.
		*/
		virtual void SetTooltip(bool bEnable, const MCHAR* text)=0;

		//! \brief Helper function to disable or enable this UI control based on whether the Animatable associated
		//! with it is keyframable or not (locked parameter or scripted parameters are not keyframable).
		//! When a UI control is associated with a parameter residing within a parameter block with the 
		//! optional p_ui tag then the control will be automatically disabled when the parameter is
		//! non-keyframable and this method does not need to be used.
		//! When the control is not using that automatic association this helper method can be used. Caller needs to
		//! identify the parameter associated to the control as the subanim number of an Animatable.
		//! \param[in] anim The parent of the Animatable this control is associated with (if NULL, this method will do nothing).
		//! \param[in] subNum The subanim number of the Animatable associated with this control.
		//! For IParamBlock2 use IParamBlock2::GetAnimNum to get subNum from ParamID (see sample code,
		//! for instance maxsdk\samples\materials\stdShaders.cpp).
		CoreExport void UpdateEnableState( Animatable * anim, int subNum );
	};

// This is a bitmap brush where the bitmap is a gray and white checker board.
CoreExport HBRUSH GetLTGrayBrush();
CoreExport HBRUSH GetDKGrayBrush();

// Makes the grid pattern brushes solid for screen shots
CoreExport void MakeBrushesSolid(BOOL onOff);


#define FONT_POINT_SIZE  0
#define FONT_PIXEL_SIZE  1
#define FONT_CELL_HEIGHT 2
/*! \defgroup FixedFontMethods Fixed font related methods
 * 3ds Max uses fixed, non-anti-aliased fonts only when this is strictly required. 
 * For example, its easier to ignore text written on top of a rendered image if 
 * the text has hard edges and it's displayed in a solid color. Thus, such text 
 * could be written with the font returned by the GetFixedFont() function.
 *
 * In general, text displayed in user interfaces should use the font returned by GetUIFont().
 * Plug-ins should in general use the \ref UIFontMethods when displying text in 
 * traditional window based user interfaces.
 */
//!@{
//! \brief Return the fixed font used by 3ds Max
/*!
 * \return HFONT object representing the font
*/
CoreExport HFONT GetFixedFont();

//! \brief Return the fixed font used by 3ds Max, based on the 3ds Max's language and character set. For internal use only.
/*!
 * \return HFONT object representing the font
*/
CoreExport HFONT GetFixedFont_LocalCharSet();

//! \brief Return the fixed bold font used by 3ds Max.
/*!
 * \return HFONT object representing the font
*/
CoreExport HFONT GetFixedFontBold();

//! \brief Return the fixed bold font used by 3ds Max, based on the 3ds Max's language and character set. For internal use only.
/*!
 * \return HFONT object representing the font
*/
CoreExport HFONT GetFixedFontBold_LocalCharSet();

//! \brief Returns the height of the fixed font used by 3ds Max.
/*! 
 * \return Font height (14 for English & European languages, 12 for Asian languages)
*/
CoreExport LONG GetFixedFontHeight();

//! \brief Returns the height of the small fixed font used by 3ds Max.
/*! 
 * \return Font height (8 for English & European languages, 9 for Asian languages)
*/
CoreExport LONG GetFixedFontHeightSmall();

//! \brief Returns the fixed font character set for the language currently used by 3ds Max.
/*! 
 * \return character set for the current language
*/
CoreExport DWORD GetFixedFontCharset();

//! \brief Returns the font face string for the fixed font used by 3ds Max.
/*! 
 * \return Font face string for the fixed font.
*/
CoreExport const MCHAR* GetFixedFontFace();
//!@}

/*! \defgroup UIFontMethods User Interface (UI) font related methods
 * The user interface of 3ds Max 2010 uses the font returned by GetUIFont().
 * The "UI font" is anti-aliased when the operating system provides support for it.
 *
 * If there's a need to draw text with non-antialised fonts, please use the \ref FixedFontMethods.
 */
//!@{
//! \brief Return the standard font used in 3ds Max's user interface.
/*!
 * \return HFONT object representing the font
*/
CoreExport HFONT GetUIFont();

//! \brief Return the standard font used in 3ds Max's user interface, depending on the language and charset. For internal use only.
/*!
 * \return HFONT object representing the font
*/
CoreExport HFONT GetUIFont_LocalCharSet();

//! \brief Return the standard bold font used in 3ds Max's user interface.
/*!
 * \return HFONT object representing the font
*/
CoreExport HFONT GetUIFontBold();

//! \brief Return the standard bold font used in 3ds Max's user interface, depending on the language and charset. For internal use only.
/*!
 * \return HFONT object representing the font
*/
CoreExport HFONT GetUIFontBold_LocalCharSet();

//! \brief Returns the height of standard font which used in 3ds Max's user interface.
/*! 
 * \return Font height (14 for English & European languages, 12 for Asian languages)
*/
CoreExport LONG GetUIFontHeight();

//! \brief Returns the height of small font which used in 3ds Max's user interface.
/*! 
 * \return Font height (8 for English & European languages, 9 for Asian languages)
*/
CoreExport LONG GetUIFontHeightSmall();

//! \brief Returns character set for the current language used in 3ds Max's user interface.
/*! 
 * \return character set for the current language
*/
CoreExport DWORD GetUIFontCharset();

//! \brief Returns the font face string for the standard font used in 3ds Max's user interface.
/*! 
 * \return Font face string for the standard font.
*/
CoreExport const MCHAR* GetUIFontFace();

//! \brief Returns the font point size 8 points for the standard font used in 3ds Max's user interface. 
/*! 
 * \return Font point size for the standard font.
*/
CoreExport float GetUIFontPointSize();

//! \brief Return the customized font with the given font name, size and if the font size is point size.
	//! \param[in] fontName The font typeface name.
	//! \param[in] fontSizeUnit Specify if the fontSize is measured by the point size(0), the pixels(1), or the cell height(2).
    //! \param[in] fontSize The font size, it could be the point size, the pixel size, or the cell height.
    //! \param[in] weight The font weight.
	//! \param[in] isItalic Specify if the font is italic.
	//! \param[in] isUnderlined Specify if the font is underlined.
/*! 
 * \return The customized font defined by the users.
*/
CoreExport HFONT CreateCustomizedFont(const MSTR& fontName, int fontSizeUnit, float fontSize, long weight = 400, bool isItalic = false, bool isUnderlined = false);

//! \brief Return the standard small font 6 points used in 3ds Max's user interface.
/*!
 * \return HFONT object representing the font
*/
CoreExport HFONT GetUIFontSmall();

//! \brief Return the standard light font used in 3ds Max's user interface, weight = FW_LIGHT(300).
/*!
 * \return HFONT object representing the font
*/
CoreExport HFONT GetUIFontLight();

//! \brief Return the standard italic font used in 3ds Max's user interface.
/*!
 * \return HFONT object representing the font
*/
CoreExport HFONT GetUIFontItalic();

//!@}
// The hand cursor used for panning.
CoreExport HCURSOR GetPanCursor();

// Used to update the new mouse-tracking outlined buttons
CoreExport void UpdateButtonOutlines();

// CUI Frame position types
/*! \defgroup cuiFramePositionTypes CUI Frame Position Types
These can be ORed together, as in CUI_HORIZ_DOCK | CUI_VERT_DOCK | CUI_FLOATABLE | CUI_SM_HANDLES. 
*/
//!@{
#define CUI_TOP_DOCK		(1<<0)	//!< Can be docked at the top.
#define CUI_BOTTOM_DOCK		(1<<1)	//!< Can be docked at the bottom.
#define CUI_LEFT_DOCK		(1<<2)	//!< Can be docked on the left.
#define CUI_RIGHT_DOCK		(1<<3)	//!< Can be docked at the right.
#define CUI_ALL_DOCK		(CUI_TOP_DOCK|CUI_BOTTOM_DOCK|CUI_LEFT_DOCK|CUI_RIGHT_DOCK)	//!< Can be docked at any of the four positions.
#define CUI_HORIZ_DOCK		(CUI_TOP_DOCK|CUI_BOTTOM_DOCK)	//!<  Can be docked at the top or bottom.
#define CUI_VERT_DOCK		(CUI_LEFT_DOCK|CUI_RIGHT_DOCK)	//!<  Can be docked at the left or right.
#define CUI_FLOATABLE		(1<<4)	//!< Can be floated.
#define CUI_FLOATING		(1<<4)	//!< Synonym for CUI_FLOATABLE.
#define CUI_CONNECTABLE		(1<<5)	//!< Not currently implemented.
#define CUI_SM_HANDLES		(1<<6)	//!< Set if frame should display size/move handles
#define CUI_SLIDING			(1<<7)	//!< Frame doesn't butt up against the one next to it.
#define CUI_MAX_SIZED		(1<<8)	//!< Frame takes up the entire row.  Nothing can be docked next to it.
#define CUI_DONT_SAVE		(1<<9)	//!< Don't save this CUI frame in the .cui file
#define CUI_HAS_MENUBAR		(1<<10)	//!< CUI frames that have a menu bar need to be treated differently
//!@}

// orientation parameters
/*! \defgroup cuiFrameOrientations CUI Frame Orientations
*/
//!@{
#define CUI_NONE			0
#define CUI_HORIZ			CUI_HORIZ_DOCK	//!< Docked horizontally.
#define CUI_VERT			CUI_VERT_DOCK	//!< Docked vertically.
#define CUI_FLOAT			CUI_FLOATING	//!< Floating.
//!@}

#define CUI_MIN_TB_WIDTH	25		// minimum width of a CUIFrame-based toolbar

#define CUI_MENU_HIDE			0
#define CUI_MENU_SHOW_ENABLED	1
#define CUI_MENU_SHOW_DISABLED	2

// CUI size parameters
/*! \defgroup cuiFrameSizeTypes CUI Frame Size Types */
//!@{
#define CUI_MIN_SIZE			0	//!< The minimum size.
#define CUI_MAX_SIZE			1	//!< The maximum size.
/*! 3ds Max does not currently take advantage of this size, only MIN and MAX are used. */
#define CUI_PREF_SIZE			2	//!< The preferred size.
//!@}

// CUI bitmap button image size (in pixels: 16x15, 16x16 or 24x24)
#define CUI_SIZE_16				16
#define CUI_SIZE_24				24

// CUI bitmap button image mask options
#define CUI_MASK_NONE			0	// no mask -- MAX should generate one
#define CUI_MASK_MONO			1	// normal Windows convention
#define CUI_MASK_ALPHA			2	// 8-bit alpha channel present
#define CUI_MASK_ALPHA_PREMULT	3	// 8-bit pre-multiplied alpha channel present

// CUI edit types -- not all implemented (yet?)
#define CUI_EDIT_NONE			0
#define CUI_EDIT_KBD			(1<<0)
#define CUI_EDIT_SCRIPT			(1<<1)
#define CUI_EDIT_MACRO			(CUI_EDIT_KBD | CUI_EDIT_SCRIPT)
#define CUI_EDIT_ORDER			(1<<2)

class ICustButton;
class ICustStatus;

#define CUI_MODE_NORMAL		0
#define CUI_MODE_EDIT		1

/*! \sa Class ICustToolbar,
Class ICustomControl,
Class ICustStatus,
Class MAXBmpFileIcon.\n\n
\par Description:
***reflect changes with MAXBMPFileIcon class***\n\n
This object controls the overall operation of the individual CUI Frames (the
name given to the windows that contain toolbars, menus, the command panel,
etc.). There is one instance of this CUIFrameMgr class (obtained by calling the
global function <b>GetCUIFrameMgr()</b>). Methods of this class are available
to do things like get pointers to button and status controls, 
and bring up the standard toolbar right click menu .\n\n
Note: Developers may use their own images on icon buttons that are managed by
this class but the following guidelines must be followed:\n\n
BMP files must be put in the <b>/UI/icons</b> folder. This is the UI directory
under the 3ds Max EXE directory. This is hard coded because it must be
retrieved before 3ds Max is fully started and thus there is no configurable
path for it. There is a command line option however, (-c), which specifies for
3ds Max to look in an alternate directory for the CUI file. In that case the
bitmap files should be located in the same directory.\n\n
For more information on the new icon image system refer to the chapter on
~{ Icons }~. */
class CUIFrameMgr : public BaseInterfaceServer {
protected:
	CUIFrameMgrPrivate* mPrivate;
	//! \brief Constructor made protected to prevent instantiation
	/*! Blocking default constructor - use GetCUIFrameMgr() accessor
	*/
	/*! \remarks Constructor. */
	CoreExport CUIFrameMgr();
	//! \brief Copy Constructor made protected to prevent instantiation
	/*! Blocking Copy constructor - use GetCUIFrameMgr() accessor
	*/
	CUIFrameMgr(CUIFrameMgr& frame);
	/*! \remarks Destructor. */
	CoreExport virtual ~CUIFrameMgr();

public:
	/*! \remarks Returns the directory name of the custom user interface (CUI)
	file location. */
	CoreExport const MCHAR *GetCUIDirectory();

	/*! \remarks	This is a very important method. It redraws all the visible CUI buttons in
	3ds Max, calling the "IsEnabled" and<br> "IsChecked" handlers on the
	ActionItems associated with each button (if it has one). If a the
	"IsEnabled" handler returns FALSE, the button is grayed out. If the
	"IsChecked" handler return TRUE, the button is draw pressed in.\n\n
	This method is called internally by the system on selection changes and
	command mode changes. This handles the majority of the cases where buttons
	need to be redrawn. However, if a 3rd party plug-in changes some sort of
	internal state that might affect the return value of an ActionItem's
	IsEnables or IsChecked handler, then the plug-in should call this method to
	update the button states. If this method isn't called, buttons may look
	disabled or pressed (or visa versa) when they shouldn't be. See
	Class ActionItem.
	\param force This parameter, if TRUE, tells the system to redraw the button even if its
	state hasn't changed since the last time it was redrawn. Normally this
	argument is FALSE so it only redraws the buttons that changed state. */
	CoreExport void SetMacroButtonStates(BOOL force);

	/*! \remarks This method is for internal use only. */
	CoreExport void SetMode(int md);
	/*! \remarks This method is for internal use only. */
	CoreExport int GetMode();

	/*! \remarks Returns the window handle for the item whose ID is passed.
	This correspond to the method in <b>ICustToolbar</b> but which should no
	longer be called for Tool Palettes. It is now also a method of this class
	because the CUI system doesn't know which toolbar a particular button is
	on. For example, a 3ds Max user in 3.0 can drag a button from one tool
	palette to another. No longer then can one use the previous
	<b>GetItemHwnd()</b> method since the button has moved to a different
	toolbar.
	\param id The ID of the control. */
	CoreExport HWND GetItemHwnd(int id);
	/*! \remarks Returns a pointer to the custom button whose ID is passed (or
	NULL if not found). In the <b>CUIFrameMgr</b> implementation of this method
	it loops through each toolbar that it has control over and calls
	<b>ICustToolbar::GetICustButton()</b> on it. That method returns NULL if it
	doesn't find the specified ID. The CUIFrameMgr keeps looping through the
	toolbars until it gets a non-NULL value. When it finds it it returns the
	<b>ICustButton</b> pointer.
	\param id The ID of the control. */
	CoreExport ICustButton *GetICustButton( int id );
	/*! \remarks Returns a pointer to the custom status control whose ID is
	passed.\n\n
	Returns a pointer to the custom status control whose ID is passed (or NULL
	if not found). In the <b>CUIFrameMgr</b> implementation of this method it
	loops through each toolbar that it has control over and calls
	<b>ICustToolbar::GetICustStatus()</b> on it. That method returns NULL if it
	doesn't find the specified ID. The <b>CUIFrameMgr</b> keeps looping through
	the toolbars until it gets a non-NULL value. When it finds it it returns
	the <b>ICustStatus</b> pointer.
	\param id The ID of the control. */
	CoreExport ICustStatus *GetICustStatus( int id );

	/*! \remarks This method is for internal use only. */
	CoreExport void HorizTextButtons(BOOL b);
	/*! \remarks This method is for internal use only. */
	CoreExport int GetHorizTextButtons();
	/*! \remarks This method is for internal use only. */
	CoreExport void FixedWidthTextButtons(BOOL b);
	/*! \remarks This method is for internal use only. */
	CoreExport int GetFixedWidthTextButtons();
	/*! \remarks This method is for internal use only. */
	CoreExport void SetTextButtonWidth(int w);
	/*! \remarks This method is for internal use only. */
	CoreExport int GetTextButtonWidth();

	/*! \remarks This method is for internal use only. */
	CoreExport int SetConfigFile(const MCHAR *cfg);
	/*! \remarks This returns the path to the CUI file in use. This may be a
	UNC name. */
	CoreExport const MCHAR *GetConfigFile();

	/*! \remarks This method is for internal use only. */
	CoreExport void SetImageSize(int size);
	/*! \remarks This method is for internal use only. */
	CoreExport int GetImageSize();
	/*! \remarks Returns the bitmap button image height for the specified
	size.
	\param sz The size to check. If 0 is passed then the current icon size is checked.
	One of the following values:\n\n
	<b>CUI_SIZE_16</b>\n
	<b>CUI_SIZE_24</b> */
	CoreExport int GetButtonHeight(int sz = 0);
	/*! \remarks Returns the bitmap button image width for the specified size.
	\param sz The size to check. One of the following values:\n\n
	<b>CUI_SIZE_16</b>\n
	<b>CUI_SIZE_24</b> */
	CoreExport int GetButtonWidth(int sz = 0);

	/*! \remarks	This method is used internally to create a MaxBmpFileIcon for a given
	object type. These methods retrieve the file name and base index in the
	file of the icon for the given object class. They are used in the
	constructor for MaxBmpFileIcon that takes a class ID and super class ID.
	This method is for internal use only. */
	CoreExport int GetDefaultImageListBaseIndex(SClass_ID sid, Class_ID cid);
	/*! \remarks	This method is used internally to create a MaxBmpFileIcon for a given
	object type. These methods retrieve the file name and base index in the
	file of the icon for the given object class. They are used in the
	constructor for MaxBmpFileIcon that takes a class ID and super class ID.
	This method is for internal use only. */
	CoreExport MSTR* GetDefaultImageListFilePrefix(SClass_ID sid, Class_ID cid);

	/*! \remarks	This method is for internal use only. It is used to add images to the icon
	manager. The icon manager, which is used to implement the
	<b>MaxBmpFileIcon</b> class, reads all the .bmp files in the UI/Icons
	directory at startup time. These icons are specified by an image file and
	an alpha mask. The icons support two sizes. Large, which is 24 by 24 and
	small, which is 15 by 16. The icon manager stores the unprocessed image and
	alpha masks (the "raw" images). Whenever an instance of MaxBmpFileIcon
	needs to draw itself, it gets the image list and index of the icon in the
	imagelist using <b>GetSmallImageIndex</b> or <b>GetLargeImageIndex.</b> */
	CoreExport int AddToRawImageList(const MCHAR* pFilePrefix, int sz, HBITMAP image, HBITMAP mask);

	/*! \remarks This method is for internal use only. */
	CoreExport int LoadBitmapFile(const MCHAR *filename);
	/*! \remarks This method is for internal use only. */
    CoreExport int FindAndLoadBitmapFiles(const MaxSDK::Util::Path &filePath);
    /*! \remarks This method is for internal use only. */
	CoreExport int LoadBitmapImages();

	/*! \remarks Plug-In developers should not call this method -- it is for
	internal use only. */
	CoreExport int ReadConfig();
	/*! \remarks Plug-In developers should not call this method -- it is for
	internal use only. */
	CoreExport int WriteConfig();

	/*! \remarks This method is for internal use only. */
	CoreExport void SetLockLayout(BOOL lock);
	/*! \remarks Returns TRUE if the layout is locker; FALSE if unlocked. */
	CoreExport BOOL GetLockLayout();

	//! \brief Given a configuration filename, will attempt to find the best match
	/*! If the application is configured to use User Profiles, this function will attempt
		to match the filename in the user profile UI directory.  If this fails, it will
		check the system directory.  
		\see IPathConfigMgr::IsUsingProfileDirectories()
		\see IPathConfigMgr::IsUsingRoamingProfiles()
		\param aFilename [in] the filename to match, with extension
		\param aResult [out] the resulting absolute path for the matched file, if found
		\return true if a match is found, false otherwise
	*/
	CoreExport virtual bool ResolveReadPath(const MSTR& aFilename, MSTR& aResult) = 0;
	//! \brief Given a configuration filename, will resolve the correct write absolute path
	/*! If the application is configured to use User Profiles, this function map this configuration
		file to a user profile directory.  Otherwise, the configuration file will be resolved to
		the legacy system UI directory.
		\see IPathConfigMgr::IsUsingProfileDirectories()
		\see IPathConfigMgr::IsUsingRoamingProfiles()
		\param aFilename [in] the filename to match, with extension
		\param aResult [out] the resulting absolute path to which a client should write a config file
		\return true if resolved correctly, false if any error is encountered
	*/
	CoreExport virtual bool ResolveWritePath(const MSTR& aFilename, MSTR& aResult) = 0;

};

/*! \remarks Returns a pointer to the CUIFrameMgr which controls the overall
operation of CUI Frames (the windows which contain toolbars, menus, the command
panel, etc). */
CoreExport CUIFrameMgr *GetCUIFrameMgr();
/*! \remarks This global function presents the Customize User Interface
dialog. */
CoreExport void DoCUICustomizeDialog();
CoreExport void ResizeFloatingTB(HWND hWnd);


#define MB_TYPE_KBD			        1
#define MB_TYPE_SCRIPT		        2
#define MB_TYPE_ACTION		        3
#define MB_TYPE_ACTION_CUSTOM		4

#define MB_FLAG_ENABLED           (1 << 0)
#define MB_FLAG_CHECKED           (1 << 1)

class ActionItem;

/*! \sa  Class ToolMacroItem,
Class MacroEntry,
Class MacroDir,
Class ICustButton,
Class ICustomControl,
Class ActionItem,
~{ Custom User Interface Controls }~.\n\n
\par Description:
A Macro Button is a button which can execute either a keyboard macro or macro
script. This class contains the data and access methods for such a UI button.
This data includes a macro type, command ID, macro script ID, label, tooltip,
image name, and image ID.\n\n
This object is used in the <b>ToolMacroItem</b> constructor. There are also
methods of class <b>ICustButton</b> to get and set the macro button data.
\par Data Members:
<b>int macroType;</b>\n\n
The macro type. One of the following values:\n\n
<b>MB_TYPE_KBD</b>\n\n
A keyboard macro.\n\n
<b>MB_TYPE_SCRIPT</b>\n\n
A Script macro.\n\n
<b>ActionTableId tblID;</b>\n\n
The Shortcut Action Table ID.\n\n
<b>void *cb;</b>\n\n
The ShortcutCallback.  This is currently not used.\n\n
<b>int cmdID;</b>\n\n
The command ID. There are method of class <b>Interface</b> that provide access
to the command IDs for various keyboard shortcut tables. See
<a href="class_interface.html#A_GM_inter_keyboard_shortcut">Keyboard Shortcut
Related Methods</a>.\n\n
<b>int macroScriptID;</b>\n\n
The macroScriptID holds the id of the macroScript associated with this button.
This id is the <b>MacroID</b> that is used by the methods in the
<b>MacroDir</b> and <b>MacroEntry</b> classes (at one time it was an indirect
reference to this id and so was typed as an int). The id can have values from 0
to the number of macro scripts currently defined in the running 3ds Max or the
special value <b>UNDEFINED_MACRO</b>.\n\n
<b>MCHAR *label;</b>\n\n
The label text for a text button. This is used if <b>imageID</b> is -1.\n\n
<b>MCHAR *tip;</b>\n\n
The tooltip text.\n\n
<b>MCHAR *imageName;</b>\n\n
This is the name for the button image. This is the 'base' name only. For
example if the actual image name was <b>Spline_16i.bmp</b> then the name
supplied here would be <b>Spline</b>. See the remarks in
Class CUIFrameMgr for details on the
image naming scheme the CUI system uses.\n\n
<b>int imageID;</b>\n\n
The image ID. If this is set to -1 it indicates to use the <b>label</b>. If it
is set to 0 or greater it indicates this is an image based button and this is
the zero based index of the button that was added. This then is an ID into an
image group as specified by <b>imageName</b>. Said another way, 3ds Max builds
one large image list internally and uses the <b>imageName</b> to get an offset
into the list and then uses this <b>imageID</b> as an additional offset from
the start as indicated by the name (each <b>imageName</b> may contain multiple
icons in the single BMP).\n\n
<b>ActionItem* actionItem;</b>\n\n
A pointer to the ActionItem.\n\n
<b>DWORD flags;</b>\n\n
These flags contain the last state when redrawing  */
class MacroButtonData: public MaxHeapOperators {
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b>label = tip = imageName = NULL; imageID = -1;</b> */
	CoreExport	MacroButtonData()	{ label = tip = imageName = NULL; imageID = -1; iconName = nullptr; }
	 /*! \remarks Constructor. This one is used for keyboard macro buttons
	 (<b>MB_TYPE_KBD</b>). The data members are initialized to the values passed as
	 shown:\n\n
	 <b>macroType=MB_TYPE_KBD; tblID=tID; this-\>cb=cb; cmdID=cID; imageID=imID;
	 label=NULL; SetLabel(lbl); tip=NULL; SetTip(tp); imageName=NULL;
	 SetImageName(imName);</b> */
	CoreExport	MacroButtonData(long tID, int cID, const MCHAR *lbl, const MCHAR *tp=NULL, int imID=-1, const MCHAR *imName=NULL);
	/*! \remarks Constructor. This one is used for macro script buttons
	(<b>MB_TYPE_SCRIPT</b>). The data members are initialized to the values
	passed as shown:\n\n
	<b>macroType=MB_TYPE_SCRIPT; macroScriptID=msID; imageID=imID; label=NULL;
	SetLabel(lbl); tip=NULL; SetTip(tp); imageName=NULL;
	SetImageName(imName);</b> */
	CoreExport	MacroButtonData(int msID, const MCHAR *lbl, const MCHAR *tp=NULL, int imID=-1, const MCHAR *imName=NULL)
		{
			iconName = nullptr;
			macroType=MB_TYPE_SCRIPT; macroScriptID=msID; imageID=imID; 
			label=NULL; SetLabel(lbl); tip=NULL; SetTip(tp); imageName=NULL; SetImageName(imName);
			
		}
	/*! \remarks Destructor. Any label, tooltip or image name strings are
	deleted. */
	CoreExport	~MacroButtonData();
	
	/*! \remarks Assignment operator. */
	CoreExport	MacroButtonData & operator=(const MacroButtonData& mbd);
	
	/*! \remarks Sets the label text.
	\param lbl The label to set. */
	CoreExport	void SetLabel(const MCHAR *lbl);
	/*! \remarks Returns the label text. */
	const MCHAR *GetLabel() const { return label; }
	/*! \remarks Sets the tooltip text.
	\param tp The text to set. */
	CoreExport	void SetTip(const MCHAR *tp);
	/*! \remarks Returns the tooltip text. */
	const MCHAR *GetTip() const { return tip; }
	/*! \remarks Sets the command ID.
	\param id The command ID to set. */
	void SetCmdID(int id)	{ cmdID = id; }
	/*! \remarks Returns the command ID. */
	int GetCmdID() const { return cmdID; }
	/*! \remarks Sets the script ID.
	\param id The script ID to set. */
	void SetScriptID(int id){ macroScriptID = id; }
	/*! \remarks Returns the script ID. */
	int GetScriptID() const { return macroScriptID; }
	/*! \remarks Sets the image name. See the <b>imageName</b> data member
	above for details on the name format.
	\param imName The name to set. */
	CoreExport	void SetImageName(const MCHAR *imName);
	/*! \remarks Returns the image name. */
	const MCHAR *GetImageName() const { return imageName; }
	/*! \remarks Sets the image ID.
	\param id The image ID to set. */
	void SetImageID(int id)	{ imageID = id; }
	/*! \remarks Returns the image ID. */
	int GetImageID() const { return imageID; }

	/*! \remarks Sets a multi-resolution icon to the button.
	\param iconName The name of the icon. 
	\see QIcon MaxSDK::LoadMaxMultiResIcon( const QString& iconName )
	*/
	CoreExport void SetIconName( const MCHAR* iconName );

	/*! \remarks Returns the name of the multi-resolution icon assigned to the button.
	\see void SetIconName( const MSTR& iconName )
	*/
	const MCHAR* GetIconName() const { return iconName; }

	
	/*! \remarks	This method sets the ActionTableID ID.
	\param id The ActionTableID ID to set. */
	void SetTblID(ActionTableId id) { tblID = id; }
	/*! \remarks	This method returns the ActionTableID ID. */
	ActionTableId GetTblID() const { return tblID; }
	
	/*! \remarks	This method allows you to set the ActionItem.
	\param pAction A point to the ActionItem to set. */
	void SetActionItem(ActionItem* pAction) { actionItem = pAction; }
	/*! \remarks	This method returns a pointer to the ActionItem. */
	ActionItem* GetActionItem() { return actionItem; }
	
	/*! \remarks	This method returns TRUE if the button is an Action button. FALSE if it is
	not. */
	CoreExport BOOL IsActionButton() const
	{
		return macroType == MB_TYPE_ACTION_CUSTOM || macroType == MB_TYPE_ACTION;
	}
	
	int			  macroType;
	ActionTableId tblID; 
	int			  cmdID;
	int			  macroScriptID;
	const MCHAR*  label;
	const MCHAR*  tip;
	
	const MCHAR*  imageName;
	int			  imageID;
	ActionItem*   actionItem;
	const MCHAR*  iconName;

	// flags constrains the last state when redrawing
	DWORD         flags;
};

//---------------------------------------------------------------------------//
// Spinner control


#define SPINNERWINDOWCLASS	_M("SpinnerControl")


//! LOWORD(wParam) = ctrlID,\n\n 
//! HIWORD(wParam) = TRUE if user is dragging the spinner interactively.\n\n
//! lParam = pointer to ISpinnerControl
#define CC_SPINNER_CHANGE  		WM_USER + 600	

//! LOWORD(wParam) = ctrlID,\n\n 
//! lParam = pointer to ISpinnerControl\n\n
#define CC_SPINNER_BUTTONDOWN	WM_USER + 601

//! LOWORD(wParam) = ctrlID,\n\n 
//! HIWORD(wParam) = FALSE if user cancelled - TRUE otherwise\n\n
//! lParam = pointer to ISpinnerControl
#define CC_SPINNER_BUTTONUP		WM_USER + 602

//! LOWORD(wParam) = ctrlID,\n\n 
//! HIWORD(wParam) = Param ID in Paramblock2\n\n
//! lParam = pointer to ParamDef - range_high & range_low of ParamDef is the limits users try to set\n
//! This Message is used when users choose "Set Max Limit"/"Set Min Limit" on right-click menu of the spinner box of Custom Attributes 
#define CC_SPINNER_SETLIMIT		WM_USER + 609

//! \deprecated Deprecated as of 3ds Max 2022. Use ExecuteMAXScriptScript function.
#pragma deprecated("CC_SCRIPT_EXECUTION")
//! \deprecated Deprecated as of 3ds Max 2022.
#pragma deprecated("CC_SPINNER_VIEWEDIT")

enum EditSpinnerType {
	EDITTYPE_INT, //!< Any integer value.
	EDITTYPE_FLOAT, //!<  Any floating point value.
	EDITTYPE_UNIVERSE, //!< This is a value in world space units. It respects the system's unit settings (for example feet and inches).
	EDITTYPE_POS_INT, //!< Any integer >= 0
	EDITTYPE_POS_FLOAT, //!< Any floating point value >= 0.0
	EDITTYPE_POS_UNIVERSE, //!< This is a positive value in world space units. It respects the system's unit settings (for example feet and inches) .
	EDITTYPE_TIME //!< This is a time value. It respects the system time settings (SMPTE for example).
	};

/*! \sa  Class ICustomControl,
~{ Custom User Interface Controls }~.\n\n
\par Description:
The spinner control is used (usually in conjunction with the custom edit
control) to provide input of values limited to a fixed type. For example, the
control may be limited to the input of only positive integers. The input
options are integer, float, universe (world space coordinates), positive
integer, positive float, positive universal, and time. This control allows the
user to increment or decrement a value by clicking on the up or down arrows.
The user may also click and drag on the arrows to interactively adjust the
value. The Ctrl key may be held to accelerate the value changing speed, while
the Alt key may be held to decrease the value changing speed.\n\n
The standard size used by 3ds Max for the spinner control is 7 wide by 10 high.
If you use a larger size, the spinner control arrows will be position in the
upper left corner of the control.\n\n
<b>Important Note: The spinner control ensures that it only displays, and the
user is only allowed to input, values within the specified ranges. However the
spinner is just a front end to a controller which actually controls the value.
The user can thus circumvent the spinner constraints by editing the controller
directly (via function curves in track view, key info, etc.). Therefore, when a
plug-in gets a value from a controller (or a parameter block, which may use a
controller) it is its responsibility to clamp the value to a valid
range.</b>\n
\n \image html "spinner.gif" "Spinner Control"
\n \image html "spinedit.gif" "Spinner and Edit Control"
To initialize the pointer to the control call:\n\n
<b>ISpinnerControl *GetISpinner(HWND hCtrl);</b>\n\n
To release the control call:\n\n
<b>ReleaseISpinner(ISpinnerControl *isc);</b>\n\n
The value to use in the Class field of the Custom Control Properties dialog is:
<b>SpinnerControl</b>\n\n
The following messages may be sent by the spinner control:\n\n
This message is sent when the value of a spinner changes.\n\n
<b>CC_SPINNER_CHANGE</b>\n\n
<b>lParam</b> contains a pointer to the spinner control. You can cast this
pointer to a <b>ISpinnerControl</b> type and then call methods of the
control.\n\n
<b>LOWORD(wParam)</b> contains the ID of the spinner. This is the named
established in the ID field of the Custom Control Properties dialog.\n\n
<b>HIWORD(wParam)</b> is TRUE if the user is dragging the spinner
interactively.\n\n
This message is sent when the user presses down on the spinner buttons.\n\n
<b>CC_SPINNER_BUTTONDOWN</b>\n\n
<b>lParam</b> contains a pointer to the spinner control. You can cast this
pointer to a <b>ISpinnerControl</b> type and then call methods of the
control.\n\n
<b>LOWORD(wParam)</b> contains the ID of the spinner. This is the named
established in the ID field of the Custom Control Properties dialog.\n\n
This message is sent when the user releases a spinner button.\n\n
<b>CC_SPINNER_BUTTONUP</b>\n\n
<b>lParam</b> contains a pointer to the spinner control. You can cast this
pointer to a ISpinnerControl type and then call methods of the control.\n\n
<b>LOWORD(wParam)</b> contains the ID of the spinner. This is the named
established in the ID field of the Custom Control Properties dialog.\n\n
<b>HIWORD(wParam)</b> is FALSE if the user canceled and TRUE otherwise.\n\n
For example, if the user is interactively dragging the spinner, then does a
right click to cancel, the following messages are sent:\n\n
<b>1</b> A <b>CC_SPINNER_BUTTONDOWN</b> message indicating the user has pressed
the spinner button.\n\n
<b>2.</b> A series of <b>CC_SPINNER_CHANGE</b> where <b>HIWORD(wParam) =
TRUE</b>. This indicates that the spinner is being dragged interactively.\n\n
<b>3.</b> A <b>CC_SPINNER_CHANGE</b> where <b>HIWORD(wParam) = FALSE</b>.\n\n
<b>4.</b> A <b>CC_SPINNER_BUTTONUP</b> message where <b>HIWORD(wParam) =
FALSE</b>. This indicates the user has cancelled.  */
class ISpinnerControl : public ICustomControl {
	public:
		/*! \remarks Returns the floating point value of the control. */
		virtual float GetFVal()=0;
		/*! \remarks This method returns the integer value of the control. */
		virtual int GetIVal()=0;
		/*! \remarks This method sets the scale for the spinner based on the
		current value of the spinner. This allows the spinner to cover a larger
		range of values with less mouse motion. If you wish to use auto scale,
		pass TRUE to this method.
		\param on If you wish to use auto scale pass TRUE to this method; otherwise FALSE. */
		virtual void SetAutoScale(BOOL on=TRUE)=0;
		/*! \remarks This method sets the value which is added to or
		subtracted from the current control value as the arrow buttons are
		pressed, or the user interactively drags the spinner.
		\param s The value is added to or subtracted from the current control value. */
		virtual void SetScale( float s )=0;
		/*! \remarks This method sets the value of the control to the specific
		floating point number passed. You may pass FALSE as the notify
		parameter so the control wont send a message when you set the value.
		\param v The new value for the control.
		\param notify If TRUE a message is sent indicating the control has changed.\n\n
		Note that sometimes the <b>SetValue()</b> method is used to update the
		display of parameters in the user interface. For example, if the user
		changes the current time and the UI parameters are animated, the user
		interface controls must be updated to reflect the value at the new
		time. The programmer calls <b>SetValue()</b> to update the value
		displayed in the control. This is an example of when to pass FALSE as
		the notify parameter. If you were to pass TRUE, a message would be sent
		as if the user had actually enter a new value at this time. These are
		of course very different conditions. */
		virtual void SetValue( float v, int notify )=0;
		/*! \remarks This method sets the value to the specific integer
		passed. You may pass FALSE as the notify parameter so the control won't
		send a message when you set the value.
		\param v The new value for the control.
		\param notify If TRUE a message is sent indicating the control has changed. */
		virtual void SetValue( int v, int notify )=0;
		/*! \remarks This method establishes the allowable limits for integer
		values entered.
		\param min The minimum allowable value.
		\param max The maximum allowable value.
		\param limitCurValue  You may pass FALSE to the this parameter so the control will not send a
		spinner changed message when the limits are set. */
		virtual void SetLimits( int min, int max, int limitCurValue = TRUE )=0;
		/*! \remarks This method establishes the allowable limits for floating
		point values entered.
		\param min The minimum allowable value.
		\param max The maximum allowable value.
		\param limitCurValue  You may pass FALSE to the this parameter so the control will not send a
		spinner changed message when the limits are set. */
		virtual void SetLimits( float min, float max, int limitCurValue = TRUE )=0;
		/*! \remarks When an edit control is used in conjunction with the
		spinner control, this method is used to link the two, so values entered
		using the spinner are displayed in the edit control. This method is
		also used to set the type of value which may be entered.
		\param hEdit The handle of the edit control to link.
		\param type The type of value that may be entered. One of the following values:\n\n
		<b>EDITTYPE_INT</b>\n
		Any integer value.\n\n
		<b>EDITTYPE_FLOAT</b>\n
		Any floating point value.\n\n
		<b>EDITTYPE_UNIVERSE</b>\n
		This is a value in world space units. It respects the system's unit
		settings (for example feet and inches).\n\n
		<b>EDITTYPE_POS_INT</b>\n
		Any integer \>= 0\n\n
		<b>EDITTYPE_POS_FLOAT</b>\n\
		Any floating point value \>= 0.0\n\n
		<b>EDITTYPE_POS_UNIVERSE</b>\n
		This is a positive value in world space units. It respects the system's
		unit settings (for example feet and inches).\n\n
		<b>EDITTYPE_TIME</b>\n
		This is a time value. It respects the system time settings (SMPTE for
		example). */
		virtual void LinkToEdit( HWND hEdit, EditSpinnerType type )=0;
		/*! \remarks This method is used to show commonality. When several
		different values are being reflected by the spinner, the value is
		indeterminate. When TRUE, the value field of the spinner appears empty.
		\param i Pass TRUE to this method to set the value to indeterminate. */
		virtual void SetIndeterminate(BOOL i=TRUE)=0;
		/*! \remarks This method returns TRUE if the current state of the
		spinner is indeterminate. See <b>SetIndeterminate()</b> above. */
		virtual BOOL IsIndeterminate()=0;
		/*! \remarks A 3ds Max user may right click on the spinner buttons to
		reset them to their 'reset' value (after they have been changed). This
		method specifies the value used when the reset occurs.
		\param v The reset value. */
		virtual void SetResetValue(float v)=0;
		/*! \remarks A 3ds Max user may right click on the spinner buttons to
		reset them to their 'reset' value (after they have been changed). This
		method specifies the value used when the reset occurs.
		\param v The reset value. */
		virtual void SetResetValue(int v)=0;
		/*! \remarks Sets the display of the brackets surrounding the spinner control to on.
		This is used to indicate if a key exists for the parameter controlled
		by the spinner at the current time. These brackets turned on and off
		automatically if you are using a parameter map and parameter block to
		handle the control. If not you'll need to use this method.
		\param onOff TRUE for on; FALSE for off.
		\par Sample Code:
		This example shows how you do this if you only use a parameter
		block.\n\n
		\code
		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam))
			{
				case IDC_LENSPINNER:
					th->SetLength(th->ip->GetTime(),th->lengthSpin->GetFVal());
					th->lengthSpin->SetKeyBrackets(th->pblock->
						KeyFrameAtTime(PB_LENGTH,th->ip->GetTime()));
					break;
			}
		
		
			return TRUE;
		\endcode
		The following functions are not part of class <b>ISpinnerControl</b>
		but are available for use with spinner controls. */
		virtual void SetKeyBrackets(BOOL onOff)=0;
	};

/*! \remarks Used to initialize and return a pointer to the spinner control.
\param hCtrl window handle of the control. */
CoreExport ISpinnerControl *GetISpinner( HWND hCtrl );
/*! \remarks Used to release the control when finished.
\param isc Points to the control to release. */
CoreExport void ReleaseISpinner( ISpinnerControl *isc );

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping ISpinnerControl pointers.

Sample Code:
\code
MaxSDK::AutoPtr<ISpinnerControl, ISpinnerControlDestructorPolicy> spin(GetISpinner(GetDlgItem(parent_rollout->page, control_ID)));
spin->SetIndeterminate(newVal);
// when spin goes out of scope, its Delete method is called, which calls ReleaseISpinner
\endcode
\sa  class ISpinnerControl, class AutoPtr
*/
class ISpinnerControlDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(ISpinnerControl *spin)
	{
		ReleaseISpinner(spin);
	}
};

/*! \remarks This activates or de-activates the global spinner snap toggle.
\par Parameters:
<b>BOOL b</b>\n\n
TRUE to activate; FALSE to de-activate. */
CoreExport void SetSnapSpinner(BOOL b);
/*! \remarks Returns the global spinner snap setting; TRUE if on; FALSE if
off. */
CoreExport BOOL GetSnapSpinner();
/*! \remarks This sets the global spinner snap increment or decrement value.
\par Parameters:
<b>float f</b>\n\n
The value that is added to or subtracted from the current spinner value when
the arrow buttons are pressed. */
CoreExport void SetSnapSpinValue(float f);
/*! \remarks Returns the global spinner snap increment or decrement value. */
CoreExport float GetSnapSpinValue();

/*! \remarks Sets the precision (number of decimal places displayed) used by
the spinner control. Note that this function also affects slider controls. See
Class ISliderControl.
\par Parameters:
<b>int p</b>\n\n
The number of decimal places to display in the edit box linked to the spinner
control. */
CoreExport void SetSpinnerPrecision(int p);
/*! \remarks Returns the number of decimal places displayed in the edit box
linked to a spinner control. Note that this function also affects slider
controls. See Class ISliderControl.\n\n
Spinner controls have a global snap setting. This is set in 3ds Max using
File/Preferences... in the General page by changing the Spinner Snap setting.
When enabled this specifies an increment that is applied to the current spinner
value each time the UP or DOWN buttons are pressed on the spinner control. */
CoreExport int GetSpinnerPrecision();

#define SPINNER_WRAP_DISTANCE 40
CoreExport void SetSpinnerWrap(int w);
CoreExport int GetSpinnerWrap();


//---------------------------------------------------------------------------
// Slider control

#define SLIDERWINDOWCLASS	_M("SliderControl")

//! LOWORD(wParam) = ctrlID,\n\n 
//! HIWORD(wParam) = TRUE if user is dragging the slider interactively.\n\n
//! lParam = pointer to ISliderControl
#define CC_SLIDER_CHANGE  		WM_USER + 611

//! LOWORD(wParam) = ctrlID,\n\n
//! lParam = pointer to ISliderControl
#define CC_SLIDER_BUTTONDOWN	WM_USER + 612

//! LOWORD(wParam) = ctrlID,\n\n
//! HIWORD(wParam) = FALSE if user cancelled - TRUE otherwise\n\n
//! lParam = pointer to ISliderControl
#define CC_SLIDER_BUTTONUP		WM_USER + 613

/*! \sa  Class ICustomControl,
Class ISpinnerControl.\n\n
\par Description:
<b>Important Note: The slider control ensures that it only displays, and the
user is only allowed to input, values within the specified ranges. However the
slider is just a front end to a controller which actually controls the value.
The user can thus circumvent the slider constraints by editing the controller
directly (via function curves in track view, key info, etc.). Therefore, when a
plug-in gets a value from a controller (or a parameter block, which may use a
controller) it is its responsibility to clamp the value to a valid
range.</b>\n
\n \image html "slider1.gif" "Slider Control"
\n \image html "slider2.gif" "'Bracketed' Slider Control"
The custom slider control is functionality similar to the custom spinner
control. It supports the following features:\n\n
- can link to custom edit box. \n\n
- right click reset of value. \n\n
if not dragging, resets to default reset value. \n\n
if dragging, resets to previous value. \n\n
- shift+right click sets an animation key. \n\n
- red highlight for animated key positions. \n\n
It also supports the following functionality:\n\n
- dynamically set tick marks segment the slider track. \n\n
- default reset value and last value are visually indicated. \n\n
- left click in slider track moves button to that position. \n\n
- ctrl key snaps to nearest tick mark. \n

Also Note: Developers should use the functions <b>Get/SetSpinnerPrecision()</b>
for controlling precision of edit boxes linked to slider controls. Those
functions affect both spinners and sliders.\n\n
To initialize the pointer to the control call: \n
<code>ISliderControl *GetISlider(HWND hCtrl);</code> \n
To release the control call: \n
<code>void ReleaseISlider(ISliderControl *isc);</code> \n
The value to use in the Class field of the Custom Control Properties dialog is: SliderControl \n\n
The following messages may be sent by the slider control: \n
This message is sent when the value of a slider changes. \n
<b>CC_SLIDER_CHANGE</b> \n
lParam contains a pointer to the slider control. You can cast this pointer 
to a ISliderControl type and then call methods of the control. \n
LOWORD(wParam) contains the ID of the slider. This is the ID established in 
the ID field of the Custom Control Properties dialog. \n
HIWORD(wParam) is TRUE if the user is dragging the slider interactively. \n
This message is sent when the user presses down on the slider. \n
<b>CC_SLIDER_BUTTONDOWN</b> \n
lParam contains a pointer to the slider control. You can cast this pointer 
to a ISliderControl type and then call methods of the control. \n
LOWORD(wParam) contains the ID of the slider. This is the ID established 
in the ID field of the Custom Control Properties dialog. \n
This message is sent when the user releases a slider. \n
<b>CC_SLIDER_BUTTONUP</b> \n
lParam contains a pointer to the slider control. You can cast this pointer 
to a ISliderControl type and then call methods of the control. \n
LOWORD(wParam) contains the ID of the slider. This is the ID established 
in the ID field of the Custom Control Properties dialog. \n
HIWORD(wParam) is FALSE if the user canceled and TRUE otherwise.
*/
class ISliderControl : public ICustomControl
{
public:
	/*! \remarks Returns the floating point value of the control. */
	virtual float GetFVal()=0;
	/*! \remarks Returns the integer value of the control. */
	virtual int GetIVal()=0;
	/*! \remarks Sets the number of segments (tick marks) used by the control.
	\par Parameters:
	<b>int num</b>\n\n
	The number to set. */
	virtual void SetNumSegs( int num )=0;
	/*! \remarks This method sets the value of the control to the specific
	floating point number passed. You may pass FALSE as the notify parameter so
	the control wont send a message when you set the value.
	\par Parameters:
	<b>float v</b>\n\n
	The new value for the control.\n\n
	<b>int notify</b>\n\n
	If TRUE a message is sent indicating the control has changed; if FALSE no
	message is sent. */
	virtual void SetValue( float v, int notify )=0;
	/*! \remarks This method sets the value of the control to the specific
	integer number passed. You may pass FALSE as the notify parameter so the
	control wont send a message when you set the value.
	\par Parameters:
	<b>int v</b>\n\n
	The new value for the control.\n\n
	<b>int notify</b>\n\n
	If TRUE a message is sent indicating the control has changed; if FALSE no
	message is sent. */
	virtual void SetValue( int v, int notify )=0;
	/*! \remarks This method establishes the allowable limits for integer
	values entered.
	\par Parameters:
	<b>int min</b>\n\n
	The minimum allowable value.\n\n
	<b>int max</b>\n\n
	The maximum allowable value.\n\n
	<b>int limitCurValue = TRUE</b>\n\n
	You may pass FALSE to the this parameter so the control will not send a
	spinner changed message when the limits are set. */
	virtual void SetLimits( int min, int max, int limitCurValue = TRUE )=0;
	/*! \remarks This method establishes the allowable limits for floating
	point values entered.
	\par Parameters:
	<b>float min</b>\n\n
	The minimum allowable value.\n\n
	<b>float max</b>\n\n
	The maximum allowable value.\n\n
	<b>int limitCurValue = TRUE</b>\n\n
	You may pass FALSE to the this parameter so the control will not send a
	spinner changed message when the limits are set. */
	virtual void SetLimits( float min, float max, int limitCurValue = TRUE )=0;
	/*! \remarks When an edit control is used in conjunction with the slider
	control, this method is used to link the two, so values entered using the
	slider are displayed in the edit control. This method is also used to set
	the type of value which may be entered.
	\par Parameters:
	<b>HWND hEdit</b>\n\n
	The handle of the edit control to link.\n\n
	<b>EditSpinnerType type</b>\n\n
	The type of value that may be entered. One of the following values:\n\n
	<b>EDITTYPE_INT</b>\n\n
	Any integer value.\n\n
	<b>EDITTYPE_FLOAT</b>\n\n
	Any floating point value.\n\n
	<b>EDITTYPE_UNIVERSE</b>\n\n
	This is a value in world space units. It respects the system's unit
	settings (for example feet and inches).\n\n
	<b>EDITTYPE_POS_INT</b>\n\n
	Any integer \>= 0\n\n
	<b>EDITTYPE_POS_FLOAT</b>\n\n
	Any floating point value \>= 0.0\n\n
	<b>EDITTYPE_POS_UNIVERSE</b>\n\n
	This is a positive value in world space units. It respects the system's
	unit settings (for example feet and inches) .\n\n
	<b>EDITTYPE_TIME</b>\n\n
	This is a time value. It respects the system time settings (SMPTE for
	example). */
	virtual void LinkToEdit( HWND hEdit, EditSpinnerType type )=0;
	/*! \remarks This method is used to show commonality. When several
	different values are being reflected by the slider, the value is
	indeterminate. When TRUE, the value field of the slider appears empty.
	\par Parameters:
	<b>BOOL i=TRUE</b>\n\n
	Pass TRUE to this method to set the value to indeterminate. */
	virtual void SetIndeterminate(BOOL i=TRUE)=0;
	/*! \remarks This method returns TRUE if the current state of the slider
	is indeterminate; otherwise FALSE. See <b>SetIndeterminate()</b> above. */
	virtual BOOL IsIndeterminate()=0;
	/*! \remarks A user may right click on a slider to reset it to its 'reset'
	value (after it has been changed). This method specifies the value used
	when the reset occurs.
	\par Parameters:
	<b>float v</b>\n\n
	The reset value. */
	virtual void SetResetValue(float v)=0;
	/*! \remarks A user may right click on a slider to reset it to its 'reset'
	value (after it has been changed). This method specifies the value used
	when the reset occurs.
	\par Parameters:
	<b>int v</b>\n\n
	The reset value. */
	virtual void SetResetValue(int v)=0;
	/*! \remarks Sets the display of the 'brackets' surrounding the slider
	control. This is used to indicate if a key exists for the parameter
	controlled by the slider at the current time. These brackets turned on and
	off automatically if you are using a parameter map and parameter block to
	handle the control. If not you'll need to use this method. For a slider,
	the 'brackets' appear as a colored dot in the position marker.
	\par Parameters:
	<b>BOOL onOff</b>\n\n
	TRUE for on; FALSE for off. */
	virtual void SetKeyBrackets(BOOL onOff)=0;
};

/*! \remarks Used to initialize and return a pointer to the slider control.
\param hCtrl window handle of the control. */
CoreExport ISliderControl *GetISlider( HWND hCtrl );
/*! \remarks Used to release the control when finished.
\param isc Points to the control to release. */
CoreExport void ReleaseISlider( ISliderControl *isc );

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping ISliderControl pointers.

Sample Code:
\code
MaxSDK::AutoPtr<ISliderControl, ISliderControlDestructorPolicy> slider(GetISlider(GetDlgItem(parent_rollout->page, control_ID)));
slider->SetKeyBrackets(newVal);
// when slider goes out of scope, its Delete method is called, which calls ReleaseISlider
\endcode
\sa  class ISliderControl, class AutoPtr
*/
class ISliderControlDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(ISliderControl *slider)
	{
		ReleaseISlider(slider);
	}
};

// routines for setting up sliders.
/*! \remarks This global function is used for setting up integer sliders. It
performs the equivalent of the <b>GetISlider()</b>, <b>SetLimits()</b>,
<b>SetValue()</b>, and <b>LinkToEdit()</b>.
\par Parameters:
<b>HWND hwnd</b>\n\n
The handle of the dialog box in which the slider appears.\n\n
<b>int idSlider</b>\n\n
The ID of the slider.\n\n
<b>int idEdit</b>\n\n
The ID of the edit control.\n\n
<b>int min</b>\n\n
The minimum allowable value.\n\n
<b>int max</b>\n\n
The maximum allowable value.\n\n
<b>int val</b>\n\n
The initial value for the spinner.\n\n
<b>int numSegs</b>\n\n
The number of segments to use for the control.
\return  A pointer to the slider control. */
CoreExport ISliderControl *SetupIntSlider(HWND hwnd, int idSlider, int idEdit,  int min, int max, int val, int numSegs);
/*! \remarks This global function is used for setting up floating point
sliders. It performs the equivalent of the <b>GetISlider()</b>,
<b>SetLimits()</b>, <b>SetValue()</b>, and <b>LinkToEdit()</b>.
\par Parameters:
<b>HWND hwnd</b>\n\n
The handle of the dialog box in which the slider appears.\n\n
<b>int idSlider</b>\n\n
The ID of the slider.\n\n
<b>int idEdit</b>\n\n
The ID of the edit control.\n\n
<b>float min</b>\n\n
The minimum allowable value.\n\n
<b>float max</b>\n\n
The maximum allowable value.\n\n
<b>float val</b>\n\n
The initial value for the spinner.\n\n
<b>int numSegs</b>\n\n
The number of segments to use for the control.
\return  A pointer to the slider control. */
CoreExport ISliderControl *SetupFloatSlider(HWND hwnd, int idSlider, int idEdit,  float min, float max, float val, int numSegs);
/*! \remarks This global function is used for setting up 'universal' value
sliders (<b>EDITTYPE_UNIVERSE</b> -- these display world space units). It
performs the equivalent of the <b>GetISlider()</b>, <b>SetLimits()</b>,
<b>SetValue()</b>, and <b>LinkToEdit()</b>.
\par Parameters:
<b>HWND hwnd</b>\n\n
The handle of the dialog box in which the slider appears.\n\n
<b>int idSlider</b>\n\n
The ID of the slider.\n\n
<b>int idEdit</b>\n\n
The ID of the edit control.\n\n
<b>float min</b>\n\n
The minimum allowable value.\n\n
<b>float max</b>\n\n
The maximum allowable value.\n\n
<b>float val</b>\n\n
The initial value for the spinner.\n\n
<b>int numSegs</b>\n\n
The number of segments to use for the control.
\return  A pointer to the slider control. */
CoreExport ISliderControl *SetupUniverseSlider(HWND hwnd, int idSlider, int idEdit,  float min, float max, float val, int numSegs);

// controls whether or not sliders send notifications while the user adjusts them with the mouse
/*! \remarks This function controls whether or not sliders send
<b>CC_SLIDER_CHANGE</b> notifications while the user adjusts them with the
mouse.
\par Parameters:
<b>BOOL onOff</b>\n\n
TRUE to turn on; FALSE to turn off. */
CoreExport void SetSliderDragNotify(BOOL onOff);
/*! \remarks Returns TRUE if <b>CC_SLIDER_CHANGE</b> notifications are sent by
sliders while the user adjusts them with the mouse; FALSE if they are not sent.
*/
CoreExport BOOL GetSliderDragNotify();


//---------------------------------------------------------------------------//
// Rollup window control

#define WM_CUSTROLLUP_RECALCLAYOUT WM_USER+876

#define ROLLUPWINDOWCLASS _M("RollupWindow")

typedef void *RollupState;

// Flags passed to AppendRollup
#define APPENDROLL_CLOSED			(1<<0)	// Starts the page out rolled up.
#define DONTAUTOCLOSE    			(1<<1)	// Don't close this rollup when doing Close All
#define ROLLUP_SAVECAT    			(1<<2)	// Save the category field in the RollupOrder.cfg
#define ROLLUP_USEREPLACEDCAT		(1<<3)	// In case of ReplaceRollup, use the replaced rollups category
#define ROLLUP_NOBORDER				(1<<4)	// Don't display a title or border, don't support drag-and-drop, right-click menu, or collapse/expand.
#define ROLLUP_MINIMAL_PADDING		(1<<5)	// In Qt-based rollups, use minimal padding. Primarily for MaxScript, which performs its own padding.
#define ROLLUP_DONT_ADD_TO_CP		(1<<6)	// Use for Qt rollups that reside somewhere other than the Command Panel (does not add them to Command Panel)

class IRollupWindow;
class IRollupPanel;

/*! class IRollupCallback : public InterfaceServer
\par Description:
This class represents the abstract interface for a rollup window callback
object to assist developers in handling custom drag and drop of rollouts. <br>
*/
class IRollupCallback : public InterfaceServer
{
public:
	/*! \remarks Any plugin (or core component), that wants to implement a
	custom behavior when a rollup page is drag and dropped onto another
	rollout, it can do so, by registering a <b>IRollupCallback</b> and
	overwriting this method and <b>GetEditObjClassID().</b> After rearranging
	rollup pages the HandleDrop code should call:
	<b>GetIRollupSettings()-\>GetCatReg()-\>Save()</b>; in order to save the
	rollout order.
	\par Parameters:
	<b>IRollupPanel *src</b>\n\n
	A pointer to the source rollup panel.\n\n
	<b>IRollupPanel *targ</b>\n\n
	A pointer to the target rollup panel.\n\n
	<b>bool before</b>\n\n
	TRUE to insert before the panel it was dropped on; FALSE to insert after.
	\return  TRUE to indicate to the system, that it took over the drop
	handling, otherwise FALSE.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL HandleDrop(IRollupPanel *src,IRollupPanel *targ, bool before){ return FALSE; }
	/*! \remarks This method has to be implemented in order to support drag
	and drop for rollouts, no matter if custom drop handling is used, or not.
	The order of rollup panels is stored in the RollupOrder.cfg file in the UI
	directory. The order is stored under a SuperClassID and ClassID. The
	RollupCallback has to specify what the superclassid and classid is. E.g.
	for the Modify Panel it is the classid for the currently edited object. For
	the DisplayPanel it is only one classid, that has no real class assigned to
	it, since the rollouts are independent from the object being edited
	(DISPLAY_PANEL_ROLLUPCFG_CLASSID).
	\par Parameters:
	<b>SClass_ID \&sid</b>\n\n
	The super class ID of the object.\n\n
	<b>Class_ID \&cid</b>\n\n
	The class ID of the object.
	\return  TRUE if drag and drop is supported, otherwise FALSE.
	\par Default Implementation:
	<b>{ return FALSE;}</b> */
	virtual BOOL GetEditObjClassID(SClass_ID &sid,Class_ID &cid){ return FALSE;}
	/*! \remarks This method is called when the user selected the "open all"
	function and it currently used internally. The method will return TRUE if
	successful and FALSE otherwise.
	\par Default Implementation:
	<b>{return FALSE;}</b> */
	virtual BOOL HandleOpenAll(){return FALSE;}
	/*! \remarks This method is called when the user selected the "close all"
	function and it currently used internally. The method will return TRUE if
	successful and FALSE otherwise.
	\par Default Implementation:
	<b>{ return FALSE;}</b> */
	virtual BOOL HandleCloseAll(){ return FALSE;}
	/*! \remarks This method is called as the RollupWindow is being destroyed.
	\par Default Implementation:
	<b>{}</b> */
	virtual void HandleDestroy() {}
};

/*! class IRollupPanel : public InterfaceServer
\par Description:
This class represents the interface for a rollup panel and describes the
properties of that panel (which is one rollup). You can obtain a pointer to the
IRollupPanel class for any given specified window by calling
<b>IRollupWindow::IRollupPanel *GetPanel(HWND hWnd)</b>;  */
class IRollupPanel : public InterfaceServer
{
public:
	/*! \remarks This method returns a handle to the rollup panel instance. */
	virtual HINSTANCE GetHInst()=0;
	/*! \remarks This method returns the resource ID of the rollup panel. */
	virtual DWORD_PTR GetResID()=0;
	/*! \remarks Equality test operator. */
	virtual BOOL operator==(const IRollupPanel& id)=0;
	/*! \remarks This method returns the rollup panel category identifier. */
	virtual int GetCategory()=0;
	/*! \remarks This method allows you to set the category identifier for the
	rollup panel.
	\par Parameters:
	<b>int cat</b>\n\n
	The category identifier to set. */
	virtual void SetCategory(int cat)=0;
	/*! \remarks This method returns a handle to the rollup window. */
	virtual HWND GetHWnd()=0;
	/*! \remarks This method returns a handle to the actual panel in the
	rollup window. */
	virtual HWND GetRollupWindowHWND()=0;
	/*! \remarks This method returns a handle to the window from which you can
	get the title through the <b>GWLP_USERDATA</b>. */
	virtual HWND GetTitleWnd()=0;
	//! This function gets the main panel window handle.
	//! \return Returns the panel window handle
	virtual HWND GetPanelWnd()=0;

	//! This functions sets the height of this panel.
	//! \param[in] height  The height of the panel dlg.
	virtual void SetDlgHeight(int height)=0;

	//! This function gets the height of the panel.
	//! \return Returns the panel height.
	virtual int GetDlgHeight() const = 0;
};

/*! class IRollupRCMenuItem : public InterfaceServer
\par Description:
This class represents a right click menu item for rollups.  */
class IRollupRCMenuItem : public InterfaceServer {
public:
	/*! \remarks This method returns the text of the menu item. */
	virtual const MCHAR*		RRCMMenuText()=0;
	/*! \remarks This method is the callback that will be triggered when the
	user selects the menu item. */
	virtual void		RRCMExecute()=0;
	/*! \remarks This method allows you to control the checkmark for the menu
	item, in case it needs be shown or hidden (by returning TRUE to show or
	FALSE to hide). */
	virtual bool		RRCMShowChecked()=0;
	/*! \remarks This method should return TRUE if you wish a separator item
	before the menu item. */
	virtual bool		RRCMHasSeparator()=0;
};

/*! \sa  Class ICustomControl,
Class IRollupPanel,
~{ Custom User Interface Controls }~,
Class Interface.\n\n
\par Description:
This control is used to access existing rollup pages or if you are creating a
dialog box which will not be used in the command panel. This control may be
used to add a container area for rollup pages to be added to the dialog, and
provides a scroll bar just like the command panel itself.\n\n
Note that this is a special case. Normally, adding rollup pages to the command
panel is done using the simple <b>AddRollupPage()</b> method of the Interface
class. This control is only used when you want to have a scrolling region for
rollup pages in a dialog box.\n\n
To initialize the pointer to the control call:\n\n
<b>IRollupWindow *GetIRollup(HWND hCtrl);</b>\n\n
To release the control call:\n\n
<b>void ReleaseIRollup(IRollupWindow *irw);</b>\n\n
The value to use in the Class field of the Custom Control Properties dialog is:
<b>RollupWindow</b>  */
class IRollupWindow : public ICustomControl {
	public:
		// Shows or hides all
		/*! \remarks This causes all the rollup windows to be visible. */
		virtual void Show()=0;
		/*! \remarks This causes all the rollup windows to become invisible.
		*/
		virtual void Hide()=0;

		// Shows or hides by index
		/*! \remarks This will make the rollup window whose index is passed
		visible.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the rollup to show. */
		virtual void Show(int index)=0;
		/*! \remarks This will make the rollup window whose index is passed
		invisible.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the rollup to hide. */
		virtual void Hide(int index)=0;

		/*! \remarks Returns the handle of the rollup page whose index is
		passed.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the rollup whose handle is to be returned. */
		virtual HWND GetPanelDlg(int index)=0;
		/*! \remarks Returns an index to the rollup page given its handle.
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The handle of the rollup. */
		virtual int GetPanelIndex(HWND hWnd)=0;
        //! Returns the index of a rollup page that was created from a Qt widget.
        //! \sa int AppendRollup(QWidget& qtWidget, const MCHAR *title, DWORD rollupFlags, int category)
        virtual int GetPanelIndex(QWidget& qtWidget) = 0;
		/*! \remarks This method sets the title text displayed in the rollup
		page whose index is passed.
		\par Parameters:
		<b>int index</b>\n\n
		Specifies the rollup whose title is to be set.\n\n
		<b>MCHAR *title</b>\n\n
		The title string. */
		virtual void SetPanelTitle(int index,const MCHAR *title)=0;

		/** \brief Gets the title text displayed in the rollup page whose 
		 * index is passed.
		 * \param[in] index Specifies the rollup whose title is to be returned. 
		 * \return The title of the specified rollup.
		 * \see SetPanelTitle */
		virtual MSTR GetPanelTitle( int index ) const = 0;

		// returns index of new panel
		/*! \remarks This method is used to add a rollup page.
		\par Parameters:
		<b>HINSTANCE hInst</b>\n\n
		The DLL instance handle of the plug-in.\n\n
		<b>MCHAR *dlgTemplate</b>\n\n
		The dialog template for the rollup page.\n\n
		<b>DLGPROC dlgProc</b>\n\n
		The dialog proc to handle the message sent to the rollup page.\n\n
		<b>MCHAR *title</b>\n\n
		The title displayed in the title bar.\n\n
		<b>LPARAM param=0</b>\n\n
		Any specific data to pass along may be stored here.\n\n
		<b>DWORD flags=0</b>\n\n
		Append rollup page flags:\n\n
		<b>APPENDROLL_CLOSED</b>\n\n
		Starts the page in the rolled up state.\n\n
		<b>int category = ROLLUP_CAT_STANDARD</b>\n\n
		The category parameter provides flexibility with regard to where a
		particular rollup should be displayed in the UI. RollupPanels with
		lower category fields will be displayed before RollupPanels with higher
		category fields. For RollupPanels with equal category value the one
		that was added first will be displayed first. Although it is possible
		to pass any int value as category there exist currently 5 different
		category defines: <b>ROLLUP_CAT_SYSTEM</b>, <b>ROLLUP_CAT_STANDARD</b>,
		and <b>ROLLUP_CAT_CUSTATTRIB</b>.\n\n
		When using <b>ROLLUP_SAVECAT</b>, the rollup page will make the
		provided category sticky, meaning it will not read the category from
		the <b>RollupOrder.cfg</b> file, but rather save the category field
		that was passed as argument in the <b>CatRegistry</b> and in the
		<b>RollupOrder.cfg</b> file.\n\n
		The method will take the category of the replaced rollup in case the
		flags argument contains <b>ROLLUP_USEREPLACEDCAT</b>. This is mainly
		done, so that this system works with param maps as well.
		\return  The index of the new page is returned. */
		virtual int AppendRollup( HINSTANCE hInst, const MCHAR *dlgTemplate, 
				DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD flags=0, int category = ROLLUP_CAT_STANDARD )=0;
		/*! \remarks This method is used to add a rollup page, but is currently not used.
		\par Parameters:
		<b>HINSTANCE hInst</b>\n\n
		The DLL instance handle of the plug-in.\n\n
		<b>DLGTEMPLATE *dlgTemplate</b>\n\n
		The dialog template for the rollup page.\n\n
		<b>DLGPROC dlgProc</b>\n\n
		The dialog proc to handle the message sent to the rollup page.\n\n
		<b>MCHAR *title</b>\n\n
		The title displayed in the title bar.\n\n
		<b>LPARAM param=0</b>\n\n
		Any specific data to pass along may be stored here.\n\n
		<b>DWORD flags=0</b>\n\n
		Append rollup page flags:\n\n
		<b>APPENDROLL_CLOSED</b>\n\n
		Starts the page in the rolled up state.\n\n
		<b>int category = ROLLUP_CAT_STANDARD</b>\n\n
		The category parameter provides flexibility with regard to where a
		particular rollup should be displayed in the UI. RollupPanels with
		lower category fields will be displayed before RollupPanels with higher
		category fields. For RollupPanels with equal category value the one
		that was added first will be displayed first. Although it is possible
		to pass any int value as category there exist currently 5 different
		category defines: <b>ROLLUP_CAT_SYSTEM</b>, <b>ROLLUP_CAT_STANDARD</b>,
		and <b>ROLLUP_CAT_CUSTATTRIB</b>.\n\n
		When using <b>ROLLUP_SAVECAT</b>, the rollup page will make the
		provided category sticky, meaning it will not read the category from
		the <b>RollupOrder.cfg</b> file, but rather save the category field
		that was passed as argument in the <b>CatRegistry</b> and in the
		<b>RollupOrder.cfg</b> file.\n\n
		The method will take the category of the replaced rollup in case the
		flags argument contains <b>ROLLUP_USEREPLACEDCAT</b>. This is mainly
		done, so that this system works with param maps as well.
		\return  The index of the new page is returned. */
		virtual int AppendRollup( HINSTANCE hInst, DLGTEMPLATE *dlgTemplate, 
				DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD flags=0, int category = ROLLUP_CAT_STANDARD )=0;

        /*! Adds a rollup page that hosts a Qt dialog.
            \param qtWidget the Qt widget that will be hosted in the rollup. <b>This QWidget becomes the ownership of the system</b>; it will be deleted
                by the system once no longer needed.
            \param title The title string, displayed at the top of the rollup.
            \param rollupFlags Optional rollup flags, such as APPENDROLL_CLOSED, DONTAUTOCLOSE, ROLLUP_SAVECAT, etc.
            \param category The category controls the ordering of the rollups, with lower values being inserted before rollups of higher category. 
            \return The index of the new rollup. */
        virtual int AppendRollup(QWidget& qtWidget, const MCHAR *title, DWORD rollupFlags, int category) = 0;

		/*! \remarks This method is used to replace the rollup page whose index is
		passed.
		\par Parameters:
		<b>int index</b>\n\n
		Specifies the rollup whose to be replaced.\n\n
		<b>HINSTANCE hInst</b>\n\n
		The DLL instance handle of the plug-in.\n\n
		<b>MCHAR *dlgTemplate</b>\n\n
		The dialog template for the rollup page.\n\n
		<b>DLGPROC dlgProc</b>\n\n
		The dialog proc to handle the message sent to the rollup page.\n\n
		<b>MCHAR *title</b>\n\n
		The title displayed in the title bar.\n\n
		<b>LPARAM param=0</b>\n\n
		Any specific data to pass along may be stored here.\n\n
		<b>DWORD flags=0</b>\n\n
		Append rollup page flags:\n\n
		<b>APPENDROLL_CLOSED</b>\n\n
		Starts the page in the rolled up state.
		\return  The index of the replacement page is returned. */
		virtual int ReplaceRollup( int index, HINSTANCE hInst, const MCHAR *dlgTemplate, 
				DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD flags=0, int category = ROLLUP_CAT_STANDARD)=0;
		/*! \remarks This method is used to replace the rollup page whose index is passed,
		but is currently not used.
		\par Parameters:
		<b>int index</b>\n\n
		Specifies the rollup whose to be replaced.\n\n
		<b>HINSTANCE hInst</b>\n\n
		The DLL instance handle of the plug-in.\n\n
		<b>DLGTEMPLATE *dlgTemplate</b>\n\n
		The dialog template for the rollup page.\n\n
		<b>DLGPROC dlgProc</b>\n\n
		The dialog proc to handle the message sent to the rollup page.\n\n
		<b>MCHAR *title</b>\n\n
		The title displayed in the title bar.\n\n
		<b>LPARAM param=0</b>\n\n
		Any specific data to pass along may be stored here.\n\n
		<b>DWORD flags=0</b>\n\n
		Append rollup page flags:\n\n
		<b>APPENDROLL_CLOSED</b>\n\n
		Starts the page in the rolled up state.\n\n
		<b>int category = ROLLUP_CAT_STANDARD</b>\n\n
		The category parameter provides flexibility with regard to where a
		particular rollup should be displayed in the UI. RollupPanels with
		lower category fields will be displayed before RollupPanels with higher
		category fields. For RollupPanels with equal category value the one
		that was added first will be displayed first. Although it is possible
		to pass any int value as category there exist currently 5 different
		category defines: <b>ROLLUP_CAT_SYSTEM</b>, <b>ROLLUP_CAT_STANDARD</b>,
		and <b>ROLLUP_CAT_CUSTATTRIB</b>.\n\n
		When using <b>ROLLUP_SAVECAT</b>, the rollup page will make the
		provided category sticky, meaning it will not read the category from
		the <b>RollupOrder.cfg</b> file, but rather save the category field
		that was passed as argument in the <b>CatRegistry</b> and in the
		<b>RollupOrder.cfg</b> file.\n\n
		The method will take the category of the replaced rollup in case the
		flags argument contains <b>ROLLUP_USEREPLACEDCAT</b>. This is mainly
		done, so that this system works with param maps as well.
		\return  The index of the replacement page is returned. */
		virtual int ReplaceRollup( int index, HINSTANCE hInst, DLGTEMPLATE *dlgTemplate, 
				DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD flags=0, int category = ROLLUP_CAT_STANDARD)=0;
		/*! \remarks This method deletes the rollup pages starting at the
		index passed. The count parameter controls how many pages are deleted.
		\par Parameters:
		<b>int index</b>\n\n
		The starting index.\n\n
		<b>int count</b>\n\n
		The number of pages. */
		virtual void DeleteRollup( int index, int count )=0;
		/*! \remarks This method is used to change the height of a rollup
		page.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the rollup to change.\n\n
		<b>int height</b>\n\n
		The new height of the dialog in pixels. */
		virtual void SetPageDlgHeight(int index,int height)=0;

		/** \brief Get the height of a rollup dialog.
		 * \param[in] index The index of the rollup to change. 
		 * \return The height of the dialog inside the rollup page
		 * \see SetPageDlgHeight */
		virtual int GetPageDlgHeight( int index ) const = 0;

		/*! \remarks This method saves the state of the rollup (the position
		of the scroll bars, which pages are open, etc...).
		\par Parameters:
		<b>RollupState *hState</b>\n\n
		Pointer to storage for the rollup state. Note: <b>typedef void
		*RollupState;</b> */
		virtual void SaveState( RollupState *hState )=0;
		/*! \remarks This methods restores a saved state.
		\par Parameters:
		<b>RollupState *hState</b>\n\n
		Pointer to storage for the rollup state. Note: <b>typedef void
		*RollupState;</b> */
		virtual void RestoreState( RollupState *hState )=0;

		// Passing WM_LBUTTONDOWN, WM_MOUSEMOVE, and WM_LBUTTONUP to
		// this function allows scrolling with unused areas in the dialog.
		/*! \remarks Passing <b>WM_LBUTTONDOWN</b>, <b>WM_MOUSEMOVE</b>, and
		<b>WM_LBUTTONUP</b> to this function allows hand cursor scrolling with
		unused areas in the dialog.
		\par Parameters:
		<b>HWND hDlg</b>\n\n
		The handle of the dialog.\n\n
		<b>UINT message</b>\n\n
		The message to pass along: <b>WM_LBUTTONDOWN</b>, <b>WM_MOUSEMOVE</b>,
		or <b>WM_LBUTTONUP</b>.\n\n
		<b>WPARAM wParam</b>\n\n
		<b>LPARAM lParam</b>\n\n
		These are passed as part of the message sent in. Pass them along to
		this method. */
		virtual void DlgMouseMessage( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )=0;

		/*! \remarks This method returns the number of panels used in the
		rollup. */
		virtual int GetNumPanels()=0;
		/*! \remarks This method return TRUE if the rollup page whose index is
		passed is open and FALSE if it is closed. */
		virtual BOOL IsPanelOpen(int index) = 0;
		/*! \remarks This causes the page whose index is passed to either open
		or close. If <b>isOpen</b> is passed a value of TRUE, the page is
		opened.
		\par Parameters:
		<b>int index</b>\n\n
		The page to open or close.\n\n
		<b>BOOL isOpen</b>\n\n
		If TRUE, the page is opened, if FALSE it is closed.\n\n
		<b>BOOL ignoreFlags = TRUE</b>\n\n
		The method would close the panel if the <b>DONTAUTOCLOSE</b> flag is
		not set on the rollup. This flag indicates if it should be closed
		anyway, even if the flag is set. */
		virtual void SetPanelOpen(int index, BOOL isOpen, BOOL ignoreFlags = TRUE) =0;
		/*! \remarks This method returns the scroll position of the window. */
		virtual int GetScrollPos()=0;
		/*! \remarks This method sets the scroll position of the window.
		\par Parameters:
		<b>int spos</b>\n\n
		The scroll position to set. */
		virtual void SetScrollPos(int spos)=0;

		// This methods moves a RollupPanel to another RollupWindow. It either inserts it
		// at the top, or appends it at the end (depending on the top parameter)

		/*! \remarks This methods moves a RollupPanel to another RollupWindow. It either
		inserts it at the top, or appends it at the end (depending on the top
		parameter)
		\par Parameters:
		<b>IRollupWindow *from</b>\n\n
		A pointer to the rollup window you are moving from.\n\n
		<b>HWND hPanel</b>\n\n
		The handle to the destination panel.\n\n
		<b>BOOL top</b>\n\n
		TRUE to insert at the top; FALSE to append at the end. */
		virtual void MoveRollupPanelFrom(IRollupWindow *from, HWND hPanel, BOOL top)=0;
		
		// Returns the Height of a RollupPanel
		/*! \remarks Returns the height of the specified RollupPanel.
		\par Parameters:
		<b>int index</b>\n\n
		The zero based index of the rollup panel.\n\n
		  */
		virtual int GetPanelHeight(int index)=0;
		
		// Returns the Height of a RollupWindow, that it is longer than the visible area
		/*! \remarks Returns the height of a RollupWindow, that it is longer than the
		visible area */
		virtual int GetScrollHeight()=0;
		
		// Used internally
		/*! \remarks This method is used internally */
		virtual void UpdateLayout()=0;
		
		/*! \remarks Returns a pointer to the rollup panel for the specified window handle.
		An IRollupPanel describes the properties of a single rollup.
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The window handle to get the rollup for. */
		virtual IRollupPanel *GetPanel(HWND hWnd)=0;

		/*! \remarks This method allows you to register a rollup callback function to handle
		any custom handling for dragging and dropping rollouts.
		\par Parameters:
		<b>IRollupCallback *callb</b>\n\n
		A pointer to the callback function you wish to register. */
		virtual void RegisterRollupCallback( IRollupCallback *callb)=0;
		/*! \remarks This method allows you to unregister a rollup callback function.
		\par Parameters:
		<b>IRollupCallback *callb</b>\n\n
		A pointer to the callback function you wish to unregister. */
		virtual void UnRegisterRollupCallback( IRollupCallback *callb)=0;

		/*! \remarks This method allows you to register a rollup right-click menu item which
		will be added to the list of items. For rollups that support Drag and
		Drop this is used to register a ResetCategories RightClickMenu. Reset
		Categories will get rid of all the changes that have been made through
		drag and drop and restore the default.
		\par Parameters:
		<b>IRollupRCMenuItem *item</b>\n\n
		A pointer to the right-click menu item you wish to register. */
		virtual void RegisterRCMenuItem( IRollupRCMenuItem *item)=0;
		/*! \remarks This method allows you to unregister a rollup right-click menu item.
		\par Parameters:
		<b>IRollupRCMenuItem *item</b>\n\n
		A pointer to the right-click menu item you wish to unregister. */
		virtual void UnRegisterRCMenuItem( IRollupRCMenuItem *item)=0;

		/*! \remarks This method will reset the category information on all the panels in
		the rollup window. The plugin will have to be reloaded (EndEditParams,
		BeginEditparams) in order to show this in the UI.
		\par Parameters:
		<b>bool update = true</b>\n\n
		TRUE to update the layout, otherwise FALSE. Leave this on TRUE. */
		virtual void ResetCategories(bool update = true)=0;

		//! \brief Sets a rollup window to borderless, or bordered
		/*! A borderless rollup window has no outer line, making it appear inset.
			By default, rollup windows have a border (are not borderless).
			\param[in] borderless Pass TRUE for borderless, FALSE for bordered */
		virtual void SetBorderless( BOOL borderless )=0;
		//! \brief Returns TRUE if the rollup window is borderless, FALSE otherwise
		/*! A borderless rollup window has no outer line, making it appear inset.
			By default, rollup windows have a border (are not borderless). */
		virtual BOOL GetBorderless()=0;
	};

// This function returns TRUE if a particular rollup panel is open given
// a handle to the dialog window in the panel.
/*! \remarks This function returns TRUE if a particular rollup panel is open
given a handle to the dialog window in the panel.
\par Parameters:
<b>HWND hDlg</b>\n\n
Handle to the dialog window in the panel. */
CoreExport BOOL IsRollupPanelOpen(HWND hDlg);

/*! \remarks Used to initialize and return a pointer to the rollup window control.
\param hCtrl window handle of the control. */
CoreExport IRollupWindow *GetIRollup( HWND hCtrl );
/*! \remarks Used to release the control when finished.
\param irw Points to the control to release. */
CoreExport void ReleaseIRollup( IRollupWindow *irw );

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping IRollupWindow pointers.

Sample Code:
\code
MaxSDK::AutoPtr<IRollupWindow, IRollupWindowDestructorPolicy> rw(GetIRollup(GetParent(GetParent(page))));
IRollupPanel *panel = rw->GetPanel(page);
MoveWindow( panel->GetTitleWnd(), 6, 0, rect.right - 10, 16, TRUE );  // resize panel title
// when rw goes out of scope, its Delete method is called, which calls ReleaseIRollup
\endcode
\sa  class IRollupWindow, class AutoPtr
*/
class IRollupWindowDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(IRollupWindow *rw)
	{
		ReleaseIRollup(rw);
	}
};

//----------------------------------------------------------------------------//
// CustEdit control

#define CUSTEDITWINDOWCLASS _M("CustEdit")

// Sent when the user hits the enter key in an edit control.
// wParam = cust edit ID
// lParam = HWND of cust edit control.
#define WM_CUSTEDIT_ENTER	(WM_USER+685)

/*! \sa  Class ICustomControl, ~{ Custom User %Interface Controls }~.
\par Description:
This control is a simple text input control. The user may type any string into
the field and the plug-in is notified when the user presses the ENTER key.
There are also methods to parse and return integer and floating point values
entered in the control.

\image html "cc1.gif"

To initialize the pointer to the control call:  \n
\code 
ICustEdit *GetICustEdit(HWND hCtrl);
\endcode

To release the control call: 
\code
ReleaseICustEdit(ICustEdit *ice);
\endcode

The value to use in the Class field of the Custom Control Properties dialog is: CustEdit 
The following messages may be sent by the edit control:
`WM_CUSTEDIT_ENTER` - This message is sent when the control loses focus or when the user presses the
ENTER key while using the control.  Note that if the control is associated with a spinner or slider, 
no notification occurs on losing focus if the edit field value equals the spinner/slider value.  

* `wParam` contains the custom edit control resource ID. 
* `lParam` contains the HWND of custom edit control. 

*/
class ICustEdit : public ICustomControl {
	public:
		/*! \remarks This retrieves the text entered into the control.
		\param text Storage for the text to retrieve.
		\param ct Specifies the maximum length of the string returned. */
		virtual void GetText( MCHAR *text, int ct )=0;
		/*! \remarks This retrieves the text entered into the control.
		\param text Storage for the text to retrieve. */
		virtual void GetText ( MSTR& text) const =0;
		/*! \remarks This retrieves the length of the text entered into the control. 
		It  returns  the  length  of the  text in characters (so without
		the  terminating  NULL). Note that this  value  may  be  higher  than  the 
		actual length of the text when it contains multi-byte characters.*/
		virtual int GetTextLength() const =0;
		/*! \remarks This method places the text into the control for editing.
		\param text The text to place in the control. */
		virtual void SetText( const MCHAR *text )=0;	
		/*! \remarks This method allows you to pass in an integer value to the
		control. The integer is converted to a string and displayed in the
		control.
		\param i This value is converted to a string and displayed in the control. */
		virtual void SetText( int i )=0;
		/*! \remarks This method allows you to pass in a floating point value
		to the control. The float is converted to a string and displayed in the
		control.
		\param f This value is converted to a string and displayed in the control.
		\param precision The precision argument is simply the number of decimal places that get
		represented in the string that appears in the edit field. So if the
		arguments were (1.0f/3.0f, 3) then the string "0.333" would appear in
		the edit field. */
		virtual void SetText( float f, int precision=3 )=0;
		/*! \remarks This method parses and returns an integer value from the
		control.
		\param valid This pointer, if passed, is set to TRUE if the input is 'valid';
		otherwise FALSE. FALSE indicates that something caused the parsing of
		the input to terminate improperly. An example is a non-numeric
		character. So for example, if the user entered "123jkfksdf" into the
		field the valid pointer would be set to FALSE. */
		virtual int GetInt(BOOL *valid=NULL)=0;
		/*! \remarks This method parses and returns a floating point value
		from the control.
		\param valid This pointer, if passed, is set to TRUE if the input is 'valid';
		otherwise FALSE. FALSE indicates that something caused the parsing of
		the input to terminate improperly. An example is a non-numeric
		character. So for example, if the user entered "123jkfksdf" into the
		field this pointer would be set to FALSE. */
		virtual float GetFloat(BOOL *valid=NULL)=0;
		/*! \remarks A developer doesn't normally need to call this method.
		This offsets the text vertically in the edit control.
		\param lead This parameter specifies the number of pixels to offset. */
		virtual void SetLeading(int lead)=0;
		/*! \remarks This method allows custom handling of the RETURN key. If you pass TRUE
		to this method an <b>EN_CHANGE</b> message will be sent to the control
		when the RETURN key is pressed. The <b>EN_CHANGE</b> message is sent
		when the user has taken any action that may have altered text in an
		edit control so developer need to also call <b>GotReturn()</b>
		(documented below) to see if it was indeed a RETURN key press.
		\param yesNo If TRUE, then when the user presses the RETURN key in that control, the
		edit field will send an <b>EN_CHANGE</b> message to the owner, and
		calling <b>GotReturn()</b> will return TRUE.
		\par Sample Code:
		Below is the way this is handled by the Hit By Name dialog. In that
		dialog, when the user enters a wild card pattern into the name match
		field and presses RETURN, the dialog is exited with the items matching
		the pattern selected. The way this is accomplished is by pass TRUE to
		<b>WantReturn()</b> and then processing the <b>EN_CHANGE</b> message on
		the control. If <b>GotReturn()</b> is TRUE the Win32 function
		<b>PostMessage()</b> is used to send the <b>IDOK</b> message to exit
		the dialog. If this wasn't done, pressing RETURN in the edit control
		would only enter the text -- the user would have to move the mouse over
		the OK button and press it.\n\n
		\code
		case IDC_HBN_PATTERN:
			if (HIWORD(wParam)==EN_CHANGE)
			{
				iName = GetICustEdit(GetDlgItem(hDlg,IDC_HBN_PATTERN) );
				iName->GetText(buf,256);
				ct = _tcslen(buf);
				if (ct && ct < _countof(buf) - 1 && buf[ct - 1] != _T('*'))
					_tcscat_s(buf, _T("*"));
				SendMessage(sbn->hList, LB_RESETCONTENT, 0, 0);
				sbn->SetPattern(GetDlgItem(hDlg, IDC_HBN_PATTERN), buf);
				sbn->BuildHitList(ct);
				if(iName->GotReturn())
					PostMessage(hDlg,WM_COMMAND,IDOK,0);
				ReleaseICustEdit(iName);
			}
			break;
		\endcode     */
		virtual void WantReturn(BOOL yesNo)=0;
		/*! \remarks This method should be called on receipt of an <b>EN_CHANGE</b> message.
		It return TRUE if pressing the RETURN key generated the message;
		otherwise FALSE. */
		virtual BOOL GotReturn()=0;		// call this on receipt of EN_CHANGE
		/*! \remarks Calling this method gives the control the focus to receive input. */
		virtual void GiveFocus()=0;
		/*! \remarks Returns TRUE if the control has the focus to receive input; otherwise
		FALSE. */
		virtual BOOL HasFocus()=0;
		/*! \remarks Determines whether the TAB key may be used to jump to the next control
		in the tab sequence.
		\param yesNo TRUE to enable the TAB key to move to the next control; FALSE to
		disable the TAB key from moving the focus. */
		virtual void WantDlgNextCtl(BOOL yesNo)=0;
		/*! \remarks Normally when a user exits an edit filed the notification
		<b>WM_CUSTEDIT_ENTER</b> is sent. Many plug-ins key off this message to
		finalize the input of values. For instance, if the user is entering a
		value into an edit field and they hit the TAB key to leave the field
		the value should be entered. Normally this is the desired behavior.
		However, as a special case condition, if a developer does not want to
		update the value, this method may be called so the
		<b>WM_CUSTEDIT_ENTER</b> notification won't be sent when the edit
		control loses focus.
		\param onOff TRUE to turn on; FALSE to turn off. */
		virtual void SetNotifyOnKillFocus(BOOL onOff)=0;
		/*! \remarks Sets the text font in the edit control to display in a bold format or
		normal.
		\param onOff TRUE to turn bolding on; FALSE to turn off. */
		virtual void SetBold(BOOL onOff)=0;
		virtual void SetParamBlock(ReferenceTarget* pb, int subNum)=0;
	};

/*! \remarks Used to initialize and return a pointer to the custom edit control.
\param hCtrl window handle of the control. */
CoreExport ICustEdit *GetICustEdit( HWND hCtrl );
/*! \remarks Used to release the control when finished.
\param ice Points to the control to release. */
CoreExport void ReleaseICustEdit( ICustEdit *ice );

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping ICustEdit pointers.

Sample Code:
\code
MaxSDK::AutoPtr<ICustEdit, ICustEditDestructorPolicy> edit(GetICustEdit(edit_box));
edit->SetParamBlock(pblock, pblock->GetAnimNum(pid->id, pid->tabIndex));
// when edit goes out of scope, its Delete method is called, which calls ReleaseICustEdit
\endcode
\sa  class ICustEdit, class AutoPtr
*/
class ICustEditDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(ICustEdit *edit)
	{
		ReleaseICustEdit(edit);
	}
};

#define CUSTSTATUSEDITWINDOWCLASS _M("CustStatusEdit")

/*! \sa  Class ICustomControl.\n\n
\par Description:
This control mimics the edit control as well as a status control. It may be set
to 'read-only' so the user can read but cannot edit the displayed string.\n\n
The value to use in the Class field of the Custom Control Properties dialog is:
<b>CustStatusEdit</b> */
class ICustStatusEdit : public ICustomControl {
	public:
		/*! \remarks Retrieves the text entered into the control.
		\par Parameters:
		<b>MCHAR *text</b>\n\n
		Storage for the text to retrieve.\n\n
		<b>int ct</b>\n\n
		Specifies the maximum length of the string returned. */
		virtual void GetText( MCHAR *text, int ct )=0;
		/*! \remarks This retrieves the text entered into the control.
		\par Parameters:
		<b>MSTR& text</b>\n\n
			Storage for the text to retrieve.*/
		virtual void GetText ( MSTR& text) const =0;
		/*! \remarks This retrieves the length of the text entered into the control. 
		It  returns  the  length  of the  text in characters (so without
		the  terminating  NULL). Note that this  value  may  be  higher  than  the 
		actual length of the text when it contains multi-byte characters.*/
		virtual int GetTextLength() const =0;
		/*! \remarks This method places the text into the control for editing.
		\par Parameters:
		<b>MCHAR *text</b>\n\n
		The text to place in the control. */
		virtual void SetText( const MCHAR *text )=0;	
		/*! \remarks This method allows you to pass in an integer value to the
		control. The integer is converted to a string and displayed in the
		control.
		\par Parameters:
		<b>int i</b>\n\n
		This value is converted to a string and displayed in the control. */
		virtual void SetText( int i )=0;
		/*! \remarks This method allows you to pass in a floating point value
		to the control. The float is converted to a string and displayed in the
		control.
		\par Parameters:
		<b>float f</b>\n\n
		This value is converted to a string and displayed in the control.\n\n
		<b>int precision=3</b>\n\n
		The precision argument is simply the number of decimal places that get
		represented in the string that appears in the edit field. So if the
		arguments were (1.0f/3.0f, 3) then the string "0.333" would appear in
		the edit field. */
		virtual void SetText( float f, int precision=3 )=0;
		/*! \remarks This method parses and returns an integer value from the
		control.
		\par Parameters:
		<b>BOOL *valid=NULL</b>\n\n
		This pointer, if passed, is set to TRUE if the input is 'valid';
		otherwise FALSE. FALSE indicates that something caused the parsing of
		the input to terminate improperly. An example is a non-numeric
		character. So for example, if the user entered "123jkfksdf" into the
		field the valid pointer would be set to FALSE. */
		virtual int GetInt(BOOL *valid=NULL)=0;
		/*! \remarks This method parses and returns a floating point value
		from the control.
		\par Parameters:
		<b>BOOL *valid=NULL</b>\n\n
		This pointer, if passed, is set to TRUE if the input is 'valid';
		otherwise FALSE. FALSE indicates that something caused the parsing of
		the input to terminate improperly. An example is a non-numeric
		character. So for example, if the user entered "123jkfksdf" into the
		field this pointer would be set to FALSE. */
		virtual float GetFloat(BOOL *valid=NULL)=0;
		/*! \remarks A developer doesn't normally need to call this method.
		This offsets the text vertically in the edit control.
		\par Parameters:
		<b>int lead</b>\n\n
		This parameter specifies the number of pixels to offset. */
		virtual void SetLeading(int lead)=0;
		/*! \remarks This method allows custom handling of the RETURN key. If
		you pass TRUE to this method an EN_CHANGE message will be sent to the
		control when the RETURN key is pressed. The EN_CHANGE message is sent
		when the user has taken any action that may have altered text in an
		edit control so developer need to also call GotReturn() (documented
		below) to see if it was indeed a RETURN key press.
		\par Parameters:
		<b>BOOL yesNo</b>\n\n
		If TRUE, then when the user presses the RETURN key in that control, the
		edit field will send an EN_CHANGE message to the owner, and calling
		GotReturn() will return TRUE. */
		virtual void WantReturn(BOOL yesNo)=0;
		/*! \remarks This method should be called on receipt of an EN_CHANGE
		message. It return TRUE if pressing the RETURN key generated the
		message; otherwise FALSE. */
		virtual BOOL GotReturn()=0;		// call this on receipt of EN_CHANGE
		/*! \remarks Calling this method gives the control the focus to
		receive input. */
		virtual void GiveFocus()=0;
		/*! \remarks Returns TRUE if the control has the focus to receive
		input; otherwise FALSE. */
		virtual BOOL HasFocus()=0;
		/*! \remarks Determines whether the TAB key may be used to jump to the
		next control in the tab sequence.
		\par Parameters:
		<b>BOOL yesNo</b>\n\n
		TRUE to enable the TAB key to move to the next control; FALSE to
		disable the TAB key from moving the focus. */
		virtual void WantDlgNextCtl(BOOL yesNo)=0;
		/*! \remarks Normally when a user exits an edit filed the notification
		WM_CUSTEDIT_ENTER is sent. Many plug-ins key off this message to
		finalize the input of values. For instance, if the user is entering a
		value into an edit field and they hit the TAB key to leave the field
		the value should be entered. Normally this is the desired behavior.
		However, as a special case condition, if a developer does not want to
		update the value, this method may be called so the WM_CUSTEDIT_ENTER
		notification won't be sent when the edit control loses focus.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE to turn on; FALSE to turn off. */
		virtual void SetNotifyOnKillFocus(BOOL onOff)=0;
		/*! \remarks Sets the text font in the edit control to display in a
		bold format or normal.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE to turn bolding on; FALSE to turn off. */
		virtual void SetBold(BOOL onOff)=0;
		/*! \remarks Sets if the control is 'read-only'. That is, the string
		is displayed but cannot be edited.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for read-only; FALSE to allow editing. */
		virtual void SetReadOnly(BOOL onOff)=0;
	};

/*! \remarks Used to initialize and return a pointer to the custom status edit control.
\param hCtrl window handle of the control. */
CoreExport ICustStatusEdit *GetICustStatusEdit( HWND hCtrl );
/*! \remarks Used to release the control when finished.
\param ice Points to the control to release. */
CoreExport void ReleaseICustStatusEdit( ICustStatusEdit *ice );

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping ICustStatusEdit pointers.

Sample Code:
\code
MaxSDK::AutoPtr<ICustStatusEdit, ICustStatusEditDestructorPolicy> statusEdit(GetICustStatusEdit(hwnd));
statusEdit->SetReadOnly(newVal);
// when statusEdit goes out of scope, its Delete method is called, which calls ReleaseICustStatusEdit
\endcode
\sa  class ICustStatusEdit, class AutoPtr
*/
class ICustStatusEditDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(ICustStatusEdit *statusEdit)
	{
		ReleaseICustStatusEdit(statusEdit);
	}
};

//----------------------------------------------------------------------------//
// CustButton control

#define CUSTBUTTONWINDOWCLASS _M("CustButton")

#define CC_COMMAND  		WM_USER + 700
// send these with CC_COMMAND: wParam = CC_???
#define CC_CMD_SET_TYPE  		23		// lParam = CBT_PUSH, CBT_CHECK
#define CC_CMD_SET_STATE		24		// lParam = 0/1 for popped/pushed
#define CC_CMD_HILITE_COLOR		25		// lParam = RGB packed int

#define RED_WASH	RGB(255,192,192)
#define GREEN_WASH	(ColorMan()->GetColor(kActiveCommand))
#define BLUE_WASH	(ColorMan()->GetColor(kPressedHierarchyButton))
#define SUBOBJ_COLOR (ColorMan()->GetColor(kSubObjectColor))
#define NO_COLOR	(0xff000000)

enum CustButType { CBT_PUSH, CBT_CHECK };

// If the button is set to notify on button down, it will send a WM_COMMAND
// with this notify code when the user touches the button.
#define BN_BUTTONDOWN 	8173
// It will also send this message when the mouse is released regardless
// if the mouse is released inside the tool button rectangle
#define BN_BUTTONUP		8174

// If a button is set to notify on right clicks,  it will send a WM_COMMAND
// with this notify code when the user right clicks on the button.
#define BN_RIGHTCLICK 	8183

// When the user chooses a new fly-off item, this notify code will be sent.
#define BN_FLYOFF		8187


// When the user presses a button a WM_MENUSELECT message is sent so that
// the client can display a status prompt describing the function of
// the tool. The fuFlags parameter is set to this value:
#define CMF_TOOLBUTTON	9274

class MaxBmpFileIcon;

/*! \sa  ~{ Custom User Interface Controls }~, 
Class ICustButton.\n\n
\par Description:
This class uses four indices into the image list to describe the button in each
of the possible states: Out\&Enabled, In\&Enabled, Out\&Disabled and
In\&Disabled. An array of instances of this class are passed into the method
<b>ICustButton::SetFlyOff()</b>.
\par Data Members:
These four data members are indices into the image list. They indicate which
images to use for each of the four possible button states:\n\n
You may specify a unique image for each one of these states by passing a
different index for each state. Or you may supply a single image to be used for
all the states by specifying the same index four times.\n\n
<b>int iOutEn;</b>\n\n
Out\&Enabled.\n\n
<b>int iInEn;</b>\n\n
In\&Enabled\n\n
<b>int iOutDis;</b>\n\n
Out\&Disabled.\n\n
<b>int iInDis;</b>\n\n
In\&Disabled. */
class FlyOffData
{
public:
	int iOutEn;
	int iInEn;
	int iOutDis;
	int iInDis;
	MaxBmpFileIcon* mpIcon;
	MaxBmpFileIcon* mpInIcon;
};

// Directions the fly off will go.
#define FLY_VARIABLE	1
#define FLY_UP			2
#define FLY_DOWN		3
#define FLY_HVARIABLE	4 // horizontal variable
#define FLY_LEFT		5
#define FLY_RIGHT		6

// For defining an icon name based fly off control.
typedef MaxSDK::Array<MSTR> FlyOffIconList;


typedef LRESULT CALLBACK PaintProc(HDC hdc, Rect rect, BOOL in, BOOL checked, BOOL enabled);

/*! \sa  Class ICustomControl,
~{ Custom User Interface Controls }~,
Class ICustToolbar,
Class FlyOffData,
Class DADMgr,
Class MAXBmpFileIcon.\n\n
\par Description:
Custom buttons may be one of two different forms. A Check button (which stays
pressed in until the user clicks on it again), or a Pick button (which pops
back out as soon as it is released). Buttons may be implemented as a Fly offs.
A fly off offers several alternative buttons which fly out from the button
after it is press and held briefly.\n
\n \image html "butchk.gif"
\n \image html "butpush.gif"
\n \image html "butfly.gif"
The buttons may contain text or graphic images. Fly off buttons only use
graphic images. The plug-in developer has control over the appearance of the
button in each of its four states (Enabled\&Out, Enabled\&In, Disabled\&Out,
Disabled\&In).\n\n
Note: When the user presses a button a <b>WM_MENUSELECT</b> message is sent so
that the client can display a status prompt describing the function of the
tool. The <b>fuFlags</b> parameter is set to this value:
<b>CMF_TOOLBUTTON</b>.\n\n
In 3dsmax version 4.0 you can remove borders from an ICustButton;\n\n
<b>ICustButton *cb = ();</b>\n\n
<b>cb-\>Execute(I_EXE_CB_NO_BORDER);</b>\n\n
To initialize the pointer to the control call: \n
<code>ICustButton *GetICustButton(HWND hCtrl);</code> \n
To release the control call: \n
<code>ReleaseICustButton(ICustButton *ics);</code> \n
The value to use in the Class field of the Custom Control Properties dialog is: CustButton
*/
class ICustButton : public ICustomControl {
	public:
		/*! \remarks This retrieves the text displayed by the button.
		\param text Storage for the text to retrieve.
		\param ct Specifies the maximum length of the string returned. */
		virtual void GetText( MCHAR *text, int ct )=0;
		/*! \remarks This specifies the text displayed by the button.
		\param text The text to be displayed by the button. */
		virtual void SetText( const MCHAR *text )=0;

		/*! \remarks This retrieves the text entered into the control.
		\param text Storage for the text to retrieve. */
		virtual void GetText ( MSTR& text) const =0;
		/*! \remarks This retrieves the length of the text entered into the control. 
		It  returns  the  length  of the  text in characters (so without
		the  terminating  NULL). Note that this  value  may  be  higher  than  the 
		actual length of the text when it contains multi-byte characters.*/
		virtual int GetTextLength() const =0;

		/*! \remarks This method is used to establish the images used for the
		buttons.
		\param hImage The image list. An image list is a collection of same-sized images,
		each of which can be referred to by an index. Image lists are used to
		efficiently manage large sets of icons or bitmaps in Windows. All
		images in an image list are contained in a single, wide bitmap in
		screen device format. An image list may also include a monochrome
		bitmap that contains masks used to draw images transparently (icon
		style). The Windows API provides image list functions, which enable you
		to draw images, create and destroy image lists, add and remove images,
		replace images, and merge images.\n\n
		The next four parameters (<b>iOutEn, iInEn, iOutDis, iInDis</b>) are
		indices into the image list. They indicate which images to use for each
		of the four possible button states. You may specify a unique image for
		each one of these states by passing a different index for each state.
		Or you may supply a single image to be used for all the states by
		specifying the same index four times.
		\param iOutEn Out&Enabled.
		\param iInEn In&Enabled.
		\param iOutDis Out&Disabled.
		\param iInDis In&Disabled.
		\param w The width of the button image.
		\param h The height of the button image. */
		virtual void SetImage( HIMAGELIST hImage, 
							   int iOutEn, int iInEn, int iOutDis, int iInDis,
							   int w, int h )=0;
		// Alternate way to set an image on a button.   
		/*! \remarks This sets the icon image used for a button.
		\param pIcon Points to the icon.
		\param w The width of the button image.
		\param h The height of the button image. */
		virtual void SetIcon ( MaxBmpFileIcon* pIcon, int w, int h) = 0;
		/*! \remarks This sets the icon image used when a button is pressed.
		\param pInIcon Points to the icon.
		\param w The width of the button image.
		\param h The height of the button image. */
		virtual void SetInIcon ( MaxBmpFileIcon* pInIcon, int w, int h) = 0;

		/*! \remarks Sets a multi-resolution icon to the button.
		\param iconName The name of the icon.
		\param iconWidth The default width of the button icon at 100% dpi scale.
		An iconWidth of 0 will use the default toolbar button image size as default.
		This value will internally get scaled according to the current dpi setting.
		\param iconHeight The default height of the button icon at 100% dpi scale.
		An iconHeight of 0 will use the default toolbar button image size as default.
		This value will internally get scaled according to the current dpi setting.
		\see QIcon MaxSDK::LoadMaxMultiResIcon( const QString& iconName )
		*/
		virtual void SetIconByName( const MSTR& iconName, int iconWidth = 0, int iconHeight = 0 ) = 0;

		/*! \remarks Returns the name of the multi-resolution icon assigned to the button.
		\see void SetIconByName( const MSTR& iconName )
		*/
		virtual MSTR GetIconName() const = 0;


		/*! \remarks This method sets the button type.
		\param type One of the following values:\n\n
		<b>CBT_PUSH</b>\n
		A Push button pops back out as soon as it is released.\n\n
		<b>CBT_CHECK</b>.\n
		A Check button stays pressed in until the user clicks on it again. */
		virtual void SetType( CustButType type )=0;
		
		/*! \remarks This method sets the button to work as a fly off control.
		\param count The number of buttons in the fly off.
		\param data An array of instances of the class <b>FlyOffData</b> . This class uses four
		indices into the image list to describe the button in each of the possible
		states: Out&Enabled, In&Enabled, Out&Disabled and In&Disabled.\n\n
		In the simple case, where all the buttons have the same image, you can do the
		following:
		\code	
		 FlyOffData fod[3] = { // A three button flyoff
		  { 0,0,0,0 }, // The first button uses a single image.
		  { 1,1,1,1 }, // So does the second button...
		  { 2,2,2,2 }, // So does the third...
		  };
		\endcode 
		Each button will use the same images regardless of its pressed in / disabled
		state. Note the button is automatically drawn pushed in (i.e. shaded lighter)
		when the user is dragging the cursor over the button, but the actual image on
		the button is not changed.\n\n
		If you require different images for these states, supply different indices into
		the image list for each. See the sample program
		<b>/MAXSDK/SAMPLES/HOWTO/CUSTCTRL/CUSTCTRL.CPP</b> for an example of how this
		is done.
		\param timeOut This is the time in milliseconds the button must be held pressed before the fly
		off appears. You may specify 0 if you want the buttons to fly off immediately.
		To retrieve the value that 3ds Max uses internally for its flyoffs use a method
		of Class Interface called <b>GetFlyOffTime()</b>. This returns a value in milliseconds.
		\param init This is the initial button displayed.
		\param dir This parameter is optional. It is used to indicate which direction the buttons
		should fly off. The choices for direction are:\n\n
		<b>FLY_VARIABLE</b>\n
		The default. The system will determine the direction of the fly off.\n\n
		<b>FLY_UP</b>\n
		The buttons fly off above.\n\n
		<b>FLY_DOWN</b>\n
		The buttons fly off beneath.\n\n
		<b>FLY_HVARIABLE</b>\n
		The buttons will fly off either left or right with the system determining the
		direction.\n\n
		<b>FLY_LEFT</b>\n
		The buttons fly off to the left.\n\n
		<b>FLY_RIGHT</b>\n
		The buttons fly off to the right. 
		\param columns */
		virtual void SetFlyOff(int count,FlyOffData *data,int timeOut,
							   int init,int dir=FLY_VARIABLE, int columns=1)=0;


		/*! \remarks This method sets the button to work as a fly off control.
		\param flyOffIcons A <b>FlyOffIconList</b> of icon names. The icon names are used to create
		multi-resolution icons for the buttons in the fly off control. \n\n
		A simple fly off can be defined by:
		\code	
		  FlyOffIconList flyOffIcons; // A vector of MSTR's
		  flyOffIcons.push_back( L"MyPlugin/FlyOffOption1" ); // first icon name
		  flyOffIcons.push_back( L"MyPlugin/FlyOffOption2" ); // second icon name
		  custButton->SetFlyOffIconNames( flyOffIcons, GetCOREInterface()->GetFlyOffTime(), 0, FLY_DOWN );
		\endcode 
		\param timeOut This is the time in milliseconds the button must be held pressed before the fly
		off appears. You may specify 0 if you want the buttons to fly off immediately.
		To retrieve the value that 3ds Max uses internally for its flyoffs use a method
		of Class Interface called <b>GetFlyOffTime()</b>. This returns a value in milliseconds.
		\param init This is the initial button displayed.
		\param dir This parameter is optional. It is used to indicate which direction the buttons
		should fly off. The choices for direction are:\n\n
		<b>FLY_VARIABLE</b>\n
		The default. The system will determine the direction of the fly off.\n\n
		<b>FLY_UP</b>\n
		The buttons fly off above.\n\n
		<b>FLY_DOWN</b>\n
		The buttons fly off beneath.\n\n
		<b>FLY_HVARIABLE</b>\n
		The buttons will fly off either left or right with the system determining the
		direction.\n\n
		<b>FLY_LEFT</b>\n
		The buttons fly off to the left.\n\n
		<b>FLY_RIGHT</b>\n
		The buttons fly off to the right. 
		\param columns
		\see QIcon MaxSDK::LoadMaxMultiResIcon( const QString& iconName ) */
		virtual void SetFlyOffIconsByName( const FlyOffIconList& flyOffIcons, 
			int timeOut, int init, int dir=FLY_VARIABLE, int columns=1 ) = 0;


		/*! \remarks Returns a list of the multi-resolution icon names used 
		in the button fly off control.
		\see void SetFlyOffIconsByName( const FlyOffIconList& flyOffIcons, 
			int timeOut, int init, int dir=FLY_VARIABLE, int columns=1 )
		*/
		virtual FlyOffIconList GetFlyOffIconNames() const = 0;


		/*! \remarks This method establishes which button is displayed by
		passing its index.
		\param f The index of the flyoff button to display.
		\param notify This indicates if the call to this method should notify the dialog
		proc. If TRUE it is notified; otherwise it isn't. */
		virtual void SetCurFlyOff(int f,BOOL notify=FALSE)=0;
		/*! \remarks Returns the index of the button which is currently
		displayed. */
		virtual int GetCurFlyOff()=0;

		/*! \remarks Determines if the button is checked. This method returns
		TRUE if the check button is currently in the In state (i.e. checked)
		and FALSE otherwise. */
		virtual BOOL IsChecked()=0;
		/*! \remarks Passing TRUE to this method sets the button to the In or
		checked state.
		\param checked If TRUE the button is set to the checked state; if FALSE the button is unchecked. */
		virtual void SetCheck( BOOL checked )=0;
		/*! \remarks This method controls if the check button is displayed in
		the highlight color when pressed in.
		\param highlight TRUE if you want the button to use the highlight color; otherwise pass FALSE. */
		virtual void SetCheckHighlight( BOOL highlight )=0;

		/*! \remarks Specifies if messages are sent when the user clicks or
		releases the button. If this method is called with TRUE, a message is
		sent immediately whenever the button is pressed down or released. The
		message <b>BN_BUTTONDOWN</b> is sent on button down and
		<b>BN_BUTTONUP</b> is sent when the button is released. The
		<b>BN_BUTTONUP</b> message is sent even if the button is released
		outside the button rectangle.
		\param notify TRUE if notification should be send by the button; FALSE if
		notifications should not be sent. */
		virtual void SetButtonDownNotify(BOOL notify)=0;
		/*! \remarks Specifies if messages are sent when the user right clicks
		the button.
		\param notify If TRUE, the <b>BN_RIGHTCLICK</b> message is sent whenever the users
		right clicks on the button. If FALSE no message are sent on right clicks. */
		virtual void SetRightClickNotify(BOOL notify)=0;

		/*! \remarks This methods sets the highlight color for the check
		button.
		\param clr The color for the button. It may be specified using the RGB macro, for
		example:\n\n
		 <b>SetHighlightColor(RGB(0,0,255));</b>
		 */
		virtual void SetHighlightColor(COLORREF clr)=0;
		/*! \remarks This methods returns the highlight color for the check button. */
		virtual COLORREF GetHighlightColor()=0;

		/*! \remarks Sets the drag and drop manager for this button control.
		\param dad A pointer to the drag and drop manager to set. */
		virtual void SetDADMgr(DADMgr *dad)=0;
		/*! \remarks Returns a pointer to the drag and drop manager for this button control.
		*/
		virtual DADMgr *GetDADMgr()=0;

		/*! \remarks Sets the macro data for this button.
		\param md The data to set. See Class MacroButtonData. */
		virtual void SetMacroButtonData(MacroButtonData *md)=0;

		/*! \remarks Returns a pointer to any macro button data for this button. See
		Class MacroButtonData. */
		virtual MacroButtonData *GetMacroButtonData()=0;

		virtual const MCHAR* GetCaptionText(void)=0;
		virtual bool SetCaptionText(const MCHAR* text)=0;

		/*! \remarks If set to true, the button will be displayed with both an icon (if present) and text */
		virtual void UseIconAndText(bool sw=true)=0;
	};

/*! \remarks Used to initialize and return a pointer to the button control.
\param hCtrl window handle of the control. */
CoreExport ICustButton *GetICustButton( HWND hCtrl );
/*! \remarks Used to release the control when finished.
\param icb Points to the control to release. */
CoreExport void ReleaseICustButton( ICustButton *icb );

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping ICustButton pointers.

Sample Code:
\code
MaxSDK::AutoPtr<ICustButton, ICustButtonDestructorPolicy> btn(GetICustButton(hbtn));
btn->SetType(CBT_PUSH);
btn->SetRightClickNotify(FALSE);
btn->SetButtonDownNotify(FALSE);
// when btn goes out of scope, its Delete method is called, which calls ReleaseICustButton
\endcode
\sa  class ICustButton, class AutoPtr
*/
class ICustButtonDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(ICustButton *btn)
	{
		ReleaseICustButton(btn);
	}
};

//---------------------------------------------------------------------------//
// CustStatus

#define CUSTSTATUSWINDOWCLASS _M("CustStatus")

enum StatusTextFormat {
	STATUSTEXT_LEFT,
	STATUSTEXT_CENTERED,
	STATUSTEXT_RIGHT };


/*! \sa  Class ICustomControl,
~{ Custom User Interface Controls }~,
Class ICustStatusEdit.\n\n
\par Description:
The custom status control provide a recessed area of the dialog which the
developer may use as a status prompt display.\n
\n \image html "status.gif"
To initialize the pointer to the control call:
<code>ICustStatus *GetICustStatus(HWND hCtrl);</code> \n
To release the control call: \n
<code>ReleaseICustStatus(ICustStatus *ics); </code> \n
The value to use in the Class field of the Custom Control Properties dialog is: CustStatus
*/
class ICustStatus : public ICustomControl {
	public:
		/*! \remarks This method specifies the text message to display.
		\par Parameters:
		<b>MCHAR *text</b>\n\n
		Points to the text to display. */
		virtual void SetText(const MCHAR *text)=0;
		/*! \remarks This methods controls the formatting of the text in the
		status control.
		\par Parameters:
		<b>StatusTextFormat f</b>\n\n
		One of the following options:\n\n
		<b>STATUSTEXT_LEFT</b>\n\n
		Left justified in the control.\n\n
		<b>STATUSTEXT_CENTERED</b>\n\n
		Centered in the control.\n\n
		<b>STATUSTEXT_RIGHT</b>\n\n
		Right justified in the control. */
		virtual void SetTextFormat(StatusTextFormat f)=0;
		/*! \remarks This retrieves the text entered into the control.
		\par Parameters:
		<b>MSTR& text</b>\n\n
		Storage for the text to retrieve.*/
		virtual void GetText ( MSTR& text) const =0;
		/*! \remarks This retrieves the length of the text entered into the control. 
		It  returns  the  length  of the  text in characters (so without
		the  terminating  NULL). Note that this  value  may  be  higher  than  the 
		actual length of the text when it contains multi-byte characters.*/
		virtual int GetTextLength() const =0;
		/*! \remarks Retrieves the text currently displayed in the custom status control.
		\par Parameters:
		<b>MCHAR *text</b>\n\n
		A pointer to storage for the text to return.\n\n
		<b>int ct</b>\n\n
		The maximum length of the string to return. */
		virtual void GetText(MCHAR *text, int ct)=0;
	};

/*! \remarks Used to initialize and return a pointer to the custom status control.
\param hCtrl window handle of the control. */
CoreExport ICustStatus *GetICustStatus( HWND hCtrl );
/*! \remarks Used to release the control when finished.
\param ics Points to the control to release. */
CoreExport void ReleaseICustStatus( ICustStatus *ics );

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping ICustStatus pointers.

Sample Code:
\code
MaxSDK::AutoPtr<ICustStatus, ICustStatusDestructorPolicy> custStatus(GetICustStatus(hwnd));
custStatus->GetText(theText);
// when custStatus goes out of scope, its Delete method is called, which calls ReleaseICustStatus
\endcode
\sa  class ICustStatus, class AutoPtr
*/
class ICustStatusDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(ICustStatus *custStatus)
	{
		ReleaseICustStatus(custStatus);
	}
};

//---------------------------------------------------------------------------//
// CustSeparator -- for use on toolbars

#define CUSTSEPARATORWINDOWCLASS _M("CustSeparator")

/*! \sa  Class ICustomControl,
~{ Custom User Interface Controls }~.\n\n
\par Description:
This provides a simple separator item. Methods are available to get and set the
visibility.\n\n
The value to use in the Class field of the Custom Control Properties dialog is:
<b>CustSeparator</b> */
class ICustSeparator : public ICustomControl {
	public:
		/*! \remarks Sets the visibility of the control to on or off.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetVisibility(BOOL onOff)=0;
		/*! \remarks Returns TRUE if the control is visible; otherwise FALSE.
		*/
		virtual BOOL GetVisibility()=0;
	};

/*! \remarks Used to initialize and return a pointer to the separator control.
\param hCtrl window handle of the control. */
CoreExport ICustSeparator *GetICustSeparator( HWND hCtrl );
/*! \remarks Used to release the control when finished.
\param ics Points to the control to release. */
CoreExport void ReleaseICustSeparator( ICustSeparator *ics );

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping ICustSeparator pointers.

Sample Code:
\code
MaxSDK::AutoPtr<ICustSeparator, ICustSeparatorDestructorPolicy> custSep(GetICustSeparator(hwnd));
custSep->SetVisibility(newVal);
// when custSep goes out of scope, its Delete method is called, which calls ReleaseICustSeparator
\endcode
\sa  class ICustSeparator, class AutoPtr
*/
class ICustSeparatorDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(ICustSeparator *custSep)
	{
		ReleaseICustSeparator(custSep);
	}
};

//----------------------------------------------------------------------------//
// CustToolbar control

#define CUSTTOOLBARWINDOWCLASS _M("CustToolbar")

	#define VERTTOOLBARWINDOWCLASS _M("VertToolbar")

// Sent in a WM_COMMAND when the user right clicks in open space
// on a toolbar.
#define TB_RIGHTCLICK 	0x2861

/*! \defgroup toolItemTypes Tool Item Types
\sa Class ToolItem */
//!@{
enum ToolItemType { 
	CTB_PUSHBUTTON,		//!< Button pops back out as soon as it is released by the user.
	CTB_CHECKBUTTON,	//!< Button stays pressed in until the user presses it again.
	CTB_MACROBUTTON,	//!< Can contain icons or text.
	CTB_SEPARATOR,		//!< Used to separate groups of items in a toolbar.
	CTB_STATUS,			//!< Can be used to display text.
	CTB_OTHER			//!< A user-defined tool type.
	, CTB_IMAGE			//!< An image control.
	};
//!@}

// toolbar orientation
#define CTB_NONE		CUI_NONE
#define CTB_HORIZ		CUI_HORIZ
#define CTB_VERT		CUI_VERT
#define CTB_FLOAT		CUI_FLOAT

/*! \sa  ~{ Custom User Interface Controls }~.\n\n
\par Description:
This class describes the properties of an item in a 3ds Max custom toolbar.
\par Data Members:
<b>ToolItemType type;</b>\n\n
See \ref toolItemTypes 
<b>int id</b>\n\n
The ID for the control.\n\n
<b>DWORD helpID</b>\n\n
For plug-in developers this id should be set to 0. Basically, the main 3ds Max
help file contains help tags that are tied to various parts of the 3ds Max UI,
allowing the right help page to come up when UI help is requested. In
particular, if you press the ? button on the toolbar, then press another
toolbar button, you'll get help on that button's functionality. This is because
internally pressing the button yields a help ID that indexes into the help
file. But since the same help ID must be compiled into the help file and into
MAX, and since the main 3ds Max help file can not be rebuilt by developers, they
cannot use this functionality.\n\n
<b>int w</b>\n\n
The width of the button image.\n\n
<b>int h</b>\n\n
The height of the button image.\n\n
<b>int orient;</b>\n\n
The orientation of the item. One of the following values:\n\n
<b>CTB_HORIZ</b>\n\n
<b>CTB_VERT</b>\n\n
<b>CTB_FLOAT</b>  */
class ToolItem: public MaxHeapOperators {
	public: 
		ToolItemType type;
		int id;
		DWORD helpID;
		int w, h;
		int orient;	// which orientations does this item apply to?
		/*! \remarks Destructor. */
		virtual ~ToolItem() {}
	};

//!	\brief This class describes the properties of a 3ds Max custom toolbar button.
/*!	Each one of these items represents a UI widget on a Toolbar in 3dsmax user interface. 
\sa  Class ToolItem, Class MAXBmpFileIcon, ~{ Custom User Interface Controls }~. */
class ToolButtonItem : public ToolItem 
{
	public:
	//! \name ImageList Members
	/*! The following four data members (<b>iOutEn, iInEn, iOutDis, iInDis</b>) are
		indices into the image list. They indicate which images to use for each of the
		four possible button states. You may specify a unique image for each one of
		these states by passing a different index for each state. Or you may supply a
		single image to be used for all the states by specifying the same index four
		times. */
	//!@{
		//! Out Enabled 
		int iOutEn;
		//! In Enabled
		int iInEn;		
		//! Out Disabled
		int iOutDis;
		//! In Disabled
		int iInDis;
	//!@}

	//! \name Dimensions
	//!@{
		//! The width of the button image.
		int iw;
		//! The height of the button image.
		int ih;
	//!@}

	//! \name Pointer Members
	//!@{
		//! The label describing the tool button item.
		const MCHAR *label;
		//! A pointer to the icon image associated with the button.
		MaxBmpFileIcon* mpIcon;
		//! A pointer to the pressed (or in) icon image associated with the button.
		MaxBmpFileIcon* mpInIcon;
	//!@}

	//! \name Multi-resolution icon
	//!@{
		/*! The icon name for loading a multi-resolution icon associated with the button.
		\see QIcon MaxSDK::LoadMaxMultiResIcon( const QString& iconName ) */
		MSTR mIconName;
	//!@}
	

	//! \name Constructors
	//!@{
		//! \brief Constructor.
		/*! \param  t - See \ref toolItemTypes.
			\param  iOE - The Out\&Enabled index.
			\param  iIE - The In\&Enabled index.
			\param  iOD - The Out\&Disabled index.
			\param  iID - The In\&Disabled index.
			\param  iW - The image width (size of the bitmap in the ImageList).
			\param  iH -	The image height (size of the bitmap in the ImageList).
			\param  wd - The width of the button.
			\param  ht - The height of the button.
			\param  ID - The ID of the control.
			\param  hID - The help ID. For plug-in developers this id should be set to 0.
			\param  *lbl = NULL - The label of the button.
			\param  ori = CTB_HORIZ|CTB_VERT|CTB_FLOAT - The allowable orientation of the item. 
				This may be one or more of the following: CTB_HORIZ - Horizontal, CTB_VERT - Vertical
				CTB_FLOAT - Floating (not docked) */
		ToolButtonItem(ToolItemType t,
			int iOE, int iIE, int iOD, int iID,
			int iW, int iH, int wd,int ht, int ID, DWORD hID=0, const MCHAR *lbl = NULL,
			int ori = CTB_HORIZ|CTB_VERT|CTB_FLOAT)
			{ 
				type = t; 
				orient = ori;
				iOutEn = iOE; iInEn = iIE; iOutDis = iOD; iInDis = iID;
				iw = iW; ih = iH; w = wd; h = ht; id = ID; helpID = hID;
				label = lbl;
				mpIcon = mpInIcon = NULL;
			}

		//! \brief Constructor.
		/*!	\param  t - See \ref toolItemTypes.
			\param *pIcon - A pointer to the icon associated with the button.
			\param  iW - The image width (size of the bitmap in the ImageList).
			\param  iH - The image height (size of the bitmap in the ImageList).
			\param  wd - The width of the button.
			\param  ht - The height of the button.
			\param  ID - The ID of the control.
			\param  hID - The help ID. For plug-in developers this id should be set to 0.
			\param  lbl - The label of the button.
			\param  ori - The orientation of the button item. */
		ToolButtonItem(ToolItemType t,
					   MaxBmpFileIcon* pIcon,
			int iW, int iH, int wd,int ht, int ID, DWORD hID=0, const MCHAR *lbl = NULL,
			int ori = CTB_HORIZ|CTB_VERT|CTB_FLOAT)
			{ 
				type = t; 
				orient = ori;
				mpIcon = pIcon;
				mpInIcon = NULL;
				iOutEn = iInEn = iOutDis = iInDis = -1;
				iw = iW; ih = iH; w = wd; h = ht; id = ID; helpID = hID;
				label = lbl;
			}

		//! \brief Constructor
		/*! \param  t - See \ref toolItemTypes.
			\param  pIcon - A pointer to the icon associated with the button.
			\param  pInIcon - A pointer to the in icon associated with the button.
			\param  iW - The image width (size of the bitmap in the ImageList).
			\param  iH - The image height (size of the bitmap in the ImageList).
			\param  wd - The width of the button.
			\param  ht - The height of the button.
			\param  ID - The ID of the control.
			\param  hID - The help ID. For plug-in developers this id should be set to 0.
			\param  lbl - The label of the button.
			\param  ori - The orientation of the button item. */
		ToolButtonItem(ToolItemType t,
					   MaxBmpFileIcon* pIcon,
					   MaxBmpFileIcon* pInIcon,
			int iW, int iH, int wd,int ht, int ID, DWORD hID=0, const MCHAR *lbl = NULL,
			int ori = CTB_HORIZ|CTB_VERT|CTB_FLOAT)
			{ 
				type = t; 
				orient = ori;
				mpIcon = pIcon;
				mpInIcon = pInIcon;
				iOutEn = iInEn = iOutDis = iInDis = -1;
				iw = iW; ih = iH; w = wd; h = ht; id = ID; helpID = hID;
				label = lbl;
			}

		//! \brief Constructor that uses a multi-resolution icon for the button.
		/*! \param  t - See \ref toolItemTypes.
			\param  iconName - The name of the icon associated with the button.
			\param  iW - The image width.
			\param  iH - The image height.
			\param  wd - The width of the button.
			\param  ht - The height of the button.
			\param  ID - The ID of the control.
			\param  hID - The help ID. For plug-in developers this id should be set to 0.
			\param  lbl - The label of the button.
			\param  ori - The orientation of the button item. 
			\see QIcon MaxSDK::LoadMaxMultiResIcon( const QString& iconName )
		*/
		ToolButtonItem( ToolItemType t,
			const MSTR& iconName,
			int iW, int iH, int wd,int ht, int ID, DWORD hID=0, const MCHAR *lbl = NULL,
			int ori = CTB_HORIZ|CTB_VERT|CTB_FLOAT)
		{ 
			type = t; 
			orient = ori;
			mpIcon = mpInIcon = NULL;
			iOutEn = iInEn = iOutDis = iInDis = -1;
			iw = iW; ih = iH; w = wd; h = ht; id = ID; helpID = hID;
			label = lbl;
			mIconName = iconName;
		}
	//!@}
};

/*! \sa  Class ToolItem,
Class MacroButtonData,
Class ICustButton,
~{ Custom User Interface Controls }~.\n\n
\par Description:
This class allows a macro item control to be added to the toolbar.\n\n

\par Data Members:
<b>MacroButtonData md;</b>\n\n
Points to the macro button data for this tool item.  */
class ToolMacroItem : public ToolItem {
	public:		
		MacroButtonData md;
		/*! \remarks Constructor.
		\par Parameters:
		<b>int wd</b>\n\n
		The width of the item.\n\n
		<b>int ht</b>\n\n
		The height of the item.\n\n
		<b>MacroButtonData *data</b>\n\n
		Points to the macro button data.\n\n
		<b>int ori = CTB_HORIZ|CTB_VERT|CTB_FLOAT</b>\n\n
		Specifies the orientation. One or more of the following values:\n\n
		<b>CTB_HORIZ</b>\n\n
		<b>CTB_VERT</b>\n\n
		<b>CTB_FLOAT</b> */
		ToolMacroItem(int wd, int ht, MacroButtonData *data, int ori = CTB_HORIZ|CTB_VERT|CTB_FLOAT)
			{ 
			type = CTB_MACROBUTTON;
			md = *data; 
			orient = ori;
			w = wd; h = ht; id = 0; helpID = 0;
			}
	};

/*! \sa  Class ToolItem,
~{ Custom User Interface Controls }~.\n\n
\par Description:
This class provides a toolbar separator object. This is used to space out
buttons in the toolbar.
\par Data Members:
<b>int vis;</b>\n\n
Visibility setting.  */
class ToolSeparatorItem : public ToolItem {
	public:
		int vis;
		/*! \remarks
		Constructor.
		\par Parameters:
		<b>int w</b>\n\n
		The width of the separator.\n\n
		<b>int h=16</b>\n\n
		The height of the separator.\n\n
		<b>BOOL vis=TRUE</b>\n\n
		TRUE for visible; FALSE for not.\n\n
		<b>int ori=CTB_HORIZ|CTB_VERT|CTB_FLOAT</b>\n\n
		The allowable orientations. One or more of the following values:\n\n
		<b>CTB_HORIZ</b>\n\n
		<b>CTB_VERT</b>\n\n
		<b>CTB_FLOAT</b> */
		ToolSeparatorItem(int w, int h=16, BOOL vis=TRUE, int ori=CTB_HORIZ|CTB_VERT|CTB_FLOAT) {
			type = CTB_SEPARATOR;
			id = 0;
			helpID = 0;
			this->w = w;
			this->h = h;
			h = 0;
			this->vis = vis;
			orient = ori;
			} 
	};

/*! \sa  Class ToolItem,
~{ Custom User Interface Controls }~.\n\n
\par Description:
This class allows a status control to be added to the toolbar.
\par Data Members:
<b>BOOL fixed;</b>\n\n
TRUE indicates a fixed width. If it is not fixed, then it will auto size itself
horizontally based on the size of the toolbar. For an example of this see the
status bar in the track view.  */
class ToolStatusItem : public ToolItem {
	public:
		BOOL fixed;
		/*! \remarks Constructor.
		\par Parameters:
		<b>int w</b>\n\n
		The width of the status box.\n\n
		<b>int h</b>\n\n
		The height of the status box.\n\n
		<b>BOOL f</b>\n\n
		The fixed data member - see above.\n\n
		<b>int id</b>\n\n
		The ID of the control.\n\n
		<b>DWORD hID=0</b>\n\n
		The help ID of the control. For plug-in developers this id should be set to 0.
		*/
		ToolStatusItem(int w, int h,BOOL f,int id, DWORD hID=0, int ori = CTB_HORIZ|CTB_FLOAT) {
			type = CTB_STATUS;
			this->w = w;
			this->h = h;
			this->id = id;
			this->helpID = hID;
			fixed = f;
			orient = ori;
			}
	};

#define CENTER_TOOL_VERTICALLY	0xffffffff

/*! \sa  Class ToolItem,
~{ Custom User Interface Controls }~.\n\n
\par Description:
This class is used to add any user defined or standard Windows control to a 3ds
Max custom toolbar.
\par Data Members:
<b>int y;</b>\n\n
The vertical justification.\n\n
<b>DWORD style;</b>\n\n
The control window style.\n\n
<b>MCHAR *className;</b>\n\n
The class name of the control. For the 3ds Max custom controls you may use one
of the following \#defines:\n\n
<b>SPINNERWINDOWCLASS</b>\n\n
<b>ROLLUPWINDOWCLASS</b>\n\n
<b>CUSTEDITWINDOWCLASS</b>\n\n
<b>CUSTBUTTONWINDOWCLASS</b>\n\n
<b>CUSTSTATUSWINDOWCLASS</b>\n\n
<b>CUSTTOOLBARWINDOWCLASS</b>\n\n
<b>CUSTIMAGEWINDOWCLASS</b>\n\n
<b>COLORSWATCHWINDOWCLASS</b>\n\n
Or it may be a literal string such as:\n\n
<b>"COMBOBOX"</b>\n\n
See the Win32 API help under <b>CreateWindow()</b> for a list of the options
here.\n\n
<b>MCHAR *windowText;</b>\n\n
The window text. This is displayed in controls that have text in them.  */
class ToolOtherItem : public ToolItem {
	public:
		int	  y;
		DWORD_PTR style;
		const MCHAR *className;
		const MCHAR *windowText;
		/*! \remarks Constructor.
		\par Parameters:
		<b>MCHAR *cls</b>\n\n
		The class name of the control. This may be one of the values listed above under
		data members.\n\n
		<b>int w</b>\n\n
		The width of the control.\n\n
		<b>int h</b>\n\n
		The height of the control.\n\n
		<b>int id</b>\n\n
		The ID of the control.\n\n
		<b>DWORD_PTR style=WS_CHILD|WS_VISIBLE</b>\n\n
		The style of the control window.\n\n
		<b>int y=CENTER_TOOL_VERTICALLY</b>\n\n
		The vertical justification. This is a y offset from the top of the toolbar in
		pixels. The default value simply centers the tool vertically.\n\n
		<b>MCHAR *wt=NULL</b>\n\n
		The window text.\n\n
		<b>DWORD hID=0</b>\n\n
		The help ID. For plug-in developers this id should be set to 0. */
		ToolOtherItem(const MCHAR *cls,int w,int h,int id,DWORD_PTR style=WS_CHILD|WS_VISIBLE,
					int y=CENTER_TOOL_VERTICALLY, const MCHAR *wt=NULL,DWORD hID=0, int ori=CTB_HORIZ|CTB_FLOAT) {
			type = CTB_OTHER;
			this->y = y;
			this->w = w;
			this->h = h;
			this->id = id;
			this->helpID = hID;
			this->style = style;
			orient = ori;
			className = cls;
			windowText = wt;
			}		
};


/*! \sa  Class ToolItem.\n\n
\par Description:
This class allows a developer to use an image in the toolbar. This is used
internally as part of the object snap code. All methods of this class are
implemented by the system.  */
class ToolImageItem : public ToolItem {
	public:
		int	  y;
		int	il_index;
		MSTR iconName;
		/*! \remarks Constructor. The data members are initialized to the values
		passed. The type parameter of ToolItem is set to CTB_IMAGE. */
		ToolImageItem(int w,int h,int k,int id, int y=CENTER_TOOL_VERTICALLY,DWORD hID=0, int ori=CTB_HORIZ|CTB_FLOAT) {
			type = CTB_IMAGE;
			this->y = y;
			this->w = w;
			this->h = h;
			this->il_index  = k;
			this->id = id;
			this->helpID = hID;
			orient = ori;
			}

		/*! \remarks Constructor. The data members are initialized to the values
		passed. The type parameter of ToolItem is set to CTB_IMAGE. 
		\par Parameters:
		<b>const MSTR& iconName</b>\n\n
		The filename of the icon.\n\n
		<b>int w</b>\n\n
		The width of the icon.\n\n
		<b>int h</b>\n\n
		The height of the icon.\n\n
		<b>int id</b>\n\n
		The ID of the icon.\n\n
		<b>DWORD hID=0</b>\n\n
		The help ID. For plug-in developers this id should be set to 0. \n\n
		<b>int ori</b>\n\n
		The orientation of the toolbar where icon locates. */
		ToolImageItem(const MSTR& iconName, int w,int h,int id, DWORD hID=0, int ori=CTB_HORIZ|CTB_FLOAT) {
			type = CTB_IMAGE;
			this->iconName = iconName;
			this->w = w;
			this->h = h;
			this->id = id;
			this->helpID = hID;
			orient = ori;
			}	
	};

/*! \sa  ~{ Custom User Interface Controls }~. 
Class ToolItem,
Class MacroButtonData,
Class ICustStatusEdit,
Class ICustStatus,
Class ICustButton.\n\n
\par Description:
This control allows the creation of toolbars containing buttons (push, check,
and fly-offs), status fields, separators (spacers), and other Windows or user
defined controls. \n\n\
The standard size for 3ds Max toolbar button icons is 16x15 or 16x16 pixels.\n\n
In 3ds Max 3.0 and later toolbars may have multiple rows, or appear vertically.
They may also have macro buttons (added with the MacroButtonData class) which
may have icons or text.\n
\n \image html "toolbar.gif"
To initialize the pointer to the control call: \n
<code>ICustToolbar *GetICustToolbar(HWND hCtrl);</code> \n
To release the control call: \n
<code>ReleaseICustToolbar(ICustToolbar *ict);</code> \n
The value to use in the Class field of the Custom Control Properties dialog is: CustToolbar \n\n
Note: The TB_RIGHTCLICK message is sent when the user right clicks in open space on a toolbar: \n
Also Note: To add tooltips to the toolbar controls you can do so by capturing the WM_NOTIFY
message in the dialog proc. For complete sample code see <b>/MAXSDK/SAMPLES/HOWTO/CUSTCTRL/CUSTCTRL.CPP</b>.
The specific message is processed as shown below. \n
\code
case WM_NOTIFY:
 // This is where we provide the tooltip text for the
 // toolbar buttons...
 if(((LPNMHDR)lParam)->code == TTN_NEEDTEXT) {
   LPTOOLTIPTEXT lpttt;
   lpttt = (LPTOOLTIPTEXT)lParam;
   switch (lpttt->hdr.idFrom) {
	 case ID_TB_1:
	   lpttt->lpszText = _M("Do Nothing Up");
	   break;
	 case ID_TB_2:
	   lpttt->lpszText = _M("Do Nothing Down");
	   break;
	 case ID_TB_3:
	   lpttt->lpszText = _M("Do Nothing Lock");
	   break;
	 case IDC_BUTTON1:
	   if (to->custCtrlButtonC->IsChecked())
		 lpttt->lpszText = _M("Button Checked");
	   else
		 lpttt->lpszText = _M("Button Un-Checked");
	   break;
	 };
   }
 break;
\endcode
*/
class ICustToolbar : public ICustomControl {
	public:
		/*! \remarks This method establishes the image list used to display
		images in the toolbar.
		\par Parameters:
		<b>HIMAGELIST hImage</b>\n\n
		The image list. An image list is a collection of same-sized images,
		each of which can be referred to by an index. Image lists are used to
		efficiently manage large sets of icons or bitmaps in Windows. All
		images in an image list are contained in a single, wide bitmap in
		screen device format. An image list may also include a monochrome
		bitmap that contains masks used to draw images transparently (icon
		style). The Windows API provides image list functions, which enable you
		to draw images, create and destroy image lists, add and remove images,
		replace images, and merge images. */
		virtual void SetImage( HIMAGELIST hImage )=0;
		/*! \remarks The developer calls this method once for each item in the
		toolbar. The items appear in the toolbar from left to right in the order that
		they were added using this method. (Note that this method adds tools to the
		custom toolbar and not the 3ds Max toolbar).
		\par Parameters:
		<b>const ToolItem\& entry</b>\n\n
		Describes the item to add to the toolbar.\n\n
		<b>int pos=-1</b>\n\n
		Controls where the added tool is inserted. The default of -1 indicates the
		control will be added at the right end of the toolbar. */
		virtual void AddTool( ToolItem& entry, int pos=-1)=0;
		/*! \remarks Currently this method is identical to the AddTool() method.
		\par Parameters:
		<b>ToolItem\& entry</b>\n\n
		Describes the item to add to the toolbar.\n\n
		<b>int pos=-1</b>\n\n
		Controls where the added tool is inserted. The default of -1 indicates
		the control will be added at the right end of the toolbar. */
		virtual void AddTool2(ToolItem& entry, int pos=-1)=0; // Adds caption buttons to toolbars
		/*! \remarks This method is used to delete tools from the toolbar.
		\par Parameters:
		<b>int start</b>\n\n
		Specifies which tool is the first to be deleted.\n\n
		<b>int num=-1</b>\n\n
		Specifies the number of tools to delete. If this parameter is -1 (the
		default) it deletes 'start' through count-1 tools. */
		virtual void DeleteTools( int start, int num=-1 )=0;  // num = -1 deletes 'start' through count-1 tools
		/*! \remarks Passing TRUE to this method draws a border beneath the
		toolbar. You can see the appearance of the bottom border in the sample
		toolbar shown above. If this is set to FALSE, the border is not drawn.
		\par Parameters:
		<b>BOOL on</b>\n\n
		TRUE to draw the border; FALSE for no border. */
		virtual void SetBottomBorder(BOOL on)=0;
		/*! \remarks Passing TRUE to this method draws a border above the
		toolbar. You can see the appearance of the top border in the sample
		toolbar shown above. If this is set to FALSE, the border is not drawn
		\par Parameters:
		<b>BOOL on</b>\n\n
		TRUE to draw the border; FALSE for no border. */
		virtual void SetTopBorder(BOOL on)=0;
		/*! \remarks Returns the width needed for specified number of rows.
		\par Parameters:
		<b>int rows</b>\n\n
		The number of rows. */
		virtual int	 GetNeededWidth(int rows)=0;	// return width needed for specified # of rows
		/*! \remarks Sets the number of rows that the toolbar may hold.
		\par Parameters:
		<b>int rows</b>\n\n
		The number of rows to set. */
		virtual void SetNumRows(int rows)=0;
		/*! \remarks This method is used to return a pointer to one of the
		toolbar's buttons. Using this pointer you can call methods on the
		button. If you use this method, you must release the control after you
		are finished with it.
		\par Parameters:
		<b>int id</b>\n\n
		Specifies the id of the toolbar button.
		\return  A pointer to one of the toolbar's buttons. If the button is
		not found it returns NULL. See Class ICustButton. */
		virtual ICustButton *GetICustButton( int id )=0;
		/*! \remarks This method is used to return a pointer to one of the
		toolbars status controls. Using this pointer you can call methods on
		the status control. If you use this method, you must release the
		control after you are finished with it.
		\par Parameters:
		<b>int id</b>\n\n
		Specifies the id of the toolbar button.
		\return  A pointer to one of the toolbars status controls. See
		Class ICustStatus. */
		virtual ICustStatus *GetICustStatus( int id )=0;
		/*! \remarks Returns the handle to the toolbar item whose ID is
		passed.
		\par Parameters:
		<b>int id</b>\n\n
		Specifies the id of the toolbar button. */
		virtual HWND GetItemHwnd(int id)=0;
		/*! \remarks Returns the number of items in the toolbar. */
		virtual int GetNumItems()=0;
		/*! \remarks Each item in the toolbar has an ID. When items are programatically
		added to the toolbar via Class ToolButtonItem an ID is passed to the
		ToolButtonItem constructor. This method returns the ID for the
		specified item in the toolbar.
		\par Parameters:
		<b>int index</b>\n\n
		Specifies which toolbar item to return the id of. This is an index
		between <b>0</b> and <b>GetNumItems()-1</b>.
		\return  When the button is added using
		Class ToolButtonItem this is
		the id that is part of that structure. When the user operates a tool
		the dialog proc get a <b>WM_COMMAND</b> message and this is also the id
		in <b>LOWORD(wParam)</b>. */
		virtual int GetItemID(int index)=0;
		/*! \remarks Returns the index into the list of toolbar entries of the item whose id
		is passed.
		\par Parameters:
		<b>int id</b>\n\n
		The id of the control to find. */
		virtual int FindItem(int id)=0;
		/*! \remarks Deletes the toolbar item whose id is passed.
		\par Parameters:
		<b>int id</b>\n\n
		The id of the control to delete. */
		virtual void DeleteItemByID(int id)=0;

		/* \brief Returns the docking delegate for the toolbar.
		* The docking delegate is used by the QmaxDockingWinHost class for passing docking
		* related messages further to the toolbar. For instance when the docking container
		* changes its orientation, the delegate will also change the orientation of the toolbar.
		* \see QmaxDockingWinHost
		*/
		virtual MaxSDK::QmaxDockingWinHostDelegate* GetDockingDelegate() = 0;

		/*! \remarks Computes the required size of a floating CUI Frame which is
		linked to a toolbar. The values returned will be zero if the frame is not
		linked to a toolbar.
		\par Parameters:
		<b>SIZE *sz</b>\n\n
		The computed size is returned here. <b>sz.cx</b> is the width, <b>sz.cy</b> is
		the height.\n\n
		<b>int rows=1</b>\n\n
		The number of rows for the toolbar used in the computation. */
		virtual void GetFloatingCUIFrameSize(SIZE *sz, int rows=1)=0;
		/*! \remarks This method is used to return a pointer to the custom status edit
		control whose id is passedIf you use this method, you must release the
		control after you are finished with it. See
		Class ICustStatusEdit.
		\par Parameters:
		<b>int id</b>\n\n
		Specifies the id of the toolbar button. */
		virtual ICustStatusEdit *GetICustStatusEdit(int id)=0;
		/*! \remarks This resets the icons in the toolbar. This tells all the buttons in
		this toolbar to delete their icon image cache. If a plug-in has created
		a toolbar with any MaxBmpFileIcons on it, it should register a callback
		for color changing, and call this method on the toolbar. See
		<a href="struct_notify_info.html">Structure NotifyInfo</a> for
		registering the color change callback. */
		virtual void ResetIconImages() = 0;

		/*! \remarks This method resizes all of the toolbar's buttons to follow the width and height specified.
		\param buttonWidth The new width of the toolbar's buttons.
		\param buttonHeight The new height of the toolbar's buttons.
		\param iconWidth The new width of the button icons.
		\param iconHeight The new height of the button icons.
		 */
		virtual void ResizeButtons(int buttonWidth, int buttonHeight, int iconWidth, int iconHeight) = 0;

	};

/*! \remarks Used to initialize and return a pointer to the toolbar control.
\param hCtrl window handle of the control. */
CoreExport ICustToolbar *GetICustToolbar( HWND hCtrl );
/*! \remarks Used to release the control when finished.
\param ict Points to the control to release. */
CoreExport void ReleaseICustToolbar( ICustToolbar *ict );

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping ICustToolbar pointers.

Sample Code:
\code
MaxSDK::AutoPtr<ICustToolbar, ICustToolbarDestructorPolicy> custToolbar(GetICustToolbar(hwnd));
custToolbar->ResetIconImages();
// when custToolbar goes out of scope, its Delete method is called, which calls ReleaseICustToolbar
\endcode
\sa  class ICustToolbar, class AutoPtr
*/
class ICustToolbarDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(ICustToolbar *custToolbar)
	{
		ReleaseICustToolbar(custToolbar);
	}
};

/*! \sa  Class ICustomControl.\n\n
\par Description:
This control allows the creation of <b>vertical</b> toolbars containing buttons
(push, check, and fly-offs), status fields, separators (spacers), and other
Windows or user defined controls. The standard size for 3ds Max toolbar button 
icons is 16x15 or 16x16 pixels.\n\n
All methods of this class are implemented by the system.\n\n
To initialize the pointer to the control call: <br>   <b>IVertToolbar
*GetIVertToolbar(HWND hCtrl);</b>    To release the control call:<br>
<b>void ReleaseIVertToolbar(IVertToolbar *ict);</b>    The value to use in the
Class field of the Custom Control Properties dialog is: <b>VertToolbar</b>
 */
class IVertToolbar : public ICustomControl {
	public:
		/*! \remarks This method establishes the image list used to display
		images in the toolbar.
		\par Parameters:
		<b>HIMAGELIST hImage</b>\n\n
		The image list. An image list is a collection of same-sized images,
		each of which can be referred to by an index. Image lists are used to
		efficiently manage large sets of icons or bitmaps in Windows. All
		images in an image list are contained in a single, wide bitmap in
		screen device format. An image list may also include a monochrome
		bitmap that contains masks used to draw images transparently (icon
		style). The Windows API provides image list functions, which enable you
		to draw images, create and destroy image lists, add and remove images,
		replace images, and merge images. */
		virtual void SetImage( HIMAGELIST hImage )=0;
		/*! \remarks The developer calls this method once for each item in the
		toolbar. The items appear in the toolbar from left to right in the
		order that they were added using this method. (Note that this method
		adds tools to the custom toolbar and not the 3ds Max toolbar).
		\par Parameters:
		<b>const ToolItem\& entry</b>\n\n
		Describes the of item to add to the toolbar.\n\n
		<b>int pos=-1</b>\n\n
		Controls where the added tool is inserted. The default of -1 indicates
		the control will be added at the right end of the toolbar. */
		virtual void AddTool( const ToolItem& entry, int pos=-1 )=0;
		/*! \remarks This method is used to delete tools from the toolbar.
		\par Parameters:
		<b>int start</b>\n\n
		Specifies which tool is the first to be deleted.\n\n
		<b>int num=-1</b>\n\n
		Specifies the number of tools to delete. If this parameter is -1 (the
		default) it deletes 'start' through count-1 tools. */
		virtual void DeleteTools( int start, int num=-1 )=0;  // num = -1 deletes 'start' through count-1 tools
		/*! \remarks Passing TRUE to this method draws a border beneath the
		toolbar.
		\par Parameters:
		<b>BOOL on</b>\n\n
		TRUE to draw the border; FALSE for no border. */
		virtual void SetBottomBorder(BOOL on)=0;
		/*! \remarks Passing TRUE to this method draws a border above the
		toolbar.
		\par Parameters:
		<b>BOOL on</b>\n\n
		TRUE to draw the border; FALSE for no border. */
		virtual void SetTopBorder(BOOL on)=0;
		/*! \remarks This method is used to return a pointer to one of the
		toolbar's buttons. Using this pointer you can call methods on the
		button. If you use this method, you must release the control after you
		are finished with it.
		\par Parameters:
		<b>int id</b>\n\n
		Specifies the id of the toolbar button.
		\return  A pointer to one of the toolbar's buttons. */
		virtual ICustButton *GetICustButton( int id )=0;
		/*! \remarks This method is used to return a pointer to one of the
		toolbars status controls. Using this pointer you can call methods on
		the status control. If you use this method, you must release the
		control after you are finished with it.
		\par Parameters:
		<b>int id</b>\n\n
		Specifies the id of the toolbar button.
		\return  A pointer to one of the toolbars status controls */
		virtual ICustStatus *GetICustStatus( int id )=0;		
		/*! \remarks Returns the handle to the toolbar item whose ID is
		passed.
		\par Parameters:
		<b>int id</b>\n\n
		Specifies the id of the toolbar button. */
		virtual HWND GetItemHwnd(int id)=0;
		virtual void Enable(BOOL onOff=TRUE){};
	};

/*! \remarks Used to initialize and return a pointer to the vertical toolbar control.
\param hCtrl window handle of the control. */
CoreExport IVertToolbar *GetIVertToolbar( HWND hCtrl );
/*! \remarks Used to release the control when finished.
\param ict Points to the control to release. */
CoreExport void ReleaseIVertToolbar( IVertToolbar *ict );

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping IVertToolbar pointers.

Sample Code:
\code
MaxSDK::AutoPtr<IVertToolbar, IVertToolbarDestructorPolicy> custVToolbar(GetIVertToolbar(hwnd));
custVToolbar->Enable(newVal);
// when custVToolbar goes out of scope, its Delete method is called, which calls ReleaseIVertToolbar
\endcode
\sa  class IVertToolbar, class AutoPtr
*/
class IVertToolbarDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(IVertToolbar *custVToolbar)
	{
		ReleaseIVertToolbar(custVToolbar);
	}
};

//---------------------------------------------------------------------------//
// CustImage


#define CUSTIMAGEWINDOWCLASS _M("CustImage")

/*! \sa  Class ICustomControl,
~{ Custom User Interface Controls }~.\n\n
\par Description:
The custom image control provides a recessed area in the dialog to display a
bitmap image.\n\n
\image html "custimg.gif"
To initialize the pointer to the control call:\n
<code>ICustImage *GetICustImage(HWND hCtrl);</code> \n
To release the control call: \n
<code>ReleaseICustImage(ICustImage *ici);</code> \n
The value to use in the Class field of the Custom Control Properties dialog is: CustImage
*/
class ICustImage : public ICustomControl {
	public:
		/*! \remarks This method sets the image to display.
		\par Parameters:
		<b>HIMAGELIST hImage</b>\n\n
		An image list. An image list is a collection of same-sized images, each
		of which can be referred to by its index. Image lists are used to
		efficiently manage large sets of icons or bitmaps in Windows. All
		images in an image list are contained in a single, wide bitmap in
		screen device format. An image list may also include a monochrome
		bitmap that contains masks used to draw images transparently (icon
		style). The Windows API provides image list functions, which enable you
		to draw images, create and destroy image lists, add and remove images,
		replace images, and merge images.\n\n
		<b>int index</b>\n\n
		This is the index of the image to display in the image list.\n\n
		<b>int w</b>\n\n
		The image width.\n\n
		<b>int h</b>\n\n
		The image height. */
		virtual void SetImage( HIMAGELIST hImage,int index, int w, int h )=0;	

		/*! \remarks Sets a multi-resolution icon to the image.
		\param iconName The name of the icon.
		\see QIcon MaxSDK::LoadMaxMultiResIcon( const QString& iconName )
		*/
		virtual void SetIconByName( const MSTR& iconName ) = 0;


};

/*! \remarks Used to initialize and return a pointer to the image control.
\param hCtrl window handle of the control. */
CoreExport ICustImage *GetICustImage( HWND hCtrl );
/*! \remarks Used to release the control when finished.
\param ici Points to the control to release. */
CoreExport void ReleaseICustImage( ICustImage *ici );

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping ICustImage pointers.

Sample Code:
\code
MaxSDK::AutoPtr<ICustImage, ICustImageDestructorPolicy> custImage(GetICustImage(hwnd));
custImage->SetImage( hImage, index, w,  h );
// when custImage goes out of scope, its Delete method is called, which calls ReleaseICustImage
\endcode
\sa  class ICustImage, class AutoPtr
*/
class ICustImageDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(ICustImage *custImage)
	{
		ReleaseICustImage(custImage);
	}
};

//---------------------------------------------------------------------------//
// CustImage 2D Version for displaying osnap icons


#define CUSTIMAGEWINDOWCLASS2D  _M("CustImage2D")

class ICustImage2D : public ICustomControl {
	public:
		virtual void SetImage( HIMAGELIST hImage,int index, int w, int h )=0;		
	};


//------------------------------------------------------------------------
// Off Screen Buffer

/*! \sa  ~{ Custom User Interface Controls }~.\n\n
\par Description:
This control provides an off screen buffer which the developer may draw into,
then quickly blit onto the actual display for flicker free image updates.\n\n
To initialize the pointer to the control call: */
class IOffScreenBuf: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~IOffScreenBuf() {;}
		/*! \remarks Returns a handle to the display device context (DC) for
		the off screen buffer. The display device context can be used in
		subsequent GDI functions to draw in the buffer. */
		virtual HDC GetDC()=0;
		/*! \remarks This method is used to erase the buffer.
		\par Parameters:
		<b>Rect *rct=NULL</b>\n\n
		Specifies the rectangular region to erase. If NULL the entire buffer is
		erased. */
		virtual void Erase(Rect *rct=NULL)=0;
		/*! \remarks This method blits (transfers the image from) the buffer
		to the display.
		\par Parameters:
		<b>Rect *rct=NULL</b>\n\n
		Specifies the rectangular region to blit. If NULL the entire buffer is
		blitted. */
		virtual void Blit(Rect *rct=NULL)=0;
		/*! \remarks This method is used to resize the buffer. */
		virtual void Resize()=0;
		/*! \remarks This sets the buffer to the specified color.
		\par Parameters:
		<b>COLORREF color</b>\n\n
		The color to set. You may use the RGB macro to set the color. */
		virtual void SetBkColor(COLORREF color)=0;
		/*! \remarks This methods retrieves the background color of the
		buffer.
		\return  The background color of the buffer. */
		virtual COLORREF GetBkColor()=0;
	};

/*!  */
CoreExport IOffScreenBuf *CreateIOffScreenBuf(HWND hWnd);
/*!  */
CoreExport void DestroyIOffScreenBuf(IOffScreenBuf *iBuf);


//------------------------------------------------------------------------
// Color swatch control
// Puts up the ColorPicker when user right clicks on it.
//

// This message is sent as the color is being adjusted in the 
// ColorPicker.
// LOWORD(wParam) = ctrlID, 
// HIWORD(wParam) = 1 if button UP 
//                = 0 if mouse drag.
// lParam = pointer to ColorSwatchControl
#define CC_COLOR_CHANGE			WM_USER + 603

// LOWORD(wParam) = ctrlID, 
// lParam = pointer to ColorSwatchControl
#define CC_COLOR_BUTTONDOWN		WM_USER + 606

// LOWORD(wParam) = ctrlID, 
// HIWORD(wParam) = FALSE if user cancelled - TRUE otherwise
// lParam = pointer to ColorSwatchControl
#define CC_COLOR_BUTTONUP		WM_USER + 607

// This message is sent if the color has been clicked on, before 
// bringing up the color picker.
// LOWORD(wParam) = ctrlID, 
// HIWORD(wParam) = 0 
// lParam = pointer to ColorSwatchControl
#define CC_COLOR_SEL			WM_USER + 604


// This message is sent if another color swatch has been dragged and dropped
// on this swatch. 
// LOWORD(wParam) = toCtrlID, 
// HIWORD(wParam) = 0
// lParam = pointer to ColorSwatchControl
#define CC_COLOR_DROP			WM_USER + 605

// This message is sent when the color picker is closed
// LOWORD(wParam) = ctrlID, 
// HIWORD(wParam) = 0 
// lParam = pointer to ColorSwatchControl
// The following macro has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.
#define CC_COLOR_CLOSE			WM_USER + 608 //RK: 05/14/00, added this
// End of 3ds max 4.2 Extension

#define COLORSWATCHWINDOWCLASS _M("ColorSwatch")

/*! \sa  Class ICustomControl,
~{ Custom User Interface Controls }~,
<a href="https://msdn.microsoft.com/en-us/library/dd183449(v=vs.85).aspx">COLORREF</a>.\n\n
\par Description:
The color swatch control puts up the standard 3ds Max modeless color selector
when the control is clicked on. The plug-in may be notified as the user
interactively selects new colors.\n\n
\image html "colorsw.gif"
To initialize the pointer to the control call: <br>  <b>*GetIColorSwatch(HWND
hCtrl, COLORREF col, MCHAR *name);</b>\n\n
For example:  <b>custCSw = GetIColorSwatch(GetDlgItem(hDlg, IDC_CSWATCH),
 RGB(255,255,255), _M("New Wireframe Color"));</b>\n\n
This returns the pointer to the control, sets the initial color selected, and
displays the text string passed in the title bar of the selection dialog.\n\n
To release the control call:  <b>ReleaseIColorSwatch(IColowSwatch
*ics);</b>\n\n
The value to use in the Class field of the Custom Control Properties dialog is:
<b>ColorSwatch</b>  This message is sent as the color is being adjusted in the
ColorPicker.\n\n
<b>CC_COLOR_CHANGE</b>\n\n
<b>lParam</b> = pointer to ColorSwatchControl\n\n
<b>LOWORD(wParam)</b> contains the ID of the control. This is the named
established in the ID field of the Custom Control Properties dialog.\n\n
<b>HIWORD(wParam)</b> contains 1 if button UP, or 0 if mouse drag.\n\n
This message is sent if the color has been clicked on, before bringing up the
color picker.\n\n
<b>CC_COLOR_SEL</b>\n\n
<b>lParam</b> contains a pointer to the ColorSwatch Control.\n\n
<b>LOWORD(wParam)</b> contains the ID of the control. This is the named
established in the ID field of the Custom Control Properties dialog.\n\n
<b>HIWORD(wParam)</b> contains 0.\n\n
This message is sent if another color swatch has been dragged and dropped on
this swatch.\n\n
<b>CC_COLOR_DROP</b>\n\n
<b>lParam</b> contains a pointer to the ColorSwatch Control.\n\n
<b>LOWORD(wParam)</b> contains the ID of the control. This is the named
established in the ID field of the Custom Control Properties dialog.\n\n
<b>HIWORD(wParam)</b> contains 0.\n\n
This message is sent when the color picker is closed.\n\n
<b>CC_COLOR_CLOSE</b>\n\n
<b>lParam</b> contains a pointer to the ColorSwatch Control.\n\n
<b>LOWORD(wParam)</b> contains the ID of the control. This is the named
established in the ID field of the Custom Control Properties dialog.\n\n
<b>HIWORD(wParam)</b> contains 0.
*/
class IColorSwatch: public ICustomControl {
	public:
		// sets only the varying color of the color picker if showing
		/*! \remarks This method sets the current color value.
		\par Parameters:
		<b>COLORREF c</b>\n\n
		You can pass specific RGB values in using the RGB macro. For example,
		to pass in pure blue you would use <b>RGB(0,0,255)</b>.\n\n
		<b>int notify=FALSE</b>\n\n
		If you pass TRUE for this parameter, the dialog proc for the dialog
		will receive the <b>CC_COLOR_CHANGE</b> message each time the color is
		changed.
		\return  This method returns the old color. */
		virtual COLORREF SetColor(COLORREF c, int notify=FALSE)=0;  // returns old color
		COLORREF SetColor(Color c, int notify=FALSE) {return SetColor(c.toRGB(), notify);}  // returns old color
		virtual AColor SetAColor(AColor c, int notify=FALSE)=0;  // returns old color

		// sets both the varying color and the "reset"color of the color picker
		virtual COLORREF InitColor(COLORREF c, int notify=FALSE)=0;  // returns old color
		COLORREF InitColor(Color c, int notify=FALSE) {return InitColor(c.toRGB(), notify);}  // returns old color
		virtual AColor InitAColor(AColor c, int notify=FALSE)=0;  // returns old color

		virtual void SetUseAlpha(BOOL onOff)=0;
		virtual BOOL GetUseAlpha()=0;

		/** \brief Sets whether the color swatch should ignore the 
		* application-wide color correction and display the raw RGB(A) value.
		* \see GetIgnoreColorCorrection() */
		virtual void SetIgnoreColorCorrection( bool ignore ) { ; }

		/** \brief Determines whether the color swatch should ignore the 
		* application-wide color correction and display the raw RGB(A) value.
		* \see SetIgnoreColorCorrection() */
		virtual bool GetIgnoreColorCorrection() const { return false; }

		/*! \remarks This method may be used to retrieve the color selected by
		the user.
		\return  The <b>COLORREF</b> structure returned may be broken down into
		individual RGB values by using the <b>GetRValue(color)</b>,
		<b>GetGValue(color)</b>, and <b>GetBValue(color)</b> macros. */
		virtual COLORREF GetColor()=0;
		virtual AColor GetAColor()=0;
		/*! \remarks This method sets if the color shown in the color swatch
		is dithered on not.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE to force the color to be dithered; otherwise FALSE. */
		virtual void ForceDitherMode(BOOL onOff)=0;
		/*! \remarks Call this method to have the color selector comes up in a
		modal dialog. This forces the user to select OK before the user may
		operate the rest of the program. */
		virtual void SetModal()=0;
		/*! \remarks This method is called to indicate that the color swatch
		is in a dialog that has been become active or inactive. A color swatch
		that is in an inactive dialog will be drawn as dithered due to the
		limited number of color registers available on an 8-bit display.
		\par Parameters:
		<b>int onOff</b>\n\n
		If TRUE the color swatch is in an active dialog. If FALSE the control
		is in an inactive dialog. */
		virtual void Activate(int onOff)=0;
		/*! \remarks If there is already a color picker up for a color swatch,
		this method switches it over to edit the color swatch on which
		<b>EditThis()</b> was called.
		\par Parameters:
		<b>BOOL startNew=TRUE</b>\n\n
		If there was no color picker up, if this parameter is set to TRUE, then
		a color picker is created. If this parameter is set to FALSE, and there
		was no color picker up, then nothing happens. */
		virtual void EditThis(BOOL startNew=TRUE)=0;
		virtual void SetKeyBrackets(BOOL onOff)=0;

		/*! \remarks This method controls when change notifications are sent, while the color picker is open.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		If you pass FALSE for this parameter, the dialog proc for the dialog
		will receive multiple <b>CC_COLOR_CHANGE</b> messages as the user makes changes
		interactively in the color picker.
		If you pass TRUE for this parameter, the dialog proc for the dialog
		will receive only one <b>CC_COLOR_CHANGE</b> message, when the color picker is closed,
		and only if the user clicks OK instead of Cancel. */
		virtual void SetNotifyAfterAccept(BOOL onOff)=0;
		/*! \remarks This methods indicates how change notifications are sent, while the color picker is open.
		\return FALSE indicates the dialog proc for the dialog receives multiple <b>CC_COLOR_CHANGE</b> messages
		as the user makes changes interactively in the color picker.
		TRUE indicates the dialog proc for the dialog receives only one <b>CC_COLOR_CHANGE</b> message,
		when the color picker is closed, and only if the user clicks OK instead of Cancel. */
		virtual BOOL GetNotifyAfterAccept()=0;
	};

/*! \remarks Used to initialize and return a pointer to the color swatch control.
\param hCtrl window handle of the control.
\param col sets the initial color selected.
\param name sets the text string displayed in the title bar of the selection dialog.*/
CoreExport IColorSwatch *GetIColorSwatch( HWND hCtrl, COLORREF col, const MCHAR *name);
/*! \remarks Used to initialize and return a pointer to the color swatch control.
\param hCtrl window handle of the control.
\param col sets the initial color selected.
\param name sets the text string displayed in the title bar of the selection dialog.*/
CoreExport IColorSwatch *GetIColorSwatch( HWND hCtrl, Color col, const MCHAR *name);
/*! \remarks Used to initialize and return a pointer to the color swatch control.
\param hCtrl window handle of the control.
\param col sets the initial color selected.
\param name sets the text string displayed in the title bar of the selection dialog.*/
CoreExport IColorSwatch *GetIColorSwatch( HWND hCtrl, AColor col, const MCHAR *name);
/*! \remarks Used to initialize and return a pointer to the color swatch control.
\param hCtrl window handle of the control.*/
CoreExport IColorSwatch *GetIColorSwatch(HWND hCtrl);
/*! \remarks Used to release the control when finished.
\param ics Points to the control to release. */
CoreExport void ReleaseIColorSwatch( IColorSwatch *ics );

//! \remarks Refresh all color swatches.
CoreExport void RefreshAllColorSwatches();

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping IColorSwatch pointers.

Sample Code:
\code
MaxSDK::AutoPtr<IColorSwatch, IColorSwatchDestructorPolicy> colorSwatch(GetIColorSwatch(hwnd));
colorSwatch->SetAColor( newColor );
// when colorSwatch goes out of scope, its Delete method is called, which calls ReleaseIColorSwatch
\endcode
\sa  class IColorSwatch, class AutoPtr
*/
class IColorSwatchDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(IColorSwatch *colorSwatch)
	{
		ReleaseIColorSwatch(colorSwatch);
	}
};

// This class is only available in versions 5.1 and later.
#define COLOR_SWATCH_RENAMER_INTERFACE_51 Interface_ID(0x5a684953, 0x1fc043dc)

/*!  \n\n
class IColorSwatchRenamer\n\n

\par Description:
This class is an interface for changing the name of a color swatch. Previously
the only way to change the name of a swatch was to reinitialize it with a new
GetIColorSwatch call, and this would not help with swatches that were already
being displayed in a color selector dialog. Using this new interface,
developers can change the name of a swatch and\n\n
simultaneously change the name of a color selector dialog that's associated
with the swatch. An example of its usage can be seen below :-\n\n
It's used in Editable Poly as follows - when the user changes the radio button
that affects whether our swatch is used as a "Select by Color" or a "Select by
Illumination" swatch, the following code is used to update the swatch.\n\n
These first three lines are adequate when the color selector is not being
displayed:\n\n
<b>mpEPoly-\>getParamBlock()-\>GetValue (ep_vert_sel_color, t,
selByColor,</b>\n\n
<b>mpEPoly-\>FOREVER); getParamBlock()-\>GetValue (ep_vert_color_selby,
t,</b>\n\n
<b>mpEPoly-\>byIllum, FOREVER);</b>\n\n
<b>iCol = GetIColorSwatch (GetDlgItem (hWnd, IDC_VERT_SELCOLOR),
selByColor,</b>\n\n
<b> GetString (byIllum ? IDS_SEL_BY_ILLUM : IDS_SEL_BY_COLOR));</b>\n\n
That will update the swatch to the correct name and color. However, if the\n\n
color selector is being displayed, we need to use the new interface to
ensure\n\n
that the name is promptly updated in the UI:\n\n
<b>pInterface = iCol-\>GetInterface
(COLOR_SWATCH_RENAMER_INTERFACE_51);</b>\n\n
<b>if (pInterface) {</b>\n\n
<b> IColorSwatchRenamer *pRenamer = (IColorSwatchRenamer *)
pInterface;</b>\n\n
<b> pRenamer-\>SetName (GetString (byIllum ? IDS_SEL_BY_ILLUM :
IDS_SEL_BY_COLOR));</b>\n\n
<b>}</b>\n\n
Finally, don't forget to release the swatch: ReleaseIColorSwatch (iCol);\n\n
  */
class IColorSwatchRenamer : public BaseInterface {
public:
	// The method we needed the interface for:
	/*! \remarks Sets the name of the color swatch, and of any associated
	color picker dialog. */
	virtual void SetName (const MCHAR *name) { }

	// Interface ID
	Interface_ID GetID() {return COLOR_SWATCH_RENAMER_INTERFACE_51;}
};

//---------------------------------------------------------------------------//
// DragAndDrop Window


#define DADWINDOWCLASS	_M("DragDropWindow")

typedef LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

/*! \sa  Class ICustomControl,
Class DADMgr,
~{ Custom User Interface Controls }~.\n\n
\par Description:
This is a new type of custom control used to provide drag and drop to and from
things other than Custom Buttons. An example of an item that uses this control
is a sample sphere window in the Material Editor.\n\n
To initialize the pointer to the control call: */
class IDADWindow : public ICustomControl {
	public:
		// Installing this makes it do drag and drop.
		/*! \remarks Set the drag and drop manager for this control.
		\par Parameters:
		<b>DADMgr *dadMgr</b>\n\n
		A pointer to the drag and drop manager for this control. */
		virtual void SetDADMgr( DADMgr *dadMgr)=0;
		/*! \remarks Returns a pointer to the drag and drop manager for this
		control. */
		virtual DADMgr *GetDADMgr()=0;
		
		// Install Window proc called to do all the normal things after 
		// drag/and/drop processing is done.
		/*! \remarks This method establishes a window proc that is called to
		handle all the normal processing after the drag and drop processing is
		done.
		\par Parameters:
		<b>WindowProc *proc</b>\n\n
		The window proc. Note the following typedef:\n\n
		<b>typedef LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM
		wParam, LPARAM lParam);</b>\n\n
		<b>\#define SLIDERWINDOWCLASS _M("SliderControl")</b>\n\n
		<b>// LOWORD(wParam) = ctrlID,</b>\n\n
		<b>// HIWORD(wParam) = TRUE if user is dragging the slider
		interactively.</b>\n\n
		<b>// lParam = pointer to ISliderControl</b>\n\n
		<b>\#define CC_SLIDER_CHANGE   WM_USER + 611</b>\n\n
		<b>// LOWORD(wParam) = ctrlID,</b>\n\n
		<b>// lParam = pointer to ISliderControl</b>\n\n
		<b>\#define CC_SLIDER_BUTTONDOWN WM_USER + 612</b>\n\n
		<b>// LOWORD(wParam) = ctrlID,</b>\n\n
		<b>// HIWORD(wParam) = FALSE if user cancelled - TRUE otherwise</b>\n\n
		<b>// lParam = pointer to ISliderControl</b>\n\n
		<b>\#define CC_SLIDER_BUTTONUP  WM_USER + 613</b> */
		virtual void SetWindowProc( WindowProc *proc)=0;

	};

/*! \remarks Used to initialize and return a pointer to the D&D window control.
\param hWnd window handle of the control. */
CoreExport IDADWindow *GetIDADWindow( HWND hWnd);
/*! \remarks Used to release the control when finished.
\param idw Points to the control to release. */
CoreExport void ReleaseIDADWindow( IDADWindow *idw );

/*! \brief Class representing the DestructorPolicy for AutoPtr instances wrapping IDADWindow pointers.

Sample Code:
\code
MaxSDK::AutoPtr<IDADWindow, IDADWindowDestructorPolicy> idw(GetIDADWindow(hwnd));
idw->SetWindowProc( winProc );
// when idw goes out of scope, its Delete method is called, which calls ReleaseIDADWindow
\endcode
\sa  class IDADWindow, class AutoPtr
*/
class IDADWindowDestructorPolicy: public MaxHeapOperators 
{
public:
	static void Delete(IDADWindow *idw)
	{
		ReleaseIDADWindow(idw);
	}
};

//------------------------------------------------------------------------
// Window thumb tack

//! \brief This function installs a thumb tack in the title bar of a window which forces the window to the top. 
/*! The window class for the window should have 4 extra bytes in the window structure for SetWindowLongPtr(). */
CoreExport void InstallThumbTack(HWND hwnd);
CoreExport void RemoveThumbTack(HWND hwnd);


/*! \brief Handy routines for setting up Integer Spinners.
	This global function (not part of class ISpinnerControl) is used
	for setting up Spinners. It performs the equivalent of GetISpinner(), SetLimits(), SetValue(), and
	LinkToEdit().
	WARNING: To prevent a Memory Leak, be sure to call ReleaseISpinner on the pointer that this function returns,
	or use AutoPtr and ISpinnerControlDestructorPolicy
	\param hwnd - The handle of the dialog box in which the spinner appears.
	\param idSpin - The ID of the spinner.
	\param idEdit - The ID of the edit control.
	\param min - The minimum allowable value.
	\param max - The maximum allowable value.
	\param val - The initial value for the spinner.
	\return - A pointer to the spinner control. */
CoreExport ISpinnerControl *SetupIntSpinner(HWND hwnd, int idSpin, int idEdit,  int min, int max, int val);

/*! \brief Handy routines for setting up Floating Point and Universe Spinners.
	This global function (not part of class ISpinnerControl) is used for setting up
	Spinners. Internally it calls the GetISpinner(), SetLimits(), SetValue()
	and LinkToEdit() functions.
	WARNING: To prevent a Memory Leak, be sure to call ReleaseISpinner on the pointer that this function returns,
	or use AutoPtr and ISpinnerControlDestructorPolicy
	\param hwnd - The handle of the dialog box in which the spinner appears.
	\param idSpin - The ID of the spinner.
	\param idEdit - The ID of the edit control.
	\param min - The minimum allowable value.
	\param max - The maximum allowable value.
	\param val - The initial value for the spinner.
	\param scale = 0.1f - The initial scale value for the spinner.
	\return  - A pointer to the spinner control.
	\par Sample Code:
	Sample code to initialize a spinner / edit control.
	\code
	ISpinnerControl* spin = GetISpinner(GetDlgItem(hDlg, IDC_SPIN_SPINNER));
	spin->SetLimits(0.0f, 100.0f, FALSE);
	spin->SetValue(100.0f, FALSE);
	spin->LinkToEdit(GetDlgItem(hDlg, IDC_SPIN_EDIT), EDITTYPE_FLOAT);
	ReleaseISpinner(spin);
	\endcode 
	The above code could be replaced with the following simplified code:
	\code
	ISpinnerControl* spin = SetupFloatSpinner(hDlg, IDC_SPIN_SPINNER, IDC_SPIN_EDIT, 0.0f, 100.0f, 100.0f);
	ReleaseISpinner(spin);
	\endcode  */
CoreExport ISpinnerControl *SetupFloatSpinner(HWND hwnd, int idSpin, int idEdit,  float min, float max, float val, float scale = 0.1f);
CoreExport ISpinnerControl *SetupUniverseSpinner(HWND hwnd, int idSpin, int idEdit,  float min, float max, float val, float scale = 0.1f);

// Controls whether or not spinners send notifications while the user adjusts them with the mouse
/*! \remarks This function controls whether or not spinners send <b>CC_SPINNER_CHANGE</b>
notifications while the user adjusts them with the mouse.
\par Parameters:
<b>BOOL onOff</b>\n\n
TRUE to turn on; FALSE to turn off. */
CoreExport void SetSpinDragNotify(BOOL onOff);
/*! \remarks Returns TRUE if <b>CC_SPINNER_CHANGE</b> notifications are sent by spinners
while the user adjusts them with the mouse; FALSE if they are not sent. */
CoreExport BOOL GetSpinDragNotify();

//---------------------------------------------------------------------------
//

CoreExport void DisableAccelerators();
CoreExport void EnableAccelerators();
CoreExport BOOL AcceleratorsEnabled();

//! \brief Explicitly marks the scene as "dirty"
/*!	Some operations are not undoable, but they do change the scene.
	These operations should call SetSaveRequiredFlag(); to explicitly indicate
	that save and autobackup are required after the operation.
	\param requireSave - If TRUE, the scene will be flagged as requiring save and autobackup. */
CoreExport void SetSaveRequiredFlag(BOOL requireSave = TRUE);

//! \brief Explicitly marks the scene as "dirty"
/*!	Similar to SetSaveRequiredFlag(BOOL), but with finer control over both "save" and "autobackup" flags.
	This is used when flags need to be reset after an operation.
	\code
		// Backup save required flags
		const BOOL needSave = GetSaveRequiredFlag();
		const BOOL needAutobackup = GetAutoBackupSaveRequiredFlag();
		... perform operation that can alter the flags...
		// Restore save flags
		SetSaveRequiredFlag(needSave, needAutobackup);
	\endcode
	\param requireSave - If TRUE, the scene will be flagged as requiring a save.
	\param requireAutoBackup - If TRUE, the scene will be flagged as requiring an autobackup. */
CoreExport void SetSaveRequiredFlag(BOOL requireSave, BOOL requireAutoBackup);

//! \brief Retrieves the internal save required flag 
/*! \see void SetSaveRequiredFlag(BOOL requireSave, BOOL requireAutoBackup)
	\return The value of the "save-required" flag as last set by the SetSaveRequiredFlag method. */
CoreExport BOOL GetSaveRequiredFlag();

//! \brief Retrieves the internal autobackup save required flag 
/*! \see void SetSaveRequiredFlag(BOOL requireSave, BOOL requireAutoBackup)
	\return The value of the "autobackup-save-required" flag as last set by the SetSaveRequiredFlag method. */
CoreExport BOOL GetAutoBackupSaveRequiredFlag();

//! \brief This tells if the scene needs to be saved 
/*! Whenever an undoable operation is executed (by the user or otherwise), or
	SetSaveRequiredFlag is called with TRUE as its first parameter, the scene is 
	considered different than its most recently saved version.
	\return TRUE if the scene needs to be saved; FALSE otherwise. */ 
CoreExport BOOL IsSaveRequired();

//!	\brief This tells if the current scene needs to be backed up. 
/*! An autosave is required when (a) something has changed in the scene (a save is 
	required) and (b) no autosave has been performed since the last scene change.
	The second condition (b) guarantees that the scene is backed up only once when
	a 3ds Max session is left unattended for a time that spans several autosave 
	(autobackup) time intervals. 
	\return TRUE if an autosave should occur; FALSE otherwise. */
CoreExport BOOL IsAutoSaveRequired();


#define CUSTOMCONTROLSOPTIONS_INTERFACE_ID Interface_ID(0xae7f4060, 0xb4f5cdd)

//! \brief Class used to handle some custom controls options
class ICustomControlsOptions : public FPStaticInterface 
{

public:
	//-------------------------------------------------------------------------
	//! Set to print or not the icons paths and to persist or not that option.
	virtual void SetPrintIconPaths(bool printIconPaths, bool persist) = 0;

	//-------------------------------------------------------------------------
	//! Indicates whether icon paths are printed to the listener.
	virtual bool GetPrintIconPaths() const = 0;
	
	//-------------------------------------------------------------------------
	//! Set to print or not ui text-clipping errors and to persist or not that option.
	virtual void SetPrintTextClippingIssues( bool printClippingErrors, bool persist ) = 0;

	//-------------------------------------------------------------------------
	//! Indicates whether ui text-clipping errors are printed to the listener.
	virtual bool GetPrintTextClippingIssues() const = 0;

	//! \brief Get the x sensitivity for reporting test-clipping errors.
	/*! Differences greater than or equal to this value will be reported*/
	virtual int  GetTextClippingIssuesSensitivityX() const = 0;

	//-------------------------------------------------------------------------
	//! Set the x sensitivity for reporting test-clipping errors and to persist or not.
	virtual void SetTextClippingIssuesSensitivityX( int val, bool persist) = 0;

	//! \brief Get the y sensitivity for reporting test-clipping errors.
	/*! Differences greater than or equal to this value will be reported*/
	virtual int  GetTextClippingIssuesSensitivityY() const = 0;

	//-------------------------------------------------------------------------
	//! Set the y sensitivity for reporting test-clipping errors and to persist or not.
	virtual void SetTextClippingIssuesSensitivityY( int val, bool persist ) = 0;

	//-------------------------------------------------------------------------
	/** \brief Gets the additional user UI scaling factor. 
	 * \return The users UI scaling factor.
	 * \see SetUIUserScaleFactor, MaxSDK::GetUIScaleFactor( int monitorID = -1 )
	 */
	virtual float GetUIUserScaleFactor() const = 0; 
	//-------------------------------------------------------------------------
	/** \brief Sets the additional UI scaling factor. 
	 * \param[in] userScaleFactor - The new scaling factor. If this value is set to 
	 *			some positive value above zero, it will affect the UI scaling 
	 *			of certain 3dsmax UI elements.
	 * \see GetUIUserScaleFactor(), MaxSDK::GetUIScaleFactor( int monitorID = -1 )
	 */
	virtual void SetUIUserScaleFactor( float userScaleFactor ) = 0; 

    //-------------------------------------------------------------------------
    //! Set to print or not the tool clip information and to persist or not that option.
    virtual void SetPrintToolClipInfo( bool printToolClipInfo, bool persist ) = 0;

    //-------------------------------------------------------------------------
    //! Indicates whether tool clip information is printed to the listener.
    virtual bool GetPrintToolClipInfo() const = 0;
	
};

#pragma warning(pop)

