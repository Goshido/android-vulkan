//**************************************************************************/
// Copyright (c) 1998-2019 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//**************************************************************************/

#pragma once

#include "maxheap.h"
#include <limits>
#include <float.h> // Not included by <limits> on Mac OS X.
#include <locale.h>
#include "coreexp.h"
#include <wtypes.h>
#include <CommCtrl.h> // for HIMAGELIST
#include "strclass.h"
#include "geom/ipoint2.h"
#include "geom/box2.h"
#include "containers/Array.h"


namespace MaxSDK
{
	//-------------------------------------------------------------------------
	/** \brief Returns the UI scaling factor relative to a default of 96 DPI.
	 * The function reads the systems monitor settings and calculates the 
	 * scaling factor based on the vertical *logical* DPI (what can be set and 
	 * changed through the windows OS control panel). 
	 * It will return 1.0f, if some value couldn't be retrieved or calculated, or 
	 * a calculation result will be 0.0f; so it is safe to use the result
	 * of this function to simply multiply UI sizes without further checking. 
	 *
	 * \param[in] monitorID The index of the monitor to get the actual logical 
	 *		vertical DPI from. Default value is set to -1, which means the 
	 *		primary system monitor.
	 *
	 *	\return The scaling factor, if it could be retrieved and calculated, 
	 *		1.0f in other cases.
	 */
	UtilExport float GetUIScaleFactor( int monitorID = -1 );

	//-------------------------------------------------------------------------
	/** \brief Get the additional user UI scaling factor. 
	 * \return The users ui scaling factor.
	 * \see SetUIUserScaleFactor, GetUIScaleFactor
	 */
	UtilExport float GetUIUserScaleFactor();

	//-------------------------------------------------------------------------
	/** \brief Returns the given value multiplied with the UI scaling factor. 
	 *
	 * \param[in] value The value which should be dpi scaled.
	 * \param[in] monitorID The index of the monitor to get the actual logical 
	 *		vertical dpi from. Default value is set to -1, which means the 
	 *		primary system monitor.
	 * \return The dpi scaled value.	 
	 * \see GetUIScaleFactor
	 */
	inline int UIScaled( int value, int monitorID = -1 )
	{
		return (int)(value * GetUIScaleFactor( monitorID ));
	}

	//-------------------------------------------------------------------------
	/** \brief Returns the given value multiplied with the UI scaling factor. 
	 *
	 * \param[in] value The value which should be dpi scaled.
	 * \param[in] monitorID The index of the monitor to get the actual logical 
	 *		vertical dpi from. Default value is set to -1, which means the 
	 *		primary system monitor.
	 * \return The dpi scaled value.	 
	 * \see UIUnScaled, GetUIScaleFactor
	 */
	inline float UIScaled( float value, int monitorID = -1 )
	{
		return value * GetUIScaleFactor( monitorID );
	}

	//-------------------------------------------------------------------------
	/** \brief Returns the given value divided with the UI scaling factor. 
	 * Use this function to do an unscaling of a previous dpi scaled value.
	 *
	 * \param[in] value The value which should be dpi unscaled.
	 * \param[in] monitorID The index of the monitor to get the actual logical 
	 *		vertical dpi from. Default value is set to -1, which means the 
	 *		primary system monitor.
	 * \return The dpi unscaled value.	 
	 * \see UIScaled, GetUIScaleFactor
	 */
	inline int UIUnScaled( int value, int monitorID = -1 )
	{
		return (int)(value / GetUIScaleFactor( monitorID ));
	}
    
    //-------------------------------------------------------------------------
	/** \brief Returns the given value divided with the UI scaling factor. 
	 * Use this function to do an unscaling of a previous dpi scaled value.
	 *
	 * \param[in] value The value which should be dpi unscaled.
	 * \param[in] monitorID The index of the monitor to get the actual logical 
	 *		vertical dpi from. Default value is set to -1, which means the 
	 *		primary system monitor.
	 * \return The dpi unscaled value.	 
	 * \see UIScaled, GetUIScaleFactor
	 */
	inline float UIUnScaled( float value, int monitorID = -1 )
	{
		return value / GetUIScaleFactor( monitorID );
	}

    //-----------------------------------------------------------------------------
    /** \brief Scaling of a rectangle values based on UIScaleFactor 
	 *
     * \param[in,out] rect  RECT struct to be scaled to UIScaleFactor
	 * \param[in] monitorID The index of the monitor to get the actual logical 
	 *		vertical dpi from. Default value is set to -1, which means the 
	 *		primary system monitor.
	 *
	 */
    inline void ScaleRect( RECT &rect, int monitorID = -1 )
    {
        float scaleFactor = GetUIScaleFactor( monitorID );
        rect.top    = (LONG)(rect.top       * scaleFactor);
        rect.bottom = (LONG)(rect.bottom    * scaleFactor);
        rect.left   = (LONG)(rect.left      * scaleFactor);
        rect.right  = (LONG)(rect.right     * scaleFactor);
    }
    //-----------------------------------------------------------------------------
    /** \brief Unscaling of a rectangle values based on UIScaleFactor 
	 *
	 * \param[in, out] rect RECT struct to be unscaled to UIScaleFactor
	 * \param[in] monitorID The index of the monitor to get the actual logical 
	 *		vertical dpi from. Default value is set to -1, which means the 
	 *		primary system monitor.
	 *
	 */
    inline void UnscaleRect( RECT &rect, int monitorID = -1 )
    {
        float scaleFactor = GetUIScaleFactor( monitorID );
        rect.top    = (LONG)(rect.top       / scaleFactor);
        rect.bottom = (LONG)(rect.bottom    / scaleFactor);
        rect.left   = (LONG)(rect.left      / scaleFactor);
        rect.right  = (LONG)(rect.right     / scaleFactor);
    }

	//-------------------------------------------------------------------------
	/** \brief Returns a scaled copy of the bitmap if the UI scaling factor is not 1.0. 
	 *
	 * \param[in] bitmap The bitmap to be dpi scaled.
	 * \param[in] monitorID The index of the monitor to get the actual logical 
	 *		vertical dpi from. Default value is set to -1, which means the 
	 *		primary system monitor.
	 * \return The dpi scaled bitmap if the UI scaling factor is not 1.0, otherwise null.	 
	 */
	UtilExport HBITMAP GetUIScaledBitmap( HBITMAP bitmap, int monitorID = -1 );

	//-------------------------------------------------------------------------
	/** \brief Annotates the control with Microsoft Active Accessibility name property. 
	 *
	 * \param[in] hWnd	The window handle of the control to be annotated
	 * \param[in] name  The name annotated to the control 
	 * \return S_OK or the error codes
	 *
	 */
	UtilExport HRESULT AnnotateControlName(HWND hWnd, const MSTR& name);

	//-------------------------------------------------------------------------
	/** \brief Removes the Microsoft Active Accessibility name from the control. 
	 *
	 * \param[in] hWnd	The window handle of the control to be removed annotation
	 * \return S_OK or the error codes
	 *
	 */
	UtilExport HRESULT RemoveAnnotatedNameFromControl(HWND hWnd);

	//-----------------------------------------------------------------------------
	//! \brief Scales the toolbar size and icons in standard file open/save dialogs.
	/*!
	Scales the toolbar containing the 'Go To Last Folder Visisted', 'Up One Level',
	'Create New Folder' buttons, and the 'View Menu' dropdown, by the system dpi
	scaling value. The hDlg parameter is HWND passed to the LPOFNHOOKPROC specified
	in the OPENFILENAME structure. Technically, this HWND is a child of the win32 file
	open / save dialog
	\code
	static INT_PTR WINAPI MinimalMaxFileOpenHookProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch(message)
		{
		case WM_INITDIALOG:
			MaxSDK::ScaleFileOpenSaveDialogToolbar(hDlg);
			...
	\endcode
	\param[in] hDlg The dialog's HWND.
	*/
	CoreExport void ScaleFileOpenSaveDialogToolbar(HWND hDlg);

	//-------------------------------------------------------------------------
	//! \brief A minimal LPOFNHOOKPROC for scaling the toolbar size and icons in standard file open/save dialogs.
	/*!
	Use this LPOFNHOOKPROC when not customization of the Open/Save File dialog is being performed.  
	Example usage:
	\code
	OPENFILENAME   ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLESIZING;
	ofn.lpfnHook = (LPOFNHOOKPROC)MaxSDK::MinimalMaxFileOpenHookProc;
	...
	BOOL res = GetSaveFileName(&ofn);
	\endcode
	Implementation:
	\code
	INT_PTR WINAPI MinimalMaxFileOpenHookProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_INITDIALOG)
			MaxSDK::ScaleFileOpenSaveDialogToolbar(hDlg);
		return FALSE;
	}
	\endcode
	*/
	CoreExport INT_PTR WINAPI MinimalMaxFileOpenHookProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

/*! Returns a floating point value parsed from the specified windows control. */
CoreExport float GetWindowFloat(HWND hwnd,BOOL* valid = NULL);
/*! Returns a floating point value parsed using formatted with user locale input. */
CoreExport float GetFloatFromText(const MCHAR* buf, BOOL *valid);
/*! Returns an integer value parsed from the specified windows control. */
CoreExport int GetWindowInt(HWND hwnd,BOOL* valid = NULL);
/*! Converts an integer value to a string for display in a windows control. */
CoreExport BOOL SetWindowTextInt( HWND hwnd, int i );
/*! Converts a floating point value to a string for display in a control. The precision parameter is used to determine the number of values to display to the right of the decimal point. */
CoreExport BOOL SetWindowTextFloat( HWND hwnd, float f, int precision = 3 );
/*! Converts a floating point value to a string in the user locale. The precision parameter is used to determine the number of values appearing to the right of the decimal point. */
CoreExport MSTR GetTextFromFloat(float f, int precision = 3);
/*! Sets the dialog box control to display the passed floating point value. Trailing zeros are removed. */
CoreExport BOOL SetDlgItemFloat( HWND hwnd, int idControl, float val );
/*! Returns a floating point value parsed from the specified dialog box control. */
CoreExport float GetDlgItemFloat( HWND hwnd, int idControl, BOOL* valid = NULL );
/*! Sets the font used for the specified dialog and all its child dialogs. */
CoreExport void SetDlgFont( HWND hDlg, HFONT hFont );
/*! Moves the window to the specified coordinates. */
CoreExport void SlideWindow( HWND hwnd, int x, int y );
/*! Stretches the window to the specified size. */
CoreExport void StretchWindow( HWND hwnd, int w, int h );
/*! Centers the specified child window in the parent window. */
CoreExport BOOL CenterWindow(HWND hWndChild, HWND hWndParent);
/*! This function is the same as the Win32 function GetClientRect() except that it returns the coordinates relative to the window's parent's client window. */
CoreExport void GetClientRectP( HWND hwnd, Rect* rect );
/*! Draws a button that has a bitmap icon. */
CoreExport void DrawIconButton( HDC hdc, HBITMAP hBitmap, Rect& wrect, Rect& brect, BOOL in );
/*! Returns the height of a list box dropdown list. */
CoreExport int GetListHieght( HWND hList );
/*! Draws a shaded vertical line at the specified x coordinate between the two given y coordinates. The in parameter indicates whether the shading that appears to the right is light or dark. */
CoreExport void ShadedVertLine( HDC hdc, int x, int y0, int y1, BOOL in );
/*! Draws a shaded horizontal line at the specified y coordinate between the two given x coordinates. The in parameter indicates whether the shading that appears below the line is light or dark. */
CoreExport void ShadedHorizLine( HDC hdc, int y, int x0, int x1, BOOL in );
/*! Draws a shaded rectangle. */
CoreExport void ShadedRect( HDC hdc, RECT& rect );
/*! Draws a rectangle with a 3D appearance. */
CoreExport void Rect3D( HDC hdc, RECT& rect, BOOL in );
/*! Draws a rectangle with a white background and a 3D appearance. */
CoreExport void WhiteRect3D( HDC hdc, RECT rect, BOOL in );
/*! Draws a toolbar button using the given rectangle and the in or out state. */
CoreExport void DrawButton( HDC hdc, RECT rect, BOOL in );
//! \brief Sets a pathname in an edit control, truncating with an ellipses if the pathname is too long.
CoreExport void SetPathWithEllipses(HWND hwnd, const MCHAR* path, HFONT hFont);

/**
 * Displays a string in an HWND (typically an edit control), trimming beginning characters with an ellipses if the string is too long.
 *
 * \param displayString   The string to be displayed.
 * \param hWnd            The hWnd to display the string in - uses existing font
 * \param additionalSpace   Reserved space in pixels that reduces string display area in hWnd
*/
UtilExport void DisplayStringWithEllipses(const MCHAR* displayString, HWND hWnd, int additionalSpace = 0);

/**
 * Displays a string in an HWND (typically an edit control), trimming beginning characters with an ellipses if the string is too long.
 *
 * \param displayString   The string to be displayed.
 * \param hWnd            The window to display the string in
 * \param hFont           The font to use in the window
 * \param additionalSpace   Reserved space in pixels that reduces string display area in hWnd
 */
UtilExport void DisplayStringWithEllipses(const MCHAR* displayString, HWND hWnd, HFONT hFont, int additionalSpace = 0);


//! \brief Updates the width of a combo-box drop-down window to make all its items fit
/*! Call this function if some strings in your combo-box are longer than the actual
 * combo-box control's width. This function will update the width of the combo-box
 * to make the widest combo-box item fit.
 * \param hComboBox The handle of the combobox to update
 * \param vScroll true if the width of a vertical scroll bar should be taken into account
 * \return -1 if the combo-box could not be updated successfully, the new width of
 * the combo-box drop down otherwise.
 */
CoreExport int UpdateComboBoxDropDownWidth(HWND hComboBox, bool vScroll = true);


//! \brief XOR drawing method of rectangle
/*! This method will draw a dotted or solid rectangle onto the specified window. This method
can provide better compatibility with Vista Aero than using GDI function directly.

\param[in] hwnd Window handle of the window on which the drawing will occur.
\param[in] p0 One corner of the rectangle.
\param[in] p1 p0's diagonal in the rectangle.
\param[in] solidToRight If zero, the rectangle will be dotted. If less than zero, and p0.x >= p1.x,
the line will be solid, otherwize dotted. If greater than zero, and p0.x <= p1.x, the line will be solid,
otherwize dotted.
\param[in] bErase Whether to erase the foreground and use the background to fill the affected pixels,
this is only valid for the window handle associated with graphic window.
\param[in] bDelayUpdate if true, the result image will be stored on the back buffer and won't be presented to 
screen immediately. If next drawing function is invoked with bDelayUpdate being false, then the image 
will be presented combined with the result of previous drawing function. If the drawing result is not 
necessary to be seen immediately, passing true to this parameter will reduce the times of back buffer presenting, 
thus optimize the performance. This is only valid for window handle associated with graphic window.
\param[in] color Optional XOR line color. Default is grey (128,128,128)
*/
CoreExport void XORDottedRect( HWND hwnd, IPoint2 p0, IPoint2 p1, int solidToRight = 0,
	bool bErase = false, bool bDelayUpdate = false, COLORREF color = RGB(128, 128, 128));
//! \brief XOR drawing method of circle
/*! This method will draw a dotted or solid circle onto the specified window. This method
can provide better compatibility with Vista Aero than using GDI function directly.

\param[in] hwnd Window handle of the window on which the drawing will occur.
\param[in] p0 One corner of the circle's bounding rectangle.
\param[in] p1 p0's diagonal in the rectangle.
\param[in] solidToRight If zero, the circle will be dotted. If less than zero, and p0.x >= p1.x,
the line will be solid, otherwize dotted. If greater than zero, and p0.x <= p1.x, the line will be solid,
otherwize dotted.
\param[in] bErase Whether to erase the foreground and use the background to fill the affected pixels,
this is only valid for the window handle associated with graphic window.
\param[in] bDelayUpdate if true,the result image will be stored on the back buffer and won't be presented to 
screen immediately. If next drawing function is invoked with bDelayUpdate being false, then the image 
will be presented combined with the result of previous drawing function. If the drawing result is not 
necessary to be seen immediately, passing true to this parameter will reduce the times of back buffer presenting, 
thus optimize the performance. This is only valid for window handle associated with graphic window.
\param[in] color Optional XOR line color. Default is grey (128,128,128)
*/
CoreExport void XORDottedCircle( HWND hwnd, IPoint2 p0, IPoint2 p1, int solidToRight = 0,
	bool bErase = false, bool bDelayUpdate = false, COLORREF color = RGB(128, 128, 128));
//! \brief XOR drawing method of polylines
/*! This method will draw dotted or solid polylines onto the specified window. This method
can provide better compatibility with Vista Aero than using GDI function directly.

\param[in] hwnd Window handle of the window on which the drawing will occur.
\param[in] count The count of corners of the polylines.
\param[in] pts Pointer to the corners.
\param[in] solid If nonzero, the polylines will be solid, otherwize dotted.
\param[in] bErase Whether to erase the foreground and use the background to fill the affected pixels,
this is only valid for the window handle associated with graphic window.
\param[in] bDelayUpdate if true, the result image will be stored on the back buffer and won't be presented to 
screen immediately. If next drawing function is invoked with bDelayUpdate being false, then the image 
will be presented combined with the result of previous drawing function. If the drawing result is not 
necessary to be seen immediately, passing true to this parameter will reduce the times of back buffer presenting, 
thus optimize the performance. This is only valid for window handle associated with graphic window.
\param[in] color Optional XOR line color. Default is grey (128,128,128)
*/
CoreExport void XORDottedPolyline( HWND hwnd, int count, IPoint2 *pts, int solid = 0,
	bool bErase = false, bool bDelayUpdate = false, COLORREF color = RGB(128, 128, 128));
/*! Draws a rectangle in XOR mode using a specified border width. */
CoreExport void XORRect(HDC hdc, RECT& r, int border = 1);
//! \brief Vista Aero related method.
/*! This method will tell whether the Vista Aero is turned on or not.
\return TRUE if Vista Aero is turned on, otherwize FALSE. On Window XP or previous Windows, it will always return FALSE.
*/
CoreExport BOOL IsVistaAeroEnabled();

//! \brief If the mouse cursor is currently hovering over a window
/*! This method will not differentiate whether the cursor is actually hovering over a window, or it's blocked by some other window.
It only does a point-rectangle containment test.
\param[in] hWnd The handle of the window.
\return TRUE if the mouse cursor is currently hovering the window, otherwise FALSE.
*/
CoreExport BOOL IsHovering(HWND hWnd);

//! \brief If the cursor position specified is currently confined in a window's rectangle
/*! This method will not differentiate whether the cursor is actually hovering over a window, or it's blocked by some other window.
It only does a point-rectangle containment test.
\param[in] hWnd The handle of the window.
\param[in] x The x-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
\param[in] y The y-coordinate of the cursor. The coordinate is relative to the upper-left corner of the client area.
\return TRUE if the mouse cursor is currently hovering the window, otherwise FALSE.
*/
CoreExport BOOL IsHovering(HWND hWnd, int x, int y);
/*! This sets the control whose window handle is passed to include the BS_AUTOCHECKBOX bit set. This creates a control which is same as a check box, except that a check mark appears in the check box when the user selects the box. The check mark disappears when the user selects the box next time. */
CoreExport void MakeButton2State(HWND hCtrl);
/*! This sets the control whose window handle is passed to include the BS_AUTO3STATE bit set. This creates a control which is the same as a three-state check box, except that the box changes its state when the user selects it. */
CoreExport void MakeButton3State(HWND hCtrl);
/*! Returns the checked state of the given control.
\li BST_CHECKED - Button is checked.
\li BST_INDETERMINATE - Button is greyed, which indicates an indeterminate state.
\li BST_UNCHECKED - Button is unchecked.
 */
CoreExport int GetCheckBox(HWND hw, int id);
/*! Set the check state of a radio button or check box to checked or unchecked. */
CoreExport void SetCheckBox(HWND hw, int id, BOOL b);

//! \brief Detects whether the file specified by the full, absolute filename exists, considering the missing file cache 
/*! Detects whether specified file exists, optionally excluding files that are directories.
Note that if the filename is a guid string, the guid string is converted to a AssetUser, and then 
the AssetUser's filename is tested to see if it exists.
\param file The null-terminated filename or guid string
\param allowDirectory If true, returns TRUE if the filename is found and is either a file or a directory. If false,
returns TRUE if the filename is found and is a file but not a directory.
\return TRUE if the file exists.
*/
CoreExport BOOL DoesFileExist(const MCHAR* file, bool allowDirectory = true);

//! \brief Detects whether the file specified by the full, absolute filename exists, potentially considering the missing file cache 
/*! Detects whether specified file exists, optionally excluding files that are directories.
Note that if the filename is a guid string, the guid string is converted to a AssetUser, and then
the AssetUser's filename is tested to see if it exists.
\param file The null-terminated filename or guid string
\param allowDirectory If true, returns TRUE if the filename is found and is either a file or a directory. If false,
returns TRUE if the filename is found and is a file but not a directory.
\param ignoreMissingPathCache If true, testing does not consider whether the file's path may be in the missing path cache.
\return TRUE if the file exists.
*/
CoreExport BOOL DoesFileExist(const MCHAR* file, bool allowDirectory, bool ignoreMissingPathCache);

//! \brief Detects whether the directory specified by the full, absolute filename exists, considering the missing file cache 
/*! Detects whether specified directory exists.
Note that if the filename is a guid string, the guid string is converted to a AssetUser, and then 
the AssetUser's filename is tested to see if it exists.
\param dir The null-terminated directory or guid string
\return TRUE if the directory exists.
*/
CoreExport BOOL DoesDirectoryExist(const MCHAR* dir);

//! \brief Detects whether the directory specified by the full, absolute filename exists, potentially considering the missing file cache 
/*! Detects whether specified directory exists.
Note that if the filename is a guid string, the guid string is converted to a AssetUser, and then
the AssetUser's filename is tested to see if it exists.
\param dir The null-terminated directory or guid string
\param ignoreMissingPathCache If true, testing does not consider whether the file's path may be in the missing path cache.
\return TRUE if the directory exists.
*/
CoreExport BOOL DoesDirectoryExist(const MCHAR* dir, bool ignoreMissingPathCache);

/*! Retrieves a file's or directory's creation, last access and last modification times.
\param [in] filePath - absolute path to the file 
\param [out] creationTime - pointer to a FILETIME structure to receive the date and 
time the file or directory was created. This parameter can be NULL.
\param [out] lastAccessTime - pointer to a FILETIME structure to receive the date and 
time the file or directory was last accessed. This parameter can be NULL.
\param [out] lastWriteTime- pointer to a FILETIME structure to receive the date and 
time the file or directory was last modified. This parameter can be NULL.
\return True if the operation was successful, false otherwise. */
CoreExport bool GetFileTime(const MCHAR* filePath, FILETIME* creationTime, FILETIME* lastAccessTime, FILETIME* lastWriteTime);

/*! Converts a FILETIME value to a string representation.
The FILETIME is converted into local time and date, and then to a string.
The format of the time and date string is given by the default user locale.
\param fileTime - the file time to convert to string
\return A string representation of the file time, or empty string if an error occurred. */
CoreExport MSTR FileTimeToString(const FILETIME& fileTime);
/*! Returns the number of adjacent color bits for each pixel in use by the desktop. */
CoreExport int GetBitsPerPixel();

// Delete superfluous zeros from float string: 1.2300000 -> 1.23
/*! Delete superfluous zeroes from a string representing a floating point value. For example, "1.2300000" becomes "1.23". */
CoreExport void StripTrailingZeros(MCHAR* buf);

template<class T> void LimitValue( T& value, T min, T max )
{
	if ( value < min ) 
		value = min;
	if ( value > max )
		value = max;
}

//! Safely casts double to float. Valid flag will indicate overflow
inline float Dbl2Flt(double val, BOOL *valid = NULL)
{
	if ( val < 0.0f )
	{
		if ( val < -FLT_MAX )
		{
			if (valid) *valid = FALSE;
			return -FLT_MAX;
		}
		if ( val > -FLT_MIN )
		{
			if (valid) *valid = FALSE;
			return -FLT_MIN;
		}
		if (valid) *valid = TRUE;
		return (float)val;
	}

	if ( val > FLT_MAX )
	{
		if (valid) *valid = FALSE;
		return FLT_MAX;
	}
	if ( val < FLT_MIN && val != 0.0 )
	{
		if (valid) *valid = FALSE;
		return FLT_MIN;
	}
	if (valid) *valid = TRUE;
	return (float)val;
}

//! Safely casts double to int. Valid flag will indicate overflow
inline int Dbl2Int(double val, BOOL* valid = NULL)
{
	if ( val > INT_MAX )
	{
		if (valid) *valid = FALSE;
		return INT_MAX;
	}
	if ( val < INT_MIN )
	{
		if (valid) *valid = FALSE;
		return INT_MIN;
	}
	if (valid) *valid = TRUE;
	return (int)val;
}

#define MAKEPOINT( lparam, pt ) { pt.x = (short)LOWORD(lparam); pt.y = (short)HIWORD(lparam); }

/*
	For window messages where lParam contains the coordinate of the cursor (such as WM_MOUSEMOVE, WM_LBUTTONDOWN), 
	do not use the LOWORD or HIWORD macros to extract the x- and y- coordinates of the cursor position because 
	these macros return incorrect results on systems with multiple monitors. 
	Systems with multiple monitors can have negative x- and y- coordinates, and LOWORD and HIWORD treat the 
	coordinates as unsigned quantities.

	Use the following code to obtain the horizontal and vertical position:

	xPos = GET_X_LPARAM(lParam); 
	yPos = GET_Y_LPARAM(lParam); 
*/

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif


// The following two functions extend list boxes. Set the list box to be
// owner draw and then call these two methods in response to the
// WM_MEASUREITEM and WM_DRAWITEM messages.
// 

// Flags to pass to CustListDrawItem
#define CUSTLIST_DISABLED		(1<<0)		// Text is gray
#define CUSTLIST_MED_DISABLED		(1<<1)		// Test is darker gray
#define CUSTLIST_SEPARATOR		(1<<2)		// Draws a separator instead of text
#define CUSTLIST_DBL_SERPARATOR	(1<<3)		// Draw a double line separator
#define CUSTLIST_RED			(1<<4)		// Text is red

//! \brief Calculate Item text extents using GetTextExtendPoint 
/*! This method will calculate in precise dimension base on control content
\param[in] hList The handle of the listbox/combobox list.
\param[in] wParam - Unused
\param[in] lParam - Pointer to the MEASUREITEMSTRUCT.
*/
CoreExport void CustListMeasureItem(HWND hList,WPARAM wParam, LPARAM lParam);
//! \brief Calculate Item text extents using TextMetrics 
/*! This method will calculate in average/maximum character size based on the control content.
Note that it will cache the values retrieved and will not make any further GDI call.
Also, it will not calculate the width extent otherwise specified and that calculation is the max size possible
\param[in] hList The handle of the listbox/combobox list.
\param[in] wParam - Unused
\param[in] lParam - Pointer to the MEASUREITEMSTRUCT.
\param[in] calcWidth (optional) - Width calculation necessary.
*/
CoreExport void FastListMeasureItem(HWND hList, WPARAM wParam, LPARAM lParam, bool calcWidth = false);
//! \brief Draw a list item 
/*! This method will Draw a list item based on previously fetched measurement using DrawText
Note that it will cache the values retrieved and will not make any further GDI call.
Also, it will not calculate the width extent otherwise specified and that calculation is the max size possible
\param[in] hList The handle of the listbox/combobox list.
\param[in] wParam - Unused
\param[in] lParam - Pointer to the DRAWITEMSTRUCT.
\param[in] flags  - Possible values (CUSTLIST_DISABLED/MED_DISABLED/RED/SEPARATOR/DBL_SERPARATOR)
*/
CoreExport void CustListDrawItem(HWND hList,WPARAM wParam, LPARAM lParam,DWORD flags);

// MAX extended message box functionality
//! Add "Hold" button
#define MAX_MB_HOLD				0x0001
//! Add "Do not show this message again" check box
#define MAX_MB_DONTSHOWAGAIN	0x0002

//! \brief Provides an extended message box functionality, This is used to support message dialogs with a 'Hold' or a 'Help' button and/or a 'Do not show this message again' check box.
/*! \deprecated MaxMsgBox has been deprecated, please use MaxSDK::QmaxMessageBox() or MaxSDK::MaxMessageBox instead. These methods provide a more complete extended message box functionality 
	that better matches the Win32 MessageBox functionality.
	The first four parameters correspond to the Win32 MessageBox method parameters, but not all Win32 MessageBox functionality is supported.
	The last two optional args add the functionality listed above - exType is used for adding the additional buttons, and exRet is used for getting the extra return info.  
	\param hWndParent - The parent window handle
	\param lpText - The main message
	\param lpCaption - The caption or title for the message window
	\param type - The type of message box. The supported types are:
	- MB_YESNO
	- MB_OK
	- MB_YESNOCANCEL
	- MB_OKCANCEL
	The following Win32 MessageBox types are not supported and they will be treated like MB_OKCANCEL:
	- MB_ABORTRETRYIGNORE
	- MB_RETRYCANCEL
	- MB_CANCELTRYCONTINUE
	The following Win32 MessageBox flags are not supported and will be ignored:
	- MB_DEFBUTTON*
	- MB_TASKMODAL
	- MB_SYSTEMMODAL
	- MB_NOFOCUS 
	- MB_SETFOREGROUND
	- MB_TOPMOST
	- MB_RIGHT 
	- MB_RTLREADING 
	The MB_HELP flag is supported, albeit in a manner different than Win32 MessageBox. If specified and 'exRet' is not null, a Help button is displayed and clicking the 
	Help button causes 3ds Max Help to be displayed using the help topic id that is passed through 'exRet'. The MB_HELP flag is ignored if MAX_MB_HOLD is specified in 'exType'
	or if a type of MB_YESNOCANCEL is specified.
	\param exType - Used for adding additional UI elements. The supported bit field values are:
	- MAX_MB_HOLD - display a 'Hold' button. This flag is ignored if a type of MB_YESNOCANCEL is specified. If the Hold button is displayed, it can be selected as the default
	  button by specifying Win32 MessageBox flag MB_DEFBUTTON4 in type.
	- MAX_MB_DONTSHOWAGAIN - display a 'Do not show this message again' check box
	\param exRet - Used for specifying the help file topic id and for returning extra info:
	- if MAX_MB_DONTSHOWAGAIN is specified as part of 'exType', on return the MAX_MB_DONTSHOWAGAIN bit will or will not be set in 'exRet' based on check box setting.
	- if MAX_MB_HOLD is specified as part of 'exType', if the button is pressed the MAX_MB_HOLD bit will set in 'exRet' and a value of IDOK is returned.
	\note See "https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-messagebox" for a description of the Win32 MessageBox method parameters.
	\note The MaxMessageBox methods provide a more complete extended message box functionality that better matches the Win32 MessageBox functionality, and should be used 
	instead of MaxMsgBox. The MaxMsgBox method will be deprecated in the future.
	*/
CoreExport INT_PTR MaxMsgBox(HWND hWndParent, LPCMSTR lpText, LPCMSTR lpCaption, UINT type, UINT exType=0, DWORD *exRet=NULL);

namespace MaxSDK
{
	/*!
	\copydoc QmaxMessageBox(QWidget* parent, const QString& text, const QString& caption, unsigned int type, unsigned int exType, DWORD *exRet)
	*/
	UtilExport int MaxMessageBox(HWND parent, LPCMSTR text, LPCMSTR caption, unsigned int type, unsigned int exType = 0, DWORD *exRet = nullptr);
}

/**********************************************************************
 *
 * alpha blended icon support...
 *
 **********************************************************************/
#include "plugapi.h"

enum LoadMAXIconErrors
{
	LMI_Ok,
	LMI_ResourceNotFound,
	LMI_ResourceLoadFailed,
	LMI_ImageAndMaskNotCompatible,
};

CoreExport LoadMAXIconErrors LoadMAXIcon(HINSTANCE hInstance,
								LPCMSTR    resID,
								LPCMSTR    resMaskID, 
								COLORREF   bkColor,
								HIMAGELIST imageList,
								int        imageIndex,
								int        preMultAlpha = TRUE);

// returns index of first image into existing imageList
CoreExport int LoadMAXIconFromBMI(LPBITMAPINFOHEADER biImage,
						    LPBITMAPINFOHEADER biMask,
						    COLORREF bkColor,
						    const MCHAR* pFilePrefix,
						    int preMultAlpha = TRUE,
						    HIMAGELIST* pImageList = NULL);

CoreExport BITMAPINFOHEADER *LoadBitmapFromFile(const MCHAR *filename);

CoreExport void DrawMAXIcon(HDC hDC, Rect& r, HIMAGELIST hList32, HIMAGELIST hList16, int index);

// Compute a good color to use for drawing XOR lines over a particular background color
/*! Returns a suggested color to use when drawing XOR lines over a specified background color. */
CoreExport COLORREF ComputeXORDrawColor(COLORREF bkgColor);
// Compute a good color to use for drawing XOR lines over a viewport
/*! Returns a suggested color to use when drawing XOR lines over a viewport. The color is suggested based on the background color settings. */
CoreExport COLORREF ComputeViewportXORDrawColor();


/**********************************************************************/

/*! \brief Dialog resizing and positioning. A generic class for resizing a dialog.
Includes functions to save and restore the initial size and position
of the dialog. */

/*!
This class makes it easy to add resizability to a dialog. You'll need
one instance of this class for each instance of a dialog. Set up
various parameters during your dialog's WM_INITDIALOG processing,
then let this class handle all WM_SIZE events.

Include library: core.lib

HOW TO USE THIS CLASS:

1. Create either a static instance in your DlgProc, or a live instance
   in the object that is driving the dialog.
2. In your WM_INITDIALOG handling, call Initialize().
3. Optionally, call SetMinimumDlgSize(). However, the default minimum size
   for a dialog is it's size at the time Initialize() is called.
4. For each control in your dialog, call SetControlInfo(); you only need 
   to do this if you want to override the default behavior for the control.
   For most, the default is to be locked to the top left corner, with a
   fixed size. Some controls may default to resizing; SysListView32 has
   some specialized behaviors you may not need to override.
5. Process the WM_SIZING message, and call Process_WM_SIZING().
6. Process the WM_SIZE message, and call Process_WM_SIZE().

**********************************************************************/

class DialogItemSizeData: public MaxHeapOperators
{
public:
	HWND hwnd;
	RECT rect;
	DWORD flags;

	DialogItemSizeData(HWND h, DWORD f = 0)
		:	hwnd(h), flags(f)
	{
		WINDOWPLACEMENT wpControl;
		wpControl.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hwnd, &wpControl);
		rect = wpControl.rcNormalPosition;
	}
	// Compiler-generated destructor, copy constructor, and assignment 
	// operator should all be fine.
private:
	// Ensure no default constructor is generated or used.
	DialogItemSizeData();
};

static BOOL CALLBACK GetInitialPositionECP(HWND hwnd, LPARAM lParam);

class DialogResizer: public MaxHeapOperators
{
public:
	DialogResizer() : mhDlg(NULL)
	{
		mMinDlgSize.left = mMinDlgSize.top = 0;
		mMinDlgSize.right = mMinDlgSize.bottom = MaxSDK::UIScaled(50);
		mOriginalClientRect.left = mOriginalClientRect.top = 0;
		mOriginalClientRect.right = mOriginalClientRect.bottom = MaxSDK::UIScaled(50);
	}
	// Compiler-generated destructor, copy constructor, and assignment 
	// operator should all be fine.

	CoreExport void Initialize(HWND hDlg);
	CoreExport void SetMinimumDlgSize(LONG wid, LONG ht);

	// If you have a control you want to have fixed width and height, and locked to 
	// top left, you do not have to call SetControlInfo, as that is the default 
	// behavior.
	enum PositionControl
		{ kLockToTopLeft=1, kLockToTopRight, kLockToBottomLeft, kLockToBottomRight, kPositionsOnly=0xff };
	enum ControlFlags
		{ kDefaultBehavior=0, kWidthChangesWithDialog=1<<8, kHeightChangesWithDialog=1<<9 };
	CoreExport void SetControlInfo(int resID, PositionControl pos, DWORD flags = kDefaultBehavior);
	CoreExport void SetControlInfo(HWND hwnd, PositionControl pos, DWORD flags = kDefaultBehavior);

	CoreExport void Process_WM_SIZING(WPARAM wParam, LPARAM lParam);
	CoreExport void Process_WM_SIZE(WPARAM wParam, LPARAM lParam);

	// Static methods to let you save and restore the size and position of your dialog
	// (whether it's resizable or not).
	// If the section name is not specified, it will default to
	// [DialogResizer_SizeAndPositions].
	// If the ini filename is not specified, it will default to the main application ini
	// file, typically 3dsmax.ini.
	CoreExport static void SaveDlgPosition(HWND hDlg, const MCHAR* keyname, 
				const MCHAR* section = NULL, const MCHAR* inifn = NULL);
	CoreExport static void LoadDlgPosition(HWND hDlg, const MCHAR* keyname, 
				const MCHAR* section = NULL, const MCHAR* inifn = NULL);

	friend BOOL CALLBACK GetInitialPositionECP(HWND hwnd, LPARAM lParam);

private:
	MaxSDK::Array<DialogItemSizeData> mControls;
	RECT mMinDlgSize;
	RECT mOriginalClientRect;
	HWND mhDlg;
};

/// \defgroup LocaleHandlerAPIs Locale Handler APIs
//! \brief 3ds Max provides APIs that help render correctly and consistently locale (region) specific information such as floating point numbers, date and time.
//! Use the GetUserLocale() API to facilitate displaying floating point, date and time values in 3ds Max's user interface. 
//! These need to respect the current locale settings, i.e. the current user's regional preferences. 
//! Use GetCNumericLocale() or class MaxLocaleHandler to read and write data to text files in a locale independent format, such as a minimal 
//! ANSI conforming environment for C translation, known as "C" locale

///@{
//! \brief Helper class for setting locale temporarily.
/*! This helper class switches the locale settings to the desired 
  value for the current scope (such as a function call), and restores the 
  previous locale setting at the end of that scope.
  \code
  // Setting a C-numeric locale ensures that a floating point number is written with 
  // a "." as decimal separator, regardless of the regional settings currently in effect
  MaxLocaleHandler numericLocaleHelper(LC_NUMERIC, _M("C"));
  TextFile::ReaderWriter file;
  BinaryStreamMemory buffer(malloc(1), 0, realloc, free);
  file.Open(&buffer, BinaryStream::ParseStreamMode(_T("rw")), 0, TextFile::Unchanged, false)
  file.Printf(_T("%5.2f\n"), 100.321); // should write 100.321 even when the current locale settings are using "," as decimal separator
  file.Close();
  \endcode
  Use an instance of this class when you need to write or read data such as floating point, 
  date and time values in a locale independent way, but the APIs do not take a locale parameter.
*/
class MaxLocaleHandler: public MaxHeapOperators 
{
public:
	//! \brief Constructor
	//! \param[in] category - The category affected by locale. See _tsetlocale in MSDN help for more info
	//! \param[in] localeSetting - The locale name. See _tsetlocale in MSDN help for more info
	UtilExport MaxLocaleHandler(int category = LC_ALL, const MCHAR* localeSetting = _M("C"));

	//! \brief Destructor, resets locale setting to original value
	UtilExport ~MaxLocaleHandler();

   //! \brief Resets locale setting to original value, before the object goes out of scope.
   UtilExport void RestoreLocale();

private:
	int m_category;
    MSTR m_oldLocale;
};

//! \brief Returns a C numeric locale specification.
/*! This method should be used when writing or reading locale independent data to or from a file.
  Floating point values need to be stored in a locale independent representation in 
  order to allow them to be used on computers using different locales or regional settings,
  i.e. they must always be written, read or formatted using dot ('.') as decimal separator.
  The locale object returned by this function can be used with APIs that use a locale 
  parameter to interpret the data they read, write or format. 
  Examples of such APIs are: _stscanf_l(), _sntprintf_s_l(), TSTR::printf_l(), BaseTextReader::Printf_l(), etc

..\code
  // Setting a C-numeric locale ensures that a floating point number is written with
  // a "." as decimal separator, regardless of the regional settings currently in effect
  TextFile::ReaderWriter file;
  BinaryStreamMemory buffer(malloc(1), 0, realloc, free);
  file.Open(&buffer, BinaryStream::ParseStreamMode(_T("rw")), 0, TextFile::Unchanged, false)
  file.Printf_l(_T("%5.2f\n"), GetCNumericLocale(), 100.321); // should write 100.321 even when the current locale settings are using "," as decimal separator
  file.Close();
..\endcode
  
  When you need to read and write date and time values in a local independent way, use class MaxLocaleHandler.
  \return A cached C-numeric locale
*/
UtilExport const _locale_t& GetCNumericLocale();

//! \brief Returns the user locale cached at the start of 3ds Max. 
/*! This method should be used when displaying data whose representation is dependent
  on regional settings in the user interface. For example, when displaying floating point
  numbers in the UI while the current regional settings are French, the user expects the
  decimal separator to be ','.

..\code
  // Rendering a floating point value into a string buffer that will be displayed in the UI
  TSTR buf;
  buf.printf_l(_T("%5.2f\n"), GetUserLocale(), 100.321); // should write 100,321 when the current locale settings are using "," as decimal separator
..\endcode

  When you need to read and write date and time values in a local independent way, use class MaxLocaleHandler, or GetCNumericLocale().
  \return The cached user locale
*/
UtilExport const _locale_t& GetUserLocale();
///@}

/**********************************************************************
 *
 * ToolTip Extender
 *
 **********************************************************************/

class ToolTipExtenderImpl; //! Internal use only. Hidden internal implementation for ToolTipExtender

/*! \sa  Class ToolTipExtender.\n\n
\par Description:
This class allows a developer to add tooltips to one or more window controls.  Controls of all types are supported.
One extender object can support all tooltips for a dialog, it's not necessary to create one extender per tooltip.
This is useful for control types which do not have native tooltip support.
*/
class ToolTipExtender : public MaxHeapOperators
{
	public:
		/*! \brief Class constructor. No heap memory is allocated until tooltips are actually set. */
		CoreExport ToolTipExtender();

		/*! \brief Class destructor. Removes all tooltips and releases heap memory. */
		CoreExport virtual ~ToolTipExtender();

		/*! \brief Sets a tooltip for an arbitrary UI control.
		Calling this twice for a single control is safe, and will remove the first tooltip.
		Calling this for multiple controls is allowed, as this class supports an unlimited number of tooltips.
		Note that tooltip resources are not automatically freed when a control is destroyed.
		Resources are freed when the extender is destroyed, or by calling RemoveToolTip() or Reset().
		\param hWnd Handle to the window control.
		\param toolTipIn ToolTip string. The string is copied, so the caller may safely delete this. */
		CoreExport virtual void SetToolTip( HWND hWnd, const MSTR& toolTipIn );

		/*! \brief Gets the tooltip for an arbitrary UI control.
		This only functions for tooltips set with this class. Returns an empty string if no tooltip is set.
		\param hWnd Handle to the window control.
		\param toolTipOut ToolTipOutput string. The method will resize the string as necessary. */
		CoreExport virtual void GetToolTip( HWND hWnd, MSTR& toolTipOut );

		/*! \brief Removes the tooltip from an arbitrary UI control.
		This only functions for tooltips set with this class.
		Note that tooltip resources are not automatically freed when a control is destroyed.
		Resources are freed when the extender is destroyed, or by calling this function or Reset().
		\param hWnd Handle to the window control. */
		CoreExport virtual void RemoveToolTip( HWND hWnd );

		/*! \brief Removes all tooltips and releases heap memory. */
		CoreExport virtual void RemoveToolTips();

		/*! \brief Returns a handle to the operating system's ToolTip object.
		This provides access to extended tooltip functionality.
		See the operating system's documentation for TOOLTIPS_CLASS. */
		CoreExport virtual HWND GetToolTipHWND(); // ToolTips window object, of type TOOLTIPS_CLASS

	protected:
		//! Internal use only.  This implementation object is created when the first tooltip is set.
		ToolTipExtenderImpl* impl;
};

/**********************************************************************
 *
 * Utility Functions
 *
 **********************************************************************/

namespace MaxSDK
{

//! \brief Multiple character search for ComboBoxes
/*! This method will select an entry in the ComboBox based on the
multiple key entries over a set time period.  The functionality mimics the 
standard windows ListView behavior, including the documented one second
time period for key entries.  It is intended to be used in conjunction with
the WM_CHAR message in the ComboBox's callback.

Example usage:
\code
	case WM_CHAR:
		return MaxSDK::SearchComboBox( hWnd, (MCHAR)wParam, _M(" "));
\endcode

\param[in] hWnd Window handle of ComboBox
\param[in] key Character entered by user
\param[in] szTrim Optional parameter.  Default is NULL.  Any leading characters in 
the ComboBox entries that are contained in szTrim will be removed prior to 
comparing to the search string.
\return true if match found, false if not
*/
CoreExport bool SearchComboBox( HWND hWnd, MCHAR key, const MCHAR* szTrim = NULL );
//! \brief Multiple character search for ListBoxes
/*! This method will select an entry in the ListBox based on the
multiple key entries over a set time period.  The functionality mimics the 
standard windows ListView behavior, including the documented one second
time period for key entries.  It is intended to be used in conjunction with
the WM_CHAR message in the ListBox's callback.

Example usage:
\code
	case WM_CHAR:
		return MaxSDK::SearchListBox( hWnd, (MCHAR)wParam, _M(" "));
\endcode

\param[in] hWnd Window handle of ListBox
\param[in] key Character entered by user
\param[in] szTrim Optional parameter.  Default is NULL.  Any leading characters in 
the ListBox entries that are contained in szTrim will be removed prior to 
comparing to the search string.
\return true if match found, false if not
*/
CoreExport bool SearchListBox( HWND hWnd, MCHAR key, const MCHAR* szTrim = NULL );

/*! \remarks Returns the default maximum width, in pixels, for tooltip windows in Max.
Tooltips wider than the maximum are broken into multiple lines. 
*/
CoreExport int GetDefaultToolTipMaxWidth();

};

/**
 * Returns a Listbox item's string as a MSTR.
 *
 * \param[in] hDlg            The hWnd of the Dialog the Listbox is in 
 * \param[in] nIDDlgItem      The id of the Listbox in the dialog.
 * \param[in] nListboxItem    The index of the Listbox item
 * \return the Listbox item's string
 *
 * If the ListBox is not found, an empty string is returned
*/
UtilExport MSTR GetListBoxItemText(HWND hDlg, int nIDDlgItem, int nListboxItem);

/**
 * Returns a Listbox item's string as a MSTR.
 *
 * \param[in] hDlg            The hWnd of the Dialog the Listbox is in 
 * \param[in] nIDDlgItem      The id of the Listbox in the dialog.
 * \param[in] nListboxItem    The index of the Listbox item
 * \param[out] itemText       The Listbox item's string 
 *
 * If the ListBox is not found, an empty string is returned
*/
UtilExport void GetListBoxItemText(HWND hDlg, int nIDDlgItem, int nListboxItem, MSTR& itemText);

/**
 * Returns a Listbox item's string as a MSTR.
 *
 * \param[in] hListBoxWnd     Window handle of ListBox 
 * \param[in] nListboxItem    The index of the Listbox item
 * \return the Listbox item's string
 *
 * If the ListBox is not found, an empty string is returned
*/
UtilExport MSTR GetListBoxItemText(HWND hListBoxWnd, int nListboxItem);

/**
 * Returns a Listbox item's string as a MSTR.
 *
 * \param[in] hListBoxWnd     Window handle of ListBox 
 * \param[in] nListboxItem    The index of the Listbox item
 * \param[out] itemText       The Listbox item's string 
 *
 * If the ListBox is not found, an empty string is returned
*/
UtilExport void GetListBoxItemText(HWND hListBoxWnd, int nListboxItem, MSTR& itemText);

/**
 * Returns a Combobox item's string as a MSTR.
 *
 * \param[in] hDlg            The hWnd of the Dialog the Combobox is in 
 * \param[in] nIDDlgItem      The id of the Combobox in the dialog.
 * \param[in] nComboboxItem   The index of the Combobox item
 * \return the Combobox item's string
 *
 * If the Combobox is not found, an empty string is returned
*/
UtilExport MSTR GetComboBoxItemText(HWND hDlg, int nIDDlgItem, int nComboboxItem);

/**
 * Returns a Combobox item's string as a MSTR.
 *
 * \param[in] hDlg            The hWnd of the Dialog the Combobox is in 
 * \param[in] nIDDlgItem      The id of the Combobox in the dialog.
 * \param[in] nComboboxItem   The index of the Combobox item
 * \param[out] itemText       The Combobox item's string 
 *
 * If the Combobox is not found, an empty string is returned
*/
UtilExport void GetComboBoxItemText(HWND hDlg, int nIDDlgItem, int nComboboxItem, MSTR& itemText);

/**
 * Returns a Combobox item's string as a MSTR.
 *
 * \param[in] hComboBoxWnd    Window handle of ComboBox 
 * \param[in] nComboboxItem   The index of the Combobox item
 * \return the Combobox item's string
 *
 * If the Combobox is not found, an empty string is returned
*/
UtilExport MSTR GetComboBoxItemText(HWND hComboBoxWnd, int nComboboxItem);

/**
 * Returns a Combobox item's string as a MSTR.
 *
 * \param[in] hComboBoxWnd    Window handle of ComboBox 
 * \param[in] nComboboxItem   The index of the Combobox item
 * \param[out] itemText       The Combobox item's string 
 *
 * If the Combobox is not found, an empty string is returned
*/
UtilExport void GetComboBoxItemText(HWND hComboBoxWnd, int nComboboxItem, MSTR& itemText);

/**
 * Returns an window's text as a MSTR.
 *
 * \param[in] hDlg            The hWnd of the Dialog the window is in 
 * \param[in] nIDDlgItem      The id of the window in the dialog.
 * \return the window's string
 *
 * If the window is not found, an empty string is returned
*/
UtilExport MSTR GetWindowText(HWND hDlg, int nIDDlgItem);

/**
 * Returns an window's text as a MSTR.
 *
 * \param[in] hDlg            The hWnd of the Dialog the window is in 
 * \param[in] nIDDlgItem      The id of the window in the dialog.
 * \param[out] windowText     The window's text 
 *
 * If the window is not found, an empty string is returned
*/
UtilExport void GetWindowText(HWND hDlg, int nIDDlgItem, MSTR& windowText);

/**
* Returns an window's text as a MSTR.
 *
 * \param[in] hWnd            The Window handle
 * \return the window's string
 *
 * If the window is not found, an empty string is returned
*/
UtilExport MSTR GetWindowText(HWND hWnd);

/**
* Returns an window's text as a MSTR.
 *
 * \param[in] hWnd            The Window handle
 * \param[out] windowText     The window's text 
 *
 * If the window is not found, an empty string is returned
*/
UtilExport void GetWindowText(HWND hWnd, MSTR& windowText);
