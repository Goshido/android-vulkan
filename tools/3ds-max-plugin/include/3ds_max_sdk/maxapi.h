// Copyright 2013 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//

#pragma once

#include "maxheap.h"
#include <ole2.h>
#include "cmdmode.h"
#include "sceneapi.h"
#include "rtclick.h"
#include "evuser.h"
#include "maxcom.h"
#include "plugin.h"
#include "log.h"
#include "ActionTable.h"
#include "iTreeVw.h"
#include "iRenderPresets.h"
#include "excllist.h"
#include "DefaultActions.h"
#include "RendType.h"
#include "AssetManagement/AssetType.h"
#include "geom/box3.h"
#include "InodeTransformModes.h"
#include "TabTypes.h" // cannot forward declare IntTab
#include "GetCOREInterface.h"
#include "ref.h" // for RefMakerHandle
#include "RollupConstants.h"
#include "geom/ipoint3.h"
#include "MaxDirectories.h"
#include "maxscript/ScriptSource.h"

// forward declarations
class ViewParams;
class ModContext;
class HitData;
class HitLog;
class CtrlHitLog;
class MtlBase;
class Mtl;
class Texmap;
class PickNodeCallback;
class PickAnimatableCallback;
class Renderer;
class IScanRenderer;
class RendParams;
class IRenderElementMgr;
class RendProgressCallback;
class Bitmap;
class BitmapInfo;
class Texmap;
class SoundObj;
class GenCamera;
class GenLight;
class NameTab;
class ShadowType;
class MacroRecorder;
class CommandLineCallback;
class GraphicsWindow;
struct SnapInfo;
class Material;
class IDerivedObject;
class DWORDTab;
class ILayerManager;

class IOsnapManager;
class IOsnapManager7;
class MouseManager;
class MtlBaseLib;
class Atmospheric;
class IRollupWindow;
class ITrackViewNode;
class DllDir;
class Effect;
class SpaceArrayCallback;
class IMenuManager;
class IColorManager;
class Modifier;
class Point2;
class BaseInterface;
class QWidget;

namespace MaxSDK
{
	namespace AssetManagement
	{
		class AssetUser;
	}
	namespace Graphics
	{
		class IRenderView2;
        class IPerViewportFilter;
	}

	class QmaxMainWindow;
} // namespace MaxSDK

/*! \defgroup renderSettingIDs Render Setting IDs
Used to define distinct sets of rendering parameters.
\sa Class Interface */
//!@{
enum RenderSettingID
{
	RS_Production = 0,	//!< The MAX Production renderer.
	RS_Draft,			//!< The MAX Draft renderer.
	RS_IReshade,		//!< The MAX Interactive Renderer.
	RS_MEdit			//!< 
};
//!@}

//! \brief The Clone type when copying nodes
enum CloneType
{
	NODE_COPY,
	NODE_INSTANCE,
	NODE_REFERENCE,
	NODE_NONE
};

/*! \defgroup DupNodeNameActions Duplicate Node Name Actions 
The following represent flags describing possible actions to be taken when merging 
nodes with names identical to the name of nodes already present in the scene.
These values can be passed to the Interface::MergeFromFile method's dupAction parameter 
and other methods as documented.
\see Interface::MergeFromFile, IObjXRefManager8::SetDupObjNameAction 
*/
//!@{
#define MERGE_DUPS_PROMPT 0    //!< Prompt user when duplicate node names are encountered
#define MERGE_DUPS_MERGE  1	   //!< Merge nodes and keep old ones in the scene
#define MERGE_DUPS_SKIP   2    //!< Do not merge nodes with duplicated names
#define MERGE_DUPS_DELOLD 3    //!< Merge nodes and delete old ones from the scene 
#define MERGE_LIST_NAMES  4    //!< Causes a list of objects to be placed into mrgList parameter of Interface::MergeFromFile. No actual merging will take place. The mergeAll parameter of Interface::MergeFromFile must be TRUE.
#define MERGE_DUPS_RENAME 1000 //!< Merge nodes and automatically rename them
//!@}

/*! \defgroup DupMtlNameActions Duplicate Material Name Actions
The following represent flags describing possible action to be taken when merging materials
with names identical to the names of materials already present in the scene.
These values can be passed to the Interface::MergeFromFile method's dupMtlAction parameter
and other methods as documented.
\see Interface::MergeFromFile, IObjXRefManager8::SetDupMtlNameAction
*/
//!@{
#define MERGE_DUP_MTL_PROMPT      MERGE_DUPS_PROMPT	//!< Prompt user when duplicate material names are encountered
#define MERGE_DUP_MTL_USE_MERGED  MERGE_DUPS_DELOLD	//!< Merge material and replace all scene materials with the same name with the merged material
#define MERGE_DUP_MTL_USE_SCENE   MERGE_DUPS_SKIP		//!< Don't merge material, use instead the scene material with the same name
#define MERGE_DUP_MTL_RENAME      MERGE_DUPS_RENAME	//!< Merge material and automatically rename them
//!@}

/*! \defgroup NodeReparentActions Node Reparent Actions
The following represent flags describing the possible actions to be taken regarding reparenting 
of nodes during a merge operation. These values can be passed to the Interface::MergeFromFile method's 
reparentAction parameter and other methods as documented.
\see Interface::MergeFromFile
*/
//!@{
#define MERGE_REPARENT_PROMPT     0    //!< Prompt user for reparenting action
#define MERGE_REPARENT_ALWAYS     1    //!< Maintain node hierarchies are maintained when merging nodes.
#define MERGE_REPARENT_NEVER      2    //!< Don't maintain node hierarchies are maintained when merging nodes. Merged nodes become children of the scene's root node.
//!@}

// ConvertFlagedNodesToXRefs() xflags bits
/*! \defgroup XRefObjectOptions Object Xref Options
The following represent options governing how scene entities are xrefed.
Note that some of these options should not be combined. For example, modifiers
should either be xrefed, merged or dropped, but not xrefed and dropped in the same time. 
\see IObjXRefManager8::AddXRefItemsFromFile, Interface7::ConvertFlagedNodesToXRefs
*/
//!@{
#define XREF_AS_PROXY				(1<<0) //!< The xref objects are created as proxies
#define XREF_XREF_MODIFIERS		0  //!< Modifiers will be part of the xrefed object, i.e. modifiers won't be accessible in the main scene's modifier panel, but will be updated on subsequent updates of the xrefed objects
#define XREF_DROP_MODIFIERS		(1<<1) //!< Modifiers will not be xrefed at all, i.e. the non-modified version of the source objects are xrefed
#define XREF_MERGE_MODIFIERS		(1<<2) //!< Modifiers will be merged, i.e modifiers will be accessible in the main scene's modifier panel, but won't get update on subsequent updates of the xrefed objects
#define XREF_MERGE_MANIPULATORS	(1<<3) //!< Manipulators will be merged rather than xrefed
#define XREF_SELECT_NODES			(1<<5) //!< The xrefed nodes are added to the current selection set
#define XREF_MERGE_CONTROLLERS	(1<<6) //!< Controllers will be merged rather than being xrefed
#define XREF_MERGE_MATERIALS		(1<<7) //!< Materials will be merged rather than xrefed
#define XREF_TRACKS_XREF			(1<<8) //!< Controllers will not be merged, will be xrefed. It means inheriting the source controller.This status can be switched to local, but not merged.
#define XREF_TRACKS_LOCAL			(1<<9) //!< Controllers will not be merged, will be local. It means supporting locally editing and reloading.This status can be switched to xrefed, but not merged.
#define XREF_MERGE_MERGE_DUPLICATEMATCHINGLAYERS	(1<<10) //!< duplicate matching Layers without an identical parent hierarchy when merging.
//!@}

/*! \defgroup propertySetOptions PropertySet Options
For internal reasons these must be bitflags
\sa Class Interface */
//!@{
#define PROPSET_SUMMARYINFO		0x01 //!< Corresponds to the File Properties Summary tab properties.
#define PROPSET_DOCSUMMARYINFO	0x02 //!< Corresponds to the File Properties Contents tab properties (Document Contents).
#define PROPSET_USERDEFINED		0x04//!< Corresponds to the File Properties Custom tab properties.
//!@}

// MAX specific Property IDs for the SummaryInformation Property Set
#define PIDSI_CONTENTSTATUS		0x00000020L  // VT_LPSTR
#define PIDSI_CONTENTTYPE		0x00000021L  // VT_LPSTR
#define PIDSI_CREATOR			0x00000022L  // VT_LPSTR
#define PIDSI_IDENTIFIER		0x00000023L  // VT_CLSID
#define PIDSI_LANGUAGE			0x00000024L  // VT_LPSTR
#define PIDSI_VERSION			0x00000025L  // VT_LPSTR

// Interface::Execute cmd values
#define  I_EXEC_ACTIVATE_TEXTURE    1  //	arg1:  Texmap *tx;	arg2:  Mtl *mtl; arg3:  int subnum;
#define  I_EXEC_DEACTIVATE_TEXTURE  2  //	arg1:  Texmap *tx;	arg2:  Mtl *mtl; arg3:  int subnum;
#define  I_EXEC_RENDER_MTL_SAMPLE 3    //   arg1:  MtlBase *mb; arg2: int size; arg3: UBYTE *image;  returns 0:fail, 1: success
#define  I_EXEC_GET_VPDISPLAY_DIB 4    // 	arg1: Texmap *tx; arg2: TimeValue t; arg3: TexHandleMaker *thm; arg4: BOOL mono; 
										//   arg5: int forceW; arg6: int forceH --	returns *BITMAPINFO

// added as complement to Interface::GetDir(), 020919  --prs.
#define  I_EXEC_SET_DIR 10             //   arg1:  int which; arg2: MCHAR *dir;

#define  I_EXEC_OFFSET_SPLINE   80     //   arg1:  INode *spline; arg2: float amount;
#define  I_EXEC_OFFSET_MEASURE  81     //   arg1:  INode *spline; arg2: Point3 *point; arg3: float *result;
#define  I_EXEC_TRIM_EXTEND     82     //   arg1:  INodeTab *all; arg2: INodeTab *act;
//#define  I_EXEC_REG_VIEWWINDOW2 85     //   arg1:  ViewWindow2* (deprecated)

//SS 3/11/2002: Added to max to support Substitute modifier
#define  I_EXEC_NEW_OBJ_XREF_DLG 83    //   arg1:  INodeTab *nodes; arg2: BOOL forceSingle;
#define	 I_EXEC_MODIFYTASK_INVALIDATEPANEL 86 //No Args

// russom - 08/27/03 - ECO 1146
#define I_EXEC_INVALIDATE_VIEWEXP 87	// arg1: ViewExp *vp -- set invalidate flag on ViewExp*; arg2: BOOL Update all view related objects

#define I_EXEC_SET_NUDGE 88	// arg1: BOOL bEnableNude -- Enable/Disable viewport arrow key nudge

#define I_EXEC_COUNT_MTL_SCENEREFS  0x2001 // arg1 Mtl *mt: returns number of refs from scene 

// Interface::Execute return values
#define I_EXEC_RET_NULL_NODE    1
#define I_EXEC_RET_NULL_OBJECT  2
#define I_EXEC_RET_NOT_SPLINE   3
#define I_EXEC_RET_OFFSET_FAIL  4

/*! \brief Creates unique names.
 *
 * Use this class to generate unique names for various scene entities such as materials, layers, etc.
 * The generated names consist of a root string ("Layer") and a numeric suffix (001, 002, etc), 
 * as in "Layer001", "Layer002", etc.
 * This class is most useful when client code has a list of root strings 
 * that can be used to initialize the name maker before asking it to create new names.
 * Although this class can be used to create node names that are unique within the
 * current scene, method Interface::MakeNameUnique() delivers equivalent results
 * to this class with identical performance.
 * Clients can use the Interface::NewNameMaker() factory method to create an instance of this class. 
 * All methods of this class are implemented by the system.  
 */
class NameMaker: public MaxHeapOperators 
{
	public:
		/** \brief Creates a unique version of a given name 
		 * Note that gaps in the numeric suffixes are not filled in. 
		 * For example if "name001" and "name003" already exist, the next 
		 * unique name that has the root "name" will be "name004" as opposed to "name002".
		 * The number of digits used in the numeric suffix can be set via Interface13::SetNameSuffixLength().
		 * \param [in, out] name The name that is to be made unique 
		 */
		virtual void MakeUniqueName(MSTR &name)=0;

		/** \brief Sets the root name used by the name maker for generating new unique names.
		 * \param [in] name The name to be added. Numeric suffixes are
		 * stripped off before the name is added to the name maker.
		 */
		virtual void AddName(MSTR &name) = 0;
		/** \brief Verifies if a root name exists already
		 * \param name [in] name The root name to look for. 
		 * \return TRUE if the root name is known to the name maker, FALSE otherwise
		 */
		virtual BOOL NameExists(MSTR& name) = 0;
		virtual ~NameMaker() {}
};

/*! \defgroup vieportDegradationFlags Viewport Degradation Flags
\brief Viewport degradation flags used by Interface7::RedrawViewportsNow */
//!@{
#define VP_DEFAULT_RENDER	0x0000	//!< same as REDRAW_INTERACTIVE
#define VP_DONT_RENDER		0x0001
#define VP_DONT_SIMPLIFY	0x0002	//!< same as REDRAW_NORMAL
#define VP_START_SEQUENCE	0x0004	//!< same as REDRAW_BEGIN
#define VP_END_SEQUENCE		0x0008	//!< same as REDRAW_END
#define VP_SECOND_PASS		0x0010	//!< for when both back- and fore-ground is rendered
//!@}

/*! \defgroup viewportRenderingLevel Viewport Rendering Levels
\brief These values are used by Interface7::SetActiveViewportRenderLevel(int level).
The values correspond to the shading options in the Viewport right-click menu */
//!@{
#define SHADE_LEVELS		10
enum ShadeType {
	SMOOTH_HIGHLIGHT,	//!< Smooth shading with highlights
	SMOOTH,				//!< Smooth shading without highlights
	FACET_HIGHLITE,		//!< Faces\\facets with highlights
	FACET,				//!< Faces\\facets without highlights
	CONSTANT,			//!< Flat shading
	HIDDENLINE,			//!< 
	LITE_WIREFRAME,		//!< Lit wireframe shading
	Z_WIREFRAME,		//!< Unused
	WIREFRAME,			//!< Wireframe shading
	BOX					//!< Displays objects as bounding boxes
};
//!@}

//JH 05/06/98 
// VIEW_OTHER must be last, since "other" types are then numbered consecutively!!!
// And the order can't be changed, or old MAX files won't load properly DB 11/98
enum ViewType { VIEW_LEFT,VIEW_RIGHT,VIEW_TOP,VIEW_BOTTOM,VIEW_FRONT,VIEW_BACK, 
	VIEW_ISO_USER, VIEW_PERSP_USER, VIEW_CAMERA, VIEW_GRID, VIEW_NONE, VIEW_TRACK, 
	VIEW_SPOT, VIEW_SHAPE, VIEW_SCHEMATIC, VIEW_RENDER, VIEW_SCENEEXPLORER, VIEW_OTHER};

/*! Meta data of ViewWindow class; it contains the description of how the window is created
*/
struct ViewWindowMetaData: public MaxHeapOperators
{
	/* Type showing how the extended window will be created
	*/
	enum CreationTypeEnum
	{
		SINGLETON,		// window is a singleton; only one instance of it can be created. The window only signalizes that it is a singleton;
						// and the core application takes care of showing only one instance of it
		//NAME_UNIQUE,	// window is unique by name; only one instance of a particular name can exist; 
						// this type is not supported yet
		NOT_UNIQUE		// window can be created in many copies
	};

	ViewWindowMetaData(CreationTypeEnum type = NOT_UNIQUE): mCreationType(type)
	{}
	CreationTypeEnum mCreationType;
};


// class for registering a window that can appear in a MAX viewport DB 10/6/98
/*! \sa  Class Interface.\n\n
\par Description:
This the base class for the creation of non-3D windows that appear in a 3ds Max
viewport. These views are called "Extended Viewports". In order for a window to
appear inside a viewport, you need to derive a class from this class. An
instance of the derived class must be registered via the
<b>RegisterViewWindow()</b> call in the <b>Interface</b> class. A given
<b>ViewWindow</b> derivative should only be registered once.\n\n
When developers have registered their window types, the list of available
extended views will appear in the view selection pop-up (either in the
right-click viewport menu or the Viewport Configuration dialog) as a submenu of
the "Extended" view label.\n\n
There are two items which should be made in the extended viewport dialog proc
code:\n\n
<b>Interface::MakeExtendedViewportActive()</b> should be called whenever the
user clicks in the non-3D window (so as to deactivate the current 3D window,
and redirect commands like the Min/Max toggle to the non-3D viewport
window).\n\n
<b>Interface::PutUpViewMenu()</b> should be called when the user right-clicks
in a dead region of the non-3D window. This brings up the view selection menu
so that the user can choose to replace the current window with a 3D or other
non-3D window without having to go to the Views | Viewport Config dialog
directly.\n\n
All methods of this class are virtual.
\par Sample Code:
\code
class TestViewWindow : public ViewWindow
{

	const MCHAR *GetName() { return _M("TestViewWindow"); }
	HWND CreateViewWindow(HWND hParent, int x, int y, int w, int h);
	void DestroyViewWindow(HWND hWnd);
};

HWND TestViewWindow::CreateViewWindow(HWND hParent, int x, int y, int w, int h)
{
	return CreateWindow("button", "Test Button", WS_VISIBLE | WS_CHILD, x, y, w, h, hParent, NULL, (HINSTANCE)GetWindowLong(hParent, GWL_HINSTANCE), NULL);
}

void TestViewWindow::DestroyViewWindow(HWND hWnd)
{
	DestroyWindow(hWnd);
}

static TestViewWindow tvw;
\endcode */
class ViewWindow : public InterfaceServer {
public:
	/*! \remarks Returns the name of the window type. For example, "Asset
	Manager". */
	virtual const MCHAR *GetName()=0;
	/*! \remarks Creates and returns a handle to a new extended view window.
	\par Parameters:
	<b>HWND hParent</b>\n\n
	The handle of the parent window.\n\n
	<b>int x</b>\n\n
	The x coordinate of the window's upper left corner.\n\n
	<b>int y</b>\n\n
	The y coordinate of the window's upper left corner.\n\n
	<b>int w</b>\n\n
	The window width.\n\n
	<b>int h</b>\n\n
	The window height. */
	virtual HWND CreateViewWindow(HWND hParent, int x, int y, int w, int h)=0;
	/*! \remarks Destroys the previously created window as specified by the
	handle.
	\par Parameters:
	<b>HWND hWnd</b>\n\n
	The handle of the window to destroy. */
	virtual void DestroyViewWindow(HWND hWnd)=0;
	// CanCreate can be overridden to return FALSE if a ViewWindow can only have
	// a single instance, and that instance is already present.  If CanCreate
	// returns FALSE, then the menu item for this ViewWindow will be grayed out.
	/*! \remarks Returns TRUE if the ViewWindow can be created; otherwise
	FALSE. This method can be overridden to return FALSE if a ViewWindow can
	only have a single instance, and that instance is already present. If this
	method returns FALSE, then the menu item for this ViewWindow will be grayed
	out.
	\par Default Implementation:
	<b>{ return TRUE; }</b> */
	virtual BOOL CanCreate() { return TRUE; }
	// mep 7/12/99
	// NumberCanCreate() returns the number of instances of a given window that
	// can be created. This allows, for example, the UI to know without physically
	// creating a window that only a limited number of windows of that type can
	// be created. A -1 implies that the count is not known.
	/*! \remarks	This method returns the number of instances of a given window that can be
	created. This allows, for example, the UI to know without physically
	creating a window that only a limited number of windows of that type can be
	created. A -1 implies that the count is not known.
	\par Default Implementation:
	<b>{ return -1; }</b> */
	virtual int NumberCanCreate() { return -1; }

	/*Get metadata of this ViewWindow; it will be used during the creation of the window object
	In particular this me is used when an inherited extended window has to be:
	- a singleton meaning that only one instance of the extended window obj can 
	exist in the app
		\returns window metadata ViewWindowMetaData
	*/
	virtual ViewWindowMetaData GetMetaData() = 0;

	/*Get the class ID of the ViewWindow as an instance of Class_ID (see maxtypes.h)
	It is used for persistence purposes to uniquely identify the particular ViewWindow class successor
	and will be saved in view panel presets and other UI settings files.
	If the Class_ID gets duplicated in different ViewWindows, the result (a created view window) will be 
	random.
	\returns a Class_ID obj of the ViewWindow
	*/
	virtual Class_ID ClassID() const = 0;
};

//docking states for opening a trackview window
static const int FLOAT_TRACKVIEW		= 0;	// float window.  can't dock on top (the default)
static const int DOCK_TRACKVIEW_TOP		= 1;	// dock on top
static const int DOCK_TRACKVIEW_BOTTOM	= 2;	// dock on bottom.  can't dock on top
static const int CAN_DOCK_TRACKVIEW_TOP	= 3;	// floating but able to dock on top

#define TRACKBAR_FILTER_ALL			1
#define TRACKBAR_FILTER_TMONLY		2
#define TRACKBAR_FILTER_CURRENTTM	3
#define TRACKBAR_FILTER_OBJECT		4
#define TRACKBAR_FILTER_MATERIAL	5

/// The TrackBar filter callback function
/// ****************************************************
/// The anim argument is the node you are filtering.
/// The parent argument is the owning animatable.  It returns the anim with its corresponding subAnimIndex.
///		anim == parent->SubAnim(subAnimIndex)
/// The grandParent argument is provided as an additional layer that is needed in case the parent is a ParamBlock(2) or something similar.  
///		For instance you may need to know that the grandParent is a Bend modifier and the subAnimIndex is 
///		an index into the param block storage unit.
/// The node argument is the originating scene node.
/// Return true if the node should be included, otherwise return false.
typedef bool (* TRACKBAR_FILTER_PROC)(Animatable* anim, Animatable* parent, int subAnimIndex, Animatable* grandParent, INode* node);

/// The TrackBar filter addition callback function
/// ****************************************************
/// The anims argument is a Tab of the parent animatables to add
/// The subAnims argument is a Tab of the SubAnim numbers.  
///		The actual animatable added will be anim = anims[i]->SubAnim(subAnims[i])
/// The node argument is the originating scene node.
///
/// The anims, subAnims, and nodes Tab should all have the same count.
/// Return true if the addition should occur or false if is should not.
typedef bool (* TRACKBAR_ADDITION_PROC)(Tab<Animatable*> &anims, Tab<int> &subAnims, Tab<INode*> &nodes);

/// The ITrackBarFilterManager interface is implemented by the TrackBar.
/// You can get an instance of this interface from the TrackBar using the following:
///	ITrackBarFilterManager* filterManager = (ITrackBarFilterManager*)GetCOREInterface()->GetTrackBar()->GetInterface(TRACKBAR_FILTER_MANAGER_INTERFACE);
#define TRACKBAR_FILTER_MANAGER_INTERFACE Interface_ID(0xc85046, 0x2def7c7d)
class ITrackBarFilterManager : public BaseInterface
{
public:
	/// Register a new filter with the filter manager.  This is usually done at startup, but does not have to be.
	/// callbackFilter or callbackAddition can be NULL
	/// The name is a UI displayable name.
	/// The ID is a unique ID so filters are not duplicated.
	/// If active is true the filter will be used to filter TrackBar key display.
	/// If stopTraversal is false. This filter will not filter out subAnims of the object is supports.  
	virtual int	RegisterFilter(TRACKBAR_FILTER_PROC callbackFilter, TRACKBAR_ADDITION_PROC callbackAddition, MSTR name, Class_ID filterID, bool active = false, bool stopTraversal = true)=0;
	/// Remove a registered filter from the filter manager.
	virtual void UnRegisterFilter(int index)=0;
	/// Remove a registered filter from the filter manager.
	virtual void UnRegisterFilter(Class_ID filterID)
	{
		for (int i = FilterCount()-1; i >=0; i--)
		{
			if (GetFilterID(i) == filterID)
			{
				UnRegisterFilter(i);
				break;
			}
		}
	}
	/// Returns the number of filters registered with the filter manager
	virtual int FilterCount()=0;
	/// Get a registered filter's ID.
	virtual Class_ID GetFilterID(int index)=0;
	/// Get a registered filter's Name.
	virtual MSTR GetFilterName(int index)=0;
	/// If the filter is active its Filter() method will be called once for 
	/// each animatable visited by the TrackBar's traverser.
	virtual bool IsFilterActive(int index)=0;
	/// Activate or deactivate the filter.  If active it will be used to filter the key display.
	virtual void SetFilterActive(int index, bool state)=0;

private:
friend class ITrackBar;
	/// MAXScript support through function publishing
	virtual int	RegisterFilter(Value* callbackFilter, Value* callbackAddition, MSTR name, int id, bool active = false, bool stopTraversal = true)=0;
};


#define TRACKBAR_INTERFACE Interface_ID(0x2aff3557, 0x16aa714b)

// class for accessing the TrackBar (the Mini TrackView)
/*! \sa  Class IKeyControl, Class Interface.\n\n
\par Description:
The track bar offers a quick way to manipulate keyframes for selected objects.
Keys are displayed on the track bar just like they are in Track View.
Developers are able to manipulate the track bar using this class. There is a
method of class <b>Interface</b> which returns a pointer to an instance of this
class:\n\n
<b>ITrackBar* GetTrackBar()</b>.\n\n
All methods of this class are Implemented by the System.  */
class ITrackBar : public FPMixinInterface {
public:
	/*! \remarks Sets the visibility of the track bar to on or off.
	\par Parameters:
	<b>BOOL bVisible</b>\n\n
	TRUE to make visible; FALSE to hide. */
	virtual void		SetVisible(BOOL bVisible) = 0;
	/*! \remarks Returns TRUE if the track bar is visible; otherwise FALSE. */
	virtual BOOL		IsVisible() = 0;
	/*! \remarks The track bar shows keys for all parametric animation as well
	as transforms. This method sets the filter used which determines which keys
	are shown in the track bar.
	\par Parameters:
	<b>UINT nFilter</b>\n\n
	One of the following values:\n\n
	<b>TRACKBAR_FILTER_ALL</b>\n\n
	Specifies to show all keys.\n\n
	<b>TRACKBAR_FILTER_TMONLY</b>\n\n
	Specifies to show transform keys only.\n\n
	<b>TRACKBAR_FILTER_CURRENTTM</b>\n\n
	Specifies to only show keys for the currently active transform (move,
	rotate or scale).\n\n
	<b>TRACKBAR_FILTER_OBJECT</b>\n\n
	Specifies to show keys for the controllers assigned anywhere in the
	pipeline to be included in the TrackBar key-display. This is essentially
	the modifiers and the base objects for the selected nodes (no transforms or
	materials).\n\n
	<b>TRACKBAR_FILTER_MATERIAL</b>\n\n
	Specifies to show keys for the controllers assigned anywhere for the
	selected nodes material - all controllers in the whole material tree will
	be included in the track bar display. */
	virtual void		SetFilter(UINT nFilter) = 0;
	/*! \remarks Returns the filter value which determines which keys are
	shown in the track bar. See the list specified in the method above for
	details. */
	virtual UINT		GetFilter() = 0;
	/*! \remarks This method returns the time of the next key given a start
	time and a flag which indicates if the search should proceed forward or
	backwards. This is similar to the behavior of
	<b>Animatable::GetNextKeyTime()</b>.
	\par Parameters:
	<b>TimeValue tStart</b>\n\n
	Specifies the time to start looking for the next key.\n\n
	<b>BOOL bForward</b>\n\n
	TRUE to return the time of the next key; FALSE for the previous key.
	\return  The time of the next (or previous) key. */
	virtual TimeValue	GetNextKey(TimeValue tStart, BOOL bForward) = 0;
	/*! \remarks	Redraws the Track Bar if required an optionally forces a redraw (even if
	not known to be required).
	\par Parameters:
	<b>bool bForce = false</b>\n\n
	Pass true to force a redraw; false to only redraw if 3ds Max deems it
	required. */
	virtual void		RedrawTrackBar(bool bForce = false) = 0;
	/*! \remarks	Controls the visibility of frame numbers in the Track Bar.
	\par Parameters:
	<b>bool b</b>\n\n
	Pass true to show frame numbers; false to turn them off. */
	virtual void		SetShowFrames(bool b) = 0;
	/*! \remarks	Returns true if frame numbers are visible in the Track Bar; false if not.
	*/
	virtual bool		GetShowFrames() = 0;
	/*! \remarks	Controls the visibility of the audio track.
	\par Parameters:
	<b>bool b</b>\n\n
	Pass true to show the audio track; false to turn it off. */
	virtual void		SetShowAudio(bool b) = 0;
	/*! \remarks	Returns true if the audio track is displayed; false if it is not. */
	virtual bool		GetShowAudio() = 0;
	/*! \remarks	Controls if the selection range bar is visible or not.
	\par Parameters:
	<b>bool b</b>\n\n
	Pass true to display the selection range bar; false to turn it off. */
	virtual void		SetShowSelectionRange(bool b) = 0;
	/*! \remarks	Returns true if the selection range bar is visible; false if it is not. */
	virtual bool		GetShowSelectionRange() = 0;
	/*! \remarks	Controls the snap to frames setting.
	\par Parameters:
	<b>bool b</b>\n\n
	Pass true to turn it on; false to turn it off. */
	virtual void		SetSnapToFrames(bool b) = 0;
	/*! \remarks	Returns true if snap to frames is on; false if it is off. */
	virtual bool		GetSnapToFrames() = 0;
	/*! \remarks	This method allows you to set the transparency of keyframes displayed in
	the trackbar.
	\par Parameters:
	<b>int xp</b>\n\n
	The transparency value between 0 and 255. */
	virtual void		SetKeyTransparency(int xp) = 0;
	/*! \remarks	This method returns the transparency of keyframes displayed in the
	trackbar.
	\return  The transparency value between 0 and 255. */
	virtual int			GetKeyTransparency() = 0;
	/*! \remarks	This method allows you to set the transparency of selected keyframes
	displayed in the trackbar.
	\par Parameters:
	<b>int xp</b>\n\n
	The transparency value between 0 and 255. */
	virtual void		SetSelKeyTransparency(int xp) = 0;
	/*! \remarks	This method returns the transparency of selected keyframes displayed in the
	trackbar.
	\return  The transparency value between 0 and 255. */
	virtual int			GetSelKeyTransparency() = 0;
	/*! \remarks	This method allows you to set the transparency of the cursor displayed in
	the trackbar.
	\par Parameters:
	<b>int xp</b>\n\n
	The transparency value between 0 and 255. */
	virtual void		SetCursorTransparency(int xp) = 0;
	/*! \remarks	This method returns the transparency of the cursor displayed in the
	trackbar.
	\return  The transparency value between 0 and 255. */
	virtual int			GetCursorTransparency() = 0;
	
	//Function ID's for Function Publishing -- Added by AF (09/27/00)
	enum { fIdgetVisible, fIdsetVisible, fIdgetFilter, fIdsetFilter, fIdgetNextKeyTime, fIdgetPreviousKeyTime, fIdredrawTrackBar,
		fIdgetShowFrames, fIdsetShowFrames, fIdgetShowAudio, fIdsetShowAudio, fIdgetShowSelectionRange, fIdsetShowSelectionRange,
		fIdgetSnapToFrames, fIdsetSnapToFrames, fIdgetKeyTransparency, fIdsetKeyTransparency, 
		fIdgetSelKeyTransparency, fIdsetSelKeyTransparency, fIdgetCursorTransparency, fIdsetCursorTransparency,
		fIdregisterFilter, fIdUnRegisterFilter, fIdFilterCount, fIdFilterID, fIdFilterName, fIdIsFilterActive, fIdSetFilterActive,
	};
	//symbolic enum ID for Function Publishing
	enum {enumIDfilterType,
	};
	//Function Publishing methods added by AF (9/27/00)
	BaseInterface* GetInterface(Interface_ID id) { return (id == TRACKBAR_INTERFACE) ? this : FPMixinInterface::GetInterface(id); }
	FPInterfaceDesc* GetDesc() { return (FPInterfaceDesc*)GetCOREInterface(TRACKBAR_INTERFACE); }

	// dispatch map for FP-published functions
	#pragma warning(push)
	#pragma warning(disable:4238)
	BEGIN_FUNCTION_MAP
	VFN_1(fIdredrawTrackBar, RedrawTrackBar, TYPE_bool);
	FNT_0(fIdgetNextKeyTime, TYPE_TIMEVALUE, fpGetNextKey);
	FNT_0(fIdgetPreviousKeyTime, TYPE_TIMEVALUE, fpGetPreviousKey);
	FN_6(fIdregisterFilter, TYPE_INDEX, fpRegisterFilter, TYPE_VALUE, TYPE_VALUE, TYPE_TSTR, TYPE_INT, TYPE_bool, TYPE_bool);
	VFN_1(fIdUnRegisterFilter, fpUnRegisterFilter, TYPE_INDEX);
	FN_0(fIdFilterCount, TYPE_INT, fpFilterCount);
	FN_1(fIdFilterID, TYPE_INT_TAB_BV, fpGetFilterID, TYPE_INDEX);
	FN_1(fIdFilterName, TYPE_TSTR_BV, fpGetFilterName, TYPE_INDEX);
	FN_1(fIdIsFilterActive, TYPE_bool, fpIsFilterActive, TYPE_INDEX);
	VFN_2(fIdSetFilterActive, fpSetFilterActive, TYPE_INDEX, TYPE_bool);

	PROP_FNS(fIdgetVisible, IsVisible, fIdsetVisible, SetVisible, TYPE_BOOL);
	PROP_FNS(fIdgetFilter, GetFilter, fIdsetFilter, SetFilter, TYPE_ENUM);
	PROP_FNS(fIdgetShowFrames, GetShowFrames, fIdsetShowFrames, SetShowFrames, TYPE_bool);
	PROP_FNS(fIdgetShowAudio, GetShowAudio, fIdsetShowAudio, SetShowAudio, TYPE_bool);
	PROP_FNS(fIdgetShowSelectionRange, GetShowSelectionRange, fIdsetShowSelectionRange, SetShowSelectionRange, TYPE_bool);
	PROP_FNS(fIdgetSnapToFrames, GetSnapToFrames, fIdsetSnapToFrames, SetSnapToFrames, TYPE_bool);
	PROP_FNS(fIdgetKeyTransparency, GetKeyTransparency, fIdsetKeyTransparency, SetKeyTransparency, TYPE_INT);
	PROP_FNS(fIdgetSelKeyTransparency, GetSelKeyTransparency, fIdsetSelKeyTransparency, SetSelKeyTransparency, TYPE_INT);
	PROP_FNS(fIdgetCursorTransparency, GetCursorTransparency, fIdsetCursorTransparency, SetCursorTransparency, TYPE_INT);
	END_FUNCTION_MAP 
	#pragma warning(pop)
private:
	//Private methods for Maxscript exposure -- Added by AF (09/27/00)
	TimeValue fpGetNextKey(TimeValue t) {return GetNextKey(t, TRUE);}
	TimeValue fpGetPreviousKey(TimeValue t) { return GetNextKey(t, FALSE);}
	int	 fpRegisterFilter(Value* callbackFilter, Value* callbackAddition, MSTR name, int id, bool active = false, bool stopTraversal = true)
		{
		ITrackBarFilterManager* filterManager = (ITrackBarFilterManager*)this->GetInterface(TRACKBAR_FILTER_MANAGER_INTERFACE);
		if (filterManager != NULL)
			return filterManager->RegisterFilter(callbackFilter, callbackAddition, name, id, active, stopTraversal);
		return -1;
		}
	void fpUnRegisterFilter(int index)
		{
		ITrackBarFilterManager* filterManager = (ITrackBarFilterManager*)this->GetInterface(TRACKBAR_FILTER_MANAGER_INTERFACE);
		if (filterManager != NULL)
			filterManager->UnRegisterFilter(index);
		}
	int fpFilterCount()
		{
		ITrackBarFilterManager* filterManager = (ITrackBarFilterManager*)this->GetInterface(TRACKBAR_FILTER_MANAGER_INTERFACE);
		if (filterManager != NULL)
			{
			return filterManager->FilterCount();
			}
		return 0;
		}
	Tab<int> fpGetFilterID(int index)
		{
		Tab<int> ids;
		ITrackBarFilterManager* filterManager = (ITrackBarFilterManager*)this->GetInterface(TRACKBAR_FILTER_MANAGER_INTERFACE);
		if (filterManager != NULL)
			{
			Class_ID id = filterManager->GetFilterID(index);
			int IDa = id.PartA();
			int IDb = id.PartB();
			ids.Append(1, &IDa);
			ids.Append(1, &IDb);
			}
		return ids;
		}
	MSTR fpGetFilterName(int index)
		{
		ITrackBarFilterManager* filterManager = (ITrackBarFilterManager*)this->GetInterface(TRACKBAR_FILTER_MANAGER_INTERFACE);
		if (filterManager != NULL)
			{
			return filterManager->GetFilterName(index);
			}
		return _M("");
		}

	bool fpIsFilterActive(int index)
		{
		ITrackBarFilterManager* filterManager = (ITrackBarFilterManager*)this->GetInterface(TRACKBAR_FILTER_MANAGER_INTERFACE);
		if (filterManager != NULL)
			{
			return filterManager->IsFilterActive(index);
			}
		return false;
		}
	void fpSetFilterActive(int index, bool state)
		{
		ITrackBarFilterManager* filterManager = (ITrackBarFilterManager*)this->GetInterface(TRACKBAR_FILTER_MANAGER_INTERFACE);
		if (filterManager != NULL)
			filterManager->SetFilterActive(index, state);
		}
	};

// This class provides functions that expose the portions of View3D
// that are exported for use by plug-ins.
/*! \sa  \ref Snap_Flags,
Class Ray,  Class Point3,  Class IPoint2, Class Matrix3, Class ModContext, Class HitData, Class GraphicsWindow, Class INode.\n\n
\par Description:
This class provides methods to access properties of the viewport, convert
points between viewport and world coordinates, snap points and lengths, etc.
Many methods associated with hit testing are also here. All the methods of this
class are implemented by the system.
*/
class ViewExp : public InterfaceServer {
	public:
		/**
		* Return the unique ID of the viewport represented by the ViewExp.
		* NOTE: Each view panel can have 4 viewports whose indexes are 
		* usually all from 0 to 3. But their view IDs are unique.
		* Besides, view ID is persistent during FileIO. So user can store this 
		* ID during saving. And then after loading, user can use the stored ID
		* to retrieve the same viewExp interface by calling
		* Interface14::GetViewExpByID(int id)
		* \return the view ID of the viewport
		*/
		virtual int GetViewID() = 0;
		/* \remark ViewExp is returned by reference now while many APIs still 
		take ViewExp pointer as an input parameter. Use this method to convert 
		reference to pointer.
		NOTE: Clients should avoid storing ViewExp pointer locally since ViewExp
		may become invalid if its corresponding viewport get deleted.
		\return A ViewExp Pointer
		*/
		virtual ViewExp* ToPointer() = 0;

		/* \remark ViewExp become invalid if its corresponding viewport get
		deleted. Calls to a invalid ViewExp would be applied to a default viewport
		to avoid a crash. However, this could cause unexpected behavior, clients 
		could use this method to check this case and handle it properly.
		\return true if the ViewExp's corresponding viewport exists and is valid, false otherwise.
		*/
		virtual bool	IsAlive() = 0;

//! \name World Space Conversion
//! @{		
		/*! \remarks Returns a point in world space on the current
		construction plane based on the specified screen coordinate.
		\param ps The 2D screen point to convert to a 3D world space coordinate.
		\return  The world space coordinate on the current construction plane.
		*/
		virtual Point3 GetPointOnCP(const IPoint2 &ps)=0;
//! @}

//! \name Snapping
//! @{
		/*! \remarks Given a 2D screen coordinate, this method returns a 3D point on
		the current construction plane based on the current snap settings and flags
		passed.
		\param in The 2D screen coordinate to snap.
		\param out The snapped 2D screen coordinate. This is used if you need to move the mouse
		position to the snapped location.
		\param plane2d  This optional argument allows you to use any plane (not just the current
		construction plane).
		\param flags See \ref Snap_Flags.
		\return  The snapped 3D point in construction plane coordinates. */
		virtual Point3 SnapPoint(const IPoint2 &in, IPoint2 &out, Matrix3 *plane2d = NULL, DWORD flags = 0)=0;

		/*! \remarks This is a method used as part of the osnap system in 3ds Max. It is the
		method that displays the snap marker in the viewports prior to the
		first point event. It's really just a call to <b>SnapPoint()</b> which
		returns nothing. This method should be called in response to a
		<b>MOUSE_FREEMOVE</b> event from any creation or transformation proc
		which calls <b>SnapPoint()</b>. Here's an example creation proc:
		\par Sample Code:
		\code
		int PointHelpObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat )
		{
			if (msg == MOUSE_FREEMOVE)
			// Show a preview snap in the viewport prior to the
			// first point event.
			vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);

			if (msg==MOUSE_POINT||msg==MOUSE_MOVE)
			{
				switch(point)
				{
				case 0:
					ob->suspendSnap = TRUE;
					mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_3D));
					break;
				case 1:
					mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_3D));
					if (msg==MOUSE_POINT)
					{
						ob->suspendSnap = FALSE;
						return 0;
					}
					break;
				}
			}
			else if (msg == MOUSE_ABORT)
			{
				return CREATE_ABORT;
			}
			return 1;
		}
		\endcode
		\param in  The 2D screen coordinate to snap.
		\param out The snapped 2D screen coordinate. This is used if you need to move the
		mouse position to the snapped location.
		\param plane2d  This optional argument allows you to use any plane (not just the
		current construction plane).
		\param flags See \ref Snap_Flags. */
		virtual void SnapPreview(const IPoint2 &in, IPoint2 &out, Matrix3 *plane2d = NULL, DWORD flags = 0)=0;
		/*! \remarks This method is used internally. It fills up it's arguments with the
		world space extents of the home grid (i.e. the extents of the grid as
		<b>displayed</b>). It doesn't work for grid helper which always display
		to their size limits. This was exposed so 3ds Max could do the grid
		snapping and is not needed by plug-in developers. */
		virtual void GetGridDims(float *MinX, float *MaxX, float *MinY, float *MaxY) = 0;

		/*! \remarks Given the distance passed, this method snaps the length
		to the nearest snap increment and returns the snapped distance.
		\param in The input distance to be snapped.
		\return  The snapped distance. */
		virtual float SnapLength(float in)=0;
//! @}

//! \name World Space Conversion
//! @{
		/*! \remarks This method returns a length in world space given a start screen
		point, an end screen point, a base point and a direction vector. For example,
		when creating a cylinder, the user clicks the mouse down to define the center
		point of the cylinder (<b>base</b>), then drags out a radius. They then drag
		out a height for the cylinder. This method is used to return intermediate and
		final heights for the cylinder based on the initial base point, the direction
		vector (the Z axis), the start mouse point, and the current point the user is
		interactively adjusting.
		\param base Base point in object space.
		\param dir Direction vector in object space.
		\param sp1 Screen start point. This is the point where the user clicked down with the mouse.
		\param sp2 Screen end point. This is the point where the user let up the mouse.
		\param snap
		\return  The length in world space based on the screen points and their
		projection onto the direction vector.
		\par Sample Code:
		\code
		float h = vpt->SnapLength(vpt->GetCPDisp(p[1],Point3(0,0,1),sp1,m));
		\endcode
From /MAXSDK/SAMPLES/OBJECTS/CYL.CPP in CylinderObjCreateCallBack::proc	 */
		virtual float GetCPDisp(const Point3 base, const Point3& dir, 
                        const IPoint2& sp1, const IPoint2& sp2, BOOL snap = FALSE )=0;
//! @}

//! \name Access to Viewport Properties
//! @{		
		/*! \remarks Returns a pointer to the instance of
		<b>GraphicsWindow</b> associated with this viewport.\n\n
		Note: A <b>GraphicsWindow</b> always has a transform associated with
		it, for faster object-to-screen space conversions. The
		<b>GraphicsWindow *</b> returned by this method may have a non-identity
		transform already in place. A developer can call
		<b>gw-\>setTransform()</b> with a node's transform for fast work in
		Display routines. But this value must be explicitly set to the identity
		for world-to-screen displays. */
		virtual GraphicsWindow* getGW()=0;

		/*! \remarks Determines if this viewport is in wire-frame rendering
		mode (as opposed to a shaded mode).
		\return  Nonzero if the viewport is in wire-frame rendering mode;
		otherwise 0. */
		virtual int IsWire()=0;
		/*! \remarks Returns the damaged rectangle of the viewport. This is
		the area that needs to be updated on the next screen refresh. This can
		be used for example, to pass into the <b>Mesh</b> method
		<b>render()</b> to only display the damaged area of the object. A
		developer could also use this in the implementation of their own
		<b>Display()</b> method.<p>
        \par Sample Code:
        \code
        int SimpleObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
        {
        	if (!OKtoDisplay(t)) 
        		return 0;

         	GraphicsWindow *gw = vpt->getGW();
          	Matrix3 mat = inode->GetObjectTM(t);

           	UpdateMesh(t); // UpdateMesh() just calls BuildMesh()
           	gw->setTransform(mat);

            mesh.render(gw, inode->Mtls(), (flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL,

            COMP_ALL, inode->NumMtls());
            return(0);
        }
        \endcode */
		virtual Rect GetDammageRect()=0;
//! @}

		/*! \remarks Given a point with view coordinates, this method maps 
		the point into screen(windows) coordinates.
		\param p Point in view coordinates.
		\return  Point in screen coordinates.
		\par Sample Code:
		\code
		Point2 p0 = vpt->MapScreenToView(affinePos);
		\endcode 
		*/
		virtual Point2 MapViewToScreen(const Point3& p) = 0;

//! \name World Space Conversion
//! @{		
		/*! \remarks Given a point on the screen (in window coordinates), and
		a depth in view coordinates, this method maps the point into view
		coordinates. This is just a scaling operation for parallel projections,
		but involves a divide by Z for perspective projections.
		\param sp Point in window coordinates.
		\param depth Depth in view coordinates.
		\return  Point in view coordinates.
		\par Sample Code:
		\code
		Point3 p0 = vpt->MapScreenToView(mBase,GetPerspMouseSpeed());
		\endcode 
		*/
		virtual Point3 MapScreenToView( IPoint2& sp, float depth )=0;
		/*! \remarks Creates a Ray in world space passing through the
		specified pixel directed toward the scene in the direction of view.
		\param sx The x screen coordinate.
		\param sy The y screen coordinate.
		\param ray The Ray in world space. See Class Ray. */
		virtual void MapScreenToWorldRay(float sx, float sy, Ray& ray)=0;

		// set the affine tm for the view and ret TRUE if the view is ISO_USER or PERSP_USER
		// else do nothing and return FALSE
		/*! \remarks This method sets the viewport affine transformation and returns TRUE if
		the view is a user view (isometric or perspective). If the view is not
		a user view then the transformation is not changed and the method
		returns FALSE. See <b>SetViewUser()</b> below.
		\param m The transformation matrix to set. */
		virtual BOOL SetAffineTM(const Matrix3& m)=0;
		/*! \remarks This method retrieves the affineTM which transforms from
		World coordinates to View coordinates. See the sample code below for an
		example of its use.
		\param tm The matrix to hold the affine TM.
		\code
		// This routine returns the view direction from the active viewport.
		Point3 Utility::GetViewDirection()
		{
			Matrix3 aTM, coordSysTM;
			ViewExp& ve = ip->GetActiveViewExp();

			// The affine TM transforms from world coords to view coords
			// so we need the inverse of this matrix
			ve.GetAffineTM(aTM);
			coordSysTM = Inverse(aTM);

			// The Z axis of this matrix is the view direction.
			Point3 viewDir = coordSysTM.GetRow(2);
			return viewDir;
		}
		\endcode
        Note: You can also get
		the view position from this matrix. For example, in the above code, the
		statement: <b>Point3 viewPt = coordSysTM.GetRow(3);</b>
		gets the point in space the view is taken from. */
		virtual void GetAffineTM( Matrix3& tm )=0;
//! @}

//! \name Perspective/Camera View Properties
//! @{
		/*! \remarks Returns the type of view. One of the following values:\n\n
		<b>enum ViewType {</b>\n\n
		<b>VIEW_LEFT,VIEW_RIGHT,VIEW_TOP,VIEW_BOTTOM,VIEW_FRONT,VIEW_BACK,</b>\n\n
		<b> VIEW_ISO_USER, VIEW_PERSP_USER, VIEW_CAMERA, VIEW_GRID,
		VIEW_NONE,</b>\n\n
		<b>VIEW_TRACK, VIEW_SPOT, VIEW_SHAPE, VIEW_SCHEMATIC,
		VIEW_OTHER</b>\n\n
		<b>};</b> */
		virtual int GetViewType() = 0;
		/*! \remarks Returns TRUE if the viewport is a perspective view;
		otherwise returns FALSE. */
		virtual BOOL IsPerspView()=0;
		/*! \remarks Returns TRUE if the viewport is 2D canvas navigation mode;
		otherwise returns FALSE. */
		virtual BOOL IsCanvasNavigationMode()=0;
		/*! \remarks  Returns the field of view of a perspective viewport in radians.\n\n
		  */
		virtual float GetFOV()=0;
		/*! \remarks Returns the focal distance of a perspective view. */
		virtual float GetFocalDist()=0;
		/*! \remarks Sets the focal distance of a perspective view.
		\param fd Specifies the focal distance to set. */
		virtual void  SetFocalDist(float fd)=0;
//! @}

		//! \brief Get the FPS from the active viewport.
		//! \return Return the frame per second of the active viewport.
		virtual float GetFPS()=0;

//! \name World Space Conversion
//! @{
		/*! \remarks Returns the screen scale factor for a point given in
		world coordinates. This factor gives the width in world-space units at
		the point's distance of the viewport.
		\param worldPoint The point in world coordinates.
		\return  The screen scale factor in world space units. */
		virtual float GetScreenScaleFactor(const Point3 worldPoint)=0;

		// return the viewPort screen width factor in world space at 
		// a point in world space
		/*! \remarks Returns the viewport screen width factor in world space
		at a point in world space.
		\param wPoint The point in world space.
		\return  The viewport screen width factor in world space. */
		virtual float GetVPWorldWidth(const Point3 wPoint)=0;

		/*! \remarks Given a point on the construction plane this method
		returns the corresponding world space point. For example, if you use
		<b>GetPointOnCP()</b> to convert a screen coordinate to a point on the
		construction plane, you could then call this method to convert that
		point on the construction plane to a world space point.
		\param cpPoint The point on the construction plane.
		\return  The world space point. */
		virtual Point3 MapCPToWorld(const Point3 cpPoint)=0;

		/*! \remarks The value returned from this method may be used as a
		scale factor that will counteract the viewport zoom. For example,
		lights, cameras, and tape helper objects use this factor so the size of
		the node in the scene remains constant when the viewport is zoomed in
		and out.\n\n
		This value is affected by the 'Non-Scaling Object Size' spinner in the
		Viewport Preferences dialog, so the user has some control over this as
		well.
        \par Sample Code:
		This sample is from /MAXSDK/SAMPLES/OBJECTS/TAPEHELP.CPP. The computed 
                matrix is used in several places like displaying, snapping, hit testing, etc.
		\code
		void TapeHelpObject::GetMat(TimeValue t, INode* inode,
									ViewExp* vpt, Matrix3& tm)
		{
			tm = inode->GetObjectTM(t);
			tm.NoScale();

			float scaleFactor = vpt->NonScalingObjectSize() *
					vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;

			tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
		}
		\endcode */
		virtual float NonScalingObjectSize()=0;  // 1.0 is "default"
//! @}

//! \name Access to Viewport Properties
//! @{
		/*! \remarks Retrieves the transformation matrix of the construction
		plane.
		\param tm The transformation matrix is returned here. */
		virtual void GetConstructionTM( Matrix3 &tm )=0;
		/*! \remarks Sets the size of the construction grid spacing.
		\param size Specifies the grid spacing. */
		virtual void SetGridSize( float size )=0;
		/*! \remarks Returns the construction grid spacing. This is the grid
		spacing on a per viewport basis. It is dependent on how far zoomed in
		or out the user is. This is the exact same value that you can see in
		the right most status panel below the viewports. */
		virtual float GetGridSize()=0;
		/*! \remarks Returns TRUE if the grid is turned on for this viewport;
		otherwise FALSE.\n\n
		  */
		virtual BOOL IsGridVisible()=0;
		/*! \remarks Set the grid visibility for this viewport.\n\n
		*/
		virtual void SetGridVisibility(BOOL bVisible)=0;
		/*! \remarks Returns the grid type. One of the following values (from
		OBJECT.H):\n\n
		<b>GRID_PLANE_NONE</b>\n\n
		<b>GRID_PLANE_TOP</b>\n\n
		<b>GRID_PLANE_LEFT</b>\n\n
		<b>GRID_PLANE_FRONT</b>\n\n
		<b>GRID_PLANE_BOTTOM</b>\n\n
		<b>GRID_PLANE_RIGHT</b>\n\n
		<b>GRID_PLANE_BACK</b> */
		virtual int GetGridType()=0;
//! @}

//! \name Perspective/Camera View Properties
//! @{
		// Get the camera if there is one.
		/*! \remarks Returns the INode pointer of the camera associated with
		this viewport. If this is not a camera view then NULL is returned. */
		virtual INode *GetViewCamera()=0;

		// Set this viewport to a camera view
		/*! \remarks Set this viewport to a camera view.
		\param camNode The camera node to set. */
		virtual void SetViewCamera(INode *camNode)=0;

		// Set this viewport to a user view 
		/*! \remarks  This method sets the viewport to be a user view, with the <b>persp</b>
		argument indicating whether this should be a perspective or iso view.
		Note that the user viewport defaults are used for field-of-view, etc.
		\param persp TRUE for perspective; FALSE for isometric. */
		virtual void SetViewUser(BOOL persp)=0;

		// Get the spot if there is one
		/*! \remarks Returns the INode pointer of the spotlight associated with this
		viewport. If this is not a spotlight view then NULL is returned. */
		virtual INode *GetViewSpot()=0;

		// Set this viewport to a spotlight view
		/*! \remarks Set this viewport to a spotlight view.
		\param spotNode The spotlight node to set. */
		virtual void SetViewSpot(INode *spotNode)=0;
//! @}

//! \name Node Level Hit-testing
//! @{		
		// node level hit-testing
		/*! \remarks Implemented by the System.\n\n
		Clears the list of node level hit records. */
		virtual void ClearHitList()=0;
		/*! \remarks Implemented by the System.\n\n
		Returns the INode pointer of the node that was the closest of all those
		hit. If none were hit, NULL is returned. */
		virtual INode *GetClosestHit()=0;
		/*! \remarks Returns the INode pointer of the 'i-th' node level hit.
		\param i The index of the hit to retrieve. */
		virtual INode *GetHit(int i)=0;
		/*! \remarks Implemented by the System.\n\n
		Returns the number of hits recorded by the last node level hit test. */
		virtual int HitCount()=0;
//! @}

//! \name Subobject Level Hit-testing
//! @{
		// subobject level hit-testing
		/*! \brief This method records a sub-object level hit record with the system using the
		specified parameters. This hit can later be retrieved using the method
		GetSubObjHitList() and the methods of class HitLog.
		\param nr The node that was hit.
		\param mc The ModContext of the modifier.
		\param dist The 'distance' of the hit. What the distance actually represents depends on the
		rendering level of the viewport. For wireframe modes, it refers to the distance
		in the screen XY plane from the mouse to the sub-object component. In a shaded
		mode, it refers to the Z depth of the sub-object component. In both cases,
		smaller values indicate that the sub-object component is 'closer' to the mouse
		cursor.
		\param info Identifies the sub-object component that was hit.
		\param hitdata  If the info data member is insufficient to indicate the sub-object
		component that was hit, pass an instance of the <b>HitData</b> class that
		contains the needed information. The system will be responsible for freeing the memory for the HitData. */
		virtual	void LogHit(INode* nr, ModContext* mc, DWORD dist, ulong info, HitData* hitdata = NULL)=0;
		/*! \remarks Returns the sub-object hit list. See
		Class HitLog. */
		virtual HitLog&	GetSubObjHitList()=0;
		/*! \remarks Clears the sub-object hit list. This deletes all
		previously saved HitRecords. */
		virtual void ClearSubObjHitList()=0;
		/*! \remarks Returns the number of sub-object hits recorded. */
		virtual int NumSubObjHits()=0;
//! @}

//! \name Controller Apparatus Hit-testing
//! @{
		// For controller apparatus hit testing
		/*! \remarks This method records a controller sub-object level hit
		record with the system using the specified parameters. This hit can
		later be retrieved using the method <b>GetCtrlHitList()</b> and the
		methods of class CtrlHitLog.
		\param nr The node that was hit.
		\param dist The 'distance' of the hit. What the distance actually represents
		depends on the rendering level of the viewport. For wireframe modes, it
		refers to the distance in the screen XY plane from the mouse to the
		sub-object component. In a shaded mode, it refers to the Z depth of the
		sub-object component. In both cases, smaller values indicate that the
		sub-object component is 'closer' to the mouse cursor.
		\param info A general unsigned long value. Most controllers will just need this to
		identity the sub-object element. The meaning of this value (how it is
		used to identify the element) is up to the plug-in.
		\param infoExtra If the above <b>hitInfo</b> data member is not sufficient to describe
		the sub-object element this data member may be used as well. */
		virtual void CtrlLogHit(INode *nr,DWORD dist,ulong info,DWORD infoExtra)=0;
		/*! \remarks Returns the list of controller gizmo hits recorded. See
		Class CtrlHitLog. */
		virtual CtrlHitLog&	GetCtrlHitList()=0;
		/*! \remarks Clears the controller hit list. This deletes all the
		HitRecords previously recorded. */
		virtual void ClearCtrlHitList()=0;
//! @}

//! \name Access to Viewport Properties
//! @{		
		// Turn on and off image background display
		/*! \remarks This method is used to turn on and off the background
		image display in this viewport. Note that it is necessary to redraw the
		viewports in order to see the effect of this method. Use the method
		<b>Interface::RedrawViews()</b> to do this.
		\param onOff TRUE to turn the background image on; FALSE to turn it off.
		\return  TRUE if the image was set; otherwise FALSE. */
		virtual BOOL setBkgImageDsp(BOOL onOff)=0;
		/*! \remarks Returns nonzero if the background image is displayed in
		this viewport; otherwise 0. */
		virtual int	getBkgImageDsp(void)=0;

		// Turn on and off safe frame display
		/*! \remarks This method may be used to turn the safe frame display on
		and off in this viewport.
		\param onOff Nonzero to turn on the safe frame; zero to turn it off. */
		virtual void setSFDisplay(int onOff)=0;
		/*! \remarks Returns nonzero if the safe frame is displayed in this
		viewport; otherwise 0. */
		virtual int getSFDisplay(void)=0;
		
		// This is the window handle of the viewport. This is the
		// same window handle past to GetViewExp() to get a ViewExp*
		/*! \remarks This returns the window handle of the viewport - this is
		the transparent window that catches mouse input. Note that this window
		handle is different than the handle that can be retrieved from the
		viewport's GraphicsWindow. <b>getGW()-\>getHWnd()</b> is the window
		that things are drawn on.
		\return  The window handle of the viewport. */
		virtual HWND GetHWnd()=0;

		// Test if the viewport is active
		/*! \remarks Returns TRUE if the viewport is the active on; otherwise FALSE. */
		virtual	BOOL IsActive() = 0;
		// Test if the viewport is enabled
		/*! \remarks Returns TRUE if the viewport is enabled; FALSE if disabled. */
		virtual	BOOL IsEnabled() = 0;
//! @}

		/**
		* Set the viewport background color mode as solid color mode or gradient color mode.
		* \param[in] bSolidColor if true, solid color mode will be set. Otherwise set gradient color mode will be set.
		*/
		virtual void SetSolidBackgroundColorMode(bool bSolidColor) = 0;
		/**
		* Get the current viewport background color mode.
		* \return Return true if in solid color mode. Return false if in gradient color mode.
		*/
		virtual bool IsSolidBackgroundColorMode() = 0;
		/**
		* Reset viewport background color mode.
		*/
		virtual void ResetBackgroundColorMode() = 0 ;

//! \name AutoGrid Related Methods
//! @{
		//methods for floating grids
		/*! \remarks If AutoGrid is enabled, this method determines a grid coordinate system
		by casting a ray into the scene through the screen coordinate <b>m</b>,
		obtaining a surface normal from the closest node , and using the
		"arbitrary axis algorithm" to orient the xy axes. You can get this
		coordinate system back by passing in a pointer to a matrix. A tripod is
		displayed in the viewports showing the orientation.
		\param m The 2D screen point that the user clicked on.
		\param mat  The implicit grid coordinate system matrix can be retrieved by passing
		a pointer to a matrix here.
		\param hitTestFlags See \ref SceneAndNodeHitTestFlags. */
		virtual void TrackImplicitGrid(IPoint2 m, Matrix3* mat = NULL, ULONG hitTestFlags = 0) = 0;
		/*! \remarks If AutoGrid is enabled, this method creates a grid and activates it.
		The <b>mouseflags</b> parameter is used to determine if the ALT key is
		down. If it is, this grid will not be deactivated in
		<b>ReleaseImplicitGrid()</b>(below).
		\param m The 2D screen point that the user clicked on.
		\param mouseflags These flags describe the state of the mouse buttons. See
		\ref Mouse_Call_Back_Flags.
		\param mat  Developers can get the implicit grid coordinate system back by passing
		in a pointer to a matrix here. */
		virtual void CommitImplicitGrid(IPoint2 m, int mouseflags, Matrix3* mat = NULL) = 0;
		/*! \remarks This method deactivates an implicit grid and restores the previously
		active grid. If the implicit grid was committed with ALT-key held down,
		then this call does nothing. */
		virtual void ReleaseImplicitGrid() = 0;
//! @}

		// Invalidate function
		/*! \remarks	This is a better way method to invalidate a viewport only if the area
		define by the <b>Rect</b> argument is not the whole size of the viewport.  Calling InvalidateRect on a 
		<b>Rect</b> defined like this: rect.top = 0; rect.bottom = gw->getWinSizeY(); rect.left = 0; rect.right = 
		gw->getWinSizeX(); Will have the same effect of invalidating the viewport. 
		\param rect The Rect that define the region on the viewport to be invalidated.
		*/
		virtual void InvalidateRect( const Rect &rect ) = 0;

		// Generic expansion function
		/*! \remarks		This is a general purpose function that allows the API to be extended in the
		future. The 3ds Max development team can assign new cmd numbers and continue to
		add functionality to this class without having to 'break' the API.
		\param cmd The index of the command to execute.
		\param arg1 Optional argument 1. See the documentation where the cmd option is discussed
		for more details on these parameters.
		\param arg2 Optional argument 2.
		\param arg3 Optional argument 3.
		\return  An integer return value. See the documentation where the cmd option is
		discussed for more details on the meaning of this value. */
		virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0)
		{
			UNUSED_PARAM(cmd);
			UNUSED_PARAM(arg1);
			UNUSED_PARAM(arg2);
			UNUSED_PARAM(arg3);
			return 0;
		}
		
		enum 
		{
			//! \brief Command id to be passed to ViewExp::Execute in order to get the viewport's ViewExp10 interface
			kEXECUTE_GET_VIEWEXP_10 = 0,
			//! \brief Command id to be passed to ViewExp::Execute in order to get the viewport's unique ID (different from its index)
			kEXECUTE_GET_VIEWPORT_ID = 1,
			//! \brief Command id to be passed to ViewExp::Execute in order to get the viewport's ViewExp11 interface
			kEXECUTE_GET_VIEWEXP_11 = 2,
			//! \brief Command id to be passed to ViewExp::Execute in order to get the viewport's ViewExp13 interface
			kEXECUTE_GET_VIEWEXP_13 = 3,
		};
	};

//! \brief Extends ViewExp 
/*! The following code example shows how to get access to this interface:
\code
	// hWnd is the handle of a viewport window. This is passed to MouseCallback::proc
	ViewExp& vp = ip->GetViewExp(hWnd);
	ViewExp10* vp10 = NULL;
	if (vp.IsAlive())
		vp10 = reinterpret_cast<ViewExp10*>(vp.Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10));
\endcode
*/
class ViewExp10 : public ViewExp
{
	public:
		//! \brief Destructor
		virtual ~ViewExp10() { }

		/*! \name Viewport navigation methods
		Viewport navigation is usually implemented by viewport command modes (\ref VIEWPORT_COMMAND).
		The viewport should be invalidated by the command mode's mouse callback procedure
		in response to user input that change the viewpoint or some view parameters. 
		See ViewExp10::Invalidate for more details.
		Viewport navigation commands should be undoable\\redoable. 
		See ViewExp10::UndoBegin and ViewExp10::UndoAccept for more details.
		The following methods work with all view types except object based ones (camera, light, etc).
		\see class CommandMode, class MouseCallBack, Sample code MaxSDK\\HowTo\\ViewportNavigation */
		//!@{
		/*! \brief Pans the viewport 
		\param [in] screenDelta - Screen space pan amount.  */
		virtual void Pan(const Point2& screenDelta) = 0;

		/*! \brief Zooms the viewport. 
		This is equivalent to the Interactive Zoom tool in the UI.
		When used on a perspective view, it ultimately modifies the focal distance, and moves the camera.
		When used on an orthogonal or isometric view, it simply calls an internal zoom function.
		Values between 0.0 and 1.0 will zoom in, and values greater than 1.0 will zoom out. For instance, 
		zooming in with a value of 0.5 will enlarge the apparent display, and effectively half the focal distance
		for perspective views. Zooming out with a value of 2.0 will effectively double the focal distance 
		for perspective views.
		Note: Setting this value to zero will set the focal distance to zero. This is highly 
		discouraged. To recover from a zero assignment, either set the focal distance
		to a positive number, or zoom using the middle mouse button.
		\param [in] zoomFactor - Zoom factor to apply to the viewport. */
		virtual void Zoom(float zoomFactor) = 0;

		/*! \brief Rotates the viewport.
		This is equivalent to the Viewport Rotation tool in the UI. 
		\param [in] rotData - Rotation to apply to the viewport.
		\param [in] center - pivot on which the viewport rotation is based. */
		virtual void Rotate(const Quat& rotData, const Point3& center = Point3::Origin) = 0;

		/*! \brief Sets the focal distance of the viewport.
		\param [in] dist - The new distance for the focal length */
		virtual void SetFocalDistance(float dist) = 0;
		//!@}

		//! \name Undo support
		//!@{
		//! \brief Marks the start of recording viewport changes for undo
		/*! Call this method before any change to the viewpoint or viewport parameters.
		Usually this method is called from within a MouseCallback::proc method in
		response to the start of a user interaction (\ref MOUSE_POINT, \ref MOUSE_KEYBOARD)
		*/
		virtual bool UndoBegin() = 0;
		//! \brief Marks the end of recording viewport changes for undo
		/*! Call this method after a change to the viewpoint or viewport parameters.
		Usually this method is called from within a MouseCallback::proc method in
		response to the end of a user interaction (\ref MOUSE_POINT sent to MouseCallback::proc with the last point)
		*/
		virtual bool UndoAccept() = 0;
		//!@}

		//! \name Viewport invalidation
		//!@{
		//! \brief Invalidates the viewport
		/*! A call to Interface::RedrawViews will cause the viewport to be redrawn
		\param [in] bForegroundOnly - If true, only the foreground plane is invalidated.
		If false, both foreground and background planes are invalidated.
		\see The topic named "Foreground\Background" in the 3ds Max SDK Programmer's Guide.
		*/
		virtual void Invalidate(bool bForegroundOnly = false) = 0;
		//! \brief Retrieves the validity state of the viewport foreground
		/*! \return true if the viewport foreground is valid, false otherwise
		*/
		virtual bool IsValid() const = 0;
		//!@}

		//! \brief Sets the Field of View for the viewport
		/*! This can be used on Perspective and Camera views. That is VIEW_PERSP_USER or VIEW_CAMERA from
			enum ViewType. To see the results of this call the viewports will have to be updated. For instance 
			you could call: Interface::RedrawViews()
			\param fov - The field of view in Radians to set the viewport to. Valid values are 0.0 to 2 PI. 
			\return TRUE if successful or FALSE if not. */
		virtual BOOL SetFOV(float fov) = 0;

		//! Gets the state of the active viewport current fps
		/*! Gets the state of the active viewport current play back fps. NOTE GetFPS returns the Adaptive degradation goal frame rate */
		virtual float GetViewportFPS() = 0;

		//! Gets the state of the active viewport clip scale value
		/*! This gets the state of the viewport clip scale value.  The minimum clip value is computed
		by taking the scene bounding box and multiplying by this value.  Making this value larger will
		make the scene display more accurately but bring the near clip plane farther from the camera.
		Making this value smaller will bring the near clip plane closer but have a more likely chance
		of have close polygons render incorrectly	
		*/
		virtual float GetViewportClipScale()= 0;
		//! Sets the state of the active viewport clip scale value
		/*! This sets the state of the viewport clip scale value.  The minimum clip value is computed
		by taking the scene bounding box and multiplying by this value.  Making this value larger will
		make the scene display more accurately but bring the near clip plane farther from the camera.
		Making this value smaller will bring the near clip plane closer but have a more likely chance
		of have close polygons render incorrectly
		\param fScale The scale value
		*/
		virtual void SetViewportClipScale(float fScale) = 0;

		//! \name Adaptive Degradation parameters
		//!@{
		
		//! \brief Gets Adaptive Deg. goal FPS
		/*! Gets Adaptive Deg. goal FPS.  This is the frame rate the adaptive deg system wants to hold */	
		virtual float GetAdaptiveDegGoalFPS() = 0;
		//! \brief Sets Adaptive Deg. goal FPS
		/*! Sets Adaptive Deg. goal FPS.  This is the frame rate the adaptive deg system wants to hold */
		virtual void SetAdaptiveDegGoalFPS(float fFPS) = 0;

		//! \brief Gets the state of the display mode current display option
		/*! Gets the state of the display mode current display option */
		virtual BOOL  GetAdaptiveDegDisplayModeCurrent() = 0;
		//! \brief Sets the display mode to the current display
		/*! Sets the display mode to the current display */
		virtual void  SetAdaptiveDegDisplayModeCurrent(BOOL bMode) = 0;

		//! \brief Gets the state of the display mode Fast Shaded display option
		/*! Gets the state of the display mode Fast Shaded display option */
		virtual BOOL  GetAdaptiveDegDisplayModeFastShaded() = 0;
		//! \brief Sets the state of the display mode Fast Shaded display option
		/*! Sets the state of the display mode Fast Shaded display option */
		virtual void  SetAdaptiveDegDisplayModeFastShaded(BOOL bMode) = 0;

		//! \brief Gets the state of the display mode Wireframe display option
		/*! Gets the state of the display mode Wireframe display option */
		virtual BOOL  GetAdaptiveDegDisplayModeWire() = 0;
		//! \brief Sets the state of the display mode Wireframe display option
		/*! Sets the state of the display mode Wireframe display option */
		virtual void  SetAdaptiveDegDisplayModeWire(BOOL bMode) = 0;

		//! \brief Gets the state of the display mode BoundingBox display option
		/*! Gets the state of the display mode BoundingBox display option */
		virtual BOOL  GetAdaptiveDegDisplayModeBox() = 0;
		//! \brief Sets the state of the display mode BoundingBox display option
		/*! Sets the state of the display mode BoundingBox display option */
		virtual void  SetAdaptiveDegDisplayModeBox(BOOL bMode) = 0;

		//! \brief Gets the state of the display mode Point display option
		/*! Gets the state of the display mode Point display option */
		virtual BOOL  GetAdaptiveDegDisplayModePoint() = 0;
		//! \brief Sets the state of the display mode Point display option
		/*! Sets the state of the display mode Point display option */
		virtual void  SetAdaptiveDegDisplayModePoint(BOOL bMode) = 0;

		//! \brief Gets the state of the display mode Hide display option
		/*! Gets the state of the display mode Hide display option */		
		virtual BOOL  GetAdaptiveDegDisplayModeHide() = 0;
		//! \brief Sets the state of the display mode Hide display option
		/*! Sets the state of the display mode Hide display option */		
		virtual void  SetAdaptiveDegDisplayModeHide(BOOL bMode) = 0;

		//! \brief Gets the state of the Draw Backface option
		/*! Gets the state of the Draw Backface option.  When on backfaces will be drawn when
		degrading improving performance*/		
		virtual BOOL  GetAdaptiveDegDrawBackface() = 0;
		//! \brief Sets the state of the Draw Backface option
		/*! Sets the state of the Draw Backface option.  When on backfaces will be drawn when 
		degrading improving performance*/		
		virtual void  SetAdaptiveDegDrawBackface(BOOL bDraw) = 0;

		//! \brief Gets the state of the Never Degrade Selected option
		/*! Gets the state of the Never Degrade Selected option.  When on selected objects will
		never get degraded allowing the user to focus on them*/	
		virtual BOOL  GetAdaptiveDegNeverDegradeSelected() = 0;
		//! \brief Sets the state of the Never Degrade Selected option
		/*! Sets the state of the Never Degrade Selected option.  When on selected objects will
		never get degraded allowing the user to focus on them*/	
		virtual void  SetAdaptiveDegNeverDegradeSelected(BOOL bNeverDegrade) = 0;

		//! \brief Gets the state of the Degrade to Default Light option
		/*! Gets the state of the Degrade to Default Light option.  When on the system will
		degrade the lights to just the default over the shoulder light */	
		virtual BOOL  GetAdaptiveDegDegradeLight() = 0;
		//! \brief Sets the state of the Degrade to Default Light option
		/*! Sets the state of the Degrade to Default Light option.  When on the system will
		degrade the lights to just the default over the shoulder light */	
		virtual void  SetAdaptiveDegDegradeLight(BOOL bDegradeLight) = 0;

		//! \brief Gets the state of the Never Redraw After Degrade option
		/*! Gets the state of the Never Redraw After Degrade option.  When on the system will
		remain in degraded state even after a mouse up.  On large scenes this helps pauses
		on full redraws*/	
		virtual BOOL  GetAdaptiveDegNeverRedrawAfterDegrade() = 0;
		//! \brief Sets the state of the Never Redraw After Degrade option
		/*! Sets the state of the Never Redraw After Degrade option.  When on the system will
		remain in degraded state even after a mouse up.  On large scenes this helps pauses
		on full redraws*/	
		virtual void  SetAdaptiveDegNeverRedrawAfterDegrade(BOOL bNeverRedraw) = 0;

		//! \brief Gets the state of the Distance From Camera Priority option
		/*! Gets the state of the Distance From Camera Priority option.  This weights whether the distance
		from camera or screen size is used to determine what gets degraded*/	
		virtual float  GetAdaptiveDegCameraDistancePriority() = 0;
		//! \brief Sets the state of the Distance From Camera Priority option
		/*! Sets the state of the Distance From Camera Priority option.  This weights whether the distance
		from camera or screen size is used to determine what gets degraded*/	
		virtual void  SetAdaptiveDegCameraDistancePriority(float fPriority) = 0;

		//! \brief Gets the state of the Screen Size Priority option
		/*! Gets the state of the Screen Size Priority option.  This weights whether the distance
		from camera or screen size is used to determine what gets degraded*/	
		virtual float  GetAdaptiveDegScreenSizePriority() = 0;
		//! \brief Sets the state of the Screen Size Priority option
		/*! Sets the state of the Screen Size Priority option.  This weights whether the distance
		from camera or screen size is used to determine what gets degraded*/	
		virtual void  SetAdaptiveDegScreenSizePriority(float fPriority) = 0;

		//! \brief Gets the state of the Minimum Draw Size option
		/*! Gets the state of the Minimum Draw Size option.  Anything smaller than the Minimum Draw Size
		in screen space will not be drawn when the system is degrading*/	
		virtual int   GetAdaptiveDegMinSize() = 0;
		//! \brief Sets the state of the Minimum Draw Size option
		/*! Sets the state of the Minimum Draw Size option.  Anything smaller than the Minimum Draw Size
		in screen space will not be drawn when the system is degrading*/	
		virtual void  SetAdaptiveDegMinSize(int iMinSize) = 0;


		//!@}

};

//! \brief Extends ViewExp10 
/*! The following code example shows how to get access to this interface:
\code
	// hWnd is the handle of a viewport window. This is passed to MouseCallback::proc
	ViewExp& vp = ip->GetViewExp(hWnd);
	ViewExp11* vp11 = NULL;
	if (vp.IsAlive())
		vp11 = reinterpret_cast<ViewExp11*>(vp.Execute(ViewExp::kEXECUTE_GET_VIEWEXP_11));
\endcode
*/
class ViewExp11 : public ViewExp10
{
	public:
		//! \brief Destructor
		virtual ~ViewExp11() { }

	//! \name Adaptive Degradation parameters
	//!@{
		//! \brief Gets whether or not we are in the process of drawing the viewports in an degraded state.
		/*! Gets whether or not we are drawing the viewports in an degraded state.  This flag thus can be queried
		    when an object is displaying itself and wants to do any custom drawing when adaptive degradation is occurring.*/		
		virtual BOOL IsDegrading() =0;
	//!@}
};

//! \brief Represents a viewport rectangle, but with coordinates in the range [0..1] instead of in pixels
class DeviceRect : public MaxHeapOperators
{
	// Warning - instances of this class are saved as a binary blob to scene file
	// Adding/removing members will break file i/o
public:
	DeviceRect() : top(0.0f), left(0.0f), bottom(0.0f), right(0.0f) {}
	float		top;
	float		left;
	float		bottom;
	float		right;
	};

//! \brief Extends ViewExp11 
/*! The following code example shows how to get access to this interface:
\code
	ViewExp& vp = ip->GetViewExp(hWnd);
	ViewExp13* vp13 = NULL;
	if (vp.IsAlive())
		vp13 = reinterpret_cast<ViewExp13*>(vp.Execute(ViewExp::kEXECUTE_GET_VIEWEXP_13));
\endcode
*/
class ViewExp13 : public ViewExp11
{
	public:
		//! \brief Destructor
		virtual ~ViewExp13() { }

		//! \brief Get whether the viewport uses one default light, or two.
		/*! Returns TRUE if the viewport uses one default light, FALSE if two. */
		virtual BOOL GetSingleDefaultLight() = 0;

		//! \brief Updates the viewport label, if the display mode has changed.
		/*! May be needed when displaying a camera view, and the camera name is changed. Primarily for internal use. */
		virtual void UpdateLabel() = 0;

		//! \brief Get the zoom value of the viewport.
		/*! Gets the zoom value of the viewport. */
		virtual float GetZoom() = 0;

		//! \brief Get the hither clipping value of the viewport.
		/*! Get the hither clipping value of the viewport. */
		virtual float GetHither() = 0;

		//! \brief Get the yon clipping value of the viewport.
		/*! Get the yon clipping value of the viewport. */
		virtual float GetYon() = 0;

		//! \brief Returns the viewport region rectangle as a DeviceRect.
		/*! Gets the viewport region rectangle, similar to Interface7::GetRegionRect(),
		    but with coordinates in the range [0..1] instead of in pixels. */
		virtual DeviceRect GetRegionDeviceRect() = 0;

		//! \brief Sets the viewport region rectangle as a DeviceRect.
		/*! Sets the viewport region rectangle, similar to Interface7::SetRegionRect(),
		    but with coordinates in the range [0..1] instead of in pixels. */
		virtual void SetRegionDeviceRect(DeviceRect r) = 0;

		//! \brief Returns the viewport blow-up rectangle as a DeviceRect.
		/*! Gets the viewport blow-up rectangle, similar to Interface7::GetBlowupRect(), 
		    but with coordinates in the range [0..1] instead of in pixels. */
		virtual DeviceRect GetBlowupDeviceRect() = 0;

		//! \brief Sets the viewport blow-up rectangle as a DeviceRect.
		/*! Sets the viewport blow-up rectangle, similar to Interface7::SetBlowupRect(),
		    but with coordinates in the range [0..1] instead of in pixels. */
		virtual void SetBlowupDeviceRect(DeviceRect r) = 0;

		//! \brief Returns the viewport bounding box of the current node selection, as a DeviceRect.
		/*! Gets the viewport bounding box rectangle of the selection, at the given slider time,
		    with coordinates in the range [0..1] instead of in pixels . */
		virtual DeviceRect GetSelDeviceRect(TimeValue t) = 0;

		//! \brief Sets if drawing edges faces should be turned on in the viewport.
		/*! A non-zero input value turns it on, 0 turns it off. */
		virtual void SetEdgedFaces(int md) = 0;

		//! \brief Gets if drawing edges faces is turned on in the viewport.
		/*! Returns non-zero if edged faces is on. */
		virtual int GetEdgedFaces(void) = 0;

		//! \brief Query the world space intersection point under the specified cursor position.
		/*! Returns true if the cursor hit any scene object, otherwise return false. */
		virtual bool QueryCursorSceneIntersection(float x, float y, Point3& intersection) = 0;
};

//! \brief Extends ViewExp13 
/*! The following code example shows how to get access to this interface:
\code
	ViewExp18& viewExp18 = static_cast<ViewExp18&>(GetCOREInterface7()->getViewExp(0));
\endcode
*/
class ViewExp18 : public ViewExp13
{
public:
	/** Get access to a nitrous render view instance.
	\sa class IRenerView2, class IRenderView
	*/
	virtual MaxSDK::Graphics::IRenderView2* GetRenderView() = 0;

	/** Append string on current viewport's popup tooltip
	This function only available during NOTIFY_PRE_VIEWPORT_TOOLTIP. 
	Max Script event callback "preViewportTooltip" uses this notification to let script user modify current tooltip.
	Each callback can append string to current tip content. The string format follows QT's rich text rule.
	\param content The string append to current tip.
	*/
	virtual void AppendTooltip(const MSTR& content) = 0;
};

//! \brief Extends ViewExp18
/*! The following code example shows how to get access to this interface:
\code
ViewExp22& viewExp22 = static_cast<ViewExp22&>(GetCOREInterface7()->getViewExp(0));
\endcode
*/
class ViewExp22 : public ViewExp18
{
public:
	/** Returns the picking selection radius scaling factor. This is used with the mouse cursor size to calculate the selection radius.
	* The selection radius scale is a global property.
	\return Returns the selection radius scaling factor.
	*/
	virtual float GetPickingSelectionRadiusScale() const = 0;

	/** Sets the picking selection radius scaling factor. This is used with the mouse cursor size to calculate the selection radius.
	* The selection radius scale is a global property.
	\param selectionRadiusScale - The new selection radius scale value. Clamped between [0.0001, 1000.0].
	*/
	virtual void SetPickingSelectionRadiusScale(float selectionRadiusScale) = 0;

	/** Returns the picking selection radius in pixels. The selection radius is a function of the cursor size (which scales with
	* the screen DPI) and the selection radius scale factor.
	* The selection radius scale is a global property.
	\return Returns the selection radius in pixels.
	*/
	virtual int GetPickingSelectionRadius() const = 0;

	/** Sets the picking selection pixel radius.
	* The selection radius scale is a global property.
	\param selectionRadius - The new selection radius value in pixels.
	*/
	virtual void SetPickingSelectionRadius(int selectionRadius) = 0;

	/**
    * Set this ViewExp in presentation mode on or off
    * The presentation mode works only on viewport in floating view panels.
	* It is a mode where there are no title bar, no grid, no view cube, no gizmos...
	* The view in presentation mode is always updated even if it is not the active view
    */
    virtual void SetInPresentationMode(bool onoff) = 0;

	/**
    * Is this ViewExp in presentation mode ?
    * The presentation mode works only on viewport in floating view panels.
	* It is a mode where there are no title bar, no grid, no view cube, no gizmos...
	* The view in presentation mode is always updated even if it is not the active view
    * \return Return true if it is in presentation mode.
    */
    virtual bool IsInPresentationMode() const = 0;
};

//! \brief Extends ViewExp22
/*! The following code example shows how to get access to this interface:
\code
ViewExp23& viewExp23 = static_cast<ViewExp22&>(GetCOREInterface7()->getViewExp(0));
\endcode
*/
class ViewExp23 : public ViewExp22
{
public:
    /** Get access to the IPerViewportFilter to know if an INode is filtered in a Viewport.
    */
    virtual MaxSDK::Graphics::IPerViewportFilter* GetIPerViewportFilter(void) = 0;
};


// return values for CreateMouseCallBack
#define CREATE_CONTINUE 1
#define CREATE_STOP	0	    // creation terminated normally
#define CREATE_ABORT -1		// delete the created object and node

//JH 5/15/01
//return values for MouseCallback
//Historically this method just returned 0 or 1 to indicate if processing should continue
//Now we'll add a return value to indicate that the Mouse proc requires window capture
#define MOUSE_STOP	0
#define MOUSE_CONTINUE	1
#define MOUSE_CAPTURE_CONTINUE 2

// This is a specific call-back proc for handling the creation process
// of a plug-in object.
// The vpt passed in will have had setTransform called with the 
// transform of the current construction plane.
/*! \sa  Class BaseObject (method <b><b>GetCreateMouseCallBack()</b></b>), 
Class ViewExp,  Class Matrix3, Class IPoint2.\n\n
\par Description:
This is the callback object for handling the creation process of a plug-in
object.  */
class CreateMouseCallBack: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~CreateMouseCallBack() {}
		/*! \remarks This is the method where the developer defines the user / mouse
		interaction that takes place during the creation phase of an object.
		\par Parameters:
		<b>ViewExp *vpt</b>\n\n
		The viewport the creation process is taking place in.\n\n
		<b>int msg</b>\n\n
		This message describes the type of event that occurred. See
		\ref Mouse_Callback_Messages
		<b>int point</b>\n\n
		The point number. this is 0 for the first click, 1 for the second,
		etc.\n\n
		<b>int flags</b>\n\n
		These flags describe the state of the mouse button and keyboard
		Shift/Ctrl/Alt keys. See \ref Mouse_Call_Back_Flags
		<b>IPoint2 m</b>\n\n
		The 2D screen point that the user clicked on. Methods in the viewport
		interface allow this point to be converted into a world space ray or a
		3D view space point. A world space ray can be intersected with the
		active construction plane which results in a point on the active
		construction plane. See Class ViewExp.\n\n
		<b>Matrix3\& mat</b>\n\n
		This represents the transformation of the object relative to the
		construction plane. Typically the plug-in would get a point on the
		construction plane based on the screen point that the user clicked on
		and set the translation component of this matrix based on that point.
		\return  Return one of the following value to indicate the state of the
		creation process:\n\n
		<b>CREATE_CONTINUE</b>\n\n
		The creation process should continue. In this case the mouse is
		captured.\n\n
		<b>CREATE_STOP</b>\n\n
		The creation process has terminated normally. In this case the mouse is
		no longer captured and input is then allowed again from any
		viewport.\n\n
		<b>CREATE_ABORT</b>\n\n
		The creation process has been aborted. The system will delete the
		created object and node.
		\note The Interface::StopCreating() method should not be called from a create 
		call-back procedure, but instead the proper code should be returned to end the 
		plugin creation process.*/
		virtual int proc( 
			ViewExp *vpt,
			int msg, 
			int point, 
			int flags, 
			IPoint2 m,
			Matrix3& mat
			)=0;
		/*! \remarks This method is used to override the default drag mode. Most plug-in
		will not need to replace the default implementation of this method.
		What this does is change the way the messages are sent relative to the
		mouse clicking.\n\n
		Normally the messages are sent as follows: When the user clicks down
		this generates a <b>MOUSE_POINT</b> message. Then the user drags the
		mouse with the button down and a series of <b>MOUSE_MOVE</b> messages
		are sent. When they let up on the mouse button a <b>MOUSE_POINT</b>
		messages is generated. Then as the mouse is moved a series of
		<b>MOUSE_MOVE</b> messages are sent. Then they click down on the mouse
		again, but this time a point message is not generated until the button
		is released. All future points are then only sent after the mouse
		button has been pressed and released.
		\par Parameters:
		<b>int mode</b>\n\n
		The current drag mode. See below.
		\return  One of the following drag modes should be returned:\n\n
		<b>CLICK_MODE_DEFAULT</b>\n\n
		Returned to indicate the use of the system mouse mode.\n\n
		<b>CLICK_DRAG_CLICK</b>\n\n
		This is the default behavior as described above.\n\n
		<b>CLICK_MOVE_CLICK</b>\n\n
		In this mode, the first point is entered by clicking the mouse button
		down and then letting it up. This generates point 0. In other words, a
		<b>MOUSE_POINT</b> message is only generated after the user has pressed
		and released the mouse button.\n\n
		<b>CLICK_DOWN_POINT</b>\n\n
		In this mode, point messages are sent on mouse-down only.
		<b>CLICK_TWO_POINTS</b>\n\n
		In this mode, each mouse click sends 2 MOUSE_POINT messages. One for button down, the other for button up.
		\par Default Implementation:
		<b>{ return mode; }</b>
		\par Sample Code:
		A sample program that uses the override method is
		<b>/MAXSDK/SAMPLES/OBJECTS/SPLINE.CPP.</b>It uses
		<b>CLICK_DOWN_POINT</b>. */
		virtual int override(int mode) { return mode; }	// Defaults to mode supplied

        // Tells the system that we aren't starting a new creation
        /*! \remarks This gets called by the CreationManager to determine if the mouse proc
        is really starting a new object. The mouse proc for creating always
        returns CREATE_STOP, which is how it keeps the mouse from being
        captured, and this function tells the system if it is really ready to
        start a new object. Thus, this is called only if the mouse proc
        returned CREATE_STOP to see if the object is really in a state to start
        a new node.
        \return  TRUE if the mouse proc is ready to start a new object;
        otherwise FALSE.
        \par Default Implementation:
        <b>{ return TRUE; }</b> */
        virtual BOOL StartNewCreation() { return TRUE; }

		//Tells the system if ortho mode makes sense for this creation
		//In general this won't be true but splines and such...
		/*! \remarks Called by the system to determine if ortho mode makes sense for this
		creation. Typically this only makes sense for splines and NURBS curves.
		\return  TRUE if ortho mode is okay; otherwise FALSE.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL TolerateOrthoMode() {	return FALSE; }
	};


class Object;
class ConstObject;
class MouseCallBack;
class IObjCreate;
class IObjParam;
class ModContext;
class ModContextList;
class INodeTab;

// Passed to getBkgFrameRange()
#define VIEWPORT_BKG_START			0
#define VIEWPORT_BKG_END			1

// Passed to setBkgORType().
#define VIEWPORT_BKG_BLANK			0
#define VIEWPORT_BKG_HOLD			1
#define VIEWPORT_BKG_LOOP			2

// Passed to setBkgImageAspect()
#define VIEWPORT_BKG_ASPECT_VIEW	0
#define VIEWPORT_BKG_ASPECT_BITMAP	1
#define VIEWPORT_BKG_ASPECT_OUTPUT	2


// Identify the transform tool buttons
#define MOVE_BUTTON		1
#define ROTATE_BUTTON	2
#define NUSCALE_BUTTON	3
#define USCALE_BUTTON	4
#define SQUASH_BUTTON	5
#define SELECT_BUTTON	6
#define PLACE_BUTTON	7

// Axis constraints.
#define AXIS_XY		2
#define AXIS_ZX		1
#define AXIS_YZ		0
#define AXIS_X		3
#define AXIS_Y		4
#define AXIS_Z		5


// Origin modes		
#define ORIGIN_LOCAL		0	// Object's pivot
#define ORIGIN_SELECTION	1	// Center of selection set (or center of individual object for local or parent space)
#define ORIGIN_SYSTEM		2	// Center of the reference coord. system

// Reference coordinate system
#define COORDS_HYBRID	0
#define COORDS_SCREEN	1
#define COORDS_WORLD	2
#define COORDS_PARENT	3
#define COORDS_LOCAL	4
#define COORDS_OBJECT	5
#define COORDS_GIMBAL	6
#define COORDS_WORKINGPIVOT	7
#define COORDS_LOCAL_ALIGNED 8

// Task Modes
#define TASK_MODE_CREATE		1
#define TASK_MODE_MODIFY		2
#define TASK_MODE_HIERARCHY		3
#define TASK_MODE_MOTION		4
#define TASK_MODE_DISPLAY		5
#define TASK_MODE_UTILITY		6

// Max cursors
#define SYSCUR_MOVE			1
#define SYSCUR_ROTATE		2
#define SYSCUR_USCALE		3
#define SYSCUR_NUSCALE		4
#define SYSCUR_SQUASH		5
#define SYSCUR_SELECT		6
#define SYSCUR_DEFARROW		7
#define SYSCUR_MOVE_SNAP	8

// flags to pass to RedrawViews
#define REDRAW_BEGIN		(1<<0)
#define REDRAW_INTERACTIVE	(1<<1)
#define REDRAW_END			(1<<2)
#define REDRAW_NORMAL		(1<<3)
#define REDRAW_RESET_PROGRESS		(1<<4)

// Return values for GetNumAxis()
#define NUMAXIS_ZERO		0 	// Nothing to transform
#define NUMAXIS_ALL			1	// Use only one axis.
#define NUMAXIS_INDIVIDUAL	2	// Do all, one at a time

// All macro's for APP_* directory paths have been moved to MaxDirectories.h

// Types for status numbers
#define STATUS_UNIVERSE					1
#define STATUS_SCALE					2
#define STATUS_ANGLE					3
#define STATUS_OTHER					4
#define STATUS_UNIVERSE_RELATIVE		5
#define STATUS_POLAR					6
#define STATUS_POLAR_RELATIVE			7


/*! \defgroup extendedDisplayModes Extended Display Modes 
These are flags used in Interface::SetExtendedDisplayMode() and
Interface::GetExtendedDisplayMode() for catching various display/selection/hittesting events. */
//!@{
#define EXT_DISP_NONE				0			//!< no flags
#define EXT_DISP_SELECTED			(1<<0)		//!< object is selected
#define EXT_DISP_TARGET_SELECTED	(1<<1)		//!< object's target is selected
#define EXT_DISP_LOOKAT_SELECTED	(1<<2)		//!< object's lookat node is selected
#define EXT_DISP_ONLY_SELECTED		(1<<3)		//!< object is only thing selected
#define EXT_DISP_DRAGGING			(1<<4)		//!< object is being "dragged"
#define EXT_DISP_ZOOM_EXT			(1<<5)		//!< object is being tested for zoom ext
#define EXT_DISP_GROUP_EXT			(1<<6)		//!< object is being tested for extents as member of group
#define EXT_DISP_ZOOMSEL_EXT		(1<<7)		//!< object is being tested for zoom selected ext
//!@}

// Render time types passed to SetRendTimeType()
#define REND_TIMESINGLE		0
#define REND_TIMESEGMENT	1
#define REND_TIMERANGE		2
#define REND_TIMEPICKUP		3

// Flag bits for hide by category.
#define HIDE_OBJECTS	0x0001
#define HIDE_SHAPES		0x0002
#define HIDE_LIGHTS		0x0004
#define HIDE_CAMERAS	0x0008
#define HIDE_HELPERS	0x0010
#define HIDE_WSMS		0x0020
#define HIDE_SYSTEMS	0x0040
#define HIDE_PARTICLES	0x0080
#define HIDE_BONEOBJECTS 0x0100
#define HIDE_NONRENDERABLES 0x0200
#define HIDE_ALL		0xffff
#define HIDE_NONE		0

// viewport layout configuration
//   VP_LAYOUT_ LEGEND
//		# is number of viewports (total) in view panel
//		V = vertical split
//		H = horizontal split
//		L/R	= left/right placement
//		T/B = top/bottom placement
//   CONSTANT LEGEND
//		bottom nibble is total number of views

/*! \defgroup ViewPanelLayoutConfigIds View Panel Layout Configurations
The following IDs represent the various default viewport configurations that
can be set using IViewPanel::SetLayout(int).
*/
//!@{
/*! 1 single full-screen viewport */
#define VP_LAYOUT_1			0x0001
/*! 2 vertical viewports */
#define VP_LAYOUT_2V		0x0012
/*! 2 horizontal viewports */
#define VP_LAYOUT_2H		0x0022
/*! 2 horizontal viewports, small on top */
#define VP_LAYOUT_2HT		0x0032
/*! 2 horizontal viewports, small on bottom */
#define VP_LAYOUT_2HB		0x0042
/*! 3 viewports, 2 vertically on left */
#define VP_LAYOUT_3VL		0x0033
/*! 3 viewports, 2 vertically on right */
#define VP_LAYOUT_3VR		0x0043
/*! 3 viewports, 2 horizontally on top */
#define VP_LAYOUT_3HT		0x0053
/*! 3 viewports, 2 horizontally on bottom */
#define VP_LAYOUT_3HB		0x0063
/* standard 4 equal viewports */
#define VP_LAYOUT_4			0x0074
/*! 4 viewports, 3 vertically on left */
#define VP_LAYOUT_4VL		0x0084
/*! 4 viewports, 3 vertically on right */
#define VP_LAYOUT_4VR		0x0094
/*! 4 viewports, 3 horizontally on top */
#define VP_LAYOUT_4HT		0x00a4
/*! 4 viewports, 3 horizontally on bottom */
#define VP_LAYOUT_4HB		0x00b4
//!@}

#define VP_LAYOUT_1C		0x00c1
#define VP_NUM_VIEWS_MASK	0x000f

/*! \defgroup interfaceSetNodeAttributes Interface7::SetNodeAttribute Defines
Node attribute flags used by Interface7::SetNodeAttribute */
//!@{
#define ATTRIB_HIDE								0	//!< If set, node is not visible in the viewport
#define	ATTRIB_FREEZE							1	//!< If set, node is frozen
#define	ATTRIB_BOXMODE							2	//!< If set, node is displayed as box
#define	ATTRIB_BACKCULL							3	//!< If set (default), faces with normals pointing away from the camera are not displayed. Applies only in wireframe viewport shading modes
#define	ATTRIB_ALLEDGES							4	//!< If set, all edges including hidden ones and polygon diagonals are displayed. Applies to all viewport shading modes where the edges are drawn
#define ATTRIB_LINKDISP							5	//!< If set, a wireframe representation of any hierarchical link (parent\\child, etc) to the node is displayed
#define ATTRIB_LINKREPL							6	//!< If set, the wireframe representation of the hierarchical link is displayed instead of the node
#define ATTRIB_UNSEL							7	//!< for internal use only; something is not selected
#define ATTRIB_HIDE_UNSEL						8	//!< for internal use only
#define ATTRIB_FREEZE_UNSEL						9	//!< for internal use only
#define ATTRIB_VERTTICKS						10	//!< If set, the vertices of the object are displayed as tick marks
#define ATTRIB_UNHIDE							11	//!< for internal use only
#define ATTRIB_UNFREEZE							12	//!< for internal use only
#define ATTRIB_CVERTS							13	//!< If set, the display of vertex color channel is enabled
#define ATTRIB_SHADE_CVERTS						14	//!< If set, the vertex color is shaded if the viewport is in a shaded display mode
#define ATTRIB_XRAY								15	//!< If set, makes the object translucent in the viewport
#define ATTRIB_IGNORE_EXT						16	//!< if set, the object is ignored by the Zoom Extents operation
#define ATTRIB_TRAJECTORY						17	//!< if set, the object's trajectory is displayed
#define ATTRIB_FRZMTL							18	//!< set, the object turns gray when it's frozen 
 // used by MXS - also unhides layer if hidden
#define ATTRIB_HIDE_WITH_LAYER					19	//!< if set, the hide\\unhide operation applies the whole layer the object is on
 // used by MXS - also unhides layer if hidden
#define ATTRIB_FREEZE_WITH_LAYER				20	//!< if set, the freeze\\unfreeze operation applies to the whole layer the object is on
#define ATTRIB_INHERITVISIBILITY				100	//!< if set, the object inherits a percentage of its visibility from its parent
#define ATTRIB_CASTSHADOWS						101	//!< If set, the object can cast shadows
#define ATTRIB_RECEIVESHADOWS					102	//!< If set, the object can receive shadows
#define ATTRIB_ISTARGET							103	//!< If set, the object is marked as the target of another object. Same as INode::SetIsTarget(BOOL b)
#define ATTRIB_RENDERABLE						104	//!< If set, the object is rendered
#define ATTRIB_RENDEROCCLUDED					105	//!< If set, the object becomes transparent from the point of view of special effects, thus the objects being occluded are affected by the special effects
#define ATTRIB_RCVCAUSTICS						106	//!< If set, the object can receive caustics
#define ATTRIB_GENERATECAUSTICS					107	//!< If set, the object can generate caustics
#define ATTRIB_RCVGLOBALILLUM					108	//!< If set, the object can receive global illumination
#define ATTRIB_GENERATEGLOBALILLUM				109	//!< If set, the object can generate global illumination
#define ATTRIB_SETWIRECOLOR						110	//!< If set, the supplied wire frame color is set on to the object
#define ATTRIB_SETGBUFID						111	//!< If set, tags an object as target for a render effect based on the supplied G-buffer channel
#define ATTRIB_PRIMARYVISIBILITY				112	//!< If set, the object is visible to cameras to an extent specified by the visibility parameter
#define ATTRIB_SECONDARYVISIBILITY				113	//!< If set, the object appears in rendered reflections and refractions
#define ATTRIB_MOTIONBLUR						114	//!< If set, motion blur is enabled for the object
//!@}


/*! \defgroup interfaceExecCommandOptions Interface::Execute Command Options
See Interface::Execute*/
//!@{
/*! For example:
\code
static GenericCallback mycb;
GetCOREInterface()->Execute(I_EXEC_REGISTER_POSTSAVE_CB,&mycb);
\endcode
and when you are done:
\code
GetCOREInterface()->Execute(I_EXEC_UNREGISTER_POSTSAVE_CB,&mycb);
\endcode
For an alternative approach, see Structure NotifyInfo. */
#define  I_EXEC_REGISTER_POSTSAVE_CB  1001 //!< Registers a callback which is called after a File / Save operation has occurred.
#define  I_EXEC_UNREGISTER_POSTSAVE_CB  1002 //!< Un-registers a callback which is called after a File / Save operation has occurred.
#define  I_EXEC_REGISTER_PRESAVE_CB  1003 //!< Registers a callback which is called before a File / Save operation has occurred.
#define  I_EXEC_UNREGISTER_PRESAVE_CB  1004 //!< Un-registers a callback which is called before a File / Save operation has occurred.
//!@}

class GenericCallback: public MaxHeapOperators {
 public:
	 virtual ~GenericCallback() {}
  virtual void Callme()=0;
 };

// A callback object passed to RegisterTimeChangeCallback()
/*! \sa  Class Interface.\n\n
\par Description:
A callback object passed to <b>Interface::RegisterTimeChangeCallback()</b>. The
method <b>TimeChanged()</b>is called every time the current animation time is changed.  */
class TimeChangeCallback: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~TimeChangeCallback() {}
		/*! \remarks This method is called every time the current animation time is changed.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The new animation time value.\n\n
		\sa  Methods <b>RegisterTimeChangeCallback()</b> and
		<b>UnRegisterTimeChangeCallback()</b> in
		Class Interface. */
		virtual void TimeChanged(TimeValue t)=0;
	};


// A callback object passed to RegisterCommandModeChangeCallback()
/*! \sa  Class Interface, Class CommandMode.\n\n
\par Description:
This is the callback object for <b>Interface::
RegisterCommandModeChangedCallback()</b>.  */
class CommandModeChangedCallback: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~CommandModeChangedCallback() {}
		/*! \remarks This method is called when the user changes command
		modes.
		\par Parameters:
		<b>CommandMode *oldM</b>\n\n
		The command mode that was replaced.\n\n
		<b>CommandMode *newM</b>\n\n
		The new command mode. */
		virtual void ModeChanged(CommandMode *oldM, CommandMode *newM)=0;
	};

//! Interface ID used with interface class ISceneCallbackPriority
#define SCENE_CALLBACK_PRIORITY_INTERFACE Interface_ID(0xc5b791e, 0x66e58a4)

/*! An interface to set priority to ViewportDisplayCallback's.
	Used in conjunction with class ViewportDisplayCallback. Callbacks with 
	higher priority will be called earlier than callbacks
	with lower priority. 
	Larger number represents higher priority. ViewportDisplayCallback's that 
	do not implement this interface (such as most internal callbacks) will 
	have priority 0.  */
class ISceneCallbackPriority : public BaseInterface
{
public:
	//! Get the priority of this scene callback.
	//! \return Return the priority of this scene callback.
	virtual int GetPriority() const = 0;

	virtual BaseInterface* GetInterface(Interface_ID id)
	{
		return (id == SCENE_CALLBACK_PRIORITY_INTERFACE) ?
			this : BaseInterface::GetInterface(id);
	}
};

/*! \brief Enables non-geometric plugins to draw in the viewport. 
This class is a callback object that enables plug-ins that aren't actually
objects (such as utility plug-ins) to draw in the 3ds Max viewports. 
Plugin classes that sub-class this can also assign a numerical priority to their callbacks 
by also implementing the ISceneCallbackPriority interface.
See the following methods in class Interface that register and unregister this callback
object:
\li Interface::RegisterViewportDisplayCallback()
\li Interface::UnRegisterViewportDisplayCallback()
\li Interface::NotifyViewportDisplayCallbackChanged()
\sa  Class Interface, Class ViewExp */
class ViewportDisplayCallback: public InterfaceServer {
	public:
		/*! \remarks Destructor. */
		virtual ~ViewportDisplayCallback() {}

		/*! This method is called to allow the plug-in to draw in the viewports.
		\param t - The current time when this method is called.
		\param vpt - An interface into the viewport.
		\param flags - These flags are used internally. */
		virtual void Display(TimeValue t, ViewExp *vpt, int flags)=0;
		
		/*! Retrieves the dimensions of the specified viewport given an ViewExp interface to it.
		\param t - The time to get the viewport rectangle.
		\param vpt - Specifies which viewport
		\param rect - The rectangle is returned here. */
		virtual void GetViewportRect( TimeValue t, ViewExp* vpt, Rect* rect )=0;

		/*! \brief Flags whether to draw in the foreground or background plane. 
		This method should return TRUE if the object changes a
		lot or FALSE if it doesn't change very much. This method relates to the
		foreground/background display system used by 3ds Max. Basically, items
		that change a lot are placed in the foreground buffer. Items that don't
		change much are placed in the background buffer and simply blitted to
		the display. See the Advanced Topics section on
		~{ Foreground / Background Planes }~ for more details. Most plug-ins can simply
		return TRUE because they are not likely to be very heavyweight objects
		(they are usually just a gizmo or apparatus image) and can simply go
		into the foreground. On the other hand, some items, for instance the
		3ds Max home grid, don't change and can always go into the background.
		The home grid only changes when the view direction is changed in which
		case everything is redrawn. */
		virtual BOOL Foreground()=0;
	};

//! \brief A callback object that will get called before the program exits.
/*! Register an instance of this class with Interface::RegisterExitMAXCallback. 
During normal shutdowns, the Exit method will be called. The Exit method is not called 
during shutdown after crash recovery.
\see ExitMAXCallback2 */
class ExitMAXCallback: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~ExitMAXCallback() {}
		//! Method called on normal shutdown
		//! \param[in] hWnd - MAX main window handle is passed in. 
		//! \return FALSE to abort the exit, TRUE otherwise.
		virtual BOOL Exit(HWND hWnd)=0;
	};

//! \brief A callback object that will get called before the program exits.
/*! Register an instance of this class with Interface8::RegisterExitMAXCallback. 
During normal shutdowns, the ExitMAXCallback::Exit method will be called. During shutdown after crash recovery,
the CrashRecoverExit method will be called. Since it is being called after crash recovery, the 
scene may or may not be in a stable state, do only absolutely necessary actions here.
*/
class ExitMAXCallback2 : public ExitMAXCallback {
public:
	//! Method called on shutdown after crash recovery.
	/*! \param [in] hWnd - MAX main window handle is passed in.
	*/
	virtual void CrashRecoverExit(HWND hWnd)=0;
};

//! \brief A callback object called for each sequence file created with Interface8::CreateRendImgSeq()
/*! Pass an instance of this class to Interface8::CreateRendImgSeq().
For each sequence file created, the callback method will be called.  The method can modify
the name of the sequence file or return false to prevent creation of the file.
*/
class CreateRendImgSeqCallback: public MaxHeapOperators {
public:
	/*! \remarks Destructor. */
	virtual ~CreateRendImgSeqCallback() {}
	//! Method called for each sequence file to create
    /*! \param[in, out] filename - Full path and name of the sequence file; may be modified by the callee
        \param[in] objType - 0 for the main render output, or 1 for render elements
        \param[in] objPtr - NULL for the main render output, or IRenderElement* for render elements
	    \return Return FALSE to prevent creation of the sequence file, TRUE otherwise. */
	virtual BOOL CreateRendImgSeq( MSTR& filename, int objType, void* objPtr )=0;
};

/*! \sa  Class Interface.\n\n
\par Description:
This class allows a custom file open dialog to be used. This object is set
using the method <b>Interface::SetMAXFileOpenDlg()</b>.  */
class MAXFileOpenDialog: public MaxHeapOperators {
	public:
	/*! \remarks Destructor. */
	virtual ~MAXFileOpenDialog() {}
	/*! \remarks This method is called to bring up the custom file open dialog. It request a
	file name from the user and stores the result in <b>fileName</b>.
	\par Parameters:
	<b>MSTR\& fileName</b>\n\n
	Set this to the file name chosen by the user.\n\n
	<b>MSTR* defDir</b>\n\n
	The default directory to look in.\n\n
	<b>MSTR* defFile</b>\n\n
	The default file name to use.
	\return  TRUE if the user OKed the dialog; FALSE on cancel. */
	virtual BOOL BrowseMAXFileOpen(MSTR& fileName, MSTR* defDir, MSTR* defFile) = 0;
	};

/*! \sa  Class Interface.\n\n
\par Description:
This class allows a custom file save dialog to be used. This object is set
using the method <b>Interface::SetMAXFileSaveDlg()</b>.  */
class MAXFileSaveDialog: public MaxHeapOperators {
	public:
	/*! \remarks Destructor. */
	virtual ~MAXFileSaveDialog() {}
	/*! \remarks This method is called to bring up the custom file save
	dialog. It request a file name from the user and stores the result in
	<b>fileName</b>.
	\par Parameters:
	<b>MSTR\& fileName</b>\n\n
	Set this to the file name chosen by the user.
	\return  TRUE if the user OKed the dialog; FALSE on cancel. */
	virtual BOOL BrowseMAXFileSave(MSTR& fileName) = 0;
	};

// A callback object to filter selection in the track view.
/*! \sa  Class Animatable.\n\n
\par Description:
This is the callback object used to filter selections in the track view. <br>
*/
class TrackViewFilter : public InterfaceServer{
	public:
		// Return TRUE to accept the anim as selectable.
		/*! \remarks This is the callback object proc used to filter selections in the track
		view.
		\par Parameters:
		<b>Animatable *anim</b>\n\n
		The item the user picked.\n\n
		<b>Animatable *client</b>\n\n
		The owner of the <b>anim</b>.\n\n
		<b>int subNum</b>\n\n
		The sub-animatable number of the <b>anim</b>.
		\return  Return TRUE to accept the anim as selectable; otherwise FALSE.
		*/
		virtual BOOL proc(Animatable *anim, Animatable *client, int subNum)=0;
		/*! \remarks This method allows the filter to control the color of the label text
		used for the anim.
		\par Parameters:
		<b>Animatable *anim</b>\n\n
		The item the user picked.\n\n
		<b>Animatable *client</b>\n\n
		The owner of the <b>anim</b>.\n\n
		<b>int subNum</b>\n\n
		The sub-animatable number of the <b>anim</b>.\n\n
		<b>COLORREF\& color</b>\n\n
		The color for the label text. See
		<a href="https://msdn.microsoft.com/en-us/library/dd183449(v=vs.85).aspx">COLORREF-DWORD format</a>.
		\return  TRUE for the Treeview to use the color in the <b>color</b>
		argument, FALSE to ignore that color and use the system default.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL TextColor(Animatable *anim, Animatable *client, int subNum, COLORREF& color)
		{
			UNUSED_PARAM(anim);
			UNUSED_PARAM(client);
			UNUSED_PARAM(subNum);
			UNUSED_PARAM(color);
			return FALSE;
		}
	};

// Stores the result of a track view pick
/*! \sa  Class ReferenceTarget, \par Description:
This class stores the result of a selection from the Track View Pick dialog.
\par Data Members:
<b>ReferenceTarget *anim;</b>\n\n
The item the user picked.\n\n
<b>ReferenceTarget *client;</b>\n\n
The owner of the <b>anim</b>.\n\n
<b>int subNum;</b>\n\n
Sub-animatable number of the <b>anim</b>. */
class TrackViewPick: public MaxHeapOperators {
	public:
		ReferenceTarget *anim;
		ReferenceTarget *client;
		int subNum;

		TrackViewPick() {anim=NULL;client=NULL;subNum=0;}

		/*! \remarks Compares this class instance to another one */
		bool operator ==( const TrackViewPick& b ) const
		{
			return anim == b.anim && client == b.client && subNum == b.subNum;
		}
	};

// A callback object passed to SetPickMode()
/*! \sa  Class IObjParam, Class ViewExp, Class Interface, Class IPoint2.\n\n
\par Description:
This class is the callback object passed to <b>Interface::SetPickMode()</b>.
 */
#pragma warning(push)
#pragma warning(disable:4100)
class PickModeCallback : public InterfaceServer
{
	public:
		// Called when ever the pick mode needs to hit test. Return TRUE if something was hit
		/*! \remarks This method is called whenever the pick mode needs to hit test.
		\par Parameters:
		<b>IObjParam *ip</b>\n\n
		An interface pointer available to call functions defined by 3ds
		Max.\n\n
		<b>HWND hWnd</b>\n\n
		The window handle.\n\n
		<b>ViewExp *vpt</b>\n\n
		An interface pointer that may be used to call methods associated with
		the viewports.\n\n
		<b>IPoint2 m</b>\n\n
		Point to check in screen coordinates.\n\n
		<b>int flags</b>\n\n
		\ref SceneAndNodeHitTestFlags</a>.
		\return  Return TRUE if something was hit; otherwise FALSE.
		\par Sample Code:
		<b>// This implementation use the Interface::PickNode method</b>\n\n
		<b>// to perform the hit test.</b>\n\n
		\code
		{
			return ip->PickNode(hWnd,m,&thePickFilt)? TRUE:FALSE;
		}
		\endcode */
		virtual BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)=0;
		
		// Called when the user picks something. The vpt should have the result of the hit test in it.
		// return TRUE to end the pick mode.
		/*! \remarks This method is called when the user picks something.
		\par Parameters:
		<b>IObjParam *ip</b>\n\n
		An interface pointer available to call functions defined by 3ds
		Max.\n\n
		<b>ViewExp *vpt</b>\n\n
		An interface pointer that may be used to call methods associated with
		the viewports. The <b>vpt</b> should have the result of the hit test in
		it.
		\return  Return TRUE to end the pick mode; FALSE to stay in the pick
		mode. Note that returning TRUE will set the command mode to MOVE. When
		a plug-in is in the create branch, setting the command mode to move
		ends the creation process. */
		virtual BOOL Pick(IObjParam *ip,ViewExp *vpt)=0;

		//Called when a node or controller is picked from Trackview or Schematic view
		//return TRUE if the Pick Mode callback accepts the animatable object.
		//the track will not be selected in trackview if the this function returns FALSE
		/*! \remarks This method gets called when a node or controller is picked from
		Trackview or Schematic view The track will not be selected in Trackview
		if the this function returns FALSE. Override this function if you wish
		to support and receive nodes or controllers picked from Trackview.
		\par Parameters:
		<b>Animatable* anim</b>\n\n
		A pointer to the animatable object.
		\return  TRUE if the Pick Mode callback accepts the animatable object,
		otherwise FALSE.
		\par Default Implementation:
		The default implementation will test if anim is an INode, and if so,
		will set it as the closest HitRecord in a view and pass it on to Pick */
		CoreExport virtual BOOL PickAnimatable(Animatable* anim);

		// Called when the user right-clicks or presses ESC
		// return TRUE to end the pick mode, FALSE to continue picking
		/*! \remarks This method is called when the user right-clicks or presses ESC.
		\par Parameters:
		<b>IObjParam *ip</b>\n\n
		An interface pointer available to call functions defined by 3ds
		Max.\n\n
		<b>ViewExp *vpt</b>\n\n
		An interface pointer that may be used to call methods associated with
		the viewports. The <b>vpt</b> should have the result of the hit test in
		it.
		\return  TRUE to end the pick mode, FALSE to continue picking.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{ return FALSE; }

		// Called when the mode is entered and exited.
		/*! \remarks Called when the mode is entered. The developer may provide any
		pre-processing here.
		\par Parameters:
		<b>IObjParam *ip</b>\n\n
		An interface pointer available to call functions defined by 3ds Max. */
		virtual void EnterMode(IObjParam *ip) {}
		/*! \remarks Called when the mode is exited. The developer may provide any
		post-processing here.
		\par Parameters:
		<b>IObjParam *ip</b>\n\n
		An interface pointer available to call functions defined by 3ds Max. */
		virtual void ExitMode(IObjParam *ip) {}

		/*! \remarks Called to get the default cursor to use.
		\return  The handle of the default cursor.
		\par Default Implementation:
		<b>{return NULL;}</b> */
		virtual HCURSOR GetDefCursor(IObjParam *ip) {return NULL;}
		/*! \remarks Called to get the hit test cursor to use.
		\return  The handle of the hit test cursor.
		\par Default Implementation:
		<b>{return NULL;}</b> */
		virtual HCURSOR GetHitCursor(IObjParam *ip) {return NULL;}

		// If the user hits the H key while in your pick mode, you
		// can provide a filter to filter the name list.
		/*! \remarks This method is called if the user hits the H key while in your pick
		mode. You can provide a filter to filter the name list. See
		Class PickNodeCallback.
		\return  A pointer to an instance of <b>PickNodeCallback</b>.
		\par Default Implementation:
		<b>{return NULL;}</b> */
		virtual PickNodeCallback *GetFilter() {return NULL;}

		/*! \remarks This method is called when the PickAnimatable mode is started. See 
		PickModeCallback::PickAnimatable method.  You can provide a filter to filter the Animatable. The filter can be used 
		differently depending on the context to identify Animatables that can be picked or not.
		See Class PickAnimatableCallback.
		\return  A pointer to an instance of <b>PickAnimatableCallback</b>.
		\par Default Implementation:
		<b>{return NULL;}</b> */
		virtual PickAnimatableCallback *GetAnimatableFilter() {return NULL;}

		// Return TRUE to allow the user to pick more than one thing.
		// In this case the Pick method may be called more than once.
		/*! \remarks Implement this method to return TRUE to allow the user to pick more
		than one thing. In that case the <b>Pick()</b> method may be called
		more than once.
		\return  TRUE to allow multiple picks; otherwise FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL AllowMultiSelect() {return FALSE;}
	};

#pragma warning(pop)
// Not to be confused with a PickMODEcallback...
// Used to filter node's during a hit test (PickNode)
/*! \sa  Class Interface.\n\n
\par Description:
This class is used to filter nodes during a hit test. See the methods
<b>Interface::PickNode()</b>. and <b>PickModeCallback::GetFilter()</b>.  */
class PickNodeCallback: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
	virtual ~PickNodeCallback() {;}
		// Return TRUE if this is an acceptable hit, FALSE otherwise.
		/*! \remarks This method should return TRUE if the node passed is an acceptable hit
		and FALSE otherwise.
		\par Parameters:
		<b>INode *node</b>\n\n
		The node to test.
		\return  Return TRUE if this is an acceptable hit, FALSE otherwise. */
		virtual BOOL Filter(INode *node)=0;

	};

/*! \sa  Class Interface.\n\n
\par Description:
This class is used to filter Animatables during a hit test. See the methods
<b>Interface::PickAnimatable()</b>. and <b>PickAnimatableCallback::GetAnimatableFilter()</b>.  */
class PickAnimatableCallback: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
	virtual ~PickAnimatableCallback() {;}

		/*! \remarks This method should return TRUE if the Animatable passed is an acceptable hit
		and FALSE otherwise. This is used in the case a PickAnimatable was started.
		\par Parameters:
		<b>Animatable *animatable</b>\n\n
		The animatable to test.
		\return  Return TRUE if this is an acceptable hit, FALSE otherwise. */
		virtual BOOL Filter(Animatable *anim)=0;
	};

/*! \sa  Class Interface13.\n\n
\par Description:
This allows added filtering of Classes that the user will be able to choose from in <b>Interface13::DoPickClassDlg()</b> below.
If an instance is passed to <b>DoPickClassDlg()</b>, it is called for each candidate Class found that matches
the passed SClass_ID. If <b>IsValidClassDesc()</b> returns true, the Class will be added to the dialog's list for the user
to choose from.
*/
class PickClassCallback : public MaxHeapOperators
{
	public:
	//! Destructor
	virtual ~PickClassCallback() {;}
		//! \brief Filters classes to display in the UI
		/*! This method should return true if the class passed is acceptable to list, false otherwise.
		\param[in] candidateClassDesc The candidate class
		\return  Return true if this is an acceptable class, false otherwise. */
		virtual bool IsValidClassDesc(ClassDesc *candidateClassDesc) = 0;
};

// Used with DoHitByNameDialog();
/*! \sa  Class Interface.\n\n
\par Description:
This is the callback object used with <b>Interface::DoHitByNameDialog()</b>.
 */
#pragma warning(push)
#pragma warning(disable:4100)
class HitByNameDlgCallback : public MaxHeapOperators
{
public:
	/*! \remarks Destructor. */
	virtual ~HitByNameDlgCallback() {}
	/*! \remarks Returns the title string displayed in the dialog.
	\par Default Implementation:
	<b>{ return _M(""); }</b> */
	virtual const MCHAR *dialogTitle()	{ return _M(""); }
	/*! \remarks Returns the text displayed in the 'Select' or 'Pick' button.
	\par Default Implementation:
	<b>{ return _M(""); }</b> */
	virtual const MCHAR *buttonText() 	{ return _M(""); }
	/*! \remarks Returns TRUE if the user may only make a single selection in the list at
	one time; otherwise FALSE.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL singleSelect()		{ return FALSE; }
	/*! \remarks This gives the callback the opportunity to filter out items from the list.
	This is called before the dialog is presented. It returns TRUE if the
	<b>filter()</b> method (below) should be called; otherwise FALSE.
	\par Default Implementation:
	<b>{ return TRUE; }</b> */
	virtual BOOL useFilter()		{ return TRUE; }
	/*! \remarks This method will be called if <b>useFilter()</b> above returned TRUE. This
	gives the callback the chance to filter out items from the list before they
	are presented to the user in the dialog. This is called once for each node
	that would otherwise be presented. Return nonzero to accept the item and
	zero to skip it.
	\par Parameters:
	<b>INode *node</b>\n\n
	The node to check for inclusion in the list.
	\return  Nonzero to accept the item and zero to skip it.
	\par Default Implementation:
	<b>{ return TRUE; }</b> */
	virtual int filter(INode *node)	{ return TRUE; }
	/*! \remarks Normally, when the user selects OK from the dialog, the system selects all
	the chosen nodes in the scene. At times a developer may want to do
	something other than select the chosen nodes. If this method returns TRUE
	then the nodes in the list will not be selected, but the <b>proc()</b>
	method is called instead (see below). If this method returns FALSE, then
	the nodes are selected in the scene and <b>proc()</b> is not called.
	\par Default Implementation:
	<b>{ return TRUE; }</b> */
	virtual BOOL useProc()			{ return TRUE; }
	/*! \remarks This allows the plug-in to process the nodes chosen from the dialog in any
	manner. For example if the developer wanted to delete the nodes chosen
	using this dialog, they would implement <b>useProc()</b> to return TRUE and
	this method to delete all the nodes in the table passed.
	\par Parameters:
	<b>INodeTab \&nodeTab</b>\n\n
	A table of those nodes selected by the user. See
	Template Class Tab.
	\par Default Implementation:
	<b>{}</b> */
	virtual void proc(INodeTab &nodeTab) {}
	/*! \remarks Normally, when the dialog is entered, the nodes in the scene that are
	selected are highlighted in the list. If this method returns TRUE, the
	developer may control which items are highlighted by implementing
	<b>doHilite()</b> (see below). If this method returns FALSE the selected
	nodes will have their names highlighted in the list.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL doCustomHilite()	{ return FALSE; }
	/*! \remarks This method is called for each item in the list if <b>doCustomHilite()</b>
	returns TRUE. This method returns TRUE or FALSE to control if each item is
	highlighted.
	\par Parameters:
	<b>INode *node</b>\n\n
	The node to check.
	\return  TRUE to highlight the node in the list; FALSE to not highlight it.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL doHilite(INode *node)	{ return FALSE; }
	/*! \remarks	This defaults to returning FALSE, which means that hidden and frozen
	objects will not be included in the select by name dialog list. If this
	method is overridden to return TRUE, then hidden and frozen nodes will be
	sent through the user-supplied filter as in version 1.x. (Note that, apart
	from Unhide by Name and Unfreeze by Name, the new default behavior is
	likely to be correct for all uses of this class.)
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL showHiddenAndFrozen()	{ return FALSE; }
};


#pragma warning(pop)

/*! \sa  Class InterfaceServer, Class Interface.\n\n
\par Description:
This is a call-back class for the selection filter drop down in the tab panel.
This allows plug-ins to add additional filters to this list.  */
class SelectFilterCallback : public InterfaceServer
{
public:
	/*! \remarks Returns the name of the filter that will appear in the drop
	down list in the tab panel. */
	virtual const MCHAR* GetName() = 0;  // the name that will appear in the drop down list when it is registered
	//the actual proc that does the filtering
	// the node, class id, and super class id are passed as parameter to check the filter again
	//return FALSE if the node is not selectable, true if it is selectable
	/*! \remarks This is the method that does the filtering of the node. It
	returns TRUE if the node may be selected; FALSE if it is not selectable.
	\par Parameters:
	<b>SClass_ID sid</b>\n\n
	The Super Class ID of the node.\n\n
	<b>Class_ID cid</b>\n\n
	The Class ID of the node.\n\n
	<b>INode *node</b>\n\n
	Points to the node to check. */
	virtual BOOL IsFiltered(SClass_ID sid, Class_ID cid, INode *node) const = 0;
	/*! \remarks Called when the callback is registered via Interface::RegisterSelectFilterCallback. */
	virtual void Registered() {}
	/*! \remarks Called when the callback is unregistered via Interface::UnRegisterSelectFilterCallback. 
	Note that the instance can safely be deleted in this method. */
	virtual void Unregistered() {};
};


//watje new display filters
//this is a call back for the display filter list in the display command panel
//it allows plugins and maxscripts to add additional filters to this list
/*! \sa  Class Interface.\n\n
\par Description:
This is a call-back class for the display filter list in the Hide by Category
rollup of the Display command panel. This allows plug-ins to add additional
filters to this list.
\par Data Members:
<b>BOOL on;</b>\n\n
Determines if the callback is on or off. If a callback is selected in the list
in the Display Panel list it is on; else it's off.  */
class DisplayFilterCallback : public InterfaceServer
{
public:
	BOOL on;
	/*! \remarks Returns the name that will appear in the drop down list in
	the display panel when the callback is registered. */
	virtual const MCHAR* GetName() = 0;  // the name that will appear in the drop down list when it is registered
	//the actual proc that does the filtering
	// the node, class id, and super class id are passed as parameter to check the filter again
	//return FALSE if the node is not selectable, true if it is selectable
	/*! \remarks This is the method that does the filtering of the node. It
	returns TRUE if the node is visible; FALSE if it is not visible.
	\par Parameters:
	<b>SClass_ID sid</b>\n\n
	The Super Class ID of the node.\n\n
	<b>Class_ID cid</b>\n\n
	The Class ID of the node.\n\n
	<b>INode *node</b>\n\n
	Points to the node to check. */
	virtual BOOL IsVisible(SClass_ID sid, Class_ID cid, INode *node) = 0;
	/*! \remarks Called when the callback is registered via Interface::RegisterDisplayFilterCallback. */
	virtual void Registered() {}
	/*! \remarks Called when the callback is unregistered via Interface.UnRegisterDisplayFilterCallback.
	Note that the instance can safely be deleted in this method. */
	virtual void Unregistered() {};
};


class Interface;

// A callback used with RegisterRedrawViewsCallback()
/*! \sa  Class Interface.\n\n
\par Description:
This is the callback used with <b>Interface::RegisterRedrawViewsCallback()</b>.
 */
class RedrawViewsCallback: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~RedrawViewsCallback() {}
		// this will be called after all the viewport have completed drawing.
		/*! \remarks This method will be called after all the viewports have completed
		drawing.
		\par Parameters:
		<b>Interface *ip</b>\n\n
		A pointer for calling functions available in 3ds Max. */
		virtual void proc(Interface *ip)=0;
	};

// A callback used with RegisterAxisChangeCallback()
/*! \sa  Class Interface.\n\n
\par Description:
This is the callback used with <b>Interface::RegisterAxisChangeCallback()</b>.
 */
class AxisChangeCallback: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~AxisChangeCallback() {}
		// this will be called when the axis system is changed
		/*! \remarks This callback object will be notified any time the user
		changes the reference coordinate system by:\n\n
		* Changing the transform coordinate system drop-down menu.\n\n
		* Changing the state of the transform center fly-off.\n\n
		* Changing X, Y, Z, XY, YZ, ZX constraint buttons/fly-off.\n\n
		* Using an accelerator or anything else that changes the above.
		\par Parameters:
		<b>Interface *ip</b>\n\n
		A pointer available for calling methods provided by 3ds Max. */
		virtual void proc(Interface *ip)=0;
	};

// parameters for creation of a preview
/*! \sa  Class Interface.
\par Description:
The data members of this class are used to specify the options for creating a
preview of the active viewport. A pointer to an instance of this class is
passed into the <b>Interface</b> method:\n\n
<b>virtual void CreatePreview(PreviewParams *pvp=nullptr, MSTR *filename=nullptr, MSTR *snippet = nullptr, MAXScript::ScriptSource source  = MAXScript::ScriptSource::NotSpecified)=0;</b>
*/
class PreviewParams: public MaxHeapOperators {
public:
	BOOL	outputType; //!< Specifies the output file or device. 0=default AVI codec, 1=user picks file, 2=user picks device
	int		pctSize; //!< Specifies the percentage (0-100) of current rendering output resolution.
	// frame limits
	int		start; //!< The start frame limit.
	int		end; //!< The end frame limit.
	int		skip; //!< Specifies how many frames are skipped between rendered frames. The is similar to the 'Every Nth Frame' parameter in the user interface.
	// frame rate
	int		fps; //!< The frames per second setting.
	// display control
	BOOL	dspGeometry; //!< If TRUE is displayed in the viewports; otherwise it is not shown.
	BOOL	dspShapes; //!< If TRUE shapes are displayed in the preview; otherwise they are not shown.
	BOOL	dspLights; //!< If TRUE lights are displayed in the preview; otherwise they are not shown.
	BOOL	dspCameras; //!< If TRUE cameras are displayed in the preview; otherwise they are not shown.
	BOOL	dspHelpers; //!< If TRUE helper objects are displayed in the preview; otherwise they are not shown.
	BOOL	dspSpaceWarps; //!< If TRUE space warp helper objects are displayed in the preview; otherwise they are not shown.
	BOOL	dspParticles; //!< If TRUE particle objects are displayed in the preview; otherwise they are not shown.
	BOOL	dspBones; //!< If TRUE bone objects are displayed in the preview; otherwise they are not shown.
	BOOL	dspGrid; //!< If TRUE the grid lines are displayed in the preview; otherwise they are not shown.
	BOOL	dspSafeFrame; //!< If TRUE the safe frames are displayed in the preview; otherwise they are not shown.
	BOOL	dspFrameNums; //!< If TRUE frame numbers are shown in the preview; otherwise they are not shown.
	BOOL	dspBkg; //!< If TRUE the background image is displayed; otherwise it is not used.

	int		rndLevel; //!< Specifies the rendering level used: 0=smooth/hi, 1=smooth, 2=facet/hi, 3=facet, 4=flat, 5=hidden line 6=lit wire, 7=wire, 8=box

	BOOL	useMPCamEffect; //!< If TRUE multipass camera effects are displayed in the preview; otherwise they are not shown.
};

// Viewport transparency rendering modes
#define VPT_TRANS_NONE			0
#define VPT_TRANS_STIPPLE		1
#define VPT_TRANS_BLEND			2
#define VPT_TRANS_SORT_BLEND	3		

// Scene Display Flags	
#define DISPLAY_WIRE_AS_MTL      1
#define DISPLAY_SHADED_AS_MTL (1<<1)
#define DISPLAY_SELECTED_ONLY (1<<2)

/*! \defgroup trackViewPickDlgFlags Track View Picking Flags IDs
flag values for Interface::TrackViewPickMultiDlg and Interface::TrackViewPickDlg */
//!@{
//! turn on display of animated tracks only
#define PICKTRACK_FLAG_ANIMATED			(1<<0)
//! turn off display of node Visibility track
#define PICKTRACK_FLAG_VISTRACKS		(1<<1)
//! turn on display of only selected nodes
#define PICKTRACK_FLAG_SELOBJECTS		(1<<2)
//! turn off display of World Space Modifiers
#define PICKTRACK_FLAG_WORLDMODS		(1<<3)
//! turn off display of Object Space Modifiers
#define PICKTRACK_FLAG_OBJECTMODS		(1<<4)
//! turn off display of node Transform track
#define PICKTRACK_FLAG_TRANSFORM		(1<<5)
//! turn off display of node base object
#define PICKTRACK_FLAG_BASEPARAMS		(1<<6)
//! turn off display of controller types
#define PICKTRACK_FLAG_CONTTYPES		(1<<7)
//! turn off display of note tracks
#define PICKTRACK_FLAG_NOTETRACKS		(1<<8)
//! turn off display of the sound track
#define PICKTRACK_FLAG_SOUND			(1<<9)
//! turn off display of maps in materials
#define PICKTRACK_FLAG_MATMAPS			(1<<10)
//! turn off display of material parameters
#define PICKTRACK_FLAG_MATPARAMS		(1<<11)
//! turn on display of hidden nodes
#define PICKTRACK_FLAG_VISIBLE_OBJS		(1<<12)
//! turn off display of hierarchy
#define PICKTRACK_FLAG_HIERARCHY		(1<<13)
//! turn off display of non-keyable tracks
#define PICKTRACK_FLAG_KEYABLE			(1<<14)
//! turn off display of nodes
#define PICKTRACK_FLAG_NODES			(1<<15)
//! turn off display of geometry nodes
#define PICKTRACK_FLAG_GEOM				(1<<16)
//! turn off display of shape nodes
#define PICKTRACK_FLAG_SHAPES			(1<<17)
//! turn off display of light nodes
#define PICKTRACK_FLAG_LIGHTS			(1<<18)
//! turn off display of camera nodes
#define PICKTRACK_FLAG_CAMERAS			(1<<19)
//! turn off display of helper nodes
#define PICKTRACK_FLAG_HELPERS			(1<<20)
//! turn off display of warp nodes
#define PICKTRACK_FLAG_WARPS			(1<<21)
//! turn off display of position controllers
#define PICKTRACK_FLAG_POSITION			(1<<22)
//! turn off display of rotation controllers
#define PICKTRACK_FLAG_ROTATION			(1<<23)
//! turn off display of scale controllers
#define PICKTRACK_FLAG_SCALE			(1<<24)
//! turn off display of bone nodes
#define PICKTRACK_FLAG_BONES			(1<<25)
//! set focus to first selected node found
#define PICKTRACK_FLAG_FOCUS_SEL_NODES	(1<<26)
//! Only show the active animation layer, instead of all.
#define PICKTRACK_FLAG_ACTIVELAYER		(1<<27) /*-- show only the active animation layer--*/
//! Only show the unlocked tracks, instead of all.
#define PICKTRACK_FLAG_LOCKED			(1<<28) /*-- turn off display of locked tracks--*/
//!@}

#define MAIN_MAX_INTERFACE Interface_ID(0x64854123, 0x7b9e551c)

// Generic interface into Max
/*! \sa  Class FPStaticInterface,  Class ViewExp, Class INode, Class INodeTab, Class CommandMode, Class Interval, Class Renderer, Class Modifier, Class Control, Class Atmospheric, Class Point3, Class Matrix3, Class ModContext, Class ReferenceTarget, Template Class Tab\n\n
\par Description:
This class provides an interface for calling functions that are exported from
the 3ds  Max executable. All the methods in this class are implemented by 3ds
Max itself.\n\n
Methods are provided for putting up many standard 3ds Max dialogs, working with
command modes, working with viewports, controlling the prompt, toolbar and
status areas, and working with selection sets. There are also methods for
creating objects and nodes in the scene, setting and getting the current time
and animation ranges, working with the standard directories of MAX, and many
more. See the Method Groups listed below for a breakdown of the various kinds
of methods available.  Note: When editing in the command panel, a developer
gets passed an interface pointer during <b>BeginEditParams()</b>. This pointer
is only valid before <b>EndEditParams()</b> is finished. A developer should not
hang on to this pointer and call methods on it after <b>EndEditParams()</b> has
returned.
*/
#pragma warning(push)
#pragma warning(disable:4100)
class Interface : public FPStaticInterface {
	public:
//! \name Fonts/Cursor Related Methods
//! @{		
		/*! \remarks Returns the handle of the font used by MAX. The default
		font is the same one returned from <b>GetAppHFont()</b>. So if a
		developer makes a dialog in the rollup page the correct font will be
		used automatically. */
		virtual HFONT GetAppHFont()=0;
//! @}

//! \name Viewport Access
//! @{
		/*! \remarks This method may be called to cause the viewports to be
		redrawn.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to redraw the viewports.\n\n
		<b>DWORD vpFlags=REDRAW_NORMAL</b>\n\n
		You may specify one of the following:\n\n
		<b>REDRAW_BEGIN</b>\n\n
		Call this before you redraw.\n\n
		<b>REDRAW_INTERACTIVE</b>\n\n
		This allows the view quality to degrade to maintain interactively.\n\n
		<b>REDRAW_END</b>\n\n
		If during interactive redraw the state degraded, this will redraw the
		views in the undegraded state.\n\n
		<b>REDRAW_NORMAL</b>\n\n
		This redraws the views in the undegraded state.\n\n
		<b>ReferenceTarget *change=NULL</b>\n\n
		This parameter is not used - always let it default to NULL.
		\par Example:
		<b>ip-\>RedrawViews(ip-\>GetTime(),REDRAW_BEGIN);</b>\n\n
		<b>// More code ...</b>\n\n
		<b>ip-\>RedrawViews(ip-\>GetTime(),REDRAW_INTERACTIVE);</b>\n\n
		<b>// More code ...</b>\n\n
		<b>ip-\>RedrawViews(ip-\>GetTime(),REDRAW_END);</b> */
		virtual void RedrawViews(TimeValue t,DWORD vpFlags=REDRAW_NORMAL, ReferenceTarget *change=NULL)=0;		
		/*! \remarks This allows you to specify the active viewport.
		\par Parameters:
		<b>HWND hwnd</b>\n\n
		The handle of the window to activate.
		\return  TRUE if the viewport was not previously active; otherwise
		FALSE. */
		virtual BOOL SetActiveViewport(HWND hwnd)=0;

		/*! \remarks Returns the <b>ViewExp</b> reference of the active MAX
		viewport. 
		NOTE: if none of the viewports are active, the first viewport of the 
		current view tab is returned*/
		virtual ViewExp& GetActiveViewExp()=0;

		// This will cause all viewports to be completely redrawn.
		// This is extremely heavy handed and should only be used when
		// necessary.
		/*! \remarks Calling this method will cause all the viewports to be
		completely redrawn. Note: This method literally forces
		<b>everything</b> (every object, every screen rectangle, every view) to
		be marked invalid and then the whole scene is regenerated. (The
		individual object pipeline caches are <b>not</b> flushed, however.) So
		this routine is guaranteed to be slow.
		\par Parameters:
		<b>BOOL doDisabled=TRUE</b>\n\n
		If TRUE disabled viewports are redrawn; otherwise they are not. */
		virtual void ForceCompleteRedraw(BOOL doDisabled=TRUE)=0;
//! @}

//! \name IObjCreate and IObjParam Pointer Casting
//! @{		
		/*! \remarks Returns this interface pointer cast as a
		<b>IObjCreate</b> pointer. */
		virtual IObjCreate *GetIObjCreate()=0;
		/*! \remarks Returns this interface pointer cast as a <b>IObjParam</b>
		pointer. */
		virtual IObjParam *GetIObjParam()=0;
//! @}

//! \name 3ds Max Main Window Access
//! @{		
		/*! \remarks Returns the window handle of 3ds Max. */
		virtual HWND GetMAXHWnd() const = 0;

		/** \brief Returns the Qt docking main window of 3ds Max.
		* Use this window to attach your toolbar or dock widget to the 3ds Max main UI.
		* When you create new Qt toolbars or dock widgets, you must use the extended 3ds Max 
		* version QmaxToolBar and QmaxDockWidget instead of the Qt standard components.
		* \see class QmaxToolBar, class QmaxDockWidget
		*/
		virtual MaxSDK::QmaxMainWindow* GetQmaxMainWindow() const = 0;
//! @}

//! \name Renderer Access
//! @{
		// This will draw the active view (if it is a camera view with a multi-pass effect
		// enabled) with that effect active.  Returns TRUE if the active view is a camera view 
		// with a multi-pass effect enabled, FALSE otherwise
		/*! \remarks This method will draw the active view (if it is a camera view with a
		multi-pass effect enabled) with that effect active.
		\return  TRUE if the active view is a camera view with a multi-pass
		effect enabled, FALSE otherwise. */
		virtual BOOL DisplayActiveCameraViewWithMultiPassEffect() = 0;
//! @}

		// Determines the way transparency will be displayed in the active viewport, based on transType setting
		//		VPT_TRANS_NONE => all objects are always opaque
		//		VPT_TRANS_STIPPLE => use screendoor transparency
		//		VPT_TRANS_BLEND => use hardware blending, based on drawing transparent objects last
		virtual BOOL SetActiveViewportTransparencyDisplay(int transType) = 0;

//! \name Viewport Access
//! @{
		//! \brief Turns off scene redraws
		/*! In order to increase the speed of certain operations, scene redraw should
		be turned off temporarily, then once the operation completed, should be turned back on.
		Scene redraw should be disabled in implementations of the Renderer::Open() method and 
		re-enabled in implementations of Renderer::Close() method. 
		\note Calls to DisableSceneRedraw() and EnableSceneRedraw() should be paired up
		since the system counts the requests for disabling the scene redraw. If several 
		scene disable requests are issued, it takes an equal number of scene enable requests
		to re-enable the redrawing of the scene.
		\see EnableSceneRedraw, IsSceneRedrawDisabled, class SuspendSceneRedrawGuard
		*/
		virtual void DisableSceneRedraw() = 0;

		//! \brief Turns on scene redraws
		/*! In order to increase the speed of certain operations, scene redraw should
		be turned off temporarily, then once the operation completed, should be turned back on.
		Scene redraw should be disabled in implementations of the Renderer::Open() method and 
		re-enabled in implementations of Renderer::Close() method. 
		\note Calls to DisableSceneRedraw() and EnableSceneRedraw() should be paired up
		since the system counts the requests for disabling the scene redraw. If several 
		scene disable requests are issued, it takes an equal number of scene enable requests
		to re-enable the redrawing of the scene.
		\see DisableSceneRedraw, IsSceneRedrawDisabled, class SuspendSceneRedrawGuard
		*/
		virtual void EnableSceneRedraw() = 0;

		//! \brief Tells whether the scene redraw is disabled
		/*! Call this method to find out whether the scene redraw is disabled.
		\return The count of scene redraw disable requests. This will be positive, 
		if the scene redraw is turned off, and zero or negative if it is turned on.
		\see EnableSceneRedraw, DisableSceneredraw, class SuspendSceneRedrawGuard
		*/
		virtual int IsSceneRedrawDisabled() = 0;
//! @}
		
		// Forward declaration
		class SuspendSceneRedrawGuard;

//! \name Callback/Notification Registration
//! @{
		// Register a call back object that gets called everytime
		// the viewports are redrawn.
		/*! \remarks Registers a call back object that gets called every time
		the viewports are redrawn. The <b>proc()</b> method is called after the
		views are finished redrawing.
		\par Parameters:
		<b>RedrawViewsCallback *cb</b>\n\n
		Points to the callback object whose <b>proc()</b> method is called when
		the viewports are redrawn. See
		Class RedrawViewsCallback. */
		virtual void RegisterRedrawViewsCallback(RedrawViewsCallback *cb)=0;
		/*! \remarks Un-registers the viewport redraw callback.
		\par Parameters:
		<b>RedrawViewsCallback *cb</b>\n\n
		Points to the callback object to un-register. See
		Class RedrawViewsCallback.\n */
		virtual void UnRegisterRedrawViewsCallback(RedrawViewsCallback *cb)=0;
//watje new selection filters
//these work just like all the other register callback function
//except they are used to register filter selection call backs
		/*! \remarks Registers a call back object that gets called to filter the selection of nodes.
		\par Parameters:
		<b>SelectFilterCallback *cb</b>\n\n
		Points to the callback object to register. See
		Class SelectFilterCallback. */
		virtual void RegisterSelectFilterCallback(SelectFilterCallback *cb)=0;
		/*! \remarks Un-registers the select filter callback.
		\par Parameters:
		<b>SelectFilterCallback *cb</b>\n\n
		Points to the callback object to un-register. See
		Class SelectFilterCallback. */
		virtual void UnRegisterSelectFilterCallback(SelectFilterCallback *cb)=0;

//watje new display filters
//these work just like all the other register callback function
//except they are used to register display selection call backs
		/*! \remarks Registers a call back object that gets called to filter the display of
		nodes.\n\n
		\par Parameters:
		<b>DisplayFilterCallback *cb</b>\n\n
		Points to the callback object to register. See
		Class DisplayFilterCallback. \n  */
		virtual void RegisterDisplayFilterCallback(DisplayFilterCallback *cb)=0;
		/*! \remarks Un-registers the display filter callback.
		\par Parameters:
		<b>DisplayFilterCallback *cb</b>\n\n
		Points to the callback object to un-register. See
		Class DisplayFilterCallback. */
		virtual void UnRegisterDisplayFilterCallback(DisplayFilterCallback *cb)=0;
//! @}

		// set the selection type to automatic window/crossing (based on dir, as set by 2nd arg), 
		// or to manual window/crossing (based on second arg)
		//
		// if autoWinCross is TRUE, then Window/Crossing is chosen by direction
		//		if winCrossOrAutoDir is set to AC_DIR_RL_CROSS, then right-to-left implies crossing
		//		if winCrossOrAutoDir is set to AC_DIR_LR_CROSS, then left-to-right implies crossing
		// if autoWinCross is FALSE, then Window/Crossing is set explicitly, as follows:
		//		if winCrossOrAutoDir is TRUE, then you get crossing selection
		//		if winCrossOrAutoDir is FALSE, then you get window selection
		virtual BOOL SetSelectionType(BOOL autoWinCross, int winCrossOrAutoDir) = 0;

//! \name Viewport Access
//! @{		
		// For use with extended views: 
		// - make the extended viewport active (set on mouse click, for example)
		// - put up the view type popup menu (put up on right-click, for example)
		/*! \remarks This method is used with Extended Viewports (see
		Class ViewWindow). It is called
		when the extended viewport needs to become active. It should be called
		whenever the user clicks in the non-3D window (so as to deactivate the
		current 3D window, and redirect commands like the Min/Max toggle to the
		non-3D viewport window).
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The handle of the window which to made active. */
		virtual void MakeExtendedViewportActive(HWND hWnd)=0;
		/*! \remarks This method is used with Extended Viewports (see
		Class ViewWindow). It is called to
		put up the view type popup menu (for example the right-click menu). It
		should be called when the user right-clicks in a dead region of the
		non-3D window. This brings up the view selection menu so that the user
		can choose to replace the current window with a 3D or other non-3D
		window without having to go to the Views | Viewport Config dialog
		directly.
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The handle of the window the menu is to appear in.\n\n
		<b>POINT pt</b>\n\n
		The point at which the menu is put up.
		\par Sample Code:
		\code
		case WM_RBUTTONDOWN:
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			GetCOREInterface()->PutUpViewMenu(h, pt);
		\endcode   */
		virtual void PutUpViewMenu(HWND hWnd, POINT pt)=0;
//! @}

//! \name Methods to Display MAX Dialogs
//! @{
		// Execute a track view pick dialog.
		/*! \remarks This method brings up the track view pick dialog. \n
		\n \image html "dlgtvp.gif"
		\par Parameters:
		<b>HWND hParent</b>\n\n
		The handle of the parent window.\n\n
		<b>TrackViewPick *res</b>\n\n
		The item chosen by the user. See
		Class TrackViewPick\n\n
		<b>TrackViewFilter *filter=NULL</b>\n\n
		The call back object to filter selection in the track view. See
		Class TrackViewFilter.\n\n
		<b>DWORD pickTrackFlags=0</b>\n\n
		See \ref trackViewPickDlgFlags for a list of possible flags.
		\return  TRUE if the user selected OK to exit the dialog; otherwise
		FALSE.
		\par Sample Code:
		This code brings up the Track View Pick Dialog and filters the input
		to MAX's controllers. After the controller is selected GetValue() is
		called on it.\n\n
		\code
		class MyTVFilter : public TrackViewFilter
		{
			BOOL proc(Animatable *anim, Animatable *client,int subNum) {
				Control *c = (Control*)anim->GetInterface(I_CONTROL);
				return (c) ? TRUE : FALSE;
			}
		};
		
		void DoTest()
		{
			TrackViewPick res;
			MyTVFilter tvf;
			BOOL okay = IP->TrackViewPickDlg(IP->GetMAXHWnd(), &res, &tvf);
			if (!okay) return;
			Control *c = (Control *)res.anim;
			SClass_ID sid = c->SuperClassID();
			GetSetMethod method = CTRL_ABSOLUTE;
			switch(sid) {
				case CTRL_FLOAT_CLASS_ID:
					float r;
					Interval ivalid;
					c->GetValue(IP->GetTime(), &r, ivalid, method);
					...
		\endcode  */
		virtual BOOL TrackViewPickDlg(HWND hParent, TrackViewPick *res, TrackViewFilter *filter=NULL, DWORD pickTrackFlags=0)=0;

		/*! \remarks This methods brings up a dialog that allows one to select multiple
		tracks. This method works much like the <b>TrackViewPickDlg</b> method
		above except it is passed a pointer to a table of a
		<b>TrackViewPick</b> items instead. \n
		\n \image html "trackvwm.gif"
		\par Parameters:
		<b>HWND hParent</b>\n\n
		The handle of the parent window.\n\n
		<b>MaxSDK::Array\<TrackViewPick\> *res</b>\n\n
		This is a table (See Template Class Tab) of items chosen by the user. 
		See Class TrackViewPick.\n\n
		<b>TrackViewFilter *filter=NULL</b>\n\n
		The call back object to filter selection in the track view. See
		Class TrackViewFilter.\n\n
		<b>DWORD pickTrackFlags=0</b>\n\n
		See \ref trackViewPickDlgFlags for a list of possible flags. */
		virtual BOOL TrackViewPickMultiDlg(HWND hParent, MaxSDK::Array<TrackViewPick>* res, TrackViewFilter *filter=NULL,DWORD pickTrackFlags=0)=0;
//! @}
		
//! \name Command Modes
//! @{
		// Command mode methods
		/*! \remarks This method pushes the specified command mode on the
		stack. Typically this is used by developers handling their own creation
		using <b>ClassDesc::BeginCreate()</b>. See Class ClassDesc for more details.\n\n
		Note: This method works as documented but a developer may not want to
		use it. The problem is that other modes can be pushed on the stack
		(such as viewport transformation modes) and it becomes complicated to
		track when it is OK to pop your mode. See the methods below for
		alternatives (such as <b>SetCommandMode</b>).
		\par Parameters:
		<b>CommandMode *m</b>\n\n
		A pointer to the command mode to push. */
		virtual void PushCommandMode( CommandMode *m )=0;
		/*! \remarks This method sets the top of the stack to the specified
		command mode. A developer should call <b>DeleteMode()</b> to delete
		their command mode when done using it.
		\par Parameters:
		<b>CommandMode *m</b>\n\n
		The command mode to set. */
		virtual void SetCommandMode( CommandMode *m )=0;
		/*! \remarks Pops the command mode off the top of the stack. */
		virtual void PopCommandMode()=0;		
		/*! \remarks Returns the current mode on the top of the stack. */
		virtual CommandMode* GetCommandMode()=0;
		/*! \remarks This is the typical method called by the developer to
		handle mouse interaction. It allows the developer to set the command
		mode to one of the standard command modes. For example: <b>CID_OBJMOVE,
		CID_OBJROTATE, CID_OBJSCALE, CID_OBJUSCALE, CID_OBJSQUASH,
		CID_OBJSELECT</b>, etc.
		\par Parameters:
		<b>int cid</b>\n\n
		The index of the command mode to set. See \ref stdCommandModes  to review the full list. */
		virtual void SetStdCommandMode( int cid )=0;
		/*! \remarks Allows the developer to push one of the standard command
		modes on the command stack. For example: <b>CID_OBJMOVE, CID_OBJROTATE,
		CID_OBJSCALE, CID_OBJUSCALE, CID_OBJSQUASH, CID_OBJSELECT</b>, etc.
		\par Parameters:
		<b>int cid</b>\n\n
		The index of the command mode to set. See \ref stdCommandModes to review the full list. */
		virtual void PushStdCommandMode( int cid )=0;		
		/*! \remarks Removes the specified command mode from the stack. This
		method pops items off the command mode stack up to and including the
		specified mode. The top item in the stack is then set as the active
		command mode. As usual, <b>ExitMode()</b> is called on the specified
		mode before it is popped and <b>EnterMode()</b> is called on the newly
		active mode.
		\par Parameters:
		<b>CommandMode *m</b>\n\n
		Points to the command mode to remove. */
		virtual void RemoveMode( CommandMode *m )=0;
		/*! \remarks If the developer sets or pushes a command mode, this
		method should be called when the developer is done with the mode to
		ensure that it is no longer anywhere in the stack. If the mode is
		already deleted this method does nothing.\n\n
		Note: It is normal for a developer to set the 'Select and Move' command
		mode to be the active one if their mode was at the top of the stack and
		is being deleted. For instance:\n\n
		<b> ip-\>SetStdCommandMode(CID_OBJMOVE);</b>\n\n
		<b> ip-\>DeleteMode(\&myCMode);</b>
		\par Parameters:
		<b>CommandMode *m</b>\n\n
		The command mode to delete. */
		virtual void DeleteMode( CommandMode *m )=0;
//! @}

		//added by AF (08/18/2000)
		//if a Pick Mode is on top of the command stack this function will return it 
		//otherwise it returns NULL
		/*! \remarks If a Pick Mode is on top of the command stack, then this function will
		return the <b>PickModeCallback.</b> If the Pick Mode is not on top of
		the command stack the it returns NULL. */
		virtual PickModeCallback* GetCurPickMode()=0;						

//! \name Methods to Display MAX Dialogs
//! @{
		// Puts up a hit by name dialog. If the callback is NULL it 
		// just does a standard select by name.
		// returns TRUE if the user OKs the dialog, FALSE otherwise.
		/*! \remarks This method is called to put up the standard MAX Hit By Name
		dialog.\n\n
		If the callback is NULL this method does a standard select by name. The
		nodes chosen by the user are selected if the user selects 'Select'
		from the dialog to exit. Use <b>Interface::GetSelNodeCount()</b> and
		<b>GetSelNode(i)</b> to retrieve the results.\n
		\n \image html "dlghbn.gif"
		\par Parameters:
		<b>HitByNameDlgCallback *hbncb=NULL</b>\n\n
		Points to the callback object. See Class HitByNameDlgCallback. 
		Developers should delete this callback when done.
		\return  TRUE if the user selects 'Select' from the dialog; otherwise
		FALSE. */
		virtual BOOL DoHitByNameDialog(HitByNameDlgCallback *hbncb=NULL)=0;
//! @}

/** \name Status Panel/Prompt Related Methods
 * Generally prompts are set by the command mode and these are the methods used.
 * You can use the DisplayTempPrompt() method to temporarily display a prompt to
 * the user independent of a command mode. 
 */
///@{		
		// status panel prompt stuff
		/*! \remarks Pushes a prompt to display on the prompt stack.
		\par Parameters:
		<b>MCHAR *s</b>\n\n
		The string to display. */
		virtual void PushPrompt( const MCHAR *s )=0;

		/*! \remarks Pops a displayed string off the prompt stack. The
		previous prompt will be restored. */
		virtual void PopPrompt()=0;
		/*! \remarks Replaces the string on the top of the prompt stack.
		\par Parameters:
		<b>MCHAR *s</b>\n\n
		The string to display. */
		virtual void ReplacePrompt( const MCHAR *s )=0;

		/*! \remarks Displays the string passed for the duration passed. After
		the time elapses, the string is popped from the stack. This may be used
		to put up a temporary error message for example.
		\par Parameters:
		<b>MCHAR *s</b>\n\n
		The string to display temporarily.\n\n
		<b>int msec=1000</b>\n\n
		The duration in milliseconds to display the string. */
		virtual void DisplayTempPrompt( const MCHAR *s, int msec=1000)=0;

		/*! \remarks Removes the temporary prompt immediately. */
		virtual void RemoveTempPrompt()=0;
		
		// Write values to x,y,z status boxes. Before doing this, mouse
		// tracking must be disabled. Typically a plug-in would disable
		// mouse tracking on mouse down and enable it on mouse up.		
		/*! \remarks Disables mouse tracking and display of coordinates to the
		X, Y, Z status boxes. Typically a plug-in would disable mouse tracking
		on mouse down and enable it on mouse up. */
		virtual void DisableStatusXYZ()=0;

		/*! \remarks Enables mouse tracking and display of coordinates to the
		X, Y, Z status boxes. Typically a plug-in would disable mouse tracking
		on mouse down and enable it on mouse up. */
		virtual void EnableStatusXYZ()=0;

		/*! \remarks Displays the point passed using the format passed in the
		X, Y, Z status boxes.
		\par Parameters:
		<b>Point3 xyz</b>\n\n
		The point to be displayed.\n\n
		<b>int type</b>\n\n
		The format of the point:\n\n
		<b>STATUS_UNIVERSE</b>\n\n
		Current system units.\n\n
		<b>STATUS_SCALE</b>\n\n
		0=0%, 1=100%.\n\n
		<b>STATUS_ANGLE</b>\n\n
		Degrees.\n\n
		<b>STATUS_OTHER</b>\n\n
		Straight floating point value. */
		virtual void SetStatusXYZ(Point3 xyz,int type)=0;

		/*! \remarks This method will convert the specified angle axis for
		status display.
		\par Parameters:
		<b>AngAxis aa</b>\n\n
		The angle axis to convert and display. */
		virtual void SetStatusXYZ(AngAxis aa)=0; // this will convert the aa for status display
///@}

//! \name Methods to Display MAX Dialogs
//! @{		
		// put up a directory choose dialog
		// hWnd = parent
		// title is dialog box title
		// dir is return value for chosen dir (empty on cancel)
		// desc, if non-null, puts up a description field and returns new desc.
		/*! \remarks This methods puts up the Choose Directory dialog box to allow the user
		to select a directory. The chosen directory including the drive and
		path is stored in <b>dir</b>. \n
		\n \image html "choosdir.gif"
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The parent window handle.\n\n
		<b>MCHAR *title</b>\n\n
		The title in the dialog box.\n\n
		<b>MCHAR *dir</b>\n\n
		The chosen directory is stored here. This points to an empty string on
		cancel.\n\n
		<b>MCHAR *desc=NULL</b>\n\n
		The string to go into the Label field of the dialog. This string may be
		changed by the user and is returned here. */
		virtual void ChooseDirectory(HWND hWnd, const MCHAR *title, MCHAR *dir, MCHAR *desc=NULL)=0;
//! @}

//! \name Auto Backup Time Related Methods
//! @{		
		//! \deprecated This has been deprecated as of 3ds Max 2023.
		//! Please use GetAutoBackupInterface()->GetInterval() in iAutobackup.h instead
		MAX_DEPRECATED virtual float GetAutoBackupTime() final { return 0; }
		//! \deprecated This has been deprecated as of 3ds Max 2023.
		//! Please use GetAutoBackupInterface()->SetInterval(float) in iAutobackup.h instead
		MAX_DEPRECATED virtual void SetAutoBackupTime(float minutes) final {}
		//! \deprecated This has been deprecated as of 3ds Max 2023.
		//! Please use GetAutoBackupInterface()->GetEnable() in iAutobackup.h instead
		MAX_DEPRECATED virtual BOOL AutoBackupEnabled() final { return false; }
		//! \deprecated This has been deprecated as of 3ds Max 2023.
		//! Please use GetAutoBackupInterface()->SetEnable(bool) in iAutobackup.h instead
		MAX_DEPRECATED virtual void EnableAutoBackup(BOOL onOff) final {}
//! @}

//! \name Progress Bar Methods
//! @{
		/*!
		Displays a progress bar in the status panel with a cancel button to the right of the progress bar.
		To represent progress of a task that can't be cancelled, use other version of ProgressStart().
		When progress bar is up, user can't interact with any UI except cancel button.
		This method must be called from the main thread.

		Note: It is not possible to use the progress bar APIs in the create or modify branches of the command panel.
		In the create or modify branch of the command panel, EndEditParams() gets called from this method.
		This is because most of MAX is "shut down" during a progress operation.
		For example, it is not appropriate for users to be moving objects during an IK calculation,
		or changing the camera lens while a preview is being created (all these operations use ProgressStart() API).
		It is for this same reason that Transform Type-in, Medit, Track View and Video Post windows
		are hidden during a progress operation. Since EndEditParams() is called, the Interface pointer a plug-in
		maintains is no longer valid after the operation is started.
		This method is useful for Utility plug-ins, and Import/Export plug-ins however.

		\param title : String visible to the left of the progress bar during entire progress
		\param dispBar : Unused parameter.
		\param fn : Pointer to a function with this signature: `DWORD WINAPI fn(LPVOID arg)`.
		Pointer can be null. This function is always executed in the main thread.
		If not null, this function is called with argument `arg` passed at the end of ProgressStart().
		\param arg : Argument passed to function `fn`. Can be null
		\return `true` when progress mode was entered successfully, `false` otherwise

		\par Sample Code:
		This code demonstrates the use of ProgressStart(), ProgressEnd(), GetCancel() and SetCancel() APIs.
		There are two ways to use the APIs:
		1. Pass a null function in ProgressStart(). Then perform work and call ProgressUpdate() from client code
		\code
		void Utility::Test1()
		{
			int percent, retval;
			Interface *ip = theUtility.ip;
			ip->ProgressStart(_M("Title String"), TRUE, nullptr, nullptr);
			for (int i = 0; i < 1000; i++)
			{
				percent = i/10;
				ip->ProgressUpdate(percent);
				if (ip->GetCancel()) {
					retval = MaxSDK::MaxMessageBox(ip->GetMAXHWnd(), _M("Really Cancel?"), _M("Question"), MB_ICONQUESTION | MB_YESNO);
					if (retval == IDYES)
						break;
					else if (retval == IDNO)
						ip->SetCancel(FALSE);
				}
			}
			ip->ProgressEnd();
		}
		\endcode

		2. Use `fn` to perform work. Call ProgressUpdate() in `fn`.
		\code
		DWORD WINAPI fn(LPVOID arg)
		{
			int percent;
			Interface *ip = theUtility.ip;
			for (int i = 0; i < 1000; i++)
			{
				percent = i/10;
				ip->ProgressUpdate(percent);
				if (ip->GetCancel())
				{
					switch(MaxSDK::MaxMessageBox(ip->GetMAXHWnd(), _M("Really Cancel"), _M("Question"), MB_ICONQUESTION | MB_YESNO))
					{
						case IDYES:
							return 0;
						case IDNO:
							ip->SetCancel(FALSE);
					}
				}
			}
			return 0;
		}

		void Utility::Test1()
		{
			Interface *ip = theUtility.ip;
			LPVOID arg;
			ip->ProgressStart(_M("Title String"), TRUE, fn, arg);
			ip->ProgressEnd();
		}
		\endcode
		\see ProgressUpdate, ProgressEnd, SetCancel, GetCancel
		*/
		virtual bool ProgressStart(const MCHAR* title, bool dispBar, LPTHREAD_START_ROUTINE fn, LPVOID arg) = 0;

		/*!
		Similar to other ProgressStart() API.
		This uses the same logic as passing null to `fn` and `arg` in other version
		Use this version with `allowCancel = false` to show progress of tasks that can't be cancelled.

		\param title : The title string for the progress bar to let the user know what is happening
		\param allowCancel : If `true`, cancel button appears on the right of progress bar.
		Hit cancel button or press ESC key to set cancel status to true
		If `false`, no cancel button appears and ESC key presses are ignored.
		\return `true` when progress mode was entered successfully, `false` otherwise.
		\see ProgressUpdate, ProgressEnd, SetCancel, GetCancel
		*/
		virtual bool ProgressStart(const MCHAR* title, bool allowCancel = true) = 0;

		/*!
		This method updates the progress bar.
		After ProgressStart() is called, this method should be called periodically to report progress.
		\param pct : Percentage completed (0 to 100). This causes the progress bar to move.
		\param showPct : If `true`, percentage appears next to progress bar.
		\param stepName : Name of the current step of the operation. This is displayed next to progress bar.
		For example, this might change from "Extruding" to "Capping" to "Welding".
		\see ProgressStart, ProgressEnd, SetCancel, GetCancel
		*/
		virtual void ProgressUpdate(int pct, bool showPct = true, const MCHAR* stepName = nullptr) = 0;

		/*! \remarks This method removes the progress bar and frees the memory
		that was allocated internally to handle the processing. */
		virtual void ProgressEnd()=0;
		/*! \remarks This method returns the progress bar cancel button
		status. Also see <b>SetCancel()</b> below.
		\return  TRUE if the user pressed the cancel button; otherwise FALSE.
		*/
		virtual BOOL GetCancel()=0;
		/*! \remarks Sets the cancelled status returned from
		<b>GetCancel()</b>. This may be used if you want to give the user a
		confirmation dialog box asking if they really want to cancel. For
		example, when a MAX user creates an preview animation this API is used.
		If the user presses cancel, the preview code reads this via
		<b>GetCancel()</b>. Then a confirmation dialog is displayed asking the
		user if they indeed want to cancel. If the user selects that they don't
		want to cancel, this method is called passing FALSE. This sets the
		class variable that is returned by <b>GetCancel()</b>. The cancel
		request is ignored and processing continues.\n\n
		Note that this will not cancel unless you implement code to process
		<b>GetCancel()</b>. It merely sets the state returned from
		<b>GetCancel()</b>.
		\par Parameters:
		<b>BOOL sw</b>\n\n
		TRUE to set the cancel flag; FALSE to clear the cancel flag. */
		virtual void SetCancel(BOOL sw)=0;
//! @}

//! \name Preview Creation
//! @{		
		// create preview from active view.
		// If pvp is NULL, this uses the parameters from the preview rendering dialog box.
		/*! \remarks This method is used to render a preview image (or
		animation) from the currently active viewport using the real-time
		(viewport) renderer.\n\n
		\par Parameters:
		<b>PreviewParams *pvp</b>\n\n
		This class defines the way the preview is generated (via its data members). If NULL, the parameters
		from the preview rendering dialog box are used. See Class PreviewParams\n\n
		<b>MSTR *filename</b>\n\n
		The filename for the preview.  If NULL, the filename setting from the preview rendering dialog is used.
		<b>MSTR *snippet</b>\n\n
		The MXS code snippet to execute on each frame.  The returned string from snippet is displayed in the rendering
		output. If this value is NULL, the setting in the preview rendering dialog is used.\n\n
		<b>MAXScript::ScriptSource scriptSource</b>\n\n If snippet is specified, the source of the snippet string. */
		virtual void CreatePreview(PreviewParams* pvp = nullptr, MSTR* filename = nullptr, MSTR* snippet = nullptr,
								   MAXScript::ScriptSource scriptSource = MAXScript::ScriptSource::NotSpecified) = 0;
//! @}

//! \name Grid Related Methods
//! @{		
		// Some info about the current grid settings
		/*! \remarks This method returns the grid spacing value that the user
		specifies in the Views / Grid and Snap Settings dialog in the Home Grid
		tab under Grid Spacing. */
		virtual float GetGridSpacing()=0;
		/*! \remarks This method returns the value that the user specifies in
		the Views / Grid and Snap Settings dialog in the Home Grid tab under
		Major Lines every Nth. */
		virtual int GetGridMajorLines()=0;
//! @}

//! \name Extended Display Modes
//! @{		
		// Extended display modes (such as camera cones that only appear when dragging a camera)
		/*! \remarks This method is used internally to set the extended
		display mode.
		\par Parameters:
		<b>int vflags</b>\n\n
		See \ref extendedDisplayModes. */
		virtual void SetExtendedDisplayMode(int vflags)=0;
		/*! \remarks Returns the extended display mode flags. This method
		provides a mechanism to retrieve some additional information about an
		object that is more dependent on MAX than on the particular object. For
		example, when a spotlight is selected, it can use this method to detect
		that it is the only item selected, and display its cone. It checks this
		using and extended display mode <b>EXT_DISP_ONLY_SELECTED</b>. 
		See \ref extendedDisplayModes. */
		virtual int GetExtendedDisplayMode()=0;
//! @}

//! \name User Interface Controls and Properties
//! @{		
		// UI flyoff timing
		/*! \remarks This sets the custom control flyoff time to the value
		passed. This is the number of milliseconds the user must hold down on a
		flyoff button before the flyoff is activated.
		\par Parameters:
		<b>int msecs</b>\n\n
		The number of milliseconds the user must hold down on the button before
		the flyoff is activated. */
		virtual void SetFlyOffTime(int msecs)=0;
		/*! \remarks Returns the number of milliseconds the user must hold
		down on a flyoff button before the flyoff is activated. */
		virtual int  GetFlyOffTime()=0;
//! @}

//! \name Fonts/Cursor Related Methods
//! @{
		// Get standard Max cursors.
		/*! \remarks This method returns the cursor handle for the standard
		MAX cursors. Use <b>SetCursor()</b> from the Windows API to set the
		cursor.
		\par Parameters:
		<b>int id</b>\n\n
		One of the following values:\n\n
		<b>SYSCUR_MOVE, SYSCUR_ROTATE, SYSCUR_USCALE, SYSCUR_NUSCALE,</b>\n\n
		<b>SYSCUR_SQUASH, SYSCUR_SELECT, SYSCUR_DEFARROW.</b>
		\return  The handle of the cursor. */
		virtual HCURSOR GetSysCursor( int id )=0;

		// Turn on or off a cross hair cursor which draws horizontal and vertical
		// lines the size of the viewport's width and height and intersect at
		// the mouse position.
		/*! \remarks This method is reserved for future use. */
		virtual void SetCrossHairCur(BOOL onOff)=0;
		/*! \remarks This method is reserved for future use. */
		virtual BOOL GetCrossHairCur()=0;
//! @}

		// This pops all modes above the create or modify mode.
		// NOTE: This is obsolete with the new modify panel design.
		virtual void RealizeParamPanel()=0;

//! \name Snap Related Methods
//! @{
		// Snap an angle value (in radians)
		/*! \remarks Normally, with angle snap off, interactive rotation of a
		node uses a rate of 1/2 degree per pixel. With angle snap on, the
		angles are snapped to the nearest MAX angle snap value.\n\n
		This method may be used when interactive rotation is taking place with
		the mouse to snap the angle passed to the nearest MAX angle snap value.
		In this method, the input value/output value snap correspondence is
		accelerated as the angle grows. This prevents the user from having to
		move the mouse too much with larger angle values. This is why this
		method does not return a linear relationship between the input angle
		and the snapped output angle.\n\n
		Note that when angle snap is off, this method just returns the input
		value unaltered.\n\n
		Note for R3: This method formerly was set up with a single parameter,
		the angle to be snapped. If the angle snap toggle was on, snapping
		occurred. If not, it did nothing. There were two problems with
		this:\n\n
		1) A multiplier was applied to the angle input, giving faster
		interactive results in object rotations, etc. This had a negative
		effect if you just wanted your input angle snapped to the nearest snap
		angle.\n\n
		2) It only snapped if the angle snap toggle was on. Sometimes, you
		might want to snap something to the angle snap value even if the primary
		toggle was off.\n\n
		To remedy these shortcomings, the method has had two parameters added
		(<b>fastSnap</b> and <b>forceSnap</b>).
		\par Parameters:
		<b>float angleIn</b>\n\n
		Angle to snap in radians.\n\n
		<b>BOOL fastSnap=TRUE</b>\n\n
		If TRUE the snapping multiplier is used; if FALSE it is not.\n\n
		<b>BOOL forceSnap=FALSE</b>\n\n
		If TRUE snapping is used even if the primary angle snap toggle is off.
		\return  Angle snapped to the nearest angle snap setting (considering
		acceleration if specified), in radians, to be used for interactive
		rotation. */
		virtual float SnapAngle(float angleIn, BOOL fastSnap=TRUE, BOOL forceSnap=FALSE)=0;

		// Snap a percentage value (1.0 = 100%)
		/*! \remarks Given a value in the range 0.0 (0%) to 1.0 (100%) this
		method snaps the value using the current percentage snap.
		\par Parameters:
		<b>float percentIn</b>\n\n
		The value to snap.
		\return  The snapped value where 0.0 = 0% and 1.0 = 100%. */
		virtual float SnapPercent(float percentIn)=0;

		// Get the snap switch state
		/*! \remarks Retrieves the snap toggle state.
		\return  TRUE if snap is on; FALSE if snap is off. */
		virtual BOOL GetSnapState()=0;

		// Get the snap type -- Absolute or Relative (grid.h)
		/*! \remarks Retrieves the current snap type.
		\return  One of the following values:\n\n
		<b>SNAPMODE_RELATIVE</b>\n\n
		<b>SNAPMODE_ABSOLUTE</b> */
		virtual int GetSnapMode()=0;

		// Set the snap mode -- Set to absolute will fail if not in screen space
		// Returns TRUE if succeeded
		/*! \remarks Set the current snap mode. Note that setting the mode to
		<b>SNAPMODE_ABSOLUTE</b> will fail if the reference coordinate system
		is not in set to Screen.
		\par Parameters:
		<b>int mode</b>\n\n
		One of the following values:\n\n
		<b>SNAPMODE_RELATIVE</b>\n\n
		<b>SNAPMODE_ABSOLUTE</b>
		\return  Returns TRUE if succeeded; otherwise FALSE. */
		virtual BOOL SetSnapMode(int mode)=0;
//! @}

//! \name Node Picking (interactive selection in the scene)
//! @{	
		// This will set the command mode to a standard pick mode.
		// The callback implements hit testing and a method that is
		// called when the user actually pick an item.
		/*! \remarks This will set the command mode to a standard pick mode.
		The callback implements hit testing and a method that is called when
		the user actually picks an item. Note that this method, if called a
		second time, will cancel the pick mode and put the user into 'Select
		and Move' mode. This can be used to handle the case where a user clicks
		on a user interface control a second time to cancel to picking.\n\n
		Sample code using this API is available in
		<b>/MAXSDK/SAMPLES/OBJECTS/MORPHOBJ.CPP</b>. A utility plug-in that
		uses this API is <b>/MAXSDK/SAMPLES/UTILITIES/ASCIIOUT.CPP</b>.
		\par Parameters:
		<b>PickModeCallback *pCB</b>\n\n
		A pointer to an instance of the class PickModeCallback. */
		virtual void SetPickMode(PickModeCallback *pCB)=0;
		
		// makes sure no pick modes are in the command stack.
		/*! \remarks This method is called to make sure there are no pick
		modes in the command stack. */
		virtual void ClearPickMode()=0;

		// Hit tests the screen position for nodes and returns a 
		// INode pointer if one is hit, NULL otherwise.
		/*! \remarks This method hit tests the screen position for nodes and
		returns an INode pointer if one is hit, NULL otherwise.
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		Handle of the window to check.\n\n
		<b>IPoint2 pt</b>\n\n
		Point to check in screen coordinates.\n\n
		<b>PickNodeCallback *filt=NULL</b>\n\n
		This callback may be used to filter nodes from being picked. See
		Class PickNodeCallback.
		\return  INode pointer of node that was hit; NULL if no node was found.
		*/
		virtual INode *PickNode(HWND hWnd,IPoint2 pt,PickNodeCallback *filt=NULL)=0;

		// Region hit testing. To access the result, use the ViewExp functions
		// GetClosestHit() or GetHit().		
		/*! \remarks This method performs a node level hit test on the specified rectangular
		region. Use either the <b>ViewExp::GetClosestHit()</b> or
		<b>ViewExp::GetHit()</b> method to access the result.
		\par Parameters:
		<b>ViewExp *vpt</b>\n\n
		The viewport to perform the hit testing.\n\n
		<b>IPoint2 *pt</b>\n\n
		These points specify the box region to hit test. The first point in the
		array is the lower left. The second point is the upper right.\n\n
		<b>BOOL crossing</b>\n\n
		If TRUE Crossing selection is used; otherwise Window selection is
		used.\n\n
		<b>PickNodeCallback *filt=NULL</b>\n\n
		This callback may be used to filter nodes from being picked. See
		Class PickNodeCallback. */
		virtual void BoxPickNode(ViewExp *vpt,IPoint2 *pt,BOOL crossing,PickNodeCallback *filt=NULL)=0;
		/*! \remarks This method performs a node level hit test on the specified circular
		region. Use either the <b>ViewExp::GetClosestHit()</b> or
		<b>ViewExp::GetHit(int i)</b> method to access the result.
		\par Parameters:
		<b>ViewExp *vpt</b>\n\n
		The viewport to perform the hit testing.\n\n
		<b>IPoint2 *pt</b>\n\n
		These points specify the circular region to hit test. The first point
		in the array is the center. The second point is a point on the
		radius.\n\n
		<b>BOOL crossing</b>\n\n
		If TRUE Crossing selection is used; otherwise Window selection is
		used.\n\n
		<b>PickNodeCallback *filt=NULL</b>\n\n
		This callback may be used to filter nodes from being picked. See
		Class PickNodeCallback. */
		virtual void CirclePickNode(ViewExp *vpt,IPoint2 *pt,BOOL crossing,PickNodeCallback *filt=NULL)=0;
		/*! \remarks This method performs a node level hit test on the specified arbitrary
		polygonal region. Use either the <b>ViewExp::GetClosestHit()</b> or
		<b>ViewExp::GetHit()</b> method to access the result.
		\par Parameters:
		<b>ViewExp *vpt</b>\n\n
		The viewport to perform the hit testing.\n\n
		<b>IPoint2 *pt</b>\n\n
		These points specify the fence region to hit test. It is assumed the
		last point is connected to the first point to close the region. The
		fence knows when it's out of points when it hits a point that has
		negative <b>x</b> and <b>y</b> values. So, the initializer
		<b>IPoint2(-1, -1)</b> added to the bottom of this <b>IPoint2</b> list
		signals the end.\n\n
		<b>BOOL crossing</b>\n\n
		If TRUE Crossing selection is used; otherwise Window selection is
		used.\n\n
		<b>PickNodeCallback *filt=NULL</b>\n\n
		This callback may be used to filter nodes from being picked. See
		Class PickNodeCallback. */
		virtual void FencePickNode(ViewExp *vpt,IPoint2 *pt,BOOL crossing,PickNodeCallback *filt=NULL)=0;
//! @}

//! \name Sub-Object Related Methods
//! @{		
		//----- Modify-related Methods--------------------------

		// Registers the sub-object types for a given plug-in object type.

		// This method is obsolete for the modify panel from 3DS MAX 4 on. 
		// All subobject types have to be registered through the new methods 
		// BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType(int i). 
		// This method is still working for motion panel subobject types.
		// Please check the SDK help file for more information.

		/*! \remarks This method registers the sub-object types for a given
		plug-in object type. See the Advanced Topics section on
		~{ sub-object selection }~ for more details.
		\par Parameters:
		<b>const MCHAR **types</b>\n\n
		Array of strings listing the sub object levels. The order the strings
		appear in the array sets the indices used for the sub-object levels.
		Level 0 is always object level, the first string corresponds to level
		1, and the second string corresponds to level 2, etc. In the sample
		code below, <b>"Center"</b> is level 1 and <b>"Gizmo"</b> is level
		2.\n\n
		<b>int count</b>\n\n
		The number of strings in the array.\n\n
		<b>int startIndex=0</b>\n\n
		Specifies which string to display initially in the sub-object type
		combo box. This is needed because the NURBS object computes its
		sub-object list dynamically, and sometimes it add a new sub-object
		level while already in a sub-object level. The default value of zero
		replicates the original behavior.
		\par Sample Code:
		<b>const MCHAR *ptype[] = { _M("Center"), _M("Gizmo") };</b>\n\n
		<b>ip-\>RegisterSubObjectTypes(ptype, 2);</b> */
		virtual void RegisterSubObjectTypes( const MCHAR **types, int count,
                                             int startIndex = 0)=0;
//! @}

//! \name Sub-Object Selection Sets (Named)
//! @{		
		// Add sub-object named selection sets the named selection set drop down.
		// This should be done whenever the selection level changes.
		/*! \remarks A modifier may call this method to add sub-object named
		selection sets to the named selection set drop down list in the MAX
		toolbar. This should be done whenever the selection level changes (in
		the Modifiers <b>BaseObject::ActivateSubobjSel()</b> method). See
		Class BaseObject for additional
		methods associated with sub-object named selection sets.
		\par Parameters:
		<b>const MCHAR *set</b>\n\n
		The named selection set to add to the list. */
		virtual void AppendSubObjectNamedSelSet(const MCHAR *set)=0;

		// Clear the named selections from the drop down.
		/*! \remarks This method clears the named sub-object selection sets
		from the drop down. */
		virtual void ClearSubObjectNamedSelSets()=0;
//! @}

//! \name Selection Sets (Named)
//! @{		
		// Clears the edit field of the named selection set drop down
		/*! \remarks This method clears the current edit field of the named
		selection set drop down. */
		virtual void ClearCurNamedSelSet()=0;

		// Sets the edit field of the named selection set drop down
		/*! \remarks Sets the edit field of the named selection set drop down to the set
		whose name is passed.
		\par Parameters:
		<b>MCHAR *setName</b>\n\n
		The name of the selection set to make current. */
		virtual void SetCurNamedSelSet(const MCHAR *setName)=0;

		// new for V2... tell the system that the named sets have changed at
		// that the drop down needs to be rebuilt.
		/*! \remarks Calling this method tells the system that the named sub-object
		selection sets have changed and that the drop down needs to be rebuilt.
		This will cause <b>BaseObject::SetupNamedSelDropDown()</b> to be called
		on the current item being edited. This is often called inside restore
		objects that undo changes to the selection set. This causes the system
		to check if the current item being edited is in sub-object selection
		mode, and if so, will cause <b>SetupNamedSelDropDown()</b> to be
		called. Note that restore objects can be invoked at any time and the
		user may not be in sub-object selection mode (for instance they might
		be in the Display panel). Restore objects however can simply call this
		method and the system will figure out if the drop down needs to be
		updated. */
		virtual void NamedSelSetListChanged()=0;
//! @}

//! \name Sub-Object Related Methods
//! @{		
		// Returns the state of the sub object drop-down. 0 is object level
		// and >= 1 refer to the levels registered by the object.
		/*! \remarks Returns the state of the sub object drop-down.
		\return  0 is object level and \>= 1 refers to the levels registered by
		the object using <b>RegisterSubObjectTypes()</b>. The value refers to
		the order the item appeared in the list. 1 is the first item, 2 is the
		second, etc. */
		virtual int GetSubObjectLevel()=0;
		
		// Sets the sub-object drop down. This will cause the object being edited
		// to receive a notification that the current subobject level has changed.
        // if force == TRUE, the it will set the level even if the current
        // level is the same as the level requested.  This is to support
        // objects that change sub-object levels on the fly, like NURBS
		/*! \remarks Sets the sub-object drop down. This will cause the object
		being edited to receive a notification that the current sub-object
		level has changed (via <b>BaseObject::ActivateSubobjSel()</b>).
		\par Parameters:
		<b>int level</b>\n\n
		The level registered by the object using
		<b>RegisterSubObjectTypes()</b>. 0 indicates object level. Values
		greater than 1 refer to the order the items appeared in the list.\n\n
		<b>BOOL force = FALSE</b>\n\n
		If this parameter is TRUE, this method will set the level even if the
		current level is the same as the level requested. This is to support
		objects that change sub-object levels on the fly, for instance NURBS.
		*/
		virtual void SetSubObjectLevel(int level, BOOL force = FALSE)=0;

		// Returns the number of entries in the sub-object drop down list.
		/*! \remarks This method returns the number of sub object levels that the currently
		edited object (or modifier) has. Like the other sub-object related
		methods in this class, this method will only return a valid answer if
		the modifier panel is displayed. */
		virtual int GetNumSubObjectLevels()=0;

		// Notifies the system that the selection level in the pipeline has changed.
		/*! \remarks Plug-ins call this method to notify the system that the
		selection level in the pipeline has changed. The selection level flows
		up the pipeline so if you change the selection level you affect things
		later in the pipeline. For example an edit modifier that changes the
		sub-object level from vertex to object level must call this method
		after making the change to notify the system. Note that it is only
		called for modifiers whose sub-object levels propagate up the pipeline.
		For example, when entering a sub-object level within the Bend modifier,
		<b>PipeSelLevelChanged()</b> is not called.\n\n
		This method should be called from within <b>ActivateSubobjSel()</b> to
		notify the system that a selection level has changed in the pipeline.
		Note that calling this method from within <b>ModifyObject()</b> is no
		good since it involves re-evaluating the pipeline, which will call
		<b>ModifyObject()</b>, which will the call <b>PipeSelLevelChanged()</b>
		again, etc. */
		virtual void PipeSelLevelChanged()=0;

		// Returns the sub-object selection level at the point in the
		// pipeline  just before the current place in the history.
		/*! \remarks Gets the sub-object selection level at the point in the
		pipeline just before the current place in the history.
		\par Parameters:
		<b>DWORDTab \&levels</b>\n\n
		The sub-object level. This value depends on the object. The only level
		defined is level 0, which means 'object' level selection. */
		virtual void GetPipelineSubObjLevel(DWORDTab& levels)=0;

		// Hit tests the object currently being edited at the sub object level.
		/*! \remarks This method may be called to perform a sub-object hit
		test. You may access the number of hits using:
		<b>vpt-\>NumSubObjHits();</b>
		See Class ViewExp for a list of the other methods that may be used to examine the results.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time of the hit testing.\n\n
		<b>int type</b>\n\n
		The hit test type. See \ref SceneAndNodeHitTestTypes 
		<b>int crossing</b>\n\n
		Nonzero for crossing selection; 0 for normal (window).\n\n
		<b>int vflags</b>\n\n
		The flags for hit testing. See \ref SceneAndNodeHitTestFlags 
		<b>IPoint2 *p</b>\n\n
		Point to check in screen coordinates.\n\n
		<b>ViewExp *vpt</b>\n\n
		An interface pointer that may be used to call methods associated with
		the viewports.
		\return  Nonzero if the item was hit; otherwise 0. */
		virtual int SubObHitTest(TimeValue t, int type, int crossing, 
			int vflags, IPoint2 *p, ViewExp *vpt)=0;
//! @}

//! \name Modifier Related Methods
//! @{
		// Gets all instance contexts for the modifier at the current
		// place in the history.
		/*! \remarks A modifier may be applied to several objects in the
		scene. This method retrieves a list of all the ModContexts for the
		modifier at the current place in the modifier stack. It also gets a
		list of the nodes the modifier is applied to. For example a modifier
		may store data into the local data portion of the ModContext for each
		object affected by the modifier. In order to get at this data it needs
		to get each ModContext.\n\n
		Note: this method returns only the ModContext of the currently selected
		objects, even if the modifier is applied to several objects.
		\par Parameters:
		<b>ModContextList\& list</b>\n\n
		A reference to the list of ModContexts. See Class ModContextList.\n\n
		<b>INodeTab\& nodes</b>\n\n
		A reference to the table of each of the nodes. See Class INodeTab.
        \par Example
        \snippet howto/objects/perfacedata/EditFaceData.cpp Interface.GetModContexts Example        
        */
		virtual void GetModContexts(ModContextList& list, INodeTab& nodes)=0;

		// Get the object (or modifier) that is currently being edited in the
		// modifier panel
		/*! \remarks Returns a pointer to the object (or modifier) that is currently being
		edited in the modifier panel. See Class BaseObject. 		
		*/
		virtual BaseObject* GetCurEditObject()=0;
//! @}

//! \name Node Selection Sets
//! @{		
		// Is the selection set frozen?
		/*! \remarks In MAX the space bar freezes the selection set. This
		keeps the selection set from being inadvertently changed. This method
		access the status of the selection set frozen state.
		\return  TRUE if the selection set is frozen; otherwise FALSE. */
		virtual BOOL SelectionFrozen()=0;
		/*! \remarks Toggles the selection set to the frozen state. */
		virtual void FreezeSelection()=0;
		/*! \remarks Toggles the selection set to the thawed state. */
		virtual void ThawSelection()=0;

		// Nodes in the current selection set.
		/*! \remarks Selection sets are handled as a virtual array and
		accessed using an index starting at 0. This method returns a pointer to
		the 'i-th' node in the current selection set. \sa
		<b>GetSelNodeCount()</b>.
		\par Parameters:
		<b>int i</b>\n\n
		Index of the node in the selection set.
		\return  Pointer to the 'i-th' node in the selection set. */
		virtual INode *GetSelNode(int i)=0;
		/*! \remarks Returns the number of nodes in the selection set. */
		virtual int GetSelNodeCount()=0;
//! @}

//! \name Show End Result Related Methods
//! @{		
		// Enable/disable, get/set show end result. 
		/*! \remarks This method is called if a modifier wants to temporarily
		disable any modifiers following it. For example the edit mesh modifier
		does not let you edit a mesh while other modifiers later in the
		pipeline are affecting the result so it calls this method to disable
		the others temporarily.
		\par Parameters:
		<b>BOOL enabled</b>\n\n
		TRUE is enabled; FALSE is disabled. */
		virtual void EnableShowEndResult(BOOL enabled)=0;
		/*! \remarks Returns TRUE if the Show End Result button is on (pressed); otherwise
		FALSE. */
		virtual BOOL GetShowEndResult ()=0;
		/*! \remarks Sets the on/off (pressed/unpressed) state of the Show End Result
		button. Note that calling this method generates a redraw.
		\par Parameters:
		<b>BOOL show</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetShowEndResult (BOOL show)=0;
//! @}

//! \name User Interface Controls and Properties
//! @{			
		// Returns the state of the 'crossing' preference for hit testing.
		/*! \remarks Returns the state of the 'crossing' preference for hit
		testing.
		\return  TRUE if crossing selection is on; FALSE if off. */
		virtual BOOL GetCrossing()=0;

		// Sets the state of one of the transform tool buttons.
		// TRUE indicates pressed, FALSE is not pressed.
		/*! \remarks Sets the state of one of the transform tool buttons.
		\par Parameters:
		<b>int button</b>\n\n
		The transform tool buttons:\n\n
		<b>MOVE_BUTTON</b>\n\n
		<b>ROTATE_BUTTON</b>\n\n
		<b>NUSCALE_BUTTON</b>\n\n
		<b>USCALE_BUTTON</b>\n\n
		<b>SQUASH_BUTTON</b>\n\n
		<b>SELECT_BUTTON</b>\n\n
		<b>PLACE_BUTTON</b>\n\n
		<b>BOOL state</b>\n\n
		TRUE indicates pressed, FALSE is not pressed. */
		virtual void SetToolButtonState(int button, BOOL state )=0;
//! @}
		virtual BOOL GetToolButtonState(int button)=0;
		virtual void EnableToolButton(int button, BOOL enable=TRUE )=0;

        // Enable and disable Undo/Redo.
        virtual void EnableUndo(BOOL enable)=0;

//! \name Command Panel and Rollup Page Methods
//! @{
		// Get and set the command panel task mode
		/*! \remarks Returns a value to indicate which branch of the command panel is
		currently active.
		\return  One of the following values:\n\n
		<b>TASK_MODE_CREATE</b>\n\n
		<b>TASK_MODE_MODIFY</b>\n\n
		<b>TASK_MODE_HIERARCHY</b>\n\n
		<b>TASK_MODE_MOTION</b>\n\n
		<b>TASK_MODE_DISPLAY</b>\n\n
		<b>TASK_MODE_UTILITY</b> */
		virtual int GetCommandPanelTaskMode()=0;
		/*! \remarks This method sets the branch of the command panel that is currently active.
		\par Parameters:
		<b>int mode</b>\n\n
		One of the following values:\n\n
		<b>TASK_MODE_CREATE</b>\n\n
		<b>TASK_MODE_MODIFY</b>\n\n
		<b>TASK_MODE_HIERARCHY</b>\n\n
		<b>TASK_MODE_MOTION</b>\n\n
		<b>TASK_MODE_DISPLAY</b>\n\n
		<b>TASK_MODE_UTILITY</b> */
		virtual void SetCommandPanelTaskMode(int mode)=0;
//! @}

		// Finds the vpt given the HWND
		/*! \remarks This method gets a viewport interface given a window
		handle. If the input HWND is NULL, the interface of the active viewport 
		will be returned. If the given non-NULL HWND doesn't match any of the viewport, 
		the interface of the default view tab's first viewport will be returned.
		\par Parameters:
		<b>HWND hwnd</b>\n\n
		The window handle of the viewport. */
		virtual ViewExp& GetViewExp( HWND hwnd ) = 0;	

//! \name Animation/Time/Playback Related Methods
//! @{		
		// Disables/Enables animate button
		/*! \remarks Sets the state of the Animate button to enabled or
		disabled. When disabled the user cannot turn on Animate mode.\n\n
		Note: Developers have additional functions available for controlling
		the state of the animate button. These functions are defined in
		<b>/MAXSDK/INCLUDE/CONTROL.H</b>. These functions can be used to
		determine if animating is on or off, or toggle it on and off without
		affecting the appearance of the Animate button in the user interface.
		In this way, a user will not be aware anything is happening. See
		\ref Additional_Controller_Related_Functions "Additional Controller Related Functions"
		for documentation.
		\par Parameters:
		<b>BOOL enable</b>\n\n
		Pass TRUE to enable the button; FALSE to disable it. */
		virtual void EnableAnimateButton(BOOL enable)=0;
		/*! \remarks Returns TRUE if the Animate button is enabled; otherwise
		FALSE. See the note in the method above as well. */
		virtual BOOL IsAnimateEnabled()=0;

		// Turns the animate button on or off
		/*! \remarks Turns the animate button (and animate mode) on or off.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE to turn on; FALSE to turn off. */
		virtual void SetAnimateButtonState(BOOL onOff)=0;

		// In some simulation task, a time sequence of matrices are computed
		// and to be set to node via
		// INode::SetNodeTM(TimeValue t, const Matrix3& tm). If the node
		// employs the standard PRS controller and the rotation is the
		// standard Euler XYZ controller, it would achieve better animation
		// result if we decides euler angles at this frame based on those at
		// the previous frame. The Progressive Mode tells the Euler
		// controller to derive angles based on the previous frame.
		// It is assumed that in this mode, SetNodeTM() are called
		// in strictly forward time order.
		// Synopsis:
		//   GetCOREInterface()->BeginProgressiveMode();
		//   for (t = start_time, t < end_time; t += time_per_frame) {
		//      node.SetNodeTM(t, tm_at_t);
		//   }
		//   GetCOREInterface()->EndProgressiveMode();
		//
		/*! \remarks Returns TRUE is 3ds Max is in progressive mode. See
		BeginProgressiveMode(). */
		virtual bool InProgressiveMode() =0;
		/*! \remarks Turns on progressive mode for SetNodeTM().\n\n
		In certain situations, a time sequence of matrices is computed and sent
		to the node via INode::SetNodeTM(TimeValue t, const Matrix3\& tm). If the
		node employs the standard PRS controller and the rotation is the
		standard Euler XYZ controller, 3ds Max can achieve better animation
		results if it computes euler angles at the current frame based on those
		from the previous frame. Progressive mode tells the Euler controller to
		derive angles based on the previous frame. It is assumed that in this
		mode, SetNodeTM() is called in a strictly forward time order.
		\par Usage:
		<b>GetCOREInterface()-\>BeginProgressiveMode();</b>\n\n
		<b>for (t = start_time, t \< end_time; t += time_per_frame) {</b>\n\n
		<b>node.SetNodeTM(t, tm_at_t);</b>\n\n
		<b>}</b>\n\n
		<b>GetCOREInterface()-\>EndProgressiveMode();</b>   \n\n
		To test whether 3ds Max is currently in progressive mode, use:\n\n
		<b>if ( GetCOREInterface()-\>InProgressiveMode() )...</b> */
		virtual void BeginProgressiveMode() =0;
		/*! \remarks Turns off progressive mode. See BeginProgressiveMode().
		*/
		virtual void EndProgressiveMode() =0;
//! @}

//! \name Callback/Notification Registration
//! @{
		// Registers a callback that gets called whenever the axis
		// system is changed.
		/*! \remarks Registers a callback object that will get called any time
		the user changes the reference coordinate system by:\n\n
		Changing the transform coordinate system drop-down menu.\n\n
		Changing the state of the transform center fly-off.\n\n
		Changing X, Y, Z, XY, YZ, ZX constraint buttons/fly-off.\n\n
		Using an accelerator or anything else that changes the above.
		\par Parameters:
		<b>AxisChangeCallback *cb</b>\n\n
		Points to the callback to register. See AxisChangeCallback. */
		virtual void RegisterAxisChangeCallback(AxisChangeCallback *cb)=0;
		/*! \remarks Un-registers the axis change callback.
		\par Parameters:
		<b>AxisChangeCallback *cb</b>\n\n
		Points to the callback to un-register. See AxisChangeCallback. */
		virtual void UnRegisterAxisChangeCallback(AxisChangeCallback *cb)=0;
//! @}
		 
//! \name User Interface Controls and Properties
//! @{
		// Gets/Sets the state of the axis constraints.
		/*! \remarks Retrieves the state of the axis constraints flyoff.
		\return  One of the following axis constraints:\n\n
		<b>AXIS_XY</b>\n\n
		<b>AXIS_ZX</b>\n\n
		<b>AXIS_YZ</b>\n\n
		<b>AXIS_X</b>\n\n
		<b>AXIS_Y</b>\n\n
		<b>AXIS_Z</b> */
		virtual int GetAxisConstraints()=0;
		/*! \remarks Sets the state of the axis constraints flyoff.
		\par Parameters:
		<b>int c</b>\n\n
		The axis constraint to set. You may pass one of the following:\n\n
		<b>AXIS_XY</b>\n\n
		<b>AXIS_ZX</b>\n\n
		<b>AXIS_YZ</b>\n\n
		<b>AXIS_X</b>\n\n
		<b>AXIS_Y</b>\n\n
		<b>AXIS_Z</b> */
		virtual void SetAxisConstraints(int c)=0;
		/*! \remarks Enables or disables the specified axis constraint.
		\par Parameters:
		<b>int c</b>\n\n
		The axis constraint. You may pass one of the following:\n\n
		<b>AXIS_XY</b>\n\n
		<b>AXIS_ZX</b>\n\n
		<b>AXIS_YZ</b>\n\n
		<b>AXIS_X</b>\n\n
		<b>AXIS_Y</b>\n\n
		<b>AXIS_Z</b>\n\n
		<b>BOOL enabled</b>\n\n
		TRUE to enable; FALSE to disable. */
		virtual void EnableAxisConstraints(int c,BOOL enabled)=0;
		// An axis constraint stack
		/*! \remarks Pushes the specified axis constraint. This push/pop mechanism is used
		so that the appropriate axis mode can be restored after the Transform
		Gizmo has been used. The Gizmo itself calls this Push method is
		response to the HitTest with certain flags.
		\par Parameters:
		<b>int c</b>\n\n
		The axis constraint. You may pass one of the following:\n\n
		<b>AXIS_XY</b>\n\n
		<b>AXIS_ZX</b>\n\n
		<b>AXIS_YZ</b>\n\n
		<b>AXIS_X</b>\n\n
		<b>AXIS_Y</b>\n\n
		<b>AXIS_Z</b> */
		virtual void PushAxisConstraints(int c) = 0;
		/*! \remarks Pops the active constraint. After the Transform Gizmo pushes a
		constraint a selection processor pops it back again after the
		manipulators are deactivated. */
		virtual void PopAxisConstraints() = 0;

		// Gets/Sets the state of the coordinate system center
		/*! \remarks Retrieves the state of the coordinate system center.
		\return  One of the following values:\n\n
		<b>ORIGIN_LOCAL</b>\n\n
		Object's pivot.\n\n
		<b>ORIGIN_SELECTION</b>\n\n
		Center of selection set (or center of individual object for local or
		parent space).\n\n
		<b>ORIGIN_SYSTEM</b>\n\n
		Center of the reference coordinate system. */
		virtual int GetCoordCenter()=0;
		/*! \remarks Sets the state of the coordinate system center.
		\par Parameters:
		<b>int c</b>\n\n
		One of the following values (from <b>MAXAPI.H</b>).\n\n
		<b>ORIGIN_LOCAL</b>\n\n
		Object's pivot.\n\n
		<b>ORIGIN_SELECTION</b>\n\n
		Center of selection set (or center of individual object for local or
		parent space).\n\n
		<b>ORIGIN_SYSTEM</b>\n\n
		Center of the reference coordinate system. */
		virtual void SetCoordCenter(int c)=0;
		/*! \remarks Enables or disables the coordinates system center.
		\par Parameters:
		<b>BOOL enabled</b>\n\n
		TRUE to enable; FALSE to disable. */
		virtual void EnableCoordCenter(BOOL enabled)=0;

		// Gets/Sets the reference coordinate systems
		/*! \remarks Retrieves the reference coordinate system setting.
		\return  One of the following reference coordinate systems:\n\n
		<b>COORDS_HYBRID</b>\n\n
		<b>COORDS_SCREEN</b>\n\n
		<b>COORDS_WORLD</b>\n\n
		<b>COORDS_PARENT</b>\n\n
		<b>COORDS_LOCAL</b>\n\n
		<b>COORDS_OBJECT</b>\n\n
		<b>COORDS_GIMBAL</b>\n\n	
		<b>COORDS_WORKINGPIVOT</b>
		<b>COORDS_LOCAL_ALIGNED</b>
		*/
		virtual int GetRefCoordSys()=0;
		/*! \remarks Sets the reference coordinate system used.
		\par Parameters:
		<b>int c</b>\n\n
		Reference coordinate system:\n\n
		<b>COORDS_HYBRID</b>\n\n
		<b>COORDS_SCREEN</b>\n\n
		<b>COORDS_WORLD</b>\n\n
		<b>COORDS_PARENT</b>\n\n
		<b>COORDS_LOCAL</b>\n\n
		<b>COORDS_OBJECT</b>\n\n
		<b>COORDS_GIMBAL</b>\n\n	
		<b>COORDS_WORKINGPIVOT</b>
		<b>COORDS_LOCAL_ALIGNED</b> */
		virtual void SetRefCoordSys(int c)=0;
		/*! \remarks Enables or disables the reference coordinates system.
		\par Parameters:
		<b>BOOL enabled</b>\n\n
		TRUE to enable; FALSE to disable. */
		virtual void EnableRefCoordSys(BOOL enabled)=0;
//! @}

//! \name Selection Filter Drop-down Related Methods
//! @{
	// watje Gets/Sets the selection filter
		/*! \remarks This method returns your current selected select filter in the toolbar. */
		virtual int GetSelectFilter()=0;
		/*! \remarks This method allows you to set the current selected select filter in the toolbar.
		\par Parameters:
		<b>int c</b>\n\n
		The index of the filter you wish to set. */
		virtual void SetSelectFilter(int c)=0;
		/*! \remarks This method returns the number of select filters in the drop down list. */
		virtual int GetNumberSelectFilters()=0;
		/*! \remarks This method returns the name that appears in the interface for the
		specified filter.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the filter. */
		virtual const MCHAR* GetSelectFilterName(int index)=0;

		/*! \remarks This method returns the state of a display filter.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the display filter that you want to check. */
		virtual BOOL GetDisplayFilter(int index) = 0;
		/*! \remarks This method allows you to set the state of a display filter.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the display filter you wish to set.\n\n
		<b>BOOL on</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetDisplayFilter(int index, BOOL on) = 0;
		/*! \remarks This method returns the number of display filters in the display panel.
		*/
		virtual int GetNumberDisplayFilters() = 0;
		/*! \remarks This method checks the display filter at index, and sees if the node,
		class id, and super class id fail the filter check or not.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the filter\n\n
		<b>int sid</b>\n\n
		The super class id\n\n
		<b>Class_ID cid</b>\n\n
		The class ID\n\n
		<b>INode *node</b>\n\n
		The node to check.
		\return  TRUE if visible, otherwise FALSE. */
		virtual BOOL DisplayFilterIsNodeVisible(int index, int sid, Class_ID cid, INode *node) = 0;
		/*! \remarks This method returns the name of the specified filter.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the filter. */
		virtual const MCHAR* GetDisplayFilterName(int index)=0;
//! @}

//! \name Axis System Related Methods
//! @{
		// Gets the axis which define the space in which transforms should
		// take place. 
		// The node and subIndex refer to the object and sub object which the axis
		// system should be based on (this should be the thing the user clicked on)
		// If 'local' is not NULL, it will be set to TRUE if the center of the axis
		// is the pivot point of the node, FALSE otherwise.
		/*! \remarks An item that is doing sub-object hit testing gets to
		specify what their sub-object axes systems are. For example a mesh may
		have separate coordinate systems for every face or group of selected
		faces, while, for instance, a bend modifier has its own axes system for
		the gizmo. This method gets the axes system for a particular node. Each
		node may have several axes systems identified by an index.\n\n
		See <b>EDITMESH.CPP</b> for an example of use.
		\par Parameters:
		<b>INode *node</b>\n\n
		The node to get the axis coordinates system of.\n\n
		<b>int subIndex</b>\n\n
		The index of the axis system of the node.\n\n
		<b>BOOL* local = NULL</b>\n\n
		If 'local' is not NULL, it will be set to TRUE if the center of the
		axis is the pivot point of the node, FALSE otherwise.
		\return  The axis system of the node. */
		/*! \remarks Returns the axis which defines the space in which transforms should
		take place.
		\par Parameters:
		<b>INode *node</b>\n\n
		The object the axis system should be based on.\n\n
		<b>int subIndex</b>\n\n
		The sub object which the axis system should be based on (the thing the
		user clicked on).\n\n
		<b>BOOL* local = NULL</b>\n\n
		If 'local' is not NULL, it will be set to TRUE if the center of the
		axis is the pivot point of the node, FALSE otherwise.
		\return  A matrix representing the axis system that transforms take
		place in. */
		virtual Matrix3 GetTransformAxis(INode *node,int subIndex,BOOL* local = NULL)=0;

		// This returns the number of axis tripods in the scene. When transforming
		// multiple sub-objects, in some cases each sub-object is transformed in
		// a different space.
		// Return Values:
		// NUMAXIS_ZERO			- Nothing to transform
		// NUMAXIS_ALL			- Use only one axis.
		// NUMAXIS_INDIVIDUAL	- Do all, one at a time
		/*! \remarks This returns the number of axis tripods in the scene.
		When transforming multiple sub-objects, in some cases each sub-object
		is transformed in a different space.
		\return  One of the following values:\n\n
		<b>NUMAXIS_ZERO</b>\n\n
		Nothing to transform.\n\n
		<b>NUMAXIS_ALL</b>\n\n
		Use only one axis.\n\n
		<b>NUMAXIS_INDIVIDUAL</b>\n\n
		Do all, one at a time. */
		virtual int GetNumAxis()=0;

		// Locks axis tripods so that they will not be updated.
		/*! \remarks This method locks axis tripods so that they will not be
		updated.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE to lock; FALSE to unlock. */
		virtual void LockAxisTripods(BOOL onOff)=0;
		/*! \remarks This method returns TRUE if axis tripods are locked. */
		virtual BOOL AxisTripodLocked()=0;
//! @}

//! \name Register Windows
//! @{		
		// Registers a dialog window so IsDlgMesage() gets called for it.
		/*! \remarks Registers a dialog window so <b>IsDialogMessage()</b>
		gets called for it. This is not required if you add rollup pages to the
		command panel as this is done automatically, however if you create a
		floating, modeless dialog you <b>must</b> call this method.\n\n
		<b>Important Note</b>: ALL modeless dialogs in MAX must be registered
		to the application window with this method so that it, and any sub
		dialogs, will behave as they should.
		\par Parameters:
		<b>HWND hDlg</b>\n\n
		The window handle of the dialog. */
		virtual void RegisterDlgWnd( HWND hDlg )=0;
		/*! \remarks Un-registers a dialog window so <b>IsDialogMessage()</b>
		is no longer called for it.
		\par Parameters:
		<b>HWND hDlg</b>\n\n
		The window handle of the dialog.
		\return  Nonzero if successful; otherwise 0. */
		virtual int UnRegisterDlgWnd( HWND hDlg )=0;
//! @}

//! \name Keyboard Accelerators
//! @{
		// Registers a keyboard accelerator table
        // These functions are obsolete.  Use the AcceleratorTable
        // function below to get tables that use the keyboard prefs dialog
		/*! \remarks Registers a keyboard accelerator table.
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The window handle.\n\n
		<b>HACCEL hAccel</b>\n\n
		The handle of the accelerator table (from the Windows API). See the
		Windows API for more details on accelerator tables. */
		virtual void RegisterAccelTable( HWND hWnd, HACCEL hAccel )=0;
		/*! \remarks Un-registers a keyboard accelerator table.
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The window handle.\n\n
		<b>HACCEL hAccel</b>\n\n
		The handle of the accelerator table (from the Windows API). See the
		Windows API for more details on accelerator tables.
		\return  Nonzero if successful; otherwise 0. */
		virtual int UnRegisterAccelTable( HWND hWnd, HACCEL hAccel )=0;
//! @}

//! \name Action Table and Menu Manager Methods
//! @{        
		// The action manager is used to manage ActionTables which
        // plug-ins can use to export operations that can be tied to UI
        // elements like keyboard shortcuts, menus and toolbars.
        /*! \remarks Returns a pointer to the Action Manager interface class. The action
        manager is used to manage ActionTables which plug-ins can use to export
        operations that can be tied to UI elements like keyboard shortcuts,
        menus and toolbars. See Class IActionManager. */
        virtual IActionManager* GetActionManager() = 0;

        // Returns a pointer to the manager for customizable menus
        // see iMenuMan.h for a description of the menu manager
        /*! \remarks Returns a pointer to the manager for customizable menus. See Class IMenuManager. */
        virtual IMenuManager* GetMenuManager() = 0;
//! @}

//! \name Command Panel and Rollup Page Methods
//! @{    
		// Adds rollup pages to the command panel. Returns the window
		// handle of the dialog that makes up the page.
		/*! \remarks This method is used to add a rollup page to the command
		panel. It returns the window handle of the rollup page.
		\par Parameters:
		<b>HINSTANCE hInst</b>\n\n
		The DLL instance handle of the plug-in.\n\n
		<b>MCHAR *dlgTemplate</b>\n\n
		The dialog template for the rollup page.\n\n
		<b>DLGPROC dlgProc</b>\n\n
		The dialog proc to handle the messages sent to the rollup page.\n\n
		<b>MCHAR *title</b>\n\n
		The title displayed in the title bar of the rollup page.\n\n
		<b>LPARAM param=0</b>\n\n
		Any specific data to pass along may be stored here. This may be later
		retrieved using the <b>GetWindowLong()</b> call from the Windows API if
		it was set in the window using <b>SetWindowLong()</b>.\n\n
		For example, at the beginning of the dialog proc do something like:\n\n
		<b>BOOL CALLBACK MyDlgProc(</b>\n\n
		<b>HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {</b>\n\n
		<b> MyUtil *u = (MyUtil *)GetWindowLong(hWnd,
		GWL_USERDATA);</b>\n\n
		<b> if (!u \&\& message != WM_INITDIALOG ) return FALSE;</b>\n\n
		<b> . . .</b>\n\n
		Then inside the code block handling the <b>WM_INITDIALOG</b> message do
		something like:\n\n
		\code
		switch (message)
		{
			case WM_INITDIALOG:
				u = (MyUtil *)lParam;
				SetWindowLong(hWnd, GWL_USERDATA, (LONG)u);
				. . .
		\endcode  
		Then later in the dialog you can access the object passed in -- in the
		case above an instance of <b>MyUtil</b> (using <b>u-\></b>).\n\n
		<b>DWORD rollUpFlags=0</b>\n\n
		The following flag value may be used:\n\n
		<b>APPENDROLL_CLOSED</b>\n\n
		Starts the page in the rolled up (closed) state.\n\n
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
		<b>rollUpFlags</b> argument contains <b>ROLLUP_USEREPLACEDCAT</b>. This is mainly
		done, so that this system works with param maps as well.
		\par Sample Code:
		\code
		hBendParams = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_BEND_ROLLUP), BendParamDialogProc,_M("Parameters"), (LPARAM)this );
		\endcode 
		\return  The window handle of the rollup page. */
		virtual HWND AddRollupPage( HINSTANCE hInst, const MCHAR *dlgTemplate, 
				DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD rollUpFlags=0, int category = ROLLUP_CAT_STANDARD )=0;

        /*! Adds a rollup page that hosts a Qt dialog.
            \param qtWidget the Qt widget that will be hosted in the rollup. <b>This QWidget becomes the ownership of the system</b>; it will be deleted
                by the system once no longer needed.
            \param title The title string, displayed at the top of the rollup.l
            \param rollupFlags Optional rollup flags, such as APPENDROLL_CLOSED, DONTAUTOCLOSE, ROLLUP_SAVECAT, etc.
            \param category The category controls the ordering of the rollups, with lower values being inserted before rollups of higher category. 
            \sa void DeleteRollupPage(QWidget& qtWidget) */
        virtual void AddRollupPage( QWidget& qtWidget, const MCHAR *title, DWORD rollupFlags = 0, int category = ROLLUP_CAT_STANDARD ) = 0;

		/*! \remarks This method is used to add a rollup page to the command panel. It
		returns the window handle of the rollup page. This method is currently
		not being used.
		\par Parameters:
		<b>HINSTANCE hInst</b>\n\n
		The DLL instance handle of the plug-in.\n\n
		<b>DLGTEMPLATE *dlgTemplate</b>\n\n
		The dialog template for the rollup page.\n\n
		<b>DLGPROC dlgProc</b>\n\n
		The dialog proc to handle the messages sent to the rollup page.\n\n
		<b>MCHAR *title</b>\n\n
		The title displayed in the title bar of the rollup page.\n\n
		<b>LPARAM param=0</b>\n\n
		Any specific data to pass along may be stored here. This may be later
		retrieved using the <b>GetWindowLong()</b> call from the Windows API if
		it was set in the window using <b>SetWindowLong()</b>.\n\n
		<b>DWORD rollUpFlags=0</b>\n\n
		The following flag value may be used:\n\n
		<b>APPENDROLL_CLOSED</b>\n\n
		Starts the page in the rolled up (closed) state.\n\n
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
		<b>rollUpFlags</b> argument contains <b>ROLLUP_USEREPLACEDCAT</b>. This is mainly
		done, so that this system works with param maps as well.
		\return  The window handle of the rollup page. */
		virtual HWND AddRollupPage( HINSTANCE hInst, DLGTEMPLATE *dlgTemplate, 
				DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD rollUpFlags =0, int category = ROLLUP_CAT_STANDARD )=0;

		// Removes a rollup page and destroys it.
		/*! \remarks Removes a rollup page and destroys it.
		\par Parameters:
		<b>HWND hRollup</b>\n\n
		The window handle of the rollup window. This is the handle returned
		from <b>AddRollupPage()</b>. */
		virtual void DeleteRollupPage( HWND hRollup )=0;

        /*! Removes a rollup page that hosts a Qt dialog.
            \remark The QWidget will get destroyed as a result of calling this.
            \sa void AddRollupPage(QWidget& qtWidget, const MCHAR *title, DWORD rollupFlags, int category) */
        virtual void DeleteRollupPage(QWidget& qtWidget) = 0;

		// Replaces existing rollup with another. (and deletes the old one)
		/*! \remarks This replaces an existing rollup with another one and (deletes the original).
		\par Parameters:
		<b>HWND hOldRollup</b>\n\n
		The window handle of the old rollup.\n\n
		<b>HINSTANCE hInst</b>\n\n
		The DLL instance handle of the plug-in.\n\n
		<b>MCHAR *dlgTemplate</b>\n\n
		The dialog template for the rollup page.\n\n
		<b>DLGPROC dlgProc</b>\n\n
		The dialog proc to handle the messages sent to the rollup page.\n\n
		<b>MCHAR *title</b>\n\n
		The title displayed in the title bar of the rollup page.\n\n
		<b>LPARAM param=0</b>\n\n
		Any specific data to pass along may be stored here.\n\n
		<b>DWORD flags=0</b>\n\n
		The following flag value may be used:\n\n
		<b>APPENDROLL_CLOSED</b>\n\n
		Starts the page in the rolled up (closed) state.\n\n
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
		\return  The window handle of the rollup. */
		/*! \remarks This replaces an existing rollup with another one and (deletes the
		original). This method is currently not being used.
		\par Parameters:
		<b>HWND hOldRollup</b>\n\n
		The window handle of the old rollup.\n\n
		<b>HINSTANCE hInst</b>\n\n
		The DLL instance handle of the plug-in.\n\n
		<b>DLGTEMPLATE *dlgTemplate</b>\n\n
		The dialog template for the rollup page.\n\n
		<b>DLGPROC dlgProc</b>\n\n
		The dialog proc to handle the messages sent to the rollup page.\n\n
		<b>MCHAR *title</b>\n\n
		The title displayed in the title bar of the rollup page.\n\n
		<b>LPARAM param=0</b>\n\n
		Any specific data to pass along may be stored here.\n\n
		<b>DWORD rollUpFlags=0</b>\n\n
		The following flag value may be used:\n\n
		<b>APPENDROLL_CLOSED</b>\n\n
		Starts the page in the rolled up (closed) state.\n\n
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
		<b>rollUpFlags</b> argument contains <b>ROLLUP_USEREPLACEDCAT</b>. This is mainly
		done, so that this system works with param maps as well.
		\return  The window handle of the rollup. */
		virtual HWND ReplaceRollupPage( HWND hOldRollup, HINSTANCE hInst, const MCHAR *dlgTemplate, 
						DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD rollUpFlags =0, int category = ROLLUP_CAT_STANDARD )=0;
		
		virtual HWND ReplaceRollupPage( HWND hOldRollup, HINSTANCE hInst, DLGTEMPLATE *dlgTemplate, 
						DLGPROC dlgProc, const MCHAR *title, LPARAM param=0,DWORD rollUpFlags =0, int category = ROLLUP_CAT_STANDARD )=0;
		
		// Gets a rollup window interface to the command panel rollup
		/*! \remarks This method returns a rollup window interface to the command panel
		rollup. This interface provides methods for showing and hiding rollups,
		adding and removing rollup pages, etc. Note: This interface does not
		need to be released with <b>ReleaseIRollup()</b> as MAX takes care of
		this when it shuts down.\n\n
		See Class IRollupWindow. */
		virtual IRollupWindow *GetCommandPanelRollup()=0;

		// When the user mouses down in dead area, the plug-in should pass
		// mouse messages to this function which will pass them on to the rollup.
		/*! \remarks This method allows hand cursor scrolling in the command
		panel when the user clicks and drags the mouse in an unused area of the
		dialog. When the user mouse-es down in dead area of the command panel,
		the plug-in should pass mouse messages to this function which will pass
		them on to the rollup.\n\n
		Note: This method is obsolete in MAX 2.0 and later. These messages no
		longer need to be passed along as this is handled internally.
		\par Parameters:
		<b>HWND hDlg</b>\n\n
		The window handle of the dialog.\n\n
		<b>UINT message</b>\n\n
		The message sent to the dialog proc.\n\n
		<b>WPARAM wParam</b>\n\n
		Passed in to the dialog proc. Pass along to this method.\n\n
		<b>LPARAM lParam</b>\n\n
		Passed in to the dialog proc. Pass along to this method.
		\par Sample Code:
		\code
		case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_MOUSEMOVE:
			ip->RollupMouseMessage(hDlg,message,wParam,lParam);
		\endcode */
		virtual void RollupMouseMessage( HWND hDlg, UINT message, 
				WPARAM wParam, LPARAM lParam )=0;
//! @}

//! \name Animation/Time/Playback Related Methods
//! @{
		// get/set the current time.
		/*! \remarks Returns the current time, i.e. the frame slider position.
		See the Advanced Topics section on
		~{ Time Functions }~ for
		an overview of time in MAX. */
		virtual TimeValue GetTime()=0;	
		/*! \remarks Set the current time, updates the frame slider, and
		optionally redraws the viewports.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to set as current.\n\n
		<b>BOOL redraw=TRUE</b>\n\n
		If set to FALSE, the current time will be set to the specified time but
		the viewports will not be redrawn. */
		virtual void SetTime(TimeValue t,BOOL redraw=TRUE)=0;

		// get/set the anim interval.
		/*! \remarks Returns the current setting of the animation interval.
		This can be used to get the total number of frames in the animation.
		Note: The values stored in the interval returned represent ticks not
		frames. */
		virtual Interval GetAnimRange()=0;
		/*! \remarks Sets the animation interval. Note: The values passed in
		the interval are ticks not frames.
		\par Parameters:
		<b>Interval range</b>\n\n
		Specifies the new animation range to set. */
		virtual void SetAnimRange(Interval range)=0;
//! @}

//! \name Callback/Notification Registration
//! @{
		// Register a callback object that will get called every time the
		// user changes the frame slider.
		/*! \remarks Registers a callback object that will get called every
		time the current animation time is changed.
		\par Parameters:
		<b>TimeChangeCallback *tc</b>\n\n
		Points to the callback object to register. See Class TimeChangeCallback. */
		virtual void RegisterTimeChangeCallback(TimeChangeCallback *tc)=0;
		/*! \remarks This method un-registers the time change callback.
		\par Parameters:
		<b>TimeChangeCallback *tc</b>\n\n
		Points to the callback object to un-register. See Class TimeChangeCallback. */
		virtual void UnRegisterTimeChangeCallback(TimeChangeCallback *tc)=0;

		// Register a callback object that will get called when the user
		// causes the command mode to change
		/*! \remarks Register a callback object that will get called when the
		user changes the command mode.
		\par Parameters:
		<b>CommandModeChangedCallback *cb</b>\n\n
		Points to the callback object to register. See Class CommandModeChangedCallback. */
		virtual void RegisterCommandModeChangedCallback(CommandModeChangedCallback *cb)=0;
		/*! \remarks Un-registers the command mode change callback object.
		\par Parameters:
		<b>CommandModeChangedCallback *cb</b>\n\n
		Points to the callback object to un-register. See Class CommandModeChangedCallback. */
		virtual void UnRegisterCommandModeChangedCallback(CommandModeChangedCallback *cb)=0;

		// Register a ViewportDisplayCallback
		// If 'preScene' is TRUE then the callback will be called before object are rendered (typically, but not always).
		/*! \remarks Registers a <b>ViewportDisplayCallback</b> whose <b>Display()</b>
		method will be called to allow a plug-in to draw in the MAX viewports.
		\par Parameters:
		<b>BOOL preScene</b>\n\n
		If TRUE the callback will be called before objects are rendered
		(typically, but not always); if FALSE the callback is called after the
		objects are rendered.\n\n
		In some cases redrawing the viewports may take two passes; once to
		re-render the background plane and once to re-render the foreground
		plane. In this case the order of events would be:\n\n
		Call pre callbacks\n\n
		Render scene\n\n
		Call post callbacks\n\n
		Call pre callbacks\n\n
		Render scene\n\n
		Call post callbacks\n\n
		The two calls to callbacks in the middle are neither pre nor post
		callbacks. However you could also look at this as two separate
		redraws.\n\n
		<b>ViewportDisplayCallback *cb</b>\n\n
		Points to the callback object. See 	Class ViewportDisplayCallback. */
		virtual void RegisterViewportDisplayCallback(BOOL preScene,ViewportDisplayCallback *cb)=0;
		/*! \remarks Call this method to un-register the viewport display callback object.
		\par Parameters:
		<b>BOOL preScene</b>\n\n
		If TRUE the callback will be called before object are rendered
		(typically, but not always); if FALSE the callback is called after the
		objects are rendered.\n\n
		<b>ViewportDisplayCallback *cb</b>\n\n
		Points to the callback object. See Class ViewportDisplayCallback. */
		virtual void UnRegisterViewportDisplayCallback(BOOL preScene,ViewportDisplayCallback *cb)=0;
		/*! \remarks This method is called to inform MAX that the viewport callback has
		changed. This is similar to when an object that is part of the
		reference hierarchy changes and it needs to call
		<b>NotifyDependents()</b> with the message \ref REFMSG_CHANGE to
		inform the items that depend on it. In a sense a
		<b>ViewportDisplayCallback</b> is like an object in the scene in that
		it has a <b>Display()</b> method that is called. However, the callback
		is not actually an object that is part of the reference hierarchy so it
		cannot send a \ref REFMSG_CHANGE message when it changes. So this
		method provides the equivalent functionality. If the callback changes
		this method needs to be called.\n\n
		To understand why this is needed consider that if a plug-in called
		<b>RedrawViews()</b> five times in a row, the viewports may be redrawn
		the first time (if something has changed), but the next four calls
		won't do anything. This is because MAX maintains some flags that
		indicate if things have changed or not and these flags are reset on the
		first redraw to indicate that everything is up to date. Therefore, when
		a <b>ViewportDisplayCallback</b> changes, it needs to call this method
		to let MAX know that changes have been made and the viewports indeed
		need to be redrawn the next time <b>RedrawViews()</b> is called.
		\par Parameters:
		<b>BOOL preScene</b>\n\n
		If TRUE the callback will be called before objects are rendered
		(typically, but not always); if FALSE the callback is called after the
		objects are rendered.\n\n
		<b>ViewportDisplayCallback *cb</b>\n\n
		Points to the callback object. See Class ViewportDisplayCallback. */
		virtual void NotifyViewportDisplayCallbackChanged(BOOL preScene,ViewportDisplayCallback *cb)=0;

		// Register a ExitMAXCallback
		// Note: the Exit method of ExitMAXCallback is not called on shutdown after crash recovery
		// See Interface8 for an overriding method.
		/*! \remarks Registers a <b>ExitMAXCallback</b> whose <b>Exit()</b> method will be
		called when MAX is about to exit. The return value from the callback
		allows the plug-in to decide if MAX exits or not.
		\par Parameters:
		<b>ExitMAXCallback *cb</b>\n\n
		Points to the callback object. See
		Class ExitMAXCallback. */
		virtual void RegisterExitMAXCallback(ExitMAXCallback *cb)=0;
		/*! \remarks Un-registers the exit callback so it's no longer called.
		\par Parameters:
		<b>ExitMAXCallback *cb</b>\n\n
		Points to the callback object. See
		Class ExitMAXCallback. */
		virtual void UnRegisterExitMAXCallback(ExitMAXCallback *cb)=0;
//! @}

//! \name Right Click Menu
//! @{		
		/*! \remarks Using this pointer a developer can add to the menu that
		pops ups when the user clicks the right mouse button. Example code is
		available in: <b>/MAXSDK/SAMPLES/MODIFIERS/EDITSPL.CPP</b>. \sa
		Class RightClickMenuManager.
		\return  A pointer to the <b>RightClickMenuManager</b>. */
		virtual RightClickMenuManager* GetRightClickMenuManager()=0;
//! @}

//! \name Delete Key Notification
//! @{
		// Delete key notification
		/*! \remarks Registers a callback invoked when the user presses the
		Delete Key. This allows the developer to override the default
		processing of the Delete Key.
		\par Parameters:
		<b>EventUser *user</b>\n\n
		Pointer to an instance of the EventUser class.
		\par Sample Code:
		See <b>/MAXSDK/SAMPLES/MODIFIERS/EDITSPL.CPP</b>. */
		virtual void RegisterDeleteUser(EventUser *user)=0;		// Register & Activate
		/*! \remarks Un-registers a Delete Key callback.
		\par Parameters:
		<b>EventUser *user</b>\n\n
		Pointer to an instance of the EventUser class. */
		virtual void UnRegisterDeleteUser(EventUser *user)=0;	// Deactivate & UnRegister
//! @}

		//----- Creation-related Methods--------------------------
		
//! \name Creating Unique
//! @{
		/*! \remarks Generates node names that are unique within the current scene.
		Given a name, this method will modify it to ensure it's unique. 
		It does this by appending a unique numeric suffix to the end of the name. 
		Notes:
		- Calling this method repeatedly with the same original name that 
		is not used by any node in the scene, it will return each time the same name.
		So, for a node name to be considered by this method as being in use, the node
		needs to be in the current scene.
		- Names of scene xrefed nodes are not considered as being part of the current scene.
		- Calling this method after Interface::CreateObjectNode(Object*) can result in the name 
		suffix to be greater than the value expected by the client code. For example, if Box001
		already exists in the scene, calling Interface::CreateObjectNode(Object*) to create a new
		Box object, then calling Interface::MakeNameUnique() to create a unique name for it, will yield 
		Box003 as new unique name, while Box002 should be the correct new name. The solution is 
		to either call Interface::CreateObjectNode(Object*) only, which will create a new name, or
		first call Interface::MakeNameUnique() with a root name and then call 
		Interface::CreateObjectNode(Object*, const MCHAR*) to create the object with the desired new name.
		See also class NameMaker for ways to create unique names for other scene
		entities than nodes, such as layers, materials, etc.
		\par Parameters:
		<b>MSTR \&name</b>\n\n
		The name to make unique. Any existent numeric suffix is stripped before the name is made unique. */
		virtual void MakeNameUnique(MSTR &name)=0;
//! @}

//! \name Creation Related Methods
//! @{
		/*! \remarks Creates a new node in the scene with the given object.
		Normally a developer will use the standard creation methods for
		procedural objects and this method is not used. However if the
		developer wants to handle the creation process on their own they may
		need to use this method. \sa
		~{ Object Creation Methods }~.
		\par Parameters:
		<b>Object *obj</b>\n\n
		Pointer to the Object to create.
		\return  Pointer to the node created. 
		See also Interface::CreateObjectNode(Object*, const MCHAR*) */
		virtual INode *CreateObjectNode( Object *obj)=0;		
		//! \brief Creates a new camera object of the given type.
		/*! Creates a new camera object of the given type.
		\param  type Camera type (FREE_CAMERA, TARGETED_CAMERA or PARALLEL_CAMERA as defined in gencam.h).
		\return  Pointer to the camera object created.
		*/
		virtual GenCamera *CreateCameraObject(int type) = 0;
		virtual Object   *CreateTargetObject() = 0;
		virtual GenLight *CreateLightObject(int type) = 0;
		/*! \remarks Creates an instance of a registered class. This will call
		<b>Create()</b> on the class descriptor.
		\par Parameters:
		<b>SClass_ID superID</b>\n\n
		The super class ID of the item to create an instance of.\n\n
		<b>Class_ID classID</b>\n\n
		The class ID of the item to create an instance of. See
		Class Class_ID.
		\return  Pointer to the created instance.
		\par Note:
		There is a global method that duplicates the functionality of this
		class method to let you create an instance of any registered class
		wherever you are (without the interface pointer):\n\n
		<b>void* CreateInstance(SClass_ID superID, Class_ID classID);</b> */
		virtual void* CreateInstance(SClass_ID superID, Class_ID classID)=0;
		/*! \remarks This method binds a node to a target using a lookat
		controller.
		\par Parameters:
		<b>INode *laNode</b>\n\n
		Pointer to the node to assign the lookat controller to.\n\n
		<b>INode *targNode</b>\n\n
		Pointer to the target node.
		\return  Returns TRUE if the node was bound; otherwise FALSE.
		\par Sample Code:
		This method is used in <b>/MAXSDK/SAMPLES/OBJECTS/CAMERA.CPP</b>. */
		virtual int BindToTarget(INode *laNode, INode *targNode)=0;
//! @}

//! \name Viewport Access
//! @{		
		/*! \remarks This method returns nonzero if the construction plane is
		'head on' in the current viewport. For example if the construction
		plane was XY and you were looking from the Front view, this method
		would return nonzero. This is used for example during object creation
		because this process doesn't work very well when the view is 'head on'.
		\return  Nonzero if the construction plane is 'head on' in the current
		viewport; otherwise 0.\n\n
		For use with extended views: - make the extended viewport active (set
		on mouse click, for example) -put up the view type popup menu (put up
		on right-click, for example) */
		virtual int IsCPEdgeOnInView()=0;
//! @}
		
//! \name Node Related Methods
//! @{
		//!	\brief Deletes the specified node from the scene.
		/*!	Call this method to delete a specific node from the scene. 
		\li The children of the deleted node will maintain their position. 
		\li Empty group heads resulting from the deletion of the node will also be deleted. 
		\li System nodes are not deleted by this method. Use the system's methods to
		modify its node structure. Use Interface9::DeleteNodes to delete all nodes of a
		system object.
		\li The selection state and deletion of the node will be undo-able after calling 
		this method, provided that the undo	system was engaged. For example, to delete 
		the first node in the current selection set, one could use the following code:
		\code
		theHold.Begin();
		Interface* ip = GetCOREInterface()
		ip->DeleteNode(ip->GetSelNode(0));
		theHold.Accept(_M("Delete"));
		\endcode
		The string used in the Accept() method appears in the undo list. 

		Note that the object reference of the node is deleted as well if the only item referencing 
		that object is the node. This is because when the node is deleted, it first 
		deletes all its references. Whenever a reference is deleted, if the item is 
		the last reference, then the system deletes the item as well. If the node
		deletion occurs while the undo system was engaged, RestoreObjs will 
		hold references to the items removed from the node, preventing their 
		immediate deletion.

		\param [in] node The node to be deleted
		\param [in] redraw If FALSE the viewports will not be redrawn after the node is 
		deleted. This allows several nodes to be deleted at once without the viewports 
		having to be redrawn after each one.
		\param [in] overrideDriven If TRUE, this method will delete nodes whose
		TM controllers implement Control::PreventNodeDeletion() to return TRUE. 
		Control::PreventNodeDeletion() controls whether the node can be deleted from the 
		scene. This parameter allows a driver controller to easily delete driven nodes
		if it needs to.
		\return The number of nodes that were deleted.
		*/
		virtual unsigned int DeleteNode(INode* node, BOOL redraw = TRUE, BOOL overrideDriven = FALSE) = 0;

		/*! \remarks Returns a pointer to the root node. From the root node
		one can retrieve the children (using <b>INode::NumberOfChildren()</b>,
		and <b>INode::GetChildNode(i)</b>). This allows a developer enumerated
		the scene by enumerating the node tree. */
		virtual INode *GetRootNode()=0;

		/*! \remarks Sets the nodes transform relative to the current
		construction plane. This may be used during creating so you can set the
		position of the node in terms of the construction plane and not in
		world units.
		\par Parameters:
		<b>INode *node</b>\n\n
		The node whose transform will be set.\n\n
		<b>Matrix3\& mat</b>\n\n
		The transformation matrix. */
		virtual void SetNodeTMRelConstPlane(INode *node, Matrix3& mat)=0;
//! @}

//! \name Node Selection Sets
//! @{		
		/*! \remarks This selects the specified node, either adding it to the
		current selection set, or creating a new selection set with the
		node.\n\n
		Note: A developer should call <b>theHold.Begin()</b> before this method
		and <b>theHold.Accept()</b> after this call. Otherwise, MAX may crash
		if the user selects undo after this call has been issued. See the
		sample code below. Also see
		~{ The Undo and Redo System }~ for
		more details on this system.
		\par Parameters:
		<b>INode *node</b>\n\n
		The node to select.\n\n
		<b>int clearSel = 1</b>\n\n
		If zero, the node is added to the current selection set. If nonzero,
		the node replaces the selection set.
		\par Sample Code:
		\code
		theHold.Begin();
		ip->SelectNode(node);
		MSTR undostr; undostr.printf("Select");
		theHold.Accept(undostr);
		\endcode */
		virtual void SelectNode( INode *node, int clearSel = 1)=0;
		/*! \remarks This method un-selects the specified node.
		\par Parameters:
		<b>INode *node</b>\n\n
		The node to deselect. */
		virtual void DeSelectNode(INode *node)=0;
		/*! \remarks This method may be used to select or deselect a group of nodes all at
		once and optionally redraw the viewports when done.
		\par Parameters:
		<b>INodeTab \&nodes</b>\n\n
		The Table of nodes to select or deselect.\n\n
		<b>BOOL sel</b>\n\n
		If TRUE the nodes are selected; otherwise they are deselected.\n\n
		<b>BOOL redraw=TRUE</b>\n\n
		If TRUE the viewports are redrawn when done; otherwise they are not. */
		virtual void SelectNodeTab(INodeTab &nodes,BOOL sel,BOOL redraw=TRUE)=0;
		/*! \remarks This method may be used to clear the current selection set and
		optionally redraw the viewports.
		\par Parameters:
		<b>BOOL redraw=TRUE</b>\n\n
		If TRUE the viewports are redrawn when done; otherwise they are not. */
		virtual void ClearNodeSelection(BOOL redraw=TRUE)=0;
//! @}

//! \name Light Related Methods
//! @{
		/*! \remarks Adds a light to the scene and registers the light with
		the viewports.
		\par Parameters:
		<b>INode *node</b>\n\n
		The light to add. */
		virtual void AddLightToScene(INode *node)=0;
		/*! \remarks Returns the light cone constraint angle (in radians).
		This is the hotspot / falloff separation angle. */
		virtual float GetLightConeConstraint()=0;
//! @}

//! \name Grid Related Methods
//! @{
		/*! \remarks If grid Auto Activation is on, sets the given node as the active grid object. This is
		used with grid helper objects to allow them to take effect. This method
		may also be used to activate the home grid.\n\n
		\par Parameters:
		<b>INode *node</b>\n\n
		The node to set as the active grid object. To activate the home grid
		pass NULL. */
		virtual void AddGridToScene(INode *node) = 0;
		/*! \remarks Sets the given node as the active grid object. This is
		used with grid helper objects to allow them to take effect. This method
		may also be used to activate the home grid.\n\n
		\par Parameters:
		<b>INode *node</b>\n\n
		The node to set as the active grid object. To activate the home grid
		pass NULL. */
		virtual void SetActiveGrid(INode *node)=0;
		/*! \remarks Returns the active grid node or NULL if the home grid is in use. */
		virtual INode *GetActiveGrid()=0;
//! @}

//! \name Creation Related Methods
//! @{
		/*! \remarks Calling this method will result in ending the currently active 
		object creation mode. The plugin's ClassDesc::EndCreate() method will also be 
		called as a result of calling StopCreating().
		Most often this method is called either from custom creation command modes in 
		response to a change in the current object selection, or from functions that 
		need to change the current object selection and could be invoked virtually 
		any time, such as via a hot-key or MAXScript.
		Note that this method should not be called from within an object's creation 
		callback in order to stop creating the current object. Rather, CREATE_STOP 
		should be returned from the creation callback procedure.
		For more information, see ~{ Object Creation Methods }~. */
		virtual void StopCreating()=0;

		// This creates a new object/node with out going through the usual
		// create mouse proc sequence.
		// The matrix is relative to the construction plane.
		/*! \remarks This creates a new object/node without going through the usual
		create mouse proc sequence. The matrix is relative to the construction plane.
		This must be called during the creation phase of an object. For example, the
		procedural sphere uses it when the user clicks on the 'Create' button after
		they type in the parameters for the sphere. See
		~{ Object Creation Methods }~.
		\par Parameters:
		<b>Matrix3 tm</b>\n\n
		The transformation matrix relative to the construction plane. */
		virtual Object *NonMouseCreate(Matrix3 tm)=0;
		/*! \remarks This method is presently only used in the Line object. It's used for
		repositioning the pivot point of the object at the end of the creation
		process. You simply pass it the new matrix, and the creation manager
		sets the TM for the node being created to the given TM * the
		construction plane TM.
		\par Parameters:
		<b>Matrix3 tm</b>\n\n
		The transformation matrix relative to the construction plane for the
		node. */
		virtual void NonMouseCreateFinish(Matrix3 tm)=0;
//! @}

//! \name Plug-In and Bitmap Paths
//! @{		
		// directories
		/*! \remarks Returns the pathname of a directory used by 3ds Max.
		\par Parameters:
		<b>int which</b>\n\n
		One of the following directories. See
		<a href="group___m_a_x_dir_i_ds.html">List of Directory Names</a>.
		\return  The pathname of the directory. */
		virtual const MCHAR *GetDir(int which)=0;		// which = APP_XXX_DIR
		/*! \remarks This is the number of entries in <b>PLUGIN.INI</b>.
		<b>PLUGIN.INI</b> contains a list of descriptions and directories used
		by plug-in DLLs. \sa
		~{ Plug-In Directory Search Mechanism }~.
		\return  The number of entries in <b>PLUGIN.INI</b>. */
		virtual int	GetPlugInEntryCount()=0;	// # of entries in PLUGIN.INI
		/*! \remarks Returns the 'i-th' description string from <b>PLUGIN.INI</b>. \n
		\sa
		~{ Plug-In Directory Search Mechanism }~.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies which description to return.
		\return  The 'i-th' description string from <b>PLUGIN.INI</b>. */
		virtual const MCHAR *GetPlugInDesc(int i)=0;	// ith description
		/*! \remarks Returns the pathname string for the 'i-th' plug-in directory from
		<b>PLUGIN.INI</b>. \sa
		~{ Plug-In Directory Search Mechanism }~.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies which directory to return.
		\return  The 'i-th' pathname string from <b>PLUGIN.INI</b>. */
		virtual const MCHAR *GetPlugInDir(int i)=0;	// ith directory
//! @}

		/*! \brief Returns the number of permanent user paths associated with a particular type of asset.
				 
			\param [in] assetType An enumeration constant representing the asset's type
			\return A non-negative integer denoting the number of paths
			\remarks Remember that this function gives you the count of permanent user paths,
			not session user paths. Session user paths are temporary. */
		virtual int GetAssetDirCount(MaxSDK::AssetManagement::AssetType assetType)=0;			// number of assetType dirs in path
		
		/*! \brief Returns the "ith" permanent user path associated with a particular asset type

			\param [in] i The index of the directory you want
			\param [in] assetType An enumeration constant representing the asset's type
			\return A c-string with an absolute path to the ith asset directory
			\remarks Remember that this function returns a permanent user path, not a session user path. A
			session user path is temporary. */
		virtual const MCHAR *GetAssetDir(int i, MaxSDK::AssetManagement::AssetType assetType)=0;		// i'th assetType dir of path
		
		/*! \brief Adds a permanent user path associated to a particular asset type.

			\param [in] dir An absolute path
			\param [in] assetType An enumeration constant representing the asset's type
			\return TRUE if the path was added, FALSE if not
			\remarks Remember that this function is for adding permanent user paths, not session user paths.
			Session user paths are temporary. */
		virtual BOOL AddAssetDir(const MCHAR *dir, MaxSDK::AssetManagement::AssetType assetType)=0;	// add an assetType path to the list
		
		/*! \brief Adds a session user path associated to a particular asset type.
			
			\param [in] dir An absolute path
			\param [in] assetType An enumeration constant representing the asset's type
			\param [in] update system configuration files with the new data
			\return TRUE if the path was added, FALSE if not
			\remarks Remember that this function is for adding permanent user paths, not session user paths */
		virtual BOOL AddSessionAssetDir(const MCHAR *dir, MaxSDK::AssetManagement::AssetType assetType,int update=TRUE)=0;
		
		/*! \brief Returns the number of session user paths associated with a particular type of asset.
		
			\param [in] assetType An enumeration constant representing the asset's type
			\return A non-negative integer denoting the number of paths
			\remarks Remember that this function gives you the count of session user paths,
			not permanent user paths. Session user paths are temporary. */
		virtual int GetSessionAssetDirCount(MaxSDK::AssetManagement::AssetType assetType)=0;
		
		/*! \brief Returns the "ith" session user path associated with a particular asset type.

			\param [in] i The index of the directory you want
			\param [in] assetType An enumeration constant representing the asset's type
			\return a c-string with an absolute path to the ith asset directory
			\remarks Remember that this function returns a session user path, not a permanent user path. */
		virtual const MCHAR *GetSessionAssetDir(int i, MaxSDK::AssetManagement::AssetType assetType)=0;
		
		/*! \brief Deletes the "ith" session user path associated with a particular asset type.

			\param [in] i The index of the directory you want to delete
			\param [in] assetType An enumeration constant representing the asset's type
			\param [in] update system configuration files with the new data
			\return a c-string with an absolute path to the ith asset directory
			\remarks Remember that this function returns a session user path, not a permanent user path.*/
		virtual BOOL DeleteSessionAssetDir(int i, MaxSDK::AssetManagement::AssetType assetType, int update=TRUE)=0;
		
		/*! \brief Returns the number of session (temporary) and permanent user paths associated with a particular type of asset
		
			\param [in] assetType An enumeration constant representing the asset's type
			\return A non-negative integer denoting the number of paths
			\remarks Remember that this function gives you the total count of both session(temporary) and permanent
			user paths. */
		virtual int GetCurAssetDirCount(MaxSDK::AssetManagement::AssetType assetType)=0;
		
		/*! \brief Returns the "ith" session\\permanent user path associated with a particular asset type. Session
			user paths come first in the list
		 
			\param [in] i The index of the directory you want
			\param [in] assetType An enumeration constant representing the asset's type
			\return a c-string with an absolute path to the ith asset directory
			\remarks Remember that session user paths come before permanent user paths. */
		virtual const MCHAR *GetCurAssetDir(int i, MaxSDK::AssetManagement::AssetType assetType)=0;
		
//! \name Methods to Display MAX Dialogs
//! @{		
		// New Method for light exclusion/inclusion lists
		/*! \remarks This brings up the standard Exclude / Include dialog box
		used for light exclusion / inclusion lists.\n
		\n \image html "dlgexcl.gif"
		\par Parameters:
		<b>ExclList *nl</b>\n\n
		If the user selects OK, this is the list of names chosen by the user.
		See Class ExclList.\n\n
		<b>BOOL doShadows=TRUE</b>\n\n
		The shadows switch.
		\return  Nonzero if the user selected OK to exit the dialog; otherwise
		0. */
		virtual int DoExclusionListDialog(ExclList *nl, BOOL doShadows=TRUE)=0;

		// Convert a NameTab to an ExclList ( the new format for Exclusion lists).
		/*! \remarks This method will convert a NameTab to an ExclList (the new format for
		Exclusion lists).
		\par Parameters:
		<b>const NameTab *nt</b>\n\n
		A pointer to the name table.\n\n
		<b>ExclList *excList</b>\n\n
		A pointer to the resulting exclusion list. */
		virtual void ConvertNameTabToExclList(const NameTab *nt, ExclList *excList)=0;
		
		/*! \remarks This method brings up the Material / Map Browser dialog box.
		\n \image html "dlgbrows.gif"
		\par Parameters:
		<b>HWND hParent</b>\n\n
		The parent window handle.\n\n
		<b>DWORD vflags</b>\n\n
		See \ref Material_Browser_Flags 
		<b>BOOL \&newMat</b>\n\n
		TRUE if the user has selected a new item; otherwise FALSE. If TRUE it
		is safe to modify this item. If FALSE the item may be an instance and a
		developer should not modify this as other materials may be using this
		same item.\n\n
		<b>BOOL \&cancel</b>\n\n
		TRUE if the user canceled the dialog; otherwise FALSE.
		\return  If <b>cancel</b> is FALSE, the item chosen by the user is
		returned. See Class MtlBase. */
		virtual MtlBase *DoMaterialBrowseDlg(HWND hParent,DWORD vflags,BOOL &newMat,BOOL &cancel)=0;

		/*! \remarks Puts the specified material into the material editor. The
		material is put to the specified slot, or if -1 is passed, a dialog is
		presented which allows the user to choose a sample slot for the
		material.\n
		\n \image html "puttomtl.gif"
		\par Parameters:
		<b>MtlBase *mb</b>\n\n
		The material to put to the material editor.\n\n
		<b>int slot=-1</b>\n\n
		The Materials Editor slot number (a value in the range 0 to 23). If a
		slot number is specified, then this method will replace that material
		in the Materials Editor without user interaction. If -1 is passed (the
		default) then the function brings up the put dialog. */
		virtual void PutMtlToMtlEditor(MtlBase *mb, int slot=-1)=0;
//! @}

//! \name Material Related Methods
//! @{
		/*! \remarks Returns a pointer to the material in the specified slot in the
		Materials Editor.
		\par Parameters:
		<b>int slot</b>\n\n
		The number of the slot in the Materials Editor (a value in the range 0
		to 23). */
		virtual MtlBase* GetMtlSlot(int slot)=0;

		/*! \remarks Returns a list of all the materials used in the scene.
		\return  See Class MtlBaseLib. 
		\see Interface17::UpdateSceneMaterialLib
		*/
		virtual MtlBaseLib* GetSceneMtls()=0;

		/*! \remarks Before assigning material to scene, call this to avoid duplicate names.
		Note that this method may replace any existing scene materials with the
		same name as the supplied material with that supplied material. If you are calling
		this method from the UI layer, make sure to call theHold.Begin() before calling this
		method, and if this method returns FALSE, call theHold.Cancel();
		\par Parameters:
		<b>MtlBase *m</b>\n\n
		The material to get the name from, and the material used to replace any existing
		scene materials with same name.\n\n
		\return  TRUE if it is okay to assign the material; FALSE if not. */
		virtual	BOOL OkMtlForScene(MtlBase *m)=0;
//! @}

//! \name Filenames and Pathnames
//! @{
		// Access names of current files
		/*! \remarks Returns the name of the current MAX file (but not the
		path). For example, if the currently loaded file is
		"<b>D:\3DSMAX\SCENES\Expgears.max</b>" this method returns "
		<b>Expgears.max</b>". */
		virtual MSTR &GetCurFileName()=0;
		/*! \remarks Returns the file and path of the current MAX file. For
		example, if the currently loaded file is
		"<b>D:\3DSMAX\SCENES\Expgears.max</b>" this method returns
		"<b>D:\3DSMAX\SCENES\Expgears.max</b> ". */
		virtual MSTR &GetCurFilePath()=0;
		/*! \remarks Returns the current material library file name. */
		virtual const MCHAR *GetMatLibFileName()=0;
//! @}

//! \name File Open/Merge/Save/Reset/Hold/Fetch Methods
//! @{		
		// These may bring up file requesters
		/*! \remarks This brings up the standard MAX file open dialog and
		allows the user to load a new scene. */
		virtual void FileOpen()=0;
		/*! \remarks This saves the current file. If the file has not been
		saved yet (and is thus unnamed) this brings up the standard MAX file
		Save As dialog box and allows the user to choose a name.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL FileSave()=0;
		/*! \remarks This brings up the standard MAX file save as dialog box
		and allows the user to save the current scene to a new file.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL FileSaveAs()=0;
		/*! \remarks This brings up the standard MAX file save selected dialog
		box and allows the user to save the selected items from the current
		scene. */
		virtual void FileSaveSelected()=0;
		/*! \remarks This resets MAX to its startup condition after a
		confirmation prompt. This performs the same operation as choosing File
		/ Reset from the MAX menus.
		The confirmation prompt is not shown if running in quiet mode.
		\par Parameters:
		<b>BOOL noPrompt=FALSE</b>\n\n
		If TRUE the confirmation prompt is not presented. */
		virtual void FileReset(BOOL noPrompt=FALSE)=0;
		/*! \remarks This allows the user to merge another MAX file with the
		current scene. This performs the same operation as choosing File /
		Merge from the MAX menus. */
		virtual void FileMerge()=0;
		/*! \remarks This saves the current state of the scene to a temporary
		hold buffer (same as Edit / Hold). This state may later be restored
		using <b>FileFetch()</b> or (Edit / Fetch from MAX). */
		virtual void FileHold()=0;
		/*! \remarks This restores the current state of the scene from the
		temporary hold buffer created using <b>FileHold()</b> (or from the MAX
		menu command Edit / Hold). */
		virtual void FileFetch()=0;
		/*! \remarks This method brings up the File Open dialog box and allows
		the user to select a material library to load.
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The parent window handle. */
		virtual void FileOpenMatLib(HWND hWnd)=0;  // Window handle is parent window
		/*! \remarks If the current material library has been saved previously
		(has been named) this method saves the material library to the same
		file. Otherwise it brings up the standard Save File As dialog box to
		allow the user to save the current material library.
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The parent window handle. */
		virtual void FileSaveMatLib(HWND hWnd)=0;
		/*! \remarks Brings up the standard Save File As dialog box to allow
		the user to save the current material library.
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The parent window handle. */
		virtual void FileSaveAsMatLib(HWND hWnd)=0;

		// This loads 3dsmax.mat (if it exists
		/*! \remarks This method loads the default material library
		<b>3DSMAX.MAT</b> (if this file exists). */
		virtual void LoadDefaultMatLib()=0;

		// These do not bring up file requesters
		/*! \remarks Loads the specified MAX file. A developer should normally
		specify a complete path name. This method does not bring up a file
		dialog.
		\par Parameters:
		<b>const MCHAR *name</b>\n\n
		The MAX file to load.\n\n
		<b>BOOL refresh=TRUE</b>\n\n
		Set this to FALSE to prevent the viewports from automatically being
		refreshed.
		\return  Nonzero if the file was loaded; otherwise 0. */
		virtual int LoadFromFile(const MCHAR *name, BOOL refresh=TRUE)=0;
		// LAM - 8/8/03 - ECO 1125 - added clearNeedSaveFlag, useNewFile
		// if clearNeedSaveFlag is false, the scene 'dirty' flag is not cleared
		// if useNewFile is false, the file is not added to the MRU list and the current scene file is not set to the saved file
		/*! \remarks Saves the current scene to the specified MAX file. This method
		does not bring up a file dialog.
		\par Parameters:
		<b>const MCHAR *name</b>\n\n
		The MAX file to save.\n\n
		<b>BOOL clearNeedSaveFlag = TRUE</b>\n\n
		if clearNeedSaveFlag is false, the scene 'dirty' flag is not cleared.\n\n
		<b>BOOL useNewFile = TRUE</b>\n\n
		if it's true, the saved file will become the current scene and the file is added to the MRU list,
		\return  Nonzero if the library was saved; otherwise 0. */
		virtual int SaveToFile(const MCHAR *fname, BOOL clearNeedSaveFlag = TRUE, BOOL useNewFile = TRUE)=0; 
		/*! \remarks Saves the selected nodes to the specified file.
		\par Parameters:
		<b>MCHAR *fname</b>\n\n
		The MAX file to save. */
		virtual void FileSaveSelected(const MCHAR *fname)=0;

		/*! \remarks Saves the specified nodes to the specified file.
		\par Parameters:
		<b>INodeTab* nodes</b>\n\n
		Points to the table of nodes to save. See
		Class INodeTab.\n\n
		<b>MCHAR *fname</b>\n\n
		The MAX file to save. */
		virtual void FileSaveNodes(INodeTab* nodes, const MCHAR *fname)=0;

		/*! \remarks Loads the specified material library. This method does
		not bring up a file dialog.
		\par Parameters:
		<b>const MCHAR *name</b>\n\n
		The material library to load.\n\n
		<b>MtlBaseLib *lib=NULL</b>\n\n
		Points to the material library to load into. If NULL the library is
		loaded into the current material library. See
		Class MtlBaseLib.\n\n
		Note: You need to call <b>MtlBaseLib::DeleteAll()</b> on the library
		during a MAX reset operation. This will remove all its references to
		the materials and set its count to zero.
		\return  Nonzero if the library was loaded; otherwise 0. */
		virtual int LoadMaterialLib(const MCHAR *name, MtlBaseLib *lib=NULL)=0;
		/*! \remarks Saves the specified material library to the specified
		file. This method does not bring up a file dialog.
		\par Parameters:
		<b>const MCHAR *name</b>\n\n
		The material library to save.\n\n
		Points to the material library to save from. If NULL the library is
		saved from the current material library. See
		Class MtlBaseLib.
		\return  Nonzero if the library was saved; otherwise 0. */
		virtual int SaveMaterialLib(const MCHAR *name, MtlBaseLib *lib=NULL)=0;
		// dupMtlAction and reparentAction added for r6.0 - 8/5/03
		/*! \remarks Merges the specified MAX file into the current scene. In MAX 2.0
		and later additional parameters allow automatic viewport updates to be
		optionally be turned off, cases of duplicate objects being merged may be
		handled, a table of names of the merged objects may be generated, and only
		specific named objects from the file may be merged.
		\par Parameters:
		<b>const MCHAR *name</b>\n\n
		The MAX file to merge.\n\n
		<b>BOOL mergeAll=FALSE</b>\n\n
		If TRUE all the items in the file are merged; otherwise the selector dialog
		appears allowing the user to choose.\n\n
		<b>selMerged=FALSE</b>\n\n
		If TRUE the nodes are selected when they are merged.\n\n
		<b>BOOL refresh=TRUE</b>\n\n
		Set this to FALSE to prevent the viewports from automatically being
		refreshed.\n\n
		<b>int dupAction = MERGE_DUPS_PROMPT</b>\n\n
		Determines what to do when duplicate named objects are encountered during the
		merge.
		See \ref DupNodeNameActions \n
		<b>NameTab* mrgList=NULL</b>\n\n
		When you specify a pointer to a <b>NameTab</b> for this parameter, and don't
		set <b>dupAction</b> to <b>MERGE_LIST_NAMES</b>, then this method will merge
		the nodes whose names are listed in the <b>mrgList</b>.\n\n
		If <b>dupAction==MERGE_LIST_NAMES</b>, (and <b>mergeAll==TRUE</b>, and
		<b>mrgList != NULL</b>) then this method puts a list of the nodes in the file
		into <b>mrgList</b>, and simply returns (no merging is done).\n\n
		See Class NameTab.\n\n
		<b>BOOL dupMtlAction=MERGE_DUP_MTL_PROMPT</b>\n\n
		Determines what to do when duplicate material names are encountered during the merge.
		Must be one of the \ref DupMtlNameActions \n
		<b>BOOL reparentAction=MERGE_REPARENT_PROMPT</b>\n\n
		Determines how to re-parent nodes during a merge operation.
		Must be one of the \ref NodeReparentActions
		<b>bool includeFullGroup=FALSE</b>\n\n
		If TRUE, expand list of nodes to merge to include all group nodes associated with those nodes
		\return  Nonzero if the file was merged; otherwise 0. */
		virtual int MergeFromFile(const MCHAR *name, 
				BOOL mergeAll=FALSE,    // when true, merge dialog is not put up
				BOOL selMerged=FALSE,   // select merged items?
				BOOL refresh=TRUE,      // refresh viewports ?
				int dupAction = MERGE_DUPS_PROMPT,  // what to do when duplicate node names are encountered
				NameTab* mrgList=NULL,  // names to be merged (mergeAll must be TRUE)
				int dupMtlAction = MERGE_DUP_MTL_PROMPT,  // what to do when duplicate material names are encountered
				int reparentAction = MERGE_REPARENT_PROMPT,   // what to do when can re-parent
				BOOL includeFullGroup = FALSE    // expand list of nodes to merge to include all associated group nodes
				)=0;
//! @}

//! \name File Import/Export Related Methods
//! @{
		/*! \remarks This method simply brings up the 'Select File to Import' dialog just as
		if the user picked this option from the File / Import... pulldown menu.
		\return  TRUE if the file was imported successfully; otherwise FALSE.
		*/
		virtual BOOL FileImport()=0;
		/*! \remarks This method simply brings up the 'Select File to Export' dialog just as
		if the user picked this option from the File / Export... pulldown menu.
		\return  TRUE if the export file was written successfully; otherwise
		FALSE. */
		virtual BOOL FileExport()=0;
		
		/*! \remarks This method allows the import of the specified file by any of the
		supported import formats. This is done by specifying a full filename
		thus bypassing the file browser. If <b>suppressPrompts</b> is set to
		TRUE, the import mechanism will not display any prompts requiring user
		action.\n\n
		Note: Developers of Import plug-ins need to support the
		<b>suppressPrompts</b> mechanism for this to work properly. Developers
		of such plug-ins should see the Class SceneImport for details.
		\par Parameters:
		<b>const MCHAR *name</b>\n\n
		The full file name (including extension -- which identifies the
		importer used) of the input file.\n\n
		<b>BOOL suppressPrompts=FALSE</b>\n\n
		If TRUE the default choices in the import plug-in are used and no
		options dialogs are presented. If FALSE any options dialogs provided by
		the importer are presented to the user.\n\n
		<b>Class_ID *importerID=NULL</b>\n\n
		The parameter specifies the Class_ID of the import module to use. This
		is for those cases where more than one import module uses the same file
		extension. Omitting this class ID operates in the pre-R3 mode, i.e. the
		first importer found is used.
		\return  TRUE if the export file was written successfully; otherwise
		FALSE. */
		virtual BOOL ImportFromFile(const MCHAR *name, BOOL suppressPrompts=FALSE, Class_ID *importerID=NULL)=0;
		/*! \remarks This method allows the export of the current MAX file to any of the
		supported export formats. This is done by specifying a full filename
		thus bypassing the file browser. If <b>suppressPrompts</b> is set to
		TRUE, the export mechanism will not display any prompts requiring user
		action.\n\n
		Note: Developers of Export plug-ins need to support the
		<b>suppressPrompts</b> mechanism for this to work properly. Developers
		of such plug-ins should see the Class SceneExport for details.
		\par Parameters:
		<b>const MCHAR *name</b>\n\n
		The full file name (including extension -- which identifies the
		exporter used) of the output file.\n\n
		<b>BOOL suppressPrompts=FALSE</b>\n\n
		If TRUE the default choices in the export plug-in are used and no
		options dialogs are presented. If FALSE any options dialogs provided by
		the exporter are presented to the user.\n\n
		<b>DWORD options</b>\n\n
		There is currently one option; <b>SCENE_EXPORT_SELECTED</b> which
		allows you to export only selected nodes.\n\n
		<b>Class_ID *exporterID=NULL</b>\n\n
		The parameter specifies the Class_ID of the export module to use. This
		is for those cases where more than one export module uses the same file
		extension. Omitting this class ID operates in the pre-R3 mode, i.e. the
		first exporter found is used.
		\return  TRUE if the export file was written successfully; otherwise
		FALSE. */
		virtual BOOL ExportToFile(const MCHAR *name, BOOL suppressPrompts=FALSE, DWORD options=0, Class_ID *exporterID=NULL)=0;
//! @}

//! \name Methods to Display MAX Dialogs
//! @{
		// Brings up the object color picker. Returns TRUE if the user
		// picks a color and FALSE if the user cancels the dialog.
		// If the user picks a color then 'col' will be set to the color.
		/*! \remarks This method brings up the standard MAX object color
		picker dialog.\n
		\n \image html "dlgnode.gif"
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		The parent window handle.\n\n
		<b>DWORD \&col</b>\n\n
		If the user picks a color then this will be set to the chosen color.
		This is stored in a 32 bit format, with the high order 8 bits as 0's,
		the next 8 bits as the Blue amount, the next 8 bits as the Green
		amount, and the low order 8 bits as the Red amount (<b>0x00BBGGRR</b>).
		See <a href="https://msdn.microsoft.com/en-us/library/dd183449(v=vs.85).aspx">COLORREF - DWORD Color Format</a>.
		\return  TRUE if the user picks a color and FALSE if the user cancels
		the dialog. */
		virtual BOOL NodeColorPicker(HWND hWnd,DWORD &col)=0;
//! @}

//! \name Node Grouping
//! @{		
		// The following grouping functions will operate on the table
		// of nodes passed in or the current selection set if the table is NULL
		
		// If name is NULL a dialog box will prompt the user to select a name. 
		// If sel group is TRUE, the group node will be selected after the operation completes.
		// returns a pointer to the group node created.
		/*! \remarks Combines the specified nodes into a MAX group.
		\par Parameters:
		<b>INodeTab *nodes=NULL</b>\n\n
		The table of nodes to combine into a MAX group. If NULL is passed, the
		current selection set is used.\n\n
		<b>MSTR *name=NULL</b>\n\n
		The name of the group to create. If the name is NULL, a dialog box will
		prompt the user to select a name.\n\n
		<b>BOOL selGroup=TRUE</b>\n\n
		If TRUE, the group of nodes will be selected after the operation is
		completed.
		\return  A pointer to the group of nodes created. */
		virtual INode *GroupNodes(INodeTab *nodes=NULL,MSTR *name=NULL,BOOL selGroup=TRUE)=0;
		/*! \remarks Un-groups the specified nodes.
		\par Parameters:
		<b>INodeTab *nodes=NULL</b>\n\n
		The table of nodes to un-group. If NULL is passed, the current
		selection set is used. */
		virtual void UngroupNodes(INodeTab *nodes=NULL)=0;
		/*! \remarks Explodes the grouped nodes. This completely un-groups
		nested groups.
		\par Parameters:
		<b>INodeTab *nodes=NULL</b>\n\n
		The table of nodes to explode. If NULL is passed, the current selection
		set is used. */
		virtual void ExplodeNodes(INodeTab *nodes=NULL)=0;
		/*! \remarks Opens the grouped nodes. Items in an opened group may be
		edited individually.
		\par Parameters:
		<b>INodeTab *nodes=NULL</b>\n\n
		The table of nodes comprising a group that will be opened. If NULL is
		passed, the current selection set is used.\n\n
		<b>BOOL selGroup=TRUE</b>\n\n
		If TRUE, the group of nodes will be selected after the operation is
		completed. */
		virtual void OpenGroup(INodeTab *nodes=NULL,BOOL clearSel=TRUE)=0;
		/*! \remarks Closes the specified group of nodes. Items in a closed
		group cannot be edited individually.
		\par Parameters:
		<b>INodeTab *nodes=NULL</b>\n\n
		The table of nodes to close. If NULL is passed, the current selection
		set is used.\n\n
		<b>BOOL selGroup=TRUE</b>\n\n
		If TRUE, the group of nodes will be selected after the operation is
		completed. */
		virtual void CloseGroup(INodeTab *nodes=NULL,BOOL selGroup=TRUE)=0;
//! @}

		/*! \remark Attach nodes to a group.It behaves just like the attach menu command
		in 3ds Max's main menu. You can't attach nodes which are already a group member to 
		other groups.
		\par Parameters:
		<b>INodeTab& nodes</b>\n\n
		The table of nodes that you want to attach to a group.\n\n
		<b>INode& pTargetNode</b>\n\n
		The target node you want attach the table of nodes to. The node should be a group head
		node or a group member node. The node can't be a member of <b>nodes</b>.\n\n
		\return true if successful; false otherwise.
		*/
		virtual bool AttachNodesToGroup(INodeTab& nodes, INode& pTargetNode) = 0;
		/*! \remark Detach nodes from the group to the scene root node. The nodes may belong to 
		different groups. It behaves just like the detach menu command in 3ds Max's main menu.
		\par Parameters:
		<b>INodeTab& nodes</b>\n\n
		The table of nodes that you want to detach from the group they belong to. The nodes must 
		belong to open groups.\n\n
		\return true if successful; false otherwise.
		*/
		virtual bool DetachNodesFromGroup(INodeTab& nodes) = 0;

//! \name Node Picking (interactive selection in the scene)
//! @{		
		// Flashes nodes (to be used to indicate the completion of a pick operation, for example)
		/*! \remarks This method is used to 'flash' a group of nodes. This is usually used
		as a confirmation of some operation (for example as an indication of
		the completion of a pick node operation.) The nodes are briefly erased
		and then redrawn in the viewport to flash them.
		\par Parameters:
		<b>INodeTab *nodes</b>\n\n
		Pointer to the table of nodes to 'flash'.
		\par Sample Code:
		\code
		INodeTab flash;
		INode *node;
		for (int i=0; i<ip->GetSelNodeCount(); i++)
		{
			node = ip->GetSelNode(i);
			flash.Append(1,&node,10);
		}
		ip->FlashNodes(&flash);
		\endcode  */
		virtual void FlashNodes(INodeTab *nodes)=0;
//! @}

//! \name Windows Messages
//! @{
		// If a plug-in needs to do a PeekMessage() and wants to actually remove the
		// message from the queue, it can use this method to have the message
		// translated and dispatched.
		/*! \remarks If a plug-in needs to do a <b>PeekMessage()</b> and wants
		to actually remove the message from the queue, it can use this method
		to have the message translated and dispatched.
		\par Parameters:
		<b>MSG \&msg</b>\n\n
		The message from <b>PeekMessage()</b>. */
		virtual void TranslateAndDispatchMAXMessage(MSG &msg)=0;
		
		// This will go into a PeekMessage loop until there are no more
		// messages left. If this method returns FALSE then the user
		// is attempting to quit MAX and the caller should return.
		/*! \remarks This will go into a <b>PeekMessage()</b> loop until there
		are no more messages left. This is a way a plug-in can relieve control
		to the system.\n\n
		There may be certain circumstances where a plug-in wants to give
		control back to MAX. For example a plug-in may put up a progress bar
		with a cancel button during a lengthy operation. However the cancel
		button would not receive any messages if the user was clicking on it
		because no messages are being dispatched.\n\n
		This method will relieve control and let any messages that are in the
		queue get processed. If there are no messages it will return right
		away. This provides a way for a plug-in to yield control.\n\n
		Note: A developer must be prepared to handle a lot of different
		conditions if this is done. For example the user could click on the
		delete key and delete the object that was being processed.
		<b>EndEditParams()</b> could be called on the plug-in. So in
		<b>EndEditParams()</b> there must be some logic to signal the other
		lengthy process that <b>EndEditParams()</b> was called.
		\return  If this method returns FALSE then the user is attempting to
		quit MAX and the caller should return. */
		virtual BOOL CheckMAXMessages()=0;
//! @}

//! \name Viewport Background Properties
//! @{
		// Access viewport background image settings.
		/*! \remarks This method is used to specify the background asset used for all viewports.
		\par Parameters:
		<b>AssetUser& asset</b>\n\n
		The background asset representing the background image.
		\return  TRUE if the asset was set; otherwise FALSE. */
		virtual BOOL		setBkgImageAsset(const MaxSDK::AssetManagement::AssetUser& asset)=0;
		/*! \remarks This method is used to retrieve the asset of the
		background image used.  */
		virtual const MaxSDK::AssetManagement::AssetUser&	getBkgImageAsset(void)=0;
		/*! \remarks Sets the background image aspect ratio. This may match
		the viewport, the bitmap, or the rendering output aspect ratio.
		\par Parameters:
		<b>int t</b>\n\n
		One of the following values:\n\n
		<b>VIEWPORT_BKG_ASPECT_VIEW</b>\n\n
		<b>VIEWPORT_BKG_ASPECT_BITMAP</b>\n\n
		<b>VIEWPORT_BKG_ASPECT_OUTPUT</b> */
		virtual void		setBkgImageAspect(int t)=0;
		/*! \remarks Retrieves the background image aspect ratio. This will be
		the viewport, the bitmap, or the rendering output aspect ratio.
		\return  One of the following values:\n\n
		<b>VIEWPORT_BKG_ASPECT_VIEW</b>\n\n
		<b>VIEWPORT_BKG_ASPECT_BITMAP</b>\n\n
		<b>VIEWPORT_BKG_ASPECT_OUTPUT</b> */
		virtual int			getBkgImageAspect()=0;
		/*! \remarks This method sets if the background image is animated in
		the viewports. If TRUE the image updates to reflect the current frame.
		If FALSE the image remains static regardless of time.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE to enable viewport background image animation; FALSE to disable
		it. */
		virtual void		setBkgImageAnimate(BOOL onOff)=0;
		/*! \remarks This method determines if the background image is set to
		update with the current frame in the viewports. */
		virtual int			getBkgImageAnimate(void)=0;
		/*! \remarks This method establishes the range of frames used for an
		animated background.
		\par Parameters:
		<b>int start</b>\n\n
		The start frame number.\n\n
		<b>int end</b>\n\n
		The end frame number.\n\n
		<b>int step=1</b>\n\n
		The frame increment. */
		virtual void		setBkgFrameRange(int start, int end, int step=1)=0;
		/*! \remarks This method retrieves either the start or end frame
		number.
		\par Parameters:
		<b>int which</b>\n\n
		One of the following values:\n\n
		<b>VIEWPORT_BKG_START</b>\n\n
		<b>VIEWPORT_BKG_END</b> */
		virtual int			getBkgFrameRangeVal(int which)=0;
		/*! \remarks Sets the background Out of Range Type. This may be the
		start or end ORT.
		\par Parameters:
		<b>int which</b>\n\n
		One of the following values:\n\n
		<b>0</b> : Sets the Start Processing ORT.\n\n
		<b>1</b> : Sets the End Processing ORT.\n\n
		<b>int type</b>\n\n
		One of the following values:\n\n
		<b>VIEWPORT_BKG_BLANK</b>\n\n
		<b>VIEWPORT_BKG_HOLD</b>\n\n
		<b>VIEWPORT_BKG_LOOP</b> */
		virtual void		setBkgORType(int which, int type)=0; // which=0 => before start, which=1 =>	after end
		/*! \remarks Retrieves the background Out of Range Type. This may be
		the start or end ORT.
		\par Parameters:
		<b>int which</b>\n\n
		One of the following values:\n\n
		<b>0</b> : Gets the Start Processing ORT.\n\n
		<b>1</b> : Gets the End Processing ORT.
		\return  One of the following values:\n\n
		<b>VIEWPORT_BKG_BLANK</b>\n\n
		<b>VIEWPORT_BKG_HOLD</b>\n\n
		<b>VIEWPORT_BKG_LOOP</b> */
		virtual int			getBkgORType(int which)=0;
		/*! \remarks This sets the "Start at" parameter from the Views /
		Background Image... dialog.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to start. */
		virtual void		setBkgStartTime(TimeValue t)=0;
		/*! \remarks This returns the "Start at" parameter from the Views /
		Background Image... dialog. */
		virtual TimeValue	getBkgStartTime()=0;
		/*! \remarks Sets the background "Sync Start to Frame" setting.
		\par Parameters:
		<b>int f</b>\n\n
		The frame number. */
		virtual void		setBkgSyncFrame(int f)=0;
		/*! \remarks Returns the background "Sync Start to Frame" setting. */
		virtual int			getBkgSyncFrame()=0;
		/*! \remarks This method will convert the TimeValue passed to a frame
		number based on the background image settings (ORTs, start/end times,
		sync frame, etc.).
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to convert.
		\return  The frame number corresponding to the time passed. */
		virtual int			getBkgFrameNum(TimeValue t)=0;
//! @}

//! \name Animation/Time/Playback Related Methods
//! @{		
		// Gets the state of the real-time animation playback toggle.
		/*! \remarks Returns the state of the real-time animation playback
		toggle. */
		virtual BOOL GetRealTimePlayback()=0;
		/*! \remarks Sets the state of the real-time animation playback
		toggle.
		\par Parameters:
		<b>BOOL realTime</b>\n\n
		TRUE if frames should be dropped if necessary for the animation to play
		back in real time. FALSE specifies that every frame should be played.
		*/
		virtual void SetRealTimePlayback(BOOL realTime)=0;
		/*! \remarks This method returns the flag controlling which viewports
		are updated when the animation is played.
		\return  TRUE if all the viewports are updated during play; FALSE if
		only the active viewport is updated. */
		virtual BOOL GetPlayActiveOnly()=0;
		/*! \remarks This method sets the flag controlling which viewports are
		updated when the animation is played. This may be all the viewports, or
		just the active one.
		\par Parameters:
		<b>BOOL playActive</b>\n\n
		If TRUE, only the active viewport is updated as the animation is
		played; otherwise all the viewports are updated. */
		virtual void SetPlayActiveOnly(BOOL playActive)=0;
		/*! \remarks Begins animation playback. The animation may be played
		for all objects, or just the selected ones.
		\par Parameters:
		<b>int selOnly=FALSE</b>\n\n
		If TRUE only the selected objects are updated as the animation is
		played; otherwise all objects are. */
		virtual void StartAnimPlayback(int selOnly=FALSE)=0;
		/*! \remarks Terminates the animation playback. */
		virtual void EndAnimPlayback()=0;
		/*! \remarks Returns TRUE if the animation is currently playing;
		otherwise FALSE. */
		virtual BOOL IsAnimPlaying()=0;
		virtual int GetPlaybackSpeed()=0;
		virtual void SetPlaybackSpeed(int s)=0;
		/*! \remarks This method returns the state of the "loop" checkbox in the time
		configuration panel. Note that the loop control is only active when
		"real time" is selected.
		\return  TRUE if loop is on; FALSE if off. */
		virtual BOOL GetPlaybackLoop()=0;
		/*! \remarks This method allows you to set the state of the "loop" checkbox in the
		time configuration panel. Note that the loop control is only active
		when "real time" is selected.
		\par Parameters:
		<b>BOOL loop</b>\n\n
		TRUE to set the loop to on; FALSE to set it off. */
		virtual void SetPlaybackLoop(BOOL loop)=0;
//! @}

		// provide access to a global validity token, used essentially as a timestamp.
		// several areas of code may use it, one being interactive reshading.
		virtual void IncrementValidityToken()=0;
		virtual unsigned int CurrentValidityToken()=0;

//! \name Renderer Access
//! @{
		// The following APIs provide a simplistic method to call
		// the renderer and render frames. The renderer just uses the
		// current user specified parameters.
		// Note that the renderer uses the width, height, and aspect
		// of the specified bitmap so the caller can control the size
		// of the rendered image rendered.

		// Renderer must be opened before frames can be rendered.
		// Either camNode or view must be non-NULL but not both.
		// 
		// Returns the result of the open call on the current renderer.
		// 0 is fail and 1 is succeed.
		/*! \remarks This method is called to open the current renderer. It
		must be opened before frames can be rendered. Either <b>camNode</b> or
		<b>view</b> must be non-NULL, but not both. Remember to close the
		renderer when you are done (using <b>CloseCurRenderer()</b>).
		\par Parameters:
		<b>INode *camNode</b>\n\n
		A pointer to the camera node to render, or NULL if a viewport should be
		rendered.\n\n
		<b>ViewExp *view</b>\n\n
		A pointer to the view to render, or NULL if the camera should be
		rendered.\n\n
		<b>RendType t = \ref RENDTYPE_NORMAL</b>\n\n
		This provides an optional way to specify the view when opening the renderer. 
		This specifies the type of render. \n\n
		<b>int w=0</b>\n\n
		This specifies the width of the rendering.\n\n
		<b>int h=0</b>\n\n
		This specifies the height of the rendering.
		\return  Nonzero indicates success; failure is zero. 
		\sa RendType*/
		virtual int OpenCurRenderer(INode *camNode,ViewExp *view, RendType t = RENDTYPE_NORMAL, int w=0, int h=0)=0;

		// optional way to specify the view when Opening the renderer.
		/*! \remarks This method is called to open the current renderer. It provides an
		optional way to specify the view when opening.
		\par Parameters:
		<b>ViewParams *vpar</b>\n\n
		This class describes the properties of a view that is being rendered.
		See Class ViewParams.\n\n
		<b>RendType t = \ref RENDTYPE_NORMAL</b>\n\n
		This provides an optional way to specify the view when opening the
		renderer. This specifies the type of render. \n\n
		<b>int w=0</b>\n\n
		This specifies the width of the rendering.\n\n
		<b>int h=0</b>\n\n
		This specifies the height of the rendering.
		\return  Nonzero indicates success; failure is 0. 
		\sa RendType*/
		virtual int OpenCurRenderer(ViewParams *vpar, RendType t = RENDTYPE_NORMAL, int w=0, int h=0)=0;

		// The renderer must be closed when you are done with it.
		/*! \remarks This method is called to close the renderer. The renderer
		must be closed when you are finished with it. */
		virtual void CloseCurRenderer()=0;

		// Renders a frame to the given bitmap.
		// The RendProgressCallback is an optional callback (the base class is
		// defined in render.h).
		//
		// Returns the result of the render call on the current renderer.
		// 0 is fail and 1 is succeed.
		/*! \remarks This method is called to render a frame to the given
		bitmap. The renderer uses the width, height, and aspect ratio of the
		specified bitmap to control the size of the rendered image.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to render the image.\n\n
		<b>Bitmap *bm</b>\n\n
		The bitmap to render to. This bitmap defines the size and aspect ratio
		of the render. See Class Bitmap.\n\n
		<b>RendProgressCallback *prog=NULL</b>\n\n
		The RendProgressCallback is an optional callback. See
		Class RendProgressCallback.\n\n
		<b>float frameDur = 1.0f</b>\n\n
		This parameter should always be set to 1.0.\n\n
		<b>ViewParams *vp=NULL</b>\n\n
		This parameter allows you to specify a different view transformation on
		each render call. For instance, you can render a given scene at a given
		time from many different viewpoints, without calling
		<b>Render::Open()</b> for each one.\n\n
		<b>RECT *regionRect = NULL</b>\n\n
		This value, if passed, defines the region to be rendered. This only
		works for <b>RENDTYPE_REGION</b> and <b>RENDTYPE_REGIONCROP</b>.
		\return  The result of the render - Nonzero if success; otherwise 0. */
		virtual int CurRendererRenderFrame(TimeValue t, Bitmap *bm, RendProgressCallback *prog=NULL, float frameDur = 1.0f, ViewParams *vp=NULL, RECT *regionRect = NULL)=0;

		// creates a default scanline renderer - must be deleted with IScanRenderer::DeleteThis()
		/*! \remarks This method creates a default scanline renderer. This renderer must be
		deleted by calling <b>IScanRenderer::DeleteThis()</b>.
		\return  A pointer to a new IScanRenderer object. */
		virtual IScanRenderer *CreateDefaultScanlineRenderer()=0;
//! @}

		// [dl | 15may2003] Creates an instance of the default renderer for the given render setting. 
		/*! \brief Creates an instance of the default renderer for the given render setting. 
			\remarks If the class ID of the default renderer (see SetDefaultRendererClassID()) does not 
			exist, then an instance of the MAX scanline renderer is created.
			The instance returned must be deleted with Renderer::DeleteThis().
			\param renderSettingID The render type to create a default renderer of
			\return The new renderer */
		virtual Renderer* CreateDefaultRenderer(RenderSettingID renderSettingID) = 0;

		/*! \brief Gets the class ID of the default renderer for the given render setting. 
		\remarks An instance of the default renderer can be created by using CreateDefaultRenderer() 
			\param renderSettingID The render setting to get the default renderer of.
			\return The Class ID of the current default Renderer for renderSettingID */
		virtual Class_ID GetDefaultRendererClassID(RenderSettingID renderSettingID) = 0;

		/*! \brief Gets the class ID of the default renderer for the given render setting. 
			\remarks An instance of the default renderer can be created by using CreateDefaultRenderer() 
			\param renderSettingID The render setting to get the default renderer of.
			\param classID The Class ID of a Renderer to set as the new default for renderSettingID */
		virtual void SetDefaultRendererClassID(RenderSettingID renderSettingID, Class_ID classID) = 0;
	
//! \name Plug-In Renderer Access
//! @{		
		// a set of functions parallel to those above, to work with any Renderer instance
		/*! \remarks This method is called to open the specified renderer. It must be opened
		before frames can be rendered. Either <b>camNode</b> or <b>view</b>
		must be non-NULL, but not both. Remember to close the renderer when you
		are done (using <b>CloseRenderer()</b>).
		\par Parameters:
		<b>Renderer *pRenderer</b>\n\n
		Points to the renderer to open.\n\n
		<b>INode *camNode</b>\n\n
		A pointer to the camera node to render, or NULL if a viewport should be
		rendered.\n\n
		<b>ViewExp *view</b>\n\n
		A pointer to the view to render, or NULL if the camera should be rendered.\n\n
		<b>RendType type = \ref RENDTYPE_NORMAL</b>\n\n
		This provides an optional way to specify the view when opening the
		renderer. This specifies the type of render. \n\n
		<b>int w=0</b>\n\n
		This specifies the width of the rendering.\n\n
		<b>int h=0</b>\n\n
		This specifies the height of the rendering.
		\return  Nonzero indicates success; failure is zero. 
		\sa RendType*/
		virtual int OpenRenderer(Renderer *pRenderer, INode *camNode,ViewExp *view, RendType type = RENDTYPE_NORMAL, int w=0, int h=0)=0;
		/*! \remarks This method is called to open the specified renderer. It provides an
		optional way to specify the view when opening.
		\par Parameters:
		<b>Renderer *pRenderer</b>\n\n
		Points to the renderer to do the rendering.\n\n
		<b>ViewParams *vpar</b>\n\n
		This class describes the properties of a view that is being rendered.\n\n
		<b>RendType type = \ref RENDTYPE_NORMAL</b>\n\n
		This provides an optional way to specify the view when opening the
		renderer. This specifies the type of render. \n\n
		<b>int w=0</b>\n\n
		This specifies the width of the rendering.\n\n
		<b>int h=0</b>\n\n
		This specifies the height of the rendering.
		\return  Nonzero indicates success; failure is 0.
		\sa RendType, Class ViewParams*/
		virtual int OpenRenderer(Renderer *pRenderer, ViewParams *vpar, RendType type = RENDTYPE_NORMAL, int w=0, int h=0)=0;
		/*! \remarks This method is called to render a frame with the specified renderer to
		the given bitmap. The renderer uses the width, height, and aspect ratio
		of the specified bitmap to control the size of the rendered image.
		\par Parameters:
		<b>Renderer *pRenderer</b>\n\n
		Points to the renderer which will do the rendering.\n\n
		<b>TimeValue t</b>\n\n
		The time to render the image.\n\n
		<b>Bitmap *bm</b>\n\n
		The bitmap to render to. This bitmap defines the size and aspect ratio
		of the render. See Class Bitmap.\n\n
		<b>RendProgressCallback *prog=NULL</b>\n\n
		The RendProgressCallback is an optional callback. See
		Class RendProgressCallback.\n\n
		<b>float frameDur = 1.0f</b>\n\n
		This parameter should always be set to 1.0.\n\n
		<b>ViewParams *vp=NULL</b>\n\n
		This parameter allows you to specify a different view transformation on
		each render call. For instance, you can render a given scene at a given
		time from many different viewpoints, without calling
		<b>Render::Open()</b> for each one.\n\n
		<b>RECT *regionRect = NULL</b>\n\n
		This value, if passed, defines the region to be rendered. This only
		works for <b>RENDTYPE_REGION</b> and <b>RENDTYPE_REGIONCROP</b>.
		\return  The result of the render - Nonzero if success; otherwise 0. */
		virtual int RendererRenderFrame(Renderer *pRenderer, TimeValue t, Bitmap *bm, RendProgressCallback *prog=NULL, float frameDur = 1.0f, ViewParams *vp=NULL, RECT *regionRect = NULL)=0;
		/*! \remarks Closes the specified renderer.
		\par Parameters:
		<b>Renderer *pRenderer</b>\n\n
		Points to the renderer to close.\n\n
		To get more control over the renderer, the renderer can be called
		directly. The following methods give access to the current renderer and
		the user's current rendering settings. Note: These methods drive the
		renderer and not video post. */
		virtual void CloseRenderer(Renderer *pRenderer)=0;

		// To get more control over the renderer, the renderer can be called directly.
		// The following methods give access to the current renderer and the the user's
		// current rendering settings.

		// gets a pointer to the renderer being used with the currently active render settings
		/*! \remarks Retrieves a pointer to the renderer currently set as the
		active renderer. This will be either the production renderer or the
		draft renderer depending upon which is active. A developer can
		determine which renderer this is by calling the <b>ClassID()</b> method
		of the renderer.
		\par Parameters:
		<b>bool createRendererIfItDoesntExist</b>\n\n
		If true, creates an instance of the renderer if one does not currently exist. The 
		core will hold a reference to this renderer instance.
		\return  A pointer to the renderer. */
		virtual Renderer *GetCurrentRenderer(bool createRendererIfItDoesntExist = true)=0;
		// gets a pointer to the renderer being used for each of the render settings
		// GetRenderer(RenderSettingID) can be used to access all settings
		/*! \remarks Retrieves a pointer to the renderer currently set as the production
		renderer. A developer can determine which renderer this is by calling
		the <b>ClassID()</b> method of the renderer.
		\par Parameters:
		<b>bool createRendererIfItDoesntExist</b>\n\n
		If true, creates an instance of the renderer if one does not currently exist. The 
		core will hold a reference to this renderer instance.
		\return  A pointer to the renderer. */
		virtual Renderer *GetProductionRenderer(bool createRendererIfItDoesntExist = true)=0;
		/*! \remarks Retrieves a pointer to the renderer currently set as the draft
		renderer. A developer can determine which renderer this is by calling
		the <b>ClassID()</b> method of the renderer.
		\par Parameters:
		<b>bool createRendererIfItDoesntExist</b>\n\n
		If true, creates an instance of the renderer if one does not currently exist. The 
		core will hold a reference to this renderer instance.
		\return  A pointer to the renderer. */
		virtual Renderer *GetDraftRenderer(bool createRendererIfItDoesntExist = true)=0;
		// this function can be used instead of the two above.
		// RS_MEdit: Returns the renderer in the MEdit slot. Does not consider the lock.
		/*! \remarks Returns a pointer to the specified renderer.
		\par Parameters:
		<b>RenderSettingID renderSettingID</b>\n\n
		One of these values: See \ref renderSettingIDs.
		<b>bool createRendererIfItDoesntExist</b>\n\n
		If true, creates an instance of the renderer if one does not currently exist. The 
		core will hold a reference to this renderer instance.
		\return  A pointer to the renderer. */
		virtual Renderer *GetRenderer(RenderSettingID renderSettingID, bool createRendererIfItDoesntExist = true)=0;

		// assigns a renderer to be used with the currently active render settings
		/*! \remarks Assigns the renderer passed for use as either the draft renderer or the
		production renderer depending upon which is active.
		\par Parameters:
		<b>Renderer *rend</b>\n\n
		The renderer to assign. */
		virtual void AssignCurRenderer(Renderer *rend)=0;
		// assigns a renderer to be used with each of the render settings
		// AssignRenderer(RenderSettingID, Renderer*) can be used to access all settings
		/*! \remarks Assigns the renderer passed as the production renderer.
		\par Parameters:
		<b>Renderer *rend</b>\n\n
		The renderer to assign. */
		virtual void AssignProductionRenderer(Renderer *rend)=0;
		/*! \remarks Assigns the renderer passed as the draft renderer.
		\par Parameters:
		<b>Renderer *rend</b>\n\n
		The renderer to assign. */
		virtual void AssignDraftRenderer(Renderer *rend)=0;
		// this function can be used instead of the two above.
		// RS_MEdit: Assign the renderer to the MEdit slot. Does not consider the lock.
		/*! \remarks Assigns the renderer passed as one of the standard MAX rendering
		options (Production, Draft, etc).
		\par Parameters:
		<b>RenderSettingID renderSettingID</b>\n\n
		One of these values: See \ref renderSettingIDs 
		<b>Renderer *rend</b>\n\n
		Points to the renderer to assign. */
		virtual void AssignRenderer(RenderSettingID renderSettingID, Renderer *rend)=0;

		// in order to support more than just two render settings, the following two functions should no
		// longer be used. instead, use ChangeRenderSetting() below.

		// SetUseDraftRenderer() sets the CurrentRenderSetting to 'production'
		// if parameter (b == 0) and sets it to 'draft' if (b != 0)
		/*! \remarks Specifies which renderer is active -- draft or production. Pass TRUE to
		use the draft renderer and FALSE to get the production renderer. */
		virtual void SetUseDraftRenderer(BOOL b)=0;

		// GetUseDraftRenderer() returns TRUE if the CurrentRenderSetting is 'draft'
		// it returns TRUE if the CurrentRenderSetting is not 'draft'
		// there is no way with this function to determine if the CurrentRenderSetting is anything but 'draft'
		/*! \remarks Determines which renderer is active -- draft or production.
		\return  TRUE for the draft renderer and FALSE for the production
		renderer. */
		virtual BOOL GetUseDraftRenderer()=0;

		// these functions can be used instead of the two above.
		// RS_MEdit cannot be the current render setting. Calling ChangeRenderSetting(RS_MEdit) has no effect.
		/*! \remarks Sets the specified renderer as active.
		\par Parameters:
		<b>RenderSettingID renderSettingID</b>\n\n
		One of these values: See \ref renderSettingIDs. */
		virtual void ChangeRenderSetting(RenderSettingID renderSettingID)=0;
		/*! \remarks Returns a value which indicates which renderer is current. 
		See \ref renderSettingIDs. */
		virtual RenderSettingID GetCurrentRenderSetting()=0;
//! @}

		//! \brief Gets the renderer assigned to the MEdit slot.\n\n
		/*!	This ignores the state of the MEdit renderer lock, and will not necessarily
			get the renderer that is actually used for MEdit.
			\sa GetActualMEditRenderer()
			\par Parameters:
			<b>bool createRendererIfItDoesntExist</b>\n\n
			If true, creates an instance of the renderer if one does not currently exist. The 
			core will hold a reference to this renderer instance.
			\return the Renderer used to create the display for the Material Editor slots */
		virtual Renderer* GetMEditRenderer(bool createRendererIfItDoesntExist = true) = 0;
		
		//! \brief Sets the renderer assigned to the MEdit slot.\n\n
		/*!	This ignores the state of the MEdit renderer lock, and will not necessarily
			set the renderer that is actually used for the Material Editor.
			\param renderer A renderer that will be used to generate the display for the Material Editor slots */
		virtual void AssignMEditRenderer(Renderer* renderer) = 0;

		//! \brief Returns the renderer to be used for MEdit.\n\n
		/*!	This takes the lock into account
			\sa GetMEditRenderer()
			\par Parameters:
			<b>bool createRendererIfItDoesntExist</b>\n\n
			If true, creates an instance of the renderer if one does not currently exist. The 
			core will hold a reference to this renderer instance.
			\return this will return the Current renderer if the MEdit lock is ON. 
				Otherwise, the renderer assigned with AssignMEditRenderer will be returned */
		virtual Renderer* GetActualMEditRenderer(bool createRendererIfItDoesntExist = true) = 0;

		/*! \brief Gets the status of the MEdit renderer lock.
			\return true if the Current renderer should be used to render in MEdit, else false to use the
				assigned renderer */
		virtual bool GetMEditRendererLocked() = 0;
		/*! \brief Sets the status of the MEdit renderer lock.
			\param locked if true, the Current renderer should be used to render in MEdit.  If false, the
				assigned renderer should be used. */
		virtual void SetMEditRendererLocked(bool locked) = 0;
		virtual bool GetMEditRendererLocked_DefaultValue() = 0;
		virtual void SetMEditRendererLocked_DefaultValue(bool locked) = 0;

//! \name Plug-In Renderer Access
//! @{		
		// gets the current (production vs. draft) render element manager
		/*! \remarks This method returns a pointer to the current render elements manager
		interface. */
		virtual IRenderElementMgr *GetCurRenderElementMgr()=0;
		// gets production or draft render element manager -- passing in renderSettingID = RS_IReshade will return NULL
		/*! \remarks This method returns a pointer to the production or draft render element
		manager -- passing in renderSettingID = RS_IReshade will return NULL.
		\par Parameters:
		<b>RenderSettingID renderSettingID</b>\n\n
		One of these values: See \ref renderSettingIDs. */
		virtual IRenderElementMgr *GetRenderElementMgr(RenderSettingID renderSettingID)=0;

		// Fills in a RendParams structure that can be passed to the
		// renderer with the user's current rendering settings.
		// A vpt pointer only needs to be passed in if the RendType
		// is RENDTYPE_REGION or RENDTYPE_BLOWUP. In these cases it will
		// set up the RendParams regxmin,regxmax,regymin,regymax from
		// values stored in the viewport.
		/*! \remarks This method is called to fill in a <b>RendParams</b>
		structure that can be passed to the renderer with the user's current
		rendering settings. This is whatever was last used, or the default
		settings.\n\n
		In MAX 1.x note the following (this needs not be done in MAX 2.0 or
		later): In order to open a renderer using this method to setup the
		<b>RendParams</b> class the following code should be used:\n\n
		<b>RendParams rp;</b>\n\n
		<b>SetupRendParams(rp, ...);</b>\n\n
		<b>rp.atmos = NULL;</b>\n\n
		<b>rp.envMap = NULL;</b>\n\n
		As shown above, this method does not automatically set the values for
		<b>envMap</b> and <b>atmos</b>. You must do this manually if you are
		using the <b>RendParams</b> object to initialize the renderer. Then you
		can call <b>Renderer::Open(..., rp, ...)</b>. In MAX 2.0 and later,
		<b>atmos</b> and <b>envMap</b> are properly initialized without the
		above code.
		\par Parameters:
		<b>RendParams \&rp</b>\n\n
		This is the class instance whose data is filled in. See
		Class RendParams.\n\n
		<b>ViewExp *vpt</b>\n\n
		This pointer only needs to be passed in if the RendType is
		<b>RENDTYPE_REGION</b> or <b>RENDTYPE_BLOWUP</b>. In these cases it
		will set up the <b>RendParams regxmin, regxmax, regymin, regymax</b>
		from values stored in the viewport. \n\n
		<b>RendType t = RendType::RENDTYPE_NORMAL</b>\n\n
		\sa Class ViewExp, RendType
		*/
		virtual void SetupRendParams(RendParams &rp, ViewExp *vpt, RendType t = RENDTYPE_NORMAL)=0;

		// fills in a ViewParams structure based on type of node passed (camera or light)
		// can be used when instantiating a renderer and calling Open(), Render(), and Close()
		// directly on it.
		/*! \remarks This method fills in the specified ViewParams structure based on type
		of node passed (camera or light). It can be used when instantiating a
		renderer and calling Open(), Render(), and Close() directly on it.
		\par Parameters:
		<b>INode* vnode</b>\n\n
		Points to the node to initialize from.\n\n
		<b>ViewParams\& vp</b>\n\n
		The ViewParams structure to initialize.\n\n
		<b>TimeValue t</b>\n\n
		The time at which to initialize the structure. */
		virtual void GetViewParamsFromNode(INode* vnode, ViewParams& vp, TimeValue t)=0;

		//! \brief This method may be called during a rendering to check if user has cancelled the render.
		/*! \return  TRUE if user has cancelled; otherwise FALSE. If not rendering
		the method returns FALSE. */
		virtual BOOL CheckForRenderAbort()=0;

		/*! \brief Calling this method during a render will set the flag 
		that indicates that the render is to be aborted */
		virtual void AbortRender()=0;

		// These give access to individual user specified render parameters
		// These are either parameters that the user specifies in the
		// render dialog or the renderer page of the preferences dialog.
		/*! \remarks Retrieves the type of time range to be rendered.
		\return  One of the following values:\n\n
		<b>REND_TIMESINGLE</b>\n\n
		A single frame.\n\n
		<b>REND_TIMESEGMENT</b>\n\n
		The active time segment.\n\n
		<b>REND_TIMERANGE</b>\n\n
		The user specified range.\n\n
		<b>REND_TIMEPICKUP</b>\n\n
		The user specified frame pickup string (for example "1,3,5-12"). */
		virtual int GetRendTimeType()=0;
		/*! \remarks Sets the type of time range rendered.
		\par Parameters:
		<b>int type</b>\n\n
		One of the following values:\n\n
		<b>REND_TIMESINGLE</b>\n\n
		A single frame.\n\n
		<b>REND_TIMESEGMENT</b>\n\n
		The active time segment.\n\n
		<b>REND_TIMERANGE</b>\n\n
		The user specified range.\n\n
		<b>REND_TIMEPICKUP</b>\n\n
		The user specified frame pickup string (for example "1,3,5-12"). */
		virtual void SetRendTimeType(int type)=0;
		/*! \remarks Retrieves the renderer's start time setting. */
		virtual TimeValue GetRendStart()=0;
		/*! \remarks Sets the renderer's start time setting.
		\par Parameters:
		<b>TimeValue start</b>\n\n
		The time to begin rendering. */
		virtual void SetRendStart(TimeValue start)=0;
		/*! \remarks Retrieves the renderer's end time setting. */
		virtual TimeValue GetRendEnd()=0;
		/*! \remarks Sets the renderer's end time setting.
		\par Parameters:
		<b>TimeValue end</b>\n\n
		The time to end rendering. */
		virtual void SetRendEnd(TimeValue end)=0;
		/*! \remarks Returns the renderer's 'n-th' frame setting. */
		virtual int GetRendNThFrame()=0;
		/*! \remarks Sets the renderer's 'n-th' frame setting.
		\par Parameters:
		<b>int n</b>\n\n
		The n-th frame setting. */
		virtual void SetRendNThFrame(int n)=0;

		/*! \remarks Retrieves the state of the renderer's show virtual frame
		buffer flag. Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendShowVFB()=0;
		/*! \remarks Sets the state of the renderer's show virtual frame
		buffer flag.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE is on; FALSE is off. */
		virtual void SetRendShowVFB(BOOL onOff)=0;

		/*! \remarks Retrieves the state of the renderer's save file flag.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendSaveFile()=0;
		/*! \remarks Sets the state of the renderer's save file flag.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE is on; FALSE is off. */
		virtual void SetRendSaveFile(BOOL onOff)=0;
		/*! \remarks Retrieves the state of the renderer's use device flag.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendUseDevice()=0;
		/*! \remarks Sets the state of the renderer's use device flag.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE is on; FALSE is off. */
		virtual void SetRendUseDevice(BOOL onOff)=0;
		/*! \remarks Retrieves the state of the renderer's use net flag.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendUseNet()=0;

		/*! \remarks Sets the state of the renderer's use net flag.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE is on; FALSE is off. */
		virtual void SetRendUseNet(BOOL onOff)=0;

		/*! \remarks Retrieves the rendering file BitmapInfo. This class
		describes the output file. See Class BitmapInfo. */
		virtual BitmapInfo& GetRendFileBI()=0;
		/*! \remarks Retrieves the rendering device BitmapInfo. This class
		describes the output device. See Class BitmapInfo. */
		virtual BitmapInfo& GetRendDeviceBI()=0;
		/*! \remarks Retrieves the rendering output width in pixels. */
		virtual int GetRendWidth()=0;
		/*! \remarks Sets the rendering output width.
		\par Parameters:
		<b>int w</b>\n\n
		The width in pixels. */
		virtual void SetRendWidth(int w)=0;
		/*! \remarks Retrieves the rendering output height in pixels. */
		virtual int GetRendHeight()=0;
		/*! \remarks Sets the rendering output height.
		\par Parameters:
		<b>int h</b>\n\n
		The height in pixels. */
		virtual void SetRendHeight(int h)=0;
		/*! \remarks Retrieves the renderer's pixel aspect ratio setting. Note
		for MAX 1.2: To get the 'Image Aspect Ratio' setting use:\n\n
		<b>float aspectRatio =</b>\n\n
		<b>((float) ip-\>GetRendWidth())/((float)
		ip-\>GetRendHeight());</b>\n\n
		In MAX 2.0 or later <b>GetRendImageAspect()</b> may be used: */
		virtual float GetRendApect()=0;

		/*! \remarks Sets the renderer's pixel aspect ratio setting.
		\par Parameters:
		<b>float a</b>\n\n
		The pixel aspect ratio to set. */
		virtual void SetRendAspect(float a)=0;


		/*! \remarks 
		\return  Returns the image aspect ratio. */
		virtual float GetRendImageAspect()=0;

		/*! \remarks 
		\return  Returns the aperture width in millimeters (mm). */
		virtual float GetRendApertureWidth()=0;	 // get aperture width in mm.

		/*! \remarks Sets the aperture width.
		\par Parameters:
		<b>float aw</b>\n\n
		The width to set in millimeters (mm). */
		virtual void SetRendApertureWidth(float aw)=0; // set aperture width in mm.

		/*! \remarks Retrieves the renderer's field render flag.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendFieldRender()=0;

		/*! \remarks Sets the renderer's field render flag.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetRendFieldRender(BOOL onOff)=0;

		/*! \remarks Retrieves the renderer's color check flag.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendColorCheck()=0;

		/*! \remarks Sets the renderer's color check flag.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetRendColorCheck(BOOL onOff)=0;

		/*! \remarks Retrieves the renderer's super black flag.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendSuperBlack()=0;

		/*! \remarks Sets the renderer's super black flag.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetRendSuperBlack(BOOL onOff)=0;


		/*! \remarks Retrieves the renderer's render hidden objects flag.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendHidden()=0;
		/*! \remarks Sets the renderer's render hidden objects flag.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetRendHidden(BOOL onOff)=0;
		/*! \remarks Retrieves the renderer's force two-sided flag.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendForce2Side()=0;
		/*! \remarks Sets the renderer's force two-sided flag. TRUE for on;
		FALSE for off. */
		virtual void SetRendForce2Side(BOOL onOff)=0;

		/*! \remarks Retrieves the renderer's uses atmospheric effects flag.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendAtmosphere()=0;

		/*! \remarks Sets if the renderer uses atmospheric effects.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetRendAtmosphere(BOOL onOff)=0;

		/*! \remarks
		\return  Returns TRUE if Rendering Effects will be used; otherwise
		FALSE. */
		virtual BOOL GetRendEffects()=0;

		/*! \remarks Sets if Rendering Effects will be used.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE to use (on); FALSE to not use (off). */
		virtual void SetRendEffects(BOOL onOff)=0;

		/*! \remarks 
		\return  Returns TRUE if rendering displacements is enabled; otherwise
		FALSE.\n\n
		Note: Developers should use the <b>flags</b> parameter of the
		<b>View</b> class which is passed into <b>GetRenderMesh()</b> to
		determine if Displacement Mapping is being used because the values may
		not the same (for instance when rendering in the Materials Editor). See
		Class View. */
		virtual BOOL GetRendDisplacement()=0;

		/*! \remarks Sets if rendering displacements are enabled.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetRendDisplacement(BOOL onOff)=0;


		/*! \remarks Retrieves the string holding the frames the user wants to
		render. For example "1,3,5-12". */
		virtual MSTR& GetRendPickFramesString()=0;
		/*! \remarks Retrieves the renderer's dither true color flag.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendDitherTrue()=0;
		/*! \remarks Sets the renderer's dither true color flag.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetRendDitherTrue(BOOL onOff)=0;
		/*! \remarks Retrieves the renderer's dither 256 color flag.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendDither256()=0;
		/*! \remarks Sets the renderer's dither 256 color flag.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetRendDither256(BOOL onOff)=0;
		/*! \remarks Retrieves the renderer's multi-threaded flag.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendMultiThread()=0;
		/*! \remarks Sets the renderer's multi-threaded flag.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetRendMultiThread(BOOL onOff)=0;
		/*! \remarks This retrieves the output file sequencing nth serial
		numbering setting.
		\return  Returns TRUE if on; FALSE if off. */
		virtual BOOL GetRendNThSerial()=0;
		/*! \remarks This sets the output file sequencing nth serial numbering
		setting.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetRendNThSerial(BOOL onOff)=0;
		/*! \remarks Retrieves the video color check method.
		\return  One of the following values:\n\n
		<b>0</b> is FLAG\n\n
		<b>1</b> is SCALE_LUMA\n\n
		<b>2</b> is SCALE_SAT */
		virtual int GetRendVidCorrectMethod()=0; // 0->FLAG, 1->SCALE_LUMA 2->SCALE_SAT
		/*! \remarks Sets the video color check method.
		\par Parameters:
		<b>int m</b>\n\n
		One of the following values:\n\n
		<b>0</b> is FLAG\n\n
		<b>1</b> is SCALE_LUMA\n\n
		<b>2</b> is SCALE_SAT */
		virtual void SetRendVidCorrectMethod(int m)=0;
		/*! \remarks Retrieves the rendering field order.
		\return  One of the following values:\n\n
		<b>0</b> is Even\n\n
		<b>1</b> is Odd */
		virtual int GetRendFieldOrder()=0; // 0->even, 1-> odd
		/*! \remarks Sets the rendering field order to even or odd.
		\par Parameters:
		<b>int fo</b>\n\n
		One of the following values:\n\n
		<b>0</b> sets Even\n\n
		<b>1</b> sets Odd */
		virtual void SetRendFieldOrder(int fo)=0;
		/*! \remarks Retrieves the video color check NTSC or PAL setting.
		\return  One of the following values:\n\n
		<b>0</b> is NTSC\n\n
		<b>1</b> is PAL */
		virtual int GetRendNTSC_PAL()=0; // 0 ->NTSC,  1 ->PAL
		/*! \remarks Sets the video color check NTSC or PAL setting.
		\par Parameters:
		<b>int np</b>\n\n
		One of the following values:\n\n
		<b>0</b> sets NTSC\n\n
		<b>1</b> sets PAL */
		virtual void SetRendNTSC_PAL(int np)=0;
		/*! \remarks Returns the super black threshold setting. */
		virtual int GetRendSuperBlackThresh()=0;

		/*! \remarks Sets the super black threshold setting.
		\par Parameters:
		<b>int sb</b>\n\n
		The super black threshold. */
		virtual void SetRendSuperBlackThresh(int sb)=0;

		/*! \remarks Returns the File Number Base in the 'Common Parameters' rollup of the
		Render Scene dialog. */
		virtual int GetRendFileNumberBase()=0;
		/*! \remarks Sets the File Number Base in the 'Common Parameters' rollup of the
		Render Scene dialog.
		\par Parameters:
		<b>int n</b>\n\n
		The number to set. */
		virtual void SetRendFileNumberBase(int n)=0;
		/*! \remarks Returns TRUE if the skip existing rendered frames state is on;
		otherwise FALSE. */
		virtual BOOL GetSkipRenderedFrames()=0;

		/*! \remarks Sets the skip existing rendered frames state to on or off.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetSkipRenderedFrames(BOOL onOff)=0;
//! @}


		/*! \brief returns the state of the toggle in the render dialog, "Area Lights/Shadows as Points". 
			\return true if area lights should be rendered as point lights. */
		virtual BOOL GetRendSimplifyAreaLights() = 0;

		//! \brief Sets the toggle in the render dialog, "Area Lights/Shadows as Points". 
		/*! When set to true, the simplyAreaLights variable in class RenderGlobalContext will be set when rendering
			\param [in] onOff - If true, set the toggle to cause area lights to be rendered as point lights. */
		virtual void SetRendSimplifyAreaLights(BOOL onOff) = 0;

//! \name Hide By Category Flags
//! @{		
		/*! \remarks Returns the state of the hide by category flags.
		\return  One or more of the following values:\n\n
		<b>HIDE_OBJECTS</b>\n\n
		<b>HIDE_SHAPES</b>\n\n
		<b>HIDE_LIGHTS</b>\n\n
		<b>HIDE_CAMERAS</b>\n\n
		<b>HIDE_HELPERS</b>\n\n
		<b>HIDE_WSMS</b>\n\n
		<b>HIDE_SYSTEMS</b>\n\n
		<b>HIDE_NONRENDERABLES</b>\n\n
		<b>HIDE_ALL</b>\n\n
		<b>HIDE_NONE</b> */
		virtual DWORD GetHideByCategoryFlags()=0;
		/*! \remarks Sets the state of the hide by category flags.
		\par Parameters:
		<b>DWORD f</b>\n\n
		One or more of the following values:\n\n
		<b>HIDE_OBJECTS</b>\n\n
		<b>HIDE_SHAPES</b>\n\n
		<b>HIDE_LIGHTS</b>\n\n
		<b>HIDE_CAMERAS</b>\n\n
		<b>HIDE_HELPERS</b>\n\n
		<b>HIDE_WSMS</b>\n\n
		<b>HIDE_SYSTEMS</b>\n\n
		<b>HIDE_NONRENDERABLES</b>\n\n
		<b>HIDE_ALL</b>\n\n
		<b>HIDE_NONE</b> */
		virtual void SetHideByCategoryFlags(DWORD f)=0;
//! @}

//! \name Viewport Access
//! @{		
		/*! \remarks This method may be called to retrieve a value that
		describes the configuration of the MAX viewports.
		\return  The viewport layout configuration. The list below uses the
		following syntax:
		\# is the total number of viewports.
		- V = vertical split 
		- H = horizontal split 
		- L/R = left/right placement 
		- T/B = top/bottom placement 

		One of the following values. Note: The bottom nibble (4-bits) is the
		total number of views. You may use the constant
		 VP_NUM_VIEWS_MASK  to mask off the 4 bits that contains the total
		number of viewports. 
		- VP_LAYOUT_1  
		- VP_LAYOUT_2V  
		- VP_LAYOUT_2H  
		- VP_LAYOUT_2HT  
		- VP_LAYOUT_2HB  
		- VP_LAYOUT_3VL  
		- VP_LAYOUT_3VR  
		- VP_LAYOUT_3HT  
		- VP_LAYOUT_3HB  
		- VP_LAYOUT_4  
		- VP_LAYOUT_4VL  
		- VP_LAYOUT_4VR  
		- VP_LAYOUT_4HT  
		- VP_LAYOUT_4HB  
		- VP_LAYOUT_1C  
		*/
		virtual int GetViewportLayout()=0;
		/*! \remarks Sets the viewport configuration layout.
		\par Parameters:
		<b>int layout</b>\n\n
		The layout to use. See the return values of <b>GetViewportLayout()</b>
		above. */
		virtual void SetViewportLayout(int layout)=0;
		/*! \remarks Returns TRUE if the current viewport is full screen;
		otherwise FALSE. */
		virtual BOOL IsViewportMaxed()=0;
		/*! \remarks This method will maximize (set to a single full screen
		view) or minimize the current viewport.
		\par Parameters:
		<b>BOOL max</b>\n\n
		If TRUE the viewport is maximized; otherwise it is minimized. */
		virtual void SetViewportMax(BOOL max)=0;

		// Zoom extents the active viewport, or all
		/*! \remarks This method performs a zoom extents on the viewport(s).
		This fills the viewport(s) with the objects of the scene.
		\par Parameters:
		<b>BOOL doAll</b>\n\n
		If TRUE all the viewports are zoomed to their extents; otherwise just
		the current viewport is.\n\n
		<b>BOOL skipPersp=FALSE</b>\n\n
		If TRUE perspective viewports are not altered; otherwise these views
		are zoomed to their extents as well. */
		virtual void ViewportZoomExtents(BOOL doAll, BOOL skipPersp=FALSE)=0;

		//watje this zooms the active viewport or all to the specified bounding box which is in world space
		/*! \remarks This method allows you to zoom the current or selected viewport to a
		bounding region.
		\par Parameters:
		<b>BOOL doAll</b>\n\n
		This flag determines whether only the selected or all viewports get
		zoomed. TRUE for all, FALSE for selected only.\n\n
		<b>Box3 box</b>\n\n
		The bounding region to zoom to. */
		virtual void ZoomToBounds(BOOL doAll, Box3 box) =0;
//! @}

//! \name Node Selection Sets
//! @{
		// Gets the world space bounding box of the selection.
		/*! \remarks This method retrieves the world space bounding box of the
		current selection set.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to retrieve the bounding box.\n\n
		<b>Box3 \&box</b>\n\n
		The bounding box is returned here. */
		virtual void GetSelectionWorldBox(TimeValue t,Box3 &box)=0;
//! @}
		
//! \name Node Related Methods
//! @{		
		// Find an INode with the given name
		/*! \remarks This method may be called to retrieve the <b>INode</b>
		pointer for the node with the given name.
		\par Parameters:
		<b>const MCHAR *name</b>\n\n
		The name of the node to find. */
		virtual INode *GetINodeByName(const MCHAR *name)=0;

		// Find an INode with the given handle
		/*! \remarks Returns a pointer to the node whose handle is specified. In 3ds Max
		version 4.0 and later each node is assigned a unique handle.
		\par Parameters:
		<b>ULONG handle</b>\n\n
		The handle of the node to find. */
		virtual INode *GetINodeByHandle(ULONG handle)=0;
//! @}

//! \name Video Post Related Methods
//! @{
		// For use with gbuffer BMM_CHAN_NODE_RENDER_ID channel during video post
		/*! \remarks This method returns the node pointer from the id in the
		<b>BMM_CHAN_NODE_RENDER_ID</b> G-Buffer channel. The renderer will set
		the RenderID of all rendered nodes, and will set all non-rendered nodes
		to <b>0xffff</b>. See \ref gBufImageChannels.
		\par Parameters:
		<b>UWORD id</b>\n\n
		The id from the G Buffer channel. */
		virtual INode *GetINodeFromRenderID(UWORD id)=0;
//! @}

//! \name Execute MAX Commands
//! @{
		// Executes a MAX command. See maxcom.h for available commands
		/*! \remarks This method may be used to execute a MAX command. These
		are the same commands that may be assigned using the MAX
		<b>Customize/Preferences.../Keyboard Tab</b> key assignment system.\n\n
		For MAX version 1.1 or later, this method may also be used to set
		various aspects of the preview display. The <b>id</b> to pass for these
		options is shown below:\n\n
		<b>ExecuteMAXCommand(MAXCOM_API_PVW_GRID_OFF)</b>\n\n
		This turns off the preview grid display.\n\n
		<b>ExecuteMAXCommand(MAXCOM_API_PVW_GRID_ON)</b>\n\n
		This turns on the preview grid display.\n\n
		<b>ExecuteMAXCommand(MAXCOM_API_PVW_SMOOTH_MODE)</b>\n\n
		This sets the preview rendering mode to "smooth".\n\n
		<b>ExecuteMAXCommand(MAXCOM_API_PVW_FACET_MODE)</b>\n\n
		This sets the preview rendering mode to "facet".\n\n
		<b>ExecuteMAXCommand(MAXCOM_API_PVW_WIRE_MODE)</b>\n\n
		This sets the preview rendering mode to "wireframe".\n\n
		Note: In MAX 2.0 and later there is an alternate (better) way to do
		this. See the method:\n\n
		<b>virtual void CreatePreview(PreviewParams *pvp=NULL)=0;</b>
		\par Parameters:
		<b>int id</b>\n\n
		The command to execute. See \ref maxCommandIDs. */
		virtual void ExecuteMAXCommand(int id)=0;
//! @}

//! \name Creating Unique
//! @{
		/*! \remarks Returns a class used for efficiently creating unique names. 
		To use it, do the following:\n\n
		\code
		NameMaker *nm = GetCOREInterface()->NewNameMaker();
		for(...) {
			nm->MakeUniqueName(nodename);
		}
		delete nm;
		\endcode
		\param initFromScene if FALSE then the name maker is not seeded with the names of the
		objects in the current scene; otherwise it is

		An instance of class NameMaker initialized at a given time with root node names 
		is best used right after initialization to create several nodes. If the same NameMaker 
		instance is to be used several times, it's best to re-initialize it with the current 
		root node names before it's used again.

		Interface::MakeNameUnique is the recommended way for creating unique node names, while
		class NameMaker is best used to create unique names based on a set of root names used to 
		initialize a NameMaker instance.
		*/
		virtual NameMaker* NewNameMaker(BOOL initFromScene = TRUE)=0;
//! @}

//! \name Viewport Background Properties
//! @{		
		// Get set the viewport background color.
		/*! \remarks Set the solid viewport background color. In 3ds Max2013, viewport background
		color can be solid or gradient. This method doesn't affect the background color in gradient mode.
		\par Parameters:
		<b>const Point3 \&color</b>\n\n
		The color to set. */
		virtual void SetViewportBGColor(const Point3 &color)=0;
		/*! \remarks Return the solid background color of the active viewport. In 3ds Max2013, viewport background
		color can be solid or gradient. This method returns the solid color.
		*/
		virtual Point3 GetViewportBGColor()=0;
//! @}

//! \name Environment Access (Ambient/Atmosphere/Background Access)
//! @{		
		// Get/Set the environment texture map, ambient light and other effects
		/*! \remarks Returns the current environment map. */
		virtual Texmap *GetEnvironmentMap()=0;
		/*! \remarks Sets the current environment map to the specified map.
		See <b>/MAXSDK/SAMPLES/UTILITIES/UTILTEST.CPP</b> for sample code.
		\par Parameters:
		<b>Texmap *map</b>\n\n
		The map to set. */
		virtual void SetEnvironmentMap(Texmap *map)=0;
		/*! \remarks  Returns TRUE if the 'Use Map' checkbox is checked in the Environment /
		Background dialog; otherwise FALSE. */
		virtual BOOL GetUseEnvironmentMap()=0;
		/*! \remarks Sets the state of the 'Use Map' checkbox in the Environment /
		Background dialog.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for checked; FALSE for unchecked. */
		virtual void SetUseEnvironmentMap(BOOL onOff)=0;

		/*! \remarks Retrieves the color of the ambient light at the time passed and updates
		the validity interval passed to reflect the validity of the ambient
		light.		
		\param t - The time to retrieve the ambient light color.
		\param valid - The validity interval to update.  
		The intersection of the provided "valid" interval with the 
		interval for the provided TimeValue and returned as the updated interval value.
		\return  The color as a <b>Point3</b>. */
		virtual Point3 GetAmbient(TimeValue t, Interval &valid) = 0;
		/*! \remarks Use this override when validity interval is irrelevant */
		Point3 GetAmbient(TimeValue t) { Interval i = FOREVER; return GetAmbient(t, i); }
		/*! \remarks Sets the color of the ambient light in the scene to the color passed at
		the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to set the color.\n\n
		<b>Point3 col</b>\n\n
		The new color for the ambient light. */
		virtual void SetAmbient(TimeValue t, Point3 col)=0;
		/*! \remarks Retrieves a pointer to the controller use to animate the ambient light.
		*/
		virtual Control *GetAmbientController()=0;
		/*! \remarks Sets the controller used for handling the animation of the ambient
		light.
		\par Parameters:
		<b>Control *c</b>\n\n
		The controller to set. */
		virtual void SetAmbientController(Control *c)=0;

		/*! \remarks Returns the rendering environment global lighting tint color at the
		specified time and updates the validity interval passed to reflect the
		validity of the tint color controller.

		\param t - The time at which to return the color.
		\param valid - The validity interval to update.  
		The intersection of the provided "valid" interval with the 
		interval for the provided TimeValue and returned as the updated interval value. */
		virtual Point3 GetLightTint(TimeValue t, Interval &valid) = 0;
		/*! \remarks Use this override when validity interval is irrelevant */
		Point3 GetLightTint(TimeValue t) { Interval i = FOREVER; return GetLightTint(t, i); }
		/*! \remarks Sets the rendering environment global lighting tint color at the
		specified time to the color passed.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to set the color.\n\n
		<b>Point3 col</b>\n\n
		The color to set. */
		virtual void SetLightTint(TimeValue t, Point3 col)=0;
		/*! \remarks Returns a pointer to the controller use to animate the tint color. */
		virtual Control *GetLightTintController()=0;
		/*! \remarks Sets the controller use to animate the tint color.
		\par Parameters:
		<b>Control *c</b>\n\n
		Points to the controller to set. */
		virtual void SetLightTintController(Control *c)=0;

		/*! \remarks Returns the rendering environment global lighting level at the
		specified time and updates the validity interval passed to reflect the
		validity of the lighting level controller.
		\param t - The time at which to return the level.
		\param valid - The validity interval to update.  
		The intersection of the provided "valid" interval with the 
		interval for the provided TimeValue and returned as the updated interval value. */
		virtual float GetLightLevel(TimeValue t, Interval &valid) = 0;
		/*! \remarks Use this override when validity interval is irrelevant */
		float GetLightLevel(TimeValue t) { Interval i = FOREVER; return GetLightLevel(t, i); }

		/*! \remarks Sets the rendering environment global lighting level at the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to set the lighting level.\n\n
		<b>float lev</b>\n\n
		The level to set. */
		virtual void SetLightLevel(TimeValue t, float lev)=0;
		/*! \remarks Returns a pointer to the controller use to animate the lighting level.
		*/
		virtual Control *GetLightLevelController()=0;
		/*! \remarks Sets the controller use to animate the lighting level.
		\par Parameters:
		<b>Control *c</b>\n\n
		Points to the controller to set. */
		virtual void SetLightLevelController(Control *c)=0;

		/*! \remarks Returns the number of atmospheric effects currently assigned. */
		virtual int NumAtmospheric()=0;
		/*! \remarks Returns a pointer to the 'i-th' atmospheric effect.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies which atmospheric effect to retrieve. */
		virtual Atmospheric *GetAtmospheric(int i)=0;
		/*! \remarks Sets the 'i-th' atmospheric effect.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies which effect to set.\n\n
		<b>Atmospheric *a</b>\n\n
		A pointer to the atmospheric effect. */
		virtual void SetAtmospheric(int i,Atmospheric *a)=0;
		/*! \remarks Adds the specified atmospheric effect to the list of effects.
		\par Parameters:
		<b>Atmospheric *a</b>\n\n
		A pointer to the atmospheric effect to add. */
		virtual void AddAtmosphere(Atmospheric *atmos)=0;
		/*! \remarks Deletes the specified atmospheric effect.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the atmospheric effect to delete. */
		virtual void DeleteAtmosphere(int i)=0;
		/*! \remarks This method selects the specified atmosphere's gizmo and displays the
		parameters for it (if any).
		\par Parameters:
		<b>Atmospheric *a</b>\n\n
		Points to the Atmospheric plug-in. See
		Class Atmospheric.\n\n
		<b>INode *gizmo=NULL</b>\n\n
		Points to the gizmo node associated with the plug-in. */
		virtual void EditAtmosphere(Atmospheric *a, INode *gizmo=NULL)=0;

		/*! \remarks Retrieves the background color at the specified time and updates the
		validity interval passed to reflect the validity of the background
		color.
		\param t - The time to retrieve the color.
		\param valid - The validity interval to update.  
		The intersection of the provided "valid" interval with the 
		interval for the provided TimeValue and returned as the updated interval value. */
		virtual Point3 GetBackGround(TimeValue t, Interval &valid) = 0;
		/*! \remarks Use this override when validity interval is irrelevant */
		Point3 GetBackGround(TimeValue t) { Interval i = FOREVER; return GetBackGround(t, i); }
		/*! \remarks Sets the background color to the specified color at the specified time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to set the color.\n\n
		<b>Point3 col</b>\n\n
		The color to set. */
		virtual void SetBackGround(TimeValue t,Point3 col)=0;
		/*! \remarks Returns a pointer to the controller animating the background color. */
		virtual Control *GetBackGroundController()=0;
		/*! \remarks Sets the controller used for animating the background color.
		\par Parameters:
		<b>Control *c</b>\n\n
		Specifies which controller to set. */
		virtual void SetBackGroundController(Control *c)=0;
//! @}

//! \name Render Effects Methods
//! @{
		/*! \remarks Returns the number of Render Effects currently assigned. */
		virtual int NumEffects()=0;
		/*! \remarks Returns a pointer to the 'i-th' Render Effect. See
		Class Effect.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the effect to return. */
		virtual Effect *GetEffect(int i)=0;
		/*! \remarks Sets the specified Render Effect to the one passed.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the effect to set.\n\n
		<b>Effect *e</b>\n\n
		Points to the Renderer Effect to set. */
		virtual void SetEffect(int i,Effect *e)=0;
		/*! \remarks Adds the specified Renderer Effect to the existing list of effects.
		\par Parameters:
		<b>Effect *eff</b>\n\n
		Points to the render effect to add. See
		Class Effect. */
		virtual void AddEffect(Effect *eff)=0;
		/*! \remarks Deletes the specified Renderer Effect.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the effect to delete. */
		virtual void DeleteEffect(int i)=0;
		/*! \remarks This method selects the specified gizmo and displays the parameters for
		it (if any).
		\par Parameters:
		<b>Effect *e</b>\n\n
		Points to the Effect plug-in. See Class Effect.\n\n
		<b>INode *gizmo=NULL</b>\n\n
		Points to the gizmo node associated with the effect. */
		virtual void EditEffect(Effect *e, INode *gizmo=NULL)=0;
//! @}

//! \name Sound Object Access
//! @{		
		// Get/Set the current sound object.
		/*! \remarks Returns the current sound object. See the sample code in
		<b>/MAXSDK/SAMPLES/UTILITIES/UTILTEST.CPP</b>.\n\n
		\sa  Class SoundObj, Class IWaveSound. */
		virtual SoundObj *GetSoundObject()=0;
		/*! \remarks Sets the current sound object to the one specified. See
		Class SoundObj.
		\par Parameters:
		<b>SoundObj *snd</b>\n\n
		The sound object to set as current. */
		virtual void SetSoundObject(SoundObj *snd)=0;
//! @}

		/*! \remarks Returns an interface pointer to the object snap manager. See
		Class IOsnapManager. Also see
		the Advanced Topics section on
		~{ Snapping }~. */
		virtual IOsnapManager *GetOsnapManager()=0;
		/*! \remarks This is used internally by the Osnap Manager. Plug-Ins don't need to
		use this method. */
		virtual MouseManager *GetMouseManager()=0;
		/*! \remarks This is used internally to invalidate the osnap drawing mechanism.
		Plug-Ins don't need to use this method. */
		virtual void InvalidateOsnapdraw()=0;

//! \name Material Related Methods
//! @{
		// Access the current mat lib loaded.
		/*! \remarks This method provides access to the currently loaded
		material library.
		\return  See Class MtlBaseLib. */
		virtual MtlBaseLib& GetMaterialLibrary()=0;

		// Activate and deactivate a texture map in the viewports. 
		// mtl is the TOP LEVEL of the material containing the texture map. If it
		// is a Multi-material, then subNum specifies which sub branch of the
		// multi contains tx.
		/*! \remarks		The method deactivates the texture map in the viewports.\n\n
		Note: In the MAX 2.0 SDK a bug prevents this function from being used -- it
		results in a link error. This is fixed in the 2.5 SDK.
		\par Parameters:
		<b>MtlBase *tx</b>\n\n
		Points to the texmap to deactivate.\n\n
		<b>Mtl *mtl</b>\n\n
		The <b>top level</b> material containing the texture map.\n\n
		<b>int subNum=-1</b>\n\n
		If <b>mtl</b> above it a Multi-material, this specifies which sub-branch of the
		material contains <b>tx</b>. */
		CoreExport void DeActivateTexture(MtlBase *tx, Mtl *mtl, int subNum=-1);
		/*! \remarks		The method activates the texture map in the viewports.\n\n
		Note: In the MAX 2.0 SDK a bug prevents this function from being used -- it
		results in a link error. This is fixed in the 2.5 SDK.
		\par Parameters:
		<b>MtlBase *tx</b>\n\n
		Points to the texmap to activate.\n\n
		<b>Mtl *mtl</b>\n\n
		The <b>top level</b> material containing the texture map.\n\n
		<b>int subNum=-1</b>\n\n
		If <b>mtl</b> above is a Multi-material, this specifies which sub-branch of the
		material contains <b>tx</b>. */
		CoreExport void ActivateTexture(MtlBase *tx, Mtl *mtl, int subNum=-1);

		// Get new material and map names to maintain name uniqueness
		/*! \remarks Modifies the name of the material to make it unique. The name is of the
		form "<b>Material #1</b>" where the number is incremented as required
		to make ensure it's unique.
		\par Parameters:
		<b>Mtl *m</b>\n\n
		The material whose name is modified. */
		virtual void AssignNewName(Mtl *m)=0;
		/*! \remarks Modifies the name of the texture to make it unique. . The name is of
		the form "<b>Map #1</b>" where the number is incremented as required to
		make ensure it's unique.
		\par Parameters:
		<b>Texmap *m</b>\n\n
		The texmap whose name is modified. */
		virtual void AssignNewName(Texmap *m)=0;
//! @}

		/**
			Returns true if the application operates in network render server mode.
			In this mode the application renders a specified scene and exists without 
			showing its main user interface. This means that when this mode is active
			some plug-ins can chose not to display their user interface or not load at all.
			When in this mode, the application is also in quiet mode, see Interface::GetQuietMode().
		*/
		virtual bool IsNetworkRenderServer() const = 0; 

//! \name Error Logging
//! @{
		//-- Logging Facilities (Replaces the old NetLog() stuff)
		// 
		//   Check log.h for methods

		/*! \remarks Returns a pointer which may be used for calling methods
		to write information to the system log. See
		Class LogSys for details. */
		virtual LogSys *Log()=0;
//! @}

//! \name DLL Directory Access
//! @{
		// get ref to the central DLL directory
		/*! \remarks Returns a reference to the central DLL directory. See
		Class DllDir. */
		virtual DllDir& GetDllDir()=0; 

		// Access the DllDir
		/*! \remarks Returns a pointer to the central DLL directory. See
		Class DllDir. */
		virtual DllDir *GetDllDirectory()=0;
//! @}

//! \name General API Expansion Function
//! @{
		// Generic expansion function
		/*! \remarks This is a general purpose function that allows the API to be extended
		in the future. The MAX development team can assign new <b>cmd</b>
		numbers and continue to add functionality to this class without having
		to 'break' the API.\n\n
		Note: In R4 the return value changed from int to INT_PTR\n\n
		<b>New in R5.1</b> An additional Command was added to allow the setting
		of certain directories such as Plugins. It looks like this:\n\n
		<b>Execute(I_EXEC_SET_DIR, (ULONG_PTR)(int) which, ULONG_PTR)(MCHAR
		*)dir)</b>\n\n
		where 'which' designates the particular Max directory to be changed
		(like the corresponding argument of Interface::GetDir()), and 'dir' is
		the path as a string. 
		\par Parameters:
		<b>int cmd</b>\n\n
		The index of the command to execute. See \ref interfaceExecCommandOptions .\n\n
		<b>ULONG arg1=0</b>\n\n
		Optional argument 1. See the documentation where the <b>cmd</b> option
		is discussed for more details on these parameters.\n\n
		<b>ULONG arg2=0</b>\n\n
		Optional argument 2.\n\n
		<b>ULONG arg3=0</b>\n\n
		Optional argument 3.\n\n
		<b>ULONG arg4=0</b>\n\n
		Optional argument 4.\n\n
		<b>ULONG arg5=0</b>\n\n
		Optional argument 5.\n\n
		<b>ULONG arg6=0</b>\n\n
		Optional argument 6.
		\return  An INT_PTR return value. See the documentation where the
		<b>cmd</b> option is discussed for more details on the meaning of this
		value. */
		virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0, ULONG_PTR arg4=0, ULONG_PTR arg5=0, ULONG_PTR arg6=0)=0;
//! @}

		virtual void *GetInterface(DWORD id)=0;

		// from InterfaceServer
		virtual BaseInterface* GetInterface(Interface_ID id)=0;

//! \name Scene Access
//! @{		
		// Get pointer to the scene.
		/*! \remarks Returns a pointer for direct access to the scene. This is primarily
		used for hanging AppData off the entire scene as opposed to a certain
		Animatable. */
		virtual ReferenceTarget *GetScenePointer()=0;
//! @}

//! \name Track Bar and Track View Related Methods
//! @{			
		// Get a pointer to the Track View root node.
		/*! \remarks This method returns a pointer to the Track View Root Node. See
		Class ITrackViewNode. */
		virtual ITrackViewNode *GetTrackViewRootNode()=0;
//! @}

//! \name Bitmap/Texmap Related Methods
//! @{		
		// Free all bitmaps used by the scene
		/*! \remarks This method traverses the scene reference hierarchy, calling
		<b>Animatable::FreeAllBitmaps()</b> on every Animatable. This will free
		up all the memory used by bitmaps. */
		virtual void FreeSceneBitmaps()=0;

		// Enumerate Bitmap Files
		/*! \remarks This method may be used to enumerate all the bitmap files in the scene.
		The <b>flags</b> allow control over which files are enumerated.
		\par Parameters:
		<b>AssetEnumCallback\& assetEnum</b>\n\n
		The callback, called once for each bitmap. See
		Class AssetEnumCallback.\n\n
		<b>DWORD vflags</b>\n\n
		See \ref EnumAuxFiles. 
		Note: if the FILE_ENUM_CHECK_AWORK1 flag bit is not set when this call is made, and the 
		FILE_ENUM_RENDER flag bit is set, the implementation of this method calls 
		ClearAFlagInAllAnimatables(A_WORK1) and sets the FILE_ENUM_CHECK_AWORK1 flag bit.*/
		virtual void EnumAuxFiles(AssetEnumCallback& assetEnum, DWORD vflags)=0;

		// Render a 2D bitmap from a procedural texture
		/*! \remarks	This method renders a textmap (or an entire textmap tree) to the specified bitmap.
		\par Parameters:
		<b>Texmap *tex</b>\n\n
		The Texmap to render to a bitmap.\n\n
		<b>Bitmap *bm</b>\n\n
		A pointer to a bitmap to render to. This bitmap must be created at the
		resolution you wish to render to.\n\n
		<b>float scale3d=1.0f</b>\n\n
		This is a scale factor applied to 3D Texmaps. This is the scale of the surface
		in 3d space that is mapped to UV. This controls how much of the texture appears
		in the bitmap representation.\n\n
		<b>BOOL filter=FALSE</b>\n\n
		If TRUE the bitmap is filtered. It is quite a bit slower to rescale bitmaps
		with filtering on.\n\n
		<b>BOOL display=FALSE</b>\n\n
		If TRUE the resulting bitmap is displayed using the virtual frame buffer;
		otherwise it is not. 
		<b>TimeValue t</b>\n\n
		The time at which to render the texmap to the bitmap, defaults to the current frame.
		<b>bool bake</b>\n\n
		Used to bake the texture to bitmap without including the effects of scale/rotation etc.
		If true, the function will render the unit square of 2D texture maps, by effectively
		disabling the UV Generator during texture rendering, so no settings like scale, rotation,
		tiling etc. have any effect on the result. This baked 2D representation can then later be
		scaled, rotated and tiled, for use in a renderer that may not support the particular
		texture, sent to a game engine, or similar.
		\n\n*/
		virtual void RenderTexmap(
			Texmap *tex, 
			Bitmap *bm, 
			float scale3d=1.0f, 
			BOOL filter=FALSE, 
			BOOL display=FALSE, 
			float z=0.0f,
			TimeValue t = GetCOREInterface()->GetTime(),
			bool bake = false
			)=0;
//! @}

//! \name Rescale World Units Method
//! @{
		// rescale world units of entire scene, or selection
		/*! \remarks This method is used to rescale the world units of the entire scene, or
		optionally the current selection set.
		\par Parameters:
		<b>float f</b>\n\n
		The scale factor to apply to the scene.\n\n
		<b>BOOL selected</b>\n\n
		TRUE to scale selected objects only; otherwise the entire scene is
		scaled. */
		virtual void RescaleWorldUnits(float f, BOOL selected)=0;
//! @}

//! \name Snap Related Methods
//! @{		
		// Initialize snap info structure with current snap settings
		// (Returns zero if snap is OFF)
		/*! \remarks Initialized the SnapInfo structure passed with the current snap
		settings.
		\par Parameters:
		<b>SnapInfo *info</b>\n\n
		Points to the SnapInfo structure to initialize. See
		Structure SnapInfo.
		\return  Returns nonzero if snap is on; zero if off. */
		virtual int InitSnapInfo(SnapInfo* info)=0;
//! @}

//! \name Time Configuration Key Steps Settings Access
//! @{
		// Time configuration dialog key step options
		/*! \remarks Returns TRUE if the Time Configuration / Key Steps / Selected Objects
		Only check box is on; otherwise FALSE. */
		virtual BOOL GetKeyStepsSelOnly()=0;
		/*! \remarks Sets the Time Configuration / Key Steps / Selected Objects Only check
		box to on or off.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetKeyStepsSelOnly(BOOL onOff)=0;
		/*! \remarks Returns TRUE if the Time Configuration / Key Steps / Use Current
		Transform check box is on; otherwise FALSE. */
		virtual BOOL GetKeyStepsUseTrans()=0;
		/*! \remarks Sets the Time Configuration / Key Steps / Use Current Transform check
		box to on or off.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetKeyStepsUseTrans(BOOL onOff)=0;
		/*! \remarks Returns TRUE if the Time Configuration / Key Steps / Position check box
		is on; otherwise FALSE. This value is only meaningful if Use Current
		Transform is off. */
		virtual BOOL GetKeyStepsPos()=0;
		/*! \remarks Sets the Time Configuration / Key Steps / Position check box is to on
		or off.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetKeyStepsPos(BOOL onOff)=0;
		/*! \remarks Returns TRUE if the Time Configuration / Key Steps / Rotation check box
		is on; otherwise FALSE. This value is only meaningful if Use Current
		Transform is off. */
		virtual BOOL GetKeyStepsRot()=0;
		/*! \remarks Sets the Time Configuration / Key Steps / Rotation check box is to on
		or off.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetKeyStepsRot(BOOL onOff)=0;
		/*! \remarks Returns TRUE if the Time Configuration / Key Steps / Scale check box is
		on; otherwise FALSE. This value is only meaningful if Use Current
		Transform is off. */
		virtual BOOL GetKeyStepsScale()=0;
		/*! \remarks Sets the Time Configuration / Key Steps / Scale check box is to on or
		off.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetKeyStepsScale(BOOL onOff)=0;
		/*! \remarks Returns the state of the Time Configuration dialog 'Key Steps / Use
		TrackBar' checkbox. TRUE if checked; FALSE if unchecked. */
		virtual BOOL GetKeyStepsUseTrackBar()=0;
		/*! \remarks Sets the state of the Time Configuration dialog 'Key Steps / Use
		TrackBar' checkbox.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetKeyStepsUseTrackBar(BOOL onOff)=0;
//! @}

//! \name Transform Gizmo Related Methods
//! @{		
		// Enables/disables the use of Transform Gizmos
		/*! \remarks Returns the state of the Transform Tools / Gizmo toggle.
		\return  TRUE if on; FALSE if off. */
		virtual BOOL GetUseTransformGizmo()=0;
		/*! \remarks This method enables or disables the use of Transform Gizmos.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetUseTransformGizmo(BOOL onOff)=0;

		// Get/Set if the TGiz restores the previous axis when released.
		/*! \remarks This method sets whether the TransformGizmo should restore the axis
		constraint when released, or if the axis constraint is permanently
		changed. The value is saved in the 3DSMAX.INI file for later sessions.
		\par Parameters:
		<b>BOOL bOnOff</b>\n\n
		Enable or disable the restoration of the axis constraint. */
		virtual void SetTransformGizmoRestoreAxis(BOOL bOnOff)=0;
		/*! \remarks Indicates if the TransformGizmo will restore the axis constraint when
		released. Returns TRUE if it will; FALSE if it won't. */
		virtual BOOL GetTransformGizmoRestoreAxis()=0;

		// Turn off axis follows transform mode AI.
		/*! \remarks Returns the state of the Transform Tools / Constant Axis toggle.
		\return  TRUE if on; FALSE if off. */
		virtual BOOL GetConstantAxisRestriction()=0;
		/*! \remarks Sets the state of the Transform Tools / Constant Axis toggle.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetConstantAxisRestriction(BOOL onOff)=0;

		// Used to hit test transform Gizmos for sub-objects
		/*! \remarks This method is used to hit test gizmos for sub-objects.
		\par Parameters:
		<b>IPoint2 *p</b>\n\n
		Point to check in screen coordinates.\n\n
		<b>ViewExp *vpt</b>\n\n
		An interface pointer that may be used to call methods associated with
		the viewports.\n\n
		<b>int axisFlags</b>\n\n
		One or more of the following values:\n\n
		<b>HIT_TRANSFORMGIZMO</b>\n\n
		This flag is passed in on a <b>MOUSE_FREEMOVE</b> message so that the
		axis is hit tested and it highlights if it is hit, but it doesn't
		actually switch the transform mode.\n\n
		<b>HIT_SWITCH_GIZMO</b>\n\n
		In case of a <b>MOUSE_POINT</b>, this flag is used, and if the axis is
		hit, the 'hit' transform mode will be pushed on the transform mode
		stack.
		\return  Nonzero if the item was hit; otherwise 0. */
		virtual int HitTestTransformGizmo(IPoint2 *p, ViewExp *vpt, int axisFlags) = 0;

		// Used to deactivate the Transform Gizmo when it is released.
		virtual void DeactivateTransformGizmo() = 0;
//! @}

//! \name Methods to Display MAX Dialogs
//! @{		
		// put up dialog to let user configure the bitmap loading paths.
		// returns 1: OK ,  0: Cancel.
		/*! \remarks This method puts up the dialog to let the user configure the bitmap
		loading paths.\n
		\n \image html "conbmap.gif"
		\return  Nonzero on user selecting OK, zero on Cancel. */
		virtual int ConfigureBitmapPaths()=0;

		// Puts up the space array dialog. If the callback is NULL it 
		// just does the standard space array tool.
		// returns TRUE if the user OKs the dialog, FALSE otherwise.
		/*! \remarks Puts up the space array dialog. If the callback is NULL it just does
		the standard space array tool.\n
		\n \image html "spcarray.gif"
		\par Parameters:
		<b>SpaceArrayCallback *sacb=NULL</b>\n\n
		The callback. See Class SpaceArrayCallback.
		\return  Returns TRUE if the user OKs the dialog, otherwise FALSE. */
		virtual BOOL DoSpaceArrayDialog(SpaceArrayCallback *sacb=NULL)=0;
//! @}

//! \name Add/Delete Class Methods
//! @{		
		// dynamically add plugin-class.
		// returns -1 if superclass was unknown
		// returns 0 if class already exists
		// returns 1 if class added successfully
		/*! \remarks This method is used to dynamically add a plug-in class. This method
		will update the control panel in the Create or Modify branches
		dynamically.
		\par Parameters:
		<b>ClassDesc *pCD</b>\n\n
		Points to the Class Descriptor to add. See
		Class ClassDesc.
		\return  Returns -1 if the superclass was unknown, 0 if the class
		already exists, or 1 if the class was added successfully. */
		virtual int AddClass(ClassDesc *pCD)=0;

		// dynamically delete plugin-class.
		// returns -1 if superclass was unknown
		// returns 0 if class does not exist
		// returns 1 if class deleted successfully
		/*! \remarks This method is used to dynamically delete a plug-in class. This method
		will update the control panel in the Create or Modify branches
		dynamically.
		\par Parameters:
		<b>ClassDesc *pCD</b>\n\n
		Points to the Class Descriptor to add. See
		Class ClassDesc.
		\return  Returns -1 if the superclass was unknown, 0 if the class does
		not exist, or 1 if the class was deleted successfully. */
		virtual int DeleteClass(ClassDesc *pCD)=0;
//! @}

//! \name Command Modes
//! @{
		// Number of CommandModes in the command mode stack
		/*! \remarks Returns the number of command modes in the command mode stack. */
		virtual int GetCommandStackSize()=0;
		// Get the CommandMode at this position in the command mode stack (0 = current)
		/*! \remarks Returns a pointer to the command mode at the specified position in the
		command mode stack. A developer may use this to determine if their
		command mode is in the stack.
		\par Parameters:
		<b>int entry</b>\n\n
		The index into the command mode stack of the entry to get. Pass 0 to
		get the current command mode. */
		virtual CommandMode* GetCommandStackEntry(int entry)=0;
//! @}

//! \name Light Related Methods
//! @{		
		// This method should be called in an light's BeginEditParams, after adding rollups 
		// to the modify panel: it puts up a rollup containing a list of all Atmospherics 
		// and Effects that use the current selected node as a "gizmo"
		/*! \remarks This method should be called in an light's <b>BeginEditParams()</b>
		method, after adding rollups to the modify panel: it puts up a rollup
		containing a list of all Atmospherics and Effects that use the current
		selected node as a "gizmo".
		\par Parameters:
		<b>ULONG vflags=0</b>\n\n
		These are reserved for future use. */
		virtual void AddSFXRollupPage(ULONG vflags=0)=0; // flags are for future use

		// This is called in a light's EndEditParams when removing rollups
		/*! \remarks This is called in a light's <b>EndEditParams()</b> when removing
		rollups. */
		virtual void DeleteSFXRollupPage()=0;

		// This is called by an Atmospheric or Effect when it adds or removes a "gizmo" reference.
		/*! \remarks An Atmospheric or Rendering Effect calls this when it adds or removes a
		"gizmo" reference. This is called to refresh the Special Effects
		rollup. The Atmospherics and Rendering Effects may also use the
		\ref REFMSG_SFX_CHANGE message, though calling this method would have
		the same effect. */
		virtual void RefreshSFXRollupPage()=0;
//! @}

/** \name Property Set Access
 * The following methods provide access to the property set data stored by MAX.
 * You can enter this data through the File Properties dialog.
 * There are three tabs in this dialog that correspond to the options
 * which can be specified for the PropertySet parameter used in the methods below.
 * The PROPSPEC and PROPVARIANT structures used below are part of the Windows API. 
 */
///@{
		// PropertySet access
		// Legal values for "int PropertySet" are defined above:
		//	PROPSET_SUMMARYINFO
		//	PROPSET_DOCSUMMARYINFO
		//	PROPSET_USERDEFINED
		/*! \remarks Returns the number of properties of the specified property set. See the
		note at the start of this group of methods above for info on property
		sets.
		\par Parameters:
		<b>int PropertySet</b>\n\n
		See \ref propertySetOptions. */
		virtual int					GetNumProperties(int PropertySet)=0;
		/*! \remarks Return the index of the specified property or -1 if it is not found.
		See the note at the start of this group of methods above for info on
		property sets.
		\par Parameters:
		<b>int PropertySet</b>\n\n
		See \ref propertySetOptions
		<b>const PROPSPEC* propspec</b>\n\n
		Points to a PROPSPEC structure of the property to find. The Windows API
		PROPSPEC structure is used by many of the methods of IPropertyStorage
		to specify a property either by its property identifier or the
		associated string name. See the Windows API for details on this
		structure.
		\return  The zero based index of the specified property or -1 if not
		found. */
		virtual int					FindProperty(int PropertySet, const PROPSPEC* propspec)=0;
		/*! \remarks Return the value of the property at this index, in PROPVARIANT form.
		See the note at the start of this group of methods above for info on
		property sets.
		\par Parameters:
		<b>int PropertySet</b>\n\n
		See  \ref propertySetOptions
		<b>int idx</b>\n\n
		The zero based index of the property variant to get.
		\return  Points to a <b>PROPVARIANT</b> structure. This Windows API
		structure is used in most of the methods of IPropertyStorage to define
		the type tag and the value of a property in a property set. See the
		Windows API for details on this structure. */
		virtual const PROPVARIANT*	GetPropertyVariant(int PropertySet, int idx)=0;
		/*! \remarks Return the name of the property at this index, in PROPSPEC form. See
		the note at the start of this group of methods above for info on
		property sets.
		\par Parameters:
		<b>int PropertySet</b>\n\n
		See  \ref propertySetOptions
		<b>int idx</b>\n\n
		The zero based index of the property name to get.
		\return  Points to a PROPSPEC structure. The Windows API PROPSPEC
		structure is used by many of the methods of IPropertyStorage to specify
		a property either by its property identifier or the associated string
		name. See the Windows API for details on this structure. */
		virtual const PROPSPEC*		GetPropertySpec(int PropertySet, int idx)=0;
		/*! \remarks This method adds a property to the specified property set. See the
		sample code in
		<b>/MAXSDK/SAMPLES/UTILITIES/PROPERTYTEST/PROPERTYTEST.CPP</b>.\n\n
		See the note at the start of this group of methods above for info on
		property sets.
		\par Parameters:
		<b>int PropertySet</b>\n\n
		See \ref propertySetOptions
		<b>const PROPSPEC* propspec</b>\n\n
		Points to a PROPSPEC structure.\n\n
		<b>const PROPVARIANT* propvar</b>\n\n
		Points to a PROPVARIANT structure. */
		virtual void				AddProperty(int PropertySet, const PROPSPEC* propspec, const PROPVARIANT* propvar)=0;
		/*! \remarks Deletes the specified property. The property will be removed and the
		memory freed. See the note at the start of this group of methods above
		for info on property sets.
		\par Parameters:
		<b>int PropertySet</b>\n\n
		See \ref propertySetOptions
		<b>const PROPSPEC* propspec</b>\n\n
		Points to a PROPSPEC structure to delete. */
		virtual void				DeleteProperty(int PropertySet, const PROPSPEC* propspec)=0;
///@}

//! \name Viewport Access
//! @{
		// register a window that can appear in a MAX viewport
		/*! \remarks This method allows you to register a window that can appear in a
		viewport.
		\par Parameters:
		<b>ViewWindow *vw</b>\n\n
		The pointer to the view window to register.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL RegisterViewWindow(ViewWindow *vw)=0;
		/*! \remarks This method allows you to unregister a window that can appear in a
		viewport.
		\par Parameters:
		<b>ViewWindow *vw</b>\n\n
		The pointer to the view window to unregister.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL UnRegisterViewWindow(ViewWindow *vw)=0;
//! @}
		
		// Get and set the global shadow generator ( used by light.cpp)
		virtual ShadowType *GetGlobalShadowGenerator()=0;
		virtual void SetGlobalShadowGenerator(ShadowType *st)=0;

//! \name File Import/Export Related Methods
//! @{		
		// Get the Import zoom-extents flag
		/*! \remarks This returns the state of the system zoom extents flag. Note that
		individual SceneImport plug-ins can override this in their
		<b>ZoomExtents()</b> method. See
		Class SceneImport.
		\return  TRUE indicates that zoom extents will occur after imports,
		FALSE indicates that no zoom extents. */
		virtual BOOL GetImportZoomExtents()=0;
		/*! \remarks Sets the state of the system zoom extents flag.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE indicates that zoom extents will occur after imports, FALSE
		indicates that no zoom extents. */
		virtual void SetImportZoomExtents(BOOL onOff)=0;

		/*! \remarks This method will check is the specified file can be imported.
		\par Parameters:
		<b>const MCHAR* filename</b>\n\n
		The file name to check.
		\return  TRUE if the specified file can be imported by one of the
		import plug-ins; otherwise FALSE. */
		virtual bool CanImportFile(const MCHAR* filename)=0;
//! @}

//! \name Track Bar and Track View Related Methods
//! @{		
		/*! \remarks This method returns an instance of the <b>ITrackBar</b> class. This
		class may be used to manipulate the track bar. See
		Class ITrackBar. */
		virtual ITrackBar*	GetTrackBar()=0;
//! @}

//! \name XRef Methods
//! @{
		// For scene XRefs. Most of the time the XRef trees (whose root node is a child of the
		// client scene's root node) are skipped when traversing the hierarchy. When this option
		// is turned on, all root nodes will include child XRef scene root nodes in any traversal
		// related functions such as NumberOfChildren() and GetChildNode(i).
		// 
		// This option is turned on automatically before rendering and turned off after so that
		// scene XRefs appear in the production renderer. This option should not be left on if
		// it is turned on since it would cause scene XRef objects to be accessible to the user in the client scene.
		/*! \remarks This method allows a plug-in to specify whether scene XRef objects are
		hidden from the hierarchy when it is traversed. Normally this parameter
		is set to FALSE except during rendering. If a plug-in wants access to
		XRef scene objects then it should set this to TRUE and traverse the
		scene and then set it back to FALSE when it's done.\n\n
		Most of the time the XRef trees (whose root node is a child of the
		client scene's root node) are skipped when traversing the hierarchy.
		When this option is turned on, all root nodes will include child XRef
		scene root nodes in any traversal related functions such as
		<b>NumberOfChildren()</b> and <b>GetChildNode(i)</b>.\n\n
		This option is turned on automatically before rendering and turned off
		after so that scene XRefs appear in the production renderer. <b>Note:
		This option should not be left on if it is turned on since it would
		cause scene XRef objects to be accessible to the user in the client
		scene.</b>\n\n
		Note that plug-ins can also access XRef objects using the
		<a href="class_i_node.html#A_GM_inode_xref">Class INode XRef 
		methods</a>.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE to include XRefs in the hierarchy; FALSE to not include them. */
		virtual void SetIncludeXRefsInHierarchy(BOOL onOff)=0;
		/*! \remarks Returns TRUE if XRefs are included in the traversal of the scene
		hierarchy; otherwise FALSE. See the method above for details. */
		virtual BOOL GetIncludeXRefsInHierarchy()=0;

		// Use these two suspend automatic reloading of XRefs. 
		/*! \remarks Returns TRUE if the automatic updating of XRefs is suspended; otherwise
		FALSE. When an XRef file is changed and that causes an XRef object to
		update, the old XRef object gets deleted from memory which can cause
		problems for some plug-ins. For example, the Dynamics system would have
		a problem if an update occurred while a solution was solving. This
		method is used to disable the automatic updating to prevent the
		problem. */
		virtual BOOL IsXRefAutoUpdateSuspended()=0;
		/*! \remarks Sets if the automatic updating of XRefs is suspended or not. See the
		note in <b>IsXRefAutoUpdateSuspended()</b> for details.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		TRUE to suspend; FALSE to restore automatic updating. */
		virtual void SetXRefAutoUpdateSuspended(BOOL onOff)=0;

		// RB: 3/30/99
		/*! \remarks Returns TRUE if the specified node is part of a scene XRef or FALSE if
		the node is a regular modifiable node in the current scene.
		\par Parameters:
		<b>INode *node</b>\n\n
		The node to check. */
		virtual BOOL IsSceneXRefNode(INode *node)=0;
//! @}

		// get the macroRecorder interface pointer
		virtual MacroRecorder* GetMacroRecorder()=0;

//! \name Material Related Methods
//! @{
		// DS 2/2/99: 
		/*! \remarks This method makes sure the Materials Editor slots correctly reflect
		which materials are used in the scene, which are used by selected
		objects, etc. This is used internally for the drag-and-drop of
		materials to nodes -- there is no reason why a plug-in developer should
		need to call it. */
		virtual void UpdateMtlEditorBrackets()=0;
//! @}

//! \name Licensing Methods
//! @{
		/*! \remarks Returns TRUE if the application is running under a trial license, as
		opposed to a full, authorized license; otherwise FALSE. */
		virtual bool IsTrialLicense() = 0;
		//! Returns true if the application is running under a network license.
		virtual bool IsNetworkLicense() = 0;
//! @}

//! \name File Open/Merge/Save/Reset/Hold/Fetch Methods
//! @{
		/*! \remarks This method can be used to write out a .MAX file if needed. First it
		ends animation if it's in progress. Next it determines whether a save
		operation is required (change marked in scene, or undo operations
		present).
		\return  If the save is not required, it returns TRUE; otherwise, it
		puts up a dialog box asking if the user wants to save. If the user
		picks No, it returns TRUE. If the user picks Cancel it returns FALSE.
		If the user picks Yes then the method proceeds as for <b>FileSave()</b>
		above. */
		virtual BOOL CheckForSave()=0;
		// CCJ - 2/12/99
		/*! \remarks This method allows a custom file open dialog to be registered.
		\par Parameters:
		<b>MAXFileOpenDialog* dlg</b>\n\n
		Points to the file open dialog object to use. See
		Class MAXFileOpenDialog. */
		virtual void SetMAXFileOpenDlg(MAXFileOpenDialog* dlg) = 0;
		/*! \remarks This method allows a custom file save dialog to be registered.
		\par Parameters:
		<b>MAXFileSaveDialog* dlg</b>\n\n
		Points to the file save dialog object to use. See
		Class MAXFileSaveDialog. */
		virtual void SetMAXFileSaveDlg(MAXFileSaveDialog* dlg) = 0;
//! @}

//! \name RAM Player
//! @{		
		/*! \remarks Brings up the RAMPlayer dialog and optionally loads one, or both
		channels with the supplied files. \n
		\n \image html "ramplay.gif"
		\par Parameters:
		<b>HWND hWndParent</b>\n\n
		The parent window handle.\n\n
		<b>MCHAR* szChanA=NULL</b>\n\n
		The file to load for channel A (for example, <b>_M("movie.avi")</b>).
		If NULL is passed no file is loaded into the channel.\n\n
		<b>MCHAR* szChanB=NULL</b>\n\n
		The file to load for channel B. If NULL is passed no file is loaded
		into the channel. */
		virtual void RAMPlayer(HWND hWndParent, const MCHAR* szChanA=NULL, const MCHAR* szChanB=NULL) = 0;
//! @}

//! \name Undo/Redo Related Methods
//! @{		
		/*! \remarks This function will flush the undo buffer. See the Advanced Topics
		section ~{ The Undo and Redo System }~. */
		virtual void FlushUndoBuffer() = 0;
//! @}

//! \name Deferred Loading Related Methods
//! @{		
		/*! \remarks In the Preferences dialog / General Tab / Plug-In Loading section there
		is a checkbox labelled 'Load Plug-Ins when Used'. This method returns
		the state of this toggle.. See the Advanced Topics section
		~{ Deferred Loading of Plug-Ins }~.
		\return  TRUE if on; FALSE if off. */
		virtual bool DeferredPluginLoadingEnabled() = 0;
		/*! \remarks In the Preferences dialog / General Tab / Plug-In Loading section there
		is a checkbox labelled 'Load Plug-Ins when Used'. This method sets the
		state of this toggle. See the Advanced Topics section
		~{ Deferred Loading of Plug-Ins }~.
		\par Parameters:
		<b>bool onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void EnableDeferredPluginLoading(bool onOff) = 0;
//! @}

//! \name File Open/Merge/Save/Reset/Hold/Fetch Methods
//! @{
		/*! \remarks Returns true if the specified file is a valid MAX file; otherwise
		false.
		\par Parameters:
		<b>const MCHAR* filename</b>\n\n
		The name of the file to check. */
		virtual bool IsMaxFile(const MCHAR* filename)=0;
		/*! \remarks Returns true if the specified file is an internet cached file;
		otherwise false.
		\par Parameters:
		<b>const MCHAR* filename</b>\n\n
		The name of the file to check. */
		virtual bool IsInternetCachedFile(const MCHAR* filename)=0;
//! @}

//! \name Bitmap/Texmap Related Methods
//! @{		
		// MEP 6/21/99
		/*! \remarks This method will check if the system can import the specified bitmap
		file.
		\par Parameters:
		<b>const MCHAR* filename</b>\n\n
		The file name to check.
		\return  TRUE if the specified file is a bitmap file of a format that
		is supported by one of the bitmap reader plug-ins; otherwise FALSE.\n\n
		  */
		virtual bool CanImportBitmap(const MCHAR* filename)=0;

		//JH 8/07/99: implementing scheme to capture subobject registration
		/*! \remarks This method is used to lock and unlock subobject mode registrations and
		is primarily used by the FileLink wrapper classes acting as proxies to
		other classes in 3D Studio VIZ. When a class calls this method with
		OnOff set to TRUE, then other classes are prevented from registering
		new subobject modes. This continues until the original class "releases"
		by calling CaptureSubObjectModes(FALSE, myClassID). The second argument
		insures that only the class which does the capture can do the release.
		Note that this is used only by certain VIZ plugins.
		\par Parameters:
		<b>bool OnOff</b>\n\n
		TRUE to prevent other classes from registering new subobject modes.\n\n
		<b>Class_ID cid</b>\n\n
		The class ID.
		\return  TRUE is successful, otherwise FALSE. */
		virtual bool CaptureSubObjectRegistration(bool OnOff, Class_ID cid)=0;

		// MEP 8/06/99: easy method to download a file from a URL
		// JH 5/4/99: adding argument to control dialog appearance
		#define DOWNLOADDLG_NOPLACE		1<<0
		/*! \remarks This method simplifies downloading files from any given URL and
		displays a floating progress dialog.
		\par Parameters:
		<b>HWND hwnd</b>\n\n
		The window handle for owner window (required for the progress floating
		dialog).\n\n
		<b>const MCHAR* url</b>\n\n
		The string for the resource/file to download.\n\n
		<b>const MCHAR* filename</b>\n\n
		The target location and filename for the downloaded file.\n\n
		<b>DWORD downloadFlags = 0</b>\n\n
		Additional controls to the download behavior. Currently only one flag
		is supported, <b>DOWNLOADDLG_NOPLACE</b>, which hides an option in the
		progress dialog that allows the user to place (move) a dropped object
		immediately after being dropped.
		\return  TRUE if successful, otherwise FALSE. */
		virtual bool DownloadUrl(HWND hwnd, const MCHAR* url, const MCHAR* filename, DWORD downloadFlags = 0)=0;

		// MEP 3/10/2000: if drag-and-dropped onto a node, you can retrieve the node with this
		/*! \remarks On drag-and-drop, if the drop type is a file, the drop handler searches
		for an importer plugin that can handle the file (based on its
		extension). Some drop operations, such as bitmaps and material XML
		files, can or must be dropped on to an object in the scene.
		\return  The node that the operation is performed on, if one is "hit"
		at the drop location. It returns NULL if no nodes were found at the
		drop location. */
		virtual INode* GetImportCtxNode(void)=0;
//! @}

//! \name Track Bar and Track View Related Methods
//! @{		
		// child treeview creation - JBW 5.25.00
		/*! \remarks This method will creates a plain treeview window (no
		title,borders,etc.) as a child window of the given window. To destroy
		the window, delete the ITreeView pointer.
		\par Parameters:
		<b>ReferenceTarget* root</b>\n\n
		Points to the root node of the hierarchy to display in the Track
		View.\n\n
		<b>HWND hParent</b>\n\n
		The window handle of the parent for the dialog.\n\n
		<b>DWORD style=0</b>\n\n
		The style flags;\n\n
		<b>TVSTYLE_MAXIMIZEBUT</b>\n\n
		Provide a maximize button.\n\n
		<b>TVSTYLE_INVIEWPORT</b>\n\n
		Display in the viewport.\n\n
		<b>TVSTYLE_NAMEABLE</b>\n\n
		The treeview is namable.\n\n
		<b>TVSTYLE_INMOTIONPAN</b>\n\n
		Used in the motion panel.\n\n
		<b>ULONG id=0</b>\n\n
		The ID of the treeview window.\n\n
		<b>int open=OPENTV_SPECIAL</b>\n\n
		One of the following values:\n\n
		<b>OPENTV_NEW</b>\n\n
		Open a new treeview.\n\n
		<b>OPENTV_SPECIAL</b>\n\n
		Open a special treeview.\n\n
		<b>OPENTV_LAST</b>\n\n
		Open the last treeview. */
		virtual ITreeView* CreateTreeViewChild(ReferenceTarget* root, HWND hParent, DWORD style=0, ULONG id=0, int open=OPENTV_SPECIAL)=0;
//! @}

//! \name Material Related Methods
//! @{
		// CCJ 6.7.00 - Create a viewport material from a renderer material
		/*! \remarks This function converts a material (class Mtl) to a viewport material
		(class Material).
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time to convert the material.\n\n
		<b>Material \&gm</b>\n\n
		The viewport material (output). See Class Material.\n\n
		<b>Mtl *mtl</b>\n\n
		The material to convert (input). See Class Mtl.\n\n
		<b>BOOL doTex</b>\n\n
		Determines whether or not to include textures. TRUE for yes; FALSE for
		no.\n\n
		<b>int subNum</b>\n\n
		If the input material <b>mtl</b> is a sub-material then pass its
		subnum; otherwise pass 0.\n\n
		<b>float vis</b>\n\n
		The visibility value in the range of 0.0 (completely transparent) to
		1.0 (fully opaque) for the viewport representation.\n\n
		<b>BOOL \&needDecal</b>\n\n
		Pass TRUE if the texture needs decal mapping; otherwise FALSE.\n\n
		<b>INode *node</b>\n\n
		Points to the node that the material is assigned to. See
		Class INode.\n\n
		<b>BitArray *needTex</b>\n\n
		A BitArray that returns which map channels are needed. The BitArray is
		enlarged if needed. See Class BitArray.\n\n
		<b>GraphicsWindow *gw</b>\n\n
		The GraphicsWindow to do the conversion for. See
		Class GraphicsWindow. */
		virtual void ConvertMtl(TimeValue t, Material& gm, Mtl* mtl,  BOOL doTex, 
							int subNum, float vis, BOOL& needDecal, INode* node, 
							BitArray* needTex, GraphicsWindow* gw)=0;
//! @}

//! \name Node Selection Sets
//! @{
		// CCJ 8.23.00 - Clone a node hierarchy
		/*! \remarks This method allows you to clone nodes and node hierarchies.
		\par Parameters:
		<b>INodeTab\& nodes</b>\n\n
		The node table containing the nodes you wish to clone.\n\n
		<b>Point3\& offset</b>\n\n
		The position offset you wish to apply to the cloned nodes.\n\n
		<b>bool expandHierarchies = true</b>\n\n
		This determines if children will be cloned in hierarchies.\n\n
		<b>CloneType cloneType = NODE_COPY</b>\n\n
		The type of cloning you wish to do, which is one of the following;
		<b>NODE_COPY</b>, <b>NODE_INSTANCE</b> or <b>NODE_REFERENCE</b>.\n\n
		<b>INodeTab* resultSource = NULL</b>\n\n
		This node table will be filled in with the original nodes to be cloned.
		The reason for this is that there can be dependencies between nodes
		that cause other nodes to be added to the list. For example
		light/camera targets, nodes part of systems, belonging to groups or
		expanded hierarchies etc.\n\n
		<b>INodeTab* resultTarget = NULL</b>\n\n
		This node table will be filled in with the new cloned nodes. There is a
		one to one relationship between the nodes in the resultSource and the
		resultTraget.
		\return  TRUE if the nodes were cloned successfully, otherwise FALSE.
		Note that with R4.0 the return value will always be TRUE since no
		actual error checking is undertaken. */
		virtual bool CloneNodes(INodeTab& nodes, Point3& offset, bool expandHierarchies = true, CloneType cloneType = NODE_COPY, INodeTab* resultSource = NULL, INodeTab* resultTarget = NULL) = 0;
//! @

		// NS: 9/9/00
		virtual void CollapseNode(INode *node, BOOL noWarning = FALSE)=0;
		virtual BOOL CollapseNodeTo(INode *node, int modIndex, BOOL noWarning = FALSE)=0;
		virtual BOOL ConvertNode(INode *node, Class_ID &cid)=0;

		// TB 5/22/03
		virtual IRenderPresetsManager* GetRenderPresetsManager()=0;

		// LAM - 6/24/03
		//-- Default Action Facilities 
		//   Check DefaultActions.h for methods
		virtual DefaultActionSys *DefaultActions()=0;

		//! Sets internal flag, returns old value. 
		/*! \param onOff - Set to TRUE to indicate that no dialogs should be displayed to user.
			\return the previous quiet mode state */
		virtual BOOL SetQuietMode(BOOL onOff)=0;

		//! The return value from this method should be called before displaying any dialogs.
		/*! Returns internal flag set by SetQuietMode. 
			\param checkServerMode- if TRUE, returns internal flag OR-ed with 
			whether 3ds Max runs in server , i.e. UI-less mode. 
			If client code needs save the current quiet mode's value in order to restore it later
			via a call to Interface::SetQuietMode(), it should retrieve it using Interface::GetQuietMode(FALSE).
			\return the current quiet mode. */
		virtual BOOL GetQuietMode(BOOL checkServerMode = TRUE)=0;

		//! Enables/disables system AutoGrid feature.
		/*! This allows plugins to use the standard creation manager but disable AutoGrid if desired.
			Use this call with "false" argument in ClassDesc::BeginCreate to disable AutoGrid;
			use with "true" argument in ClassDesc::EndCreate to re-enable.
			\param sw - true=enable false=disable */
		virtual void SetAutoGridEnable(bool sw = true)=0;

		//! Query system AutoGrid feature enable state
		/*! Returns internal flag set by SetAutoGridEnable. 
			\return the current AutoGrid enable state. */
		virtual bool GetAutoGridEnable()=0;

		//! Query system AutoGrid feature state
		/*! Returns internal flag set by SetAutoGrid. 
			\return the current AutoGrid state. */
		virtual bool GetAutoGridState()=0;

		//! Sets system AutoGrid state.
		/*! This allows plugins to turn AutoGrid on or off.
			\param sw - true=Autogrid on false=AutoGrid off */
		virtual void SetAutoGridState(bool sw = true)=0;
}; // class Interface

#pragma warning(pop)
//! \brief Scene redraw guard class
/*! Instantiating an object of this type suspends (disables) scene redraw. 
When the object is destroyed, scene redraw is enabled automatically. 
*/
class Interface::SuspendSceneRedrawGuard : public MaxHeapOperators
{
public:
	//! \brief Constructor - Suspends scene redraw
	SuspendSceneRedrawGuard() : mResumeRedrawRequired(false) {
		Suspend();
	}

	//! \brief Destructor - Resumes scene redraw
	~SuspendSceneRedrawGuard() {
		Resume();
	}

	//! \brief Suspends scene redraw
	void Suspend() {
		if (!mResumeRedrawRequired) {
			GetCOREInterface()->DisableSceneRedraw();
			mResumeRedrawRequired = true;
		}
	}

	//! \brief Resumes scene redraw
	void Resume() {
		if (mResumeRedrawRequired) {
			GetCOREInterface()->EnableSceneRedraw();
			mResumeRedrawRequired = false;
		}
	}

private:
	bool mResumeRedrawRequired;
};

/*! \sa  Class Interface.\n\n
\par Description:
This class is identical to Class Interface.
Refer to that section for a description of the methods. */
class IObjParam : public Interface{};

/*! \sa  Class Interface, Class IObjParam.\n\n
\par Description:
This class is identical to Class Interface.
Refer to that section for a description of the methods. */
class IObjCreate : public IObjParam{};

// Forward declarations
class MAXScriptPrefs;
class IMenu;
class IMenuItem;
class FrameRendParams;
class DefaultLight;
class ITabPage;

//-----------------------------------------------------------------------------
/// This class extends Max's previous version of core interface (class Interface) 
/// "7" is the major version number of Max that exposes this interface.
/// Call GetCOREInterface7 to acquire a pointer to this interface.
//-----------------------------------------------------------------------------
class Interface7 : public IObjCreate
{
public:

	//! \brief The ID for this interface.  Pass this ID to Interface::GetInterface to get an Interface7 pointer.
	CoreExport static const Interface_ID kInterface7InterfaceID;

	// --- Object Selection ------------------------------------------------ ///
	/// Retrieves the currently selected nodes into the supplied parameter.
	/// It clears the node tab supplied as parameter before using it.
	virtual void GetSelNodeTab(INodeTab& selectedNodes) const = 0;

	// --- Transform Modes and Reference Coordinate System ----------------- ///
	/// Pivot modes the system can be in
	enum PivotMode
	{
		/// Transforms will affect the objects' world transform
		kPIV_NONE = PIV_NONE,
		/// Transforms will affect only the pivot point of objects
		kPIV_PIVOT_ONLY = PIV_PIVOT_ONLY,
		/// Transforms will affect only the objects and not their pivot points
		kPIV_OBJECT_ONLY = PIV_OBJECT_ONLY,
		/// Rotation and Scale will be applied to the hierarchy by rotating or 
		/// scaling the position of the pivot point without rotating or scaling 
		/// the pivot point itself. 
		kPIV_HIERARCHY_ONLY = PIV_HIERARCHY_ONLY
	};
	
	/// Returns the current pivot mode the system is in. 
	virtual PivotMode GetPivotMode() const = 0;
	/// Set the pivot mode of the system
	virtual void SetPivotMode(PivotMode pivMode) = 0;

	/// Returns true if the transforms applied to a node will affect its children,
	/// otherwise returns false
	virtual bool GetAffectChildren() const = 0;
	/// Sets whether the transforms applied to a node will affect its children.
	/// If bAffectChildren is true, the children of a node will be transformed when
	/// their parent node is transformed. Otherwise, they won't be transformed.
	virtual void SetAffectChildren(bool bAffectChildren) = 0;

	/// Given a reference coordinate system id, returns the name of it
	virtual void GetCurRefCoordSysName(MSTR& name) const = 0;

	/// Allows for setting the specified node's axis, as the current reference 
	/// coordinate system
	virtual void AddRefCoordNode(INode *node) = 0;
	virtual INode* GetRefCoordNode() = 0;

	// --- Rendering ------------------------------------------------------- ///
	/// Only relevant for network rendering.  It is possible to set a job flag
	/// indicates that max should attempt to continue to render even when an
	/// "error" has been detected.  This method allows plug-ins to determine
	/// whether this flag has been set.
	virtual bool ShouldContinueRenderOnError() const = 0;

	virtual void SetupFrameRendParams(
		FrameRendParams &frp, 
		RendParams &rp, 
		ViewExp *vx, 
		RECT *r) = 0;
	virtual void SetupFrameRendParams_MXS(
		FrameRendParams &frp, 
		RendParams &rp, 
		ViewExp *vx, 
		RECT *r, 
		bool useSelBox) = 0;
	virtual int InitDefaultLights(
		DefaultLight *dl, 
		int maxn, 
		BOOL applyGlobalLevel = FALSE, 
		ViewExp *vx = NULL, 
		BOOL forRenderer = FALSE) = 0;

	/// Methods used to sync use with Material Editor sample rendering
	virtual void IncrRenderActive() = 0;  
	virtual void DecrRenderActive() = 0;
	virtual BOOL IsRenderActive() = 0;
	
	virtual BOOL XRefRenderBegin() = 0;
	virtual void XRefRenderEnd() = 0;

	virtual void OpenRenderDialog() = 0;
	virtual void CancelRenderDialog() = 0;
	virtual void CloseRenderDialog() = 0;
	virtual void CommitRenderDialogParameters() = 0;
	virtual void UpdateRenderDialogParameters() = 0;
	virtual BOOL RenderDialogOpen() = 0;

	virtual Bitmap* GetLastRenderedImage() = 0;

	// --- General UI ------------------------------------------------------ ///
	/// Returns the window handle of the status panel window 
	/// (this holds the MAXScript mini-listener)
	virtual HWND GetStatusPanelHWnd() = 0;
	
	// --- Maxscript ------------------------------------------------------- ///
	virtual void SetListenerMiniHWnd(HWND wnd) = 0;
	virtual HWND GetListenerMiniHWnd() = 0;
	
	/// Starts the MAXScript help
	virtual int MAXScriptHelp(const MCHAR* keyword = NULL) = 0;

	/// Retrieves maxscript preferences
	virtual MAXScriptPrefs& GetMAXScriptPrefs() = 0;

	// --- Trackview ------------------------------------------------------- ///
	virtual BOOL OpenTrackViewWindow(
		const MCHAR* tv_name, 
		const MCHAR* layoutName = NULL, 
		Point2 pos = Point2(-1.0f,-1.0f), 
		int width = -1, 
		int height = -1, 
		int dock = TV_FLOAT) = 0;
	
	/// Sets the focus to the specified track view window
	virtual BOOL BringTrackViewWindowToTop(const MCHAR* tv_name) = 0;

	virtual BOOL TrackViewZoomSelected(const MCHAR* tv_name) = 0;
	virtual BOOL TrackViewZoomOn(const MCHAR* tv_name, Animatable* parent, int subNum) = 0;
	virtual BOOL CloseTrackView(const MCHAR* tv_name) = 0;
	virtual int NumTrackViews() = 0;
	virtual const MCHAR* GetTrackViewName(int i) = 0;
	/// The mask bits are defined in MAXSDK/INCLUDE/ITREEVW.H. Internally, the mask 
	/// bits are stored in two DWORDs The 'which' param tells which to work with - 
	/// valid values are 0 and 1.
	virtual BOOL SetTrackViewFilter(
		const MCHAR* tv_name, 
		DWORD mask, 
		int which, 
		BOOL redraw = TRUE) = 0;
	virtual BOOL ClearTrackViewFilter(
		const MCHAR* tv_name, 
		DWORD mask, 
		int which, 
		BOOL redraw = TRUE) = 0;
	virtual DWORD TestTrackViewFilter(const MCHAR* tv_name, DWORD mask, int which) = 0;
	virtual void FlushAllTrackViewWindows() = 0;
	virtual void UnFlushAllTrackViewWindows() = 0;
	virtual void CloseAllTrackViewWindows() = 0;
	
	// --- Command Panel Control ------------------------------------------- ///
	virtual void SetCurEditObject(BaseObject *obj, INode *hintNode = NULL) = 0;
	/// Get the object or modifier that is currently being edited in the modifier panel
	virtual BaseObject* GetCurEditObject() = 0;
	virtual void AddModToSelection(Modifier* mod) = 0;
	virtual void InvalidateObCache(INode* node) = 0;
	virtual INode* FindNodeFromBaseObject(ReferenceTarget* obj) = 0;  
	virtual void SelectedHistoryChanged() = 0; 
	virtual BOOL CmdPanelOpen() = 0;
	virtual void CmdPanelOpen(BOOL openClose) = 0;
	
	/// Suspends / Resumes command panels specified via bits set in whichPanels param
	virtual void SuspendEditing(
		DWORD whichPanels = (1<<TASK_MODE_MODIFY), 
		BOOL alwaysSuspend = FALSE) = 0;
	virtual void ResumeEditing(
		DWORD whichPanels = (1<<TASK_MODE_MODIFY), 
		BOOL alwaysSuspend = FALSE) = 0;
	virtual void SuspendMotionEditing() = 0;
	virtual void ResumeMotionEditing() = 0;

	/// This method expands Interface::AddClass. It allows for adding new ClassDesc 
	/// dynamically to create panel 
	virtual int AddClass(
		ClassDesc *cdesc, 
		int dllNum = -1, 
		int index = -1, 
		bool load = true) = 0;
	/// Rebuilds the list of groups and categories of the Create Panel
	virtual void ReBuildSuperList() = 0;
	
	/// Returns FALSE if the editing is stopped. While stopped, it shouldn't be resumed
	virtual BOOL IsEditing() = 0;

	/// Allows for changing the modifier panel's 
	virtual void ChangeHistory(int upDown) = 0;

	// --- Object Creation ------------------------------------------------- ///
	virtual void StartCreatingObject(ClassDesc *pCD) = 0;
	virtual BOOL IsCreatingObject(const Class_ID& id) = 0;
	virtual BOOL IsCreatingObject() = 0; 
	/// Fast node creation for FileLink
	virtual void UpdateLockCheckObjectCounts() = 0;
	virtual INode *CreateObjectNode( Object *obj, const MCHAR* name) = 0;
	
	// --- Configuration Paths/Directories --------------------------------- ///
	virtual BOOL SetDir(int which, const MCHAR *dir) = 0;

	/*! \brief Adds a permanent user path associated to a particular asset type

		\param [in] dir An absolute path
		\param [in] assetType An enumeration constant representing the asset type
		\param [in] update Update system configuration files with the new data
		\return TRUE if the path was added, FALSE if not
		\remarks Remember that this function is for adding permanent user paths, not session user paths.
		Session user paths are temporary. */
	virtual BOOL AddAssetDir(const MCHAR *dir, MaxSDK::AssetManagement::AssetType assetType, int update=TRUE) = 0;
	
	/*! \brief Deletes the "ith" permanent user path associated with a particular asset type

		\param [in] i The index of the directory you want to delete
		\param [in] assetType An enumeration constant representing the asset type
		\param [in] update Update system configuration files with the new data
		\return a c-string with an absolute path to the ith asset directory
		\remarks Remember that this function returns a permanent user path, not a session user path. */
	virtual BOOL DeleteAssetDir(int i, MaxSDK::AssetManagement::AssetType assetType,int update=TRUE) = 0;
	
	/*! \brief Updates system configuration files' asset directory section of a particular asset type, 
	with changes made to the session and/or permanent asset directories of that particular asset type.

		\param [in] assetType An enumeration constant representing the asset type
		\return a c-string with an absolute path to the ith asset directory
		\remarks Remember that this function returns a permanent user path, not a session user path. */
	virtual void UpdateAssetSection(MaxSDK::AssetManagement::AssetType assetType) = 0;
	
	/// Appends a string to the current file name and file path. It also updates 
	/// the string in the application's title bar. 
	virtual BOOL AppendToCurFilePath(const MCHAR* toAppend) = 0;

	/// Returns empty MSTR if locType == LOC_REGISTRY.
	virtual MSTR GetMAXIniFile() = 0; 

	/// --- Schematic View -------------------------------------------------- ///
	virtual BOOL OpenSchematicViewWindow(const MCHAR* sv_name) = 0;
	virtual BOOL SchematicViewZoomSelected(const MCHAR* sv_name) = 0;
	virtual BOOL CloseSchematicView(const MCHAR* sv_name) = 0;
	virtual int  NumSchematicViews() = 0;
	virtual const MCHAR* GetSchematicViewName(int i) = 0;
	virtual void CloseAllSchematicViewWindows() = 0;
	virtual void FlushAllSchematicViewWindows() = 0;
	virtual void UnFlushAllSchematicViewWindows() = 0;

	// --- Scene ----------------------------------------------------------- ///
	virtual BOOL DrawingEnabled() = 0;
	virtual void EnableDrawing(BOOL onOff) = 0;
	virtual BOOL SceneResetting() = 0;
	virtual BOOL QuitingApp() = 0;

	virtual BOOL GetHideFrozen() = 0;
	/// See 'Scene Display Flags' in maxapi.h for the possible values of 'flag'
	virtual void SetSceneDisplayFlag(DWORD flag, BOOL onOff, BOOL updateUI = TRUE) = 0;
	virtual BOOL GetSceneDisplayFlag(DWORD flag) = 0;

	/// Access to the scene interface
	virtual IScene* GetScene() = 0;
	
	// ---  Materials and Material Editor ---------------------------------- ///
	virtual void SetMtlSlot(int i, MtlBase* m) = 0;
	virtual int GetActiveMtlSlot() = 0;
	virtual void SetActiveMtlSlot(int i) = 0;
	virtual int NumMtlSlots() = 0;
	virtual void FlushMtlDlg() = 0;
	virtual void UnFlushMtlDlg() = 0;
	virtual BOOL IsMtlInstanced(MtlBase* m) = 0;

	virtual Mtl *FindMtlNameInScene(MSTR &name) = 0;
	virtual void PutMaterial(
		MtlBase* mtl, 
		MtlBase* oldMtl, 
		BOOL delOld = 1, 
		RefMakerHandle skipThis = 0) = 0;

	//! \brief Returns whether any Material Editor is open.
	/*! Pertains to either the Basic or Advanced editor according to the current mode; both cannot be open simultaneously.
		\see mtlDlgMode */
	virtual BOOL IsMtlDlgShowing() = 0;

	//! \brief Opens the Material Editor.  No effect if already open.
	/*! Opens either the Basic or Advanced editor according to the current mode; both cannot be open simultaneously.
		\see mtlDlgMode */
	virtual void OpenMtlDlg() = 0;

	//! \brief Closes the Material Editor.  No effect is already closed.
	/*! Closes either the Basic or Advanced editor according to the current mode; both cannot be open simultaneously.
		\see mtlDlgMode */
	virtual void CloseMtlDlg() = 0;

	// --- Viewport -------------------------------------------------------- ///
	/// Returns the window handle for the viewport frame
	virtual HWND GetViewPanelHWnd() = 0;  
	/// Viewport access by index
	virtual int  getActiveViewportIndex() = 0;
	virtual BOOL setActiveViewport(int index) = 0;
	virtual int getNumViewports() = 0;
	// Returns the ViewExp interface of the active view panel based on the 
	// input index. 
	// If the index is invalid, the first viewport of the current
	// view tab will be returned.
	virtual ViewExp& getViewExp(int i) = 0;

	virtual void resetAllViews() = 0;
	
	/// Viewport name access
	virtual const MCHAR* getActiveViewportLabel() = 0;
	virtual const MCHAR* getViewportLabel(int index) = 0;
	
	/// Viewport blow-up and sub-region access
	virtual void SetRegionRect(int index, Rect r) = 0;
	virtual Rect GetRegionRect(int index) = 0;
	virtual void SetBlowupRect(int index, Rect r) = 0;
	virtual Rect GetBlowupRect(int index) = 0;
	virtual void SetRegionRect2(int index, Rect r) = 0;
	virtual Rect GetRegionRect2(int index) = 0;
	virtual void SetBlowupRect2(int index, Rect r) = 0;
	virtual Rect GetBlowupRect2(int index) = 0;
	virtual int GetRenderType() = 0;
	virtual void SetRenderType(int rtype) = 0;
	virtual BOOL GetLockImageAspRatio() = 0;
	virtual void SetLockImageAspRatio(BOOL on) = 0;
	virtual float GetImageAspRatio() = 0;
	virtual void SetImageAspRatio(float on) = 0;
	virtual BOOL GetLockPixelAspRatio() = 0;
	virtual void SetLockPixelAspRatio(BOOL on) = 0;
	virtual float GetPixelAspRatio() = 0;
	virtual void SetPixelAspRatio(float on) = 0;

	virtual void SetViewportGridVisible(int index, BOOL state) = 0;
	virtual BOOL GetViewportGridVisible(int index) = 0;

	virtual void ViewportInvalidate(int index) = 0; 
	virtual void ViewportInvalidateBkgImage(int index) = 0; 
	virtual void InvalidateAllViewportRects() = 0;

	/*! \param t - The time to redraw the viewports at
		\param vpFlags - See \ref vieportDegradationFlags for a list of possible values */
	virtual void RedrawViewportsNow(
		TimeValue t, 
		DWORD vpFlags = VP_DONT_SIMPLIFY) = 0;

	/*! \param t - The time to redraw the viewports at
		\param vpFlags - See \ref vieportDegradationFlags for a list of possible values */
	virtual void RedrawViewportsLater(
		TimeValue t, 
		DWORD vpFlags = VP_DONT_SIMPLIFY ) = 0;

	/*!	\brief Sets the current viewport rendering level.
		\param level - See \ref viewportRenderingLevel for possible values  */
	virtual void SetActiveViewportRenderLevel(int level) = 0;
	/*! \brief Returns the current viewport rendering level
		\return See \ref viewportRenderingLevel for possible values  */
	virtual int GetActiveViewportRenderLevel() = 0;

	/// Access to viewport show edge faces states
	virtual void SetActiveViewportShowEdgeFaces(BOOL show) = 0;
	virtual BOOL GetActiveViewportShowEdgeFaces() = 0;

	/// There are 3 levels of transparency: 0, 1 and 2.\n 
	/// 0 - no transparency\n
	/// 1 - if hardware rendering -> Blend, otherwise Stipple\n
	/// 2 - if hardware rendering -> Sorted Blend, otherwise Blend\n
	virtual void SetActiveViewportTransparencyLevel(int level) = 0;
	virtual int GetActiveViewportTransparencyLevel() = 0;

	/// Access dual plane settings
	virtual BOOL GetDualPlanes() = 0;
	virtual void SetDualPlanes(BOOL b) = 0;

	// --- Hit Testing ----------------------------------------------------- ///
	/// When the flag is On, only frozen objects are hit tested during a pick
	virtual void SetTestOnlyFrozen(int onOff) = 0;

	// --- Tool and Command Modes ------------------------------------------ ///
	/// Scale modes: CID_OBJSCALE, CID_OBJUSCALE, CID_OBJSQUASH (see cmdmode.h)
	virtual void SetScaleMode(int mode) = 0;
	/// Center modes - see 'Origin modes' in maxapi.h
	virtual void SetCenterMode(int mode) = 0;
	/// Manipulator related
	virtual BOOL InManipMode() = 0;
	virtual void StartManipulateMode() = 0;
	virtual void EndManipulateMode() = 0;
	virtual BOOL IsViewportCommandMode(CommandMode *m) = 0;
	
	/// --- XRefs ----------------------------------------------------------- ///
	// See the 'ConvertFlagedNodesToXRefs() xflags bits' in maxapi.h for the 
	// possible values the 'xflag' parameter can take
	/// \brief This function is for internal use only.
	virtual	void ConvertFlagedNodesToXRefs(
		const MaxSDK::AssetManagement::AssetUser &fname,
		INode *rootNode,
		Tab<INode*> &nodes, 
		int xFlags) = 0;
	/// 'f' parameter can take values defined as 'Xref flag bits' in inode.h
	virtual void XRefSceneSetIgnoreFlag(int index, DWORD f, BOOL onOff) = 0;
	virtual void UpdateSceneXRefState() = 0;

	// --- Object Snaps ---------------------------------------------------- ///
	virtual BOOL GetSnapActive() = 0;
	virtual void SetSnapActive(BOOL onOff) = 0;
	virtual int  GetSnapType() = 0;
	/// See 'Snap types' in snap.h for possible values of 'type'
	virtual void SetSnapType(int type) = 0;
	virtual void ToggleASnap() = 0;
	virtual int ASnapStatus() = 0;
	virtual void TogglePSnap() = 0;
	virtual int PSnapStatus() = 0;
	virtual float GetGridSpacing() = 0;
	virtual void SetGridSpacing(float newVal) = 0;
	virtual int GetGridMajorLines() = 0;
	virtual void SetGridMajorLines(float newVal) = 0;
	virtual float GetSnapAngle() = 0;
	virtual void SetSnapAngle(float newVal) = 0;
	virtual float GetSnapPercent() = 0;
	virtual void SetSnapPercent(float newVal) = 0;

	// --- Node Properties ------------------------------------------------- ///
	/*! \brief Set the given attribute on the passed nodes
	\param nodes - An array of nodes to set the attribute on
	\param whatAttrib - One of the list of \ref interfaceSetNodeAttributes
	\param onOff - If true the attribute is set on all the nodes, if it is false, the attribute is cleared */
	virtual void SetNodeAttribute(INodeTab &nodes, int whatAttrib, int onOff) = 0;

	/*! \brief Set the given attribute on the passed node
	\param node - A pointer to the node to set the attribute on
	\param whatAttrib - One of the list of \ref interfaceSetNodeAttributes
	\param onOff - If true the attribute is set on all the nodes, if it is false, the attribute is cleared */
	virtual void SetNodeAttribute(INode *node, int whatAttrib, int onOff) = 0;

	// --- Custom and General UI ------------------------------------------- ///
	virtual void SetExpertMode(int onOff) = 0;
	virtual int GetExpertMode() = 0;
	virtual void LoadCUIConfig(const MCHAR* fileName) = 0;
	virtual void WriteCUIConfig() = 0;
	virtual void SaveCUIAs() = 0;
	virtual void LoadCUI() = 0;
	virtual void RevertToBackupCUI() = 0;
	virtual void ResetToFactoryDefaultCUI() = 0;
	virtual void DoUICustomization(int startPage) = 0;

	virtual int GetDefaultImageListBaseIndex(SClass_ID sid, Class_ID cid) = 0;
	virtual MSTR* GetDefaultImageListFilePrefix(SClass_ID sid, Class_ID cid) = 0;

	virtual float GetGridIntens() = 0;
	virtual void SetGridIntens(float f) = 0;
	virtual BOOL GetWhiteOrigin() = 0;
	virtual void SetWhiteOrigin(BOOL b) = 0;
	virtual BOOL GetUseGridColor() = 0;
	virtual void SetUseGridColor(BOOL b) = 0;
	virtual void UpdateColors(
		BOOL useGridColor, 
		int gridIntensity, 
		BOOL whiteOrigin) = 0;

	virtual IMenu* GetIMenu() = 0;
	virtual IMenuItem* GetIMenuItem() = 0;

	virtual void RepaintTimeSlider() = 0;

	virtual MSTR GetTabPageTitle(ITabPage *page) = 0;

	// --- File Open/Save Dialogs ------------------------------------------ ///
	virtual BOOL DoMaxFileSaveAsDlg(MSTR &fileName, BOOL setAsCurrent=TRUE) = 0;
	virtual BOOL DoMaxFileOpenDlg(MSTR &fileName, MSTR *defDir, MSTR *defFile) = 0;
	virtual BOOL DoMaxFileMergeDlg(MSTR &fileName, MSTR *defDir, MSTR *defFile) = 0;
	virtual BOOL DoMaxFileXRefDlg(MSTR &fileName, MSTR *defDir, MSTR *defFile) = 0;

	// --- Animation ------------------------------------------------------- ///
	/// Old style playback with no immediate return
	virtual void StartAnimPlayback2(int selOnly) = 0; 

	// --- Modifiers ------------------------------------------------------- ///
	enum ResCode
	{
		kRES_INTERNAL_ERROR = -3,
		kRES_MOD_NOT_FOUND = -2,
		kRES_MOD_NOT_APPLICABLE = -1,
		kRES_SUCCESS = 0,
	};
	/// Tests of modifier applicability
	virtual BOOL IsValidModForSelection(ClassEntry* ce) = 0;
	/// Returns FALSE if the specified modifier cannot be applied to the object
	virtual BOOL IsValidModifier(INode& node, Modifier& mod) = 0;

	/// Adds the specified modifier to a node.
	/// INode& node - the node the modifier will be added to
	/// Modifier& mod - the modifier instance that will be applied to node
	/// int beforeIdx - the index in the modstack where the modifier should
	///									be inserted. 0 means at the top of the modstack 
	///									(just below the node)
	/// If the object's stack doesn't have beforeIdx number of modifiers in it,
	/// the modifier is added at the lowest possible index (just on top of the 
	/// base object)
	/// Returns one of these error codes:
	/// kRES_INTERNAL_ERROR - if an error such as invalid pointer is encountered
	/// kRES_MOD_NOT_APPLICABLE - if the modifier is not applicable
	/// kRES_SUCCESS - on success
	virtual ResCode AddModifier(INode& node, Modifier& mod, int beforeIdx = 0) = 0;

	/// Deletes the first occurrence of the specified modifier from the node's 
	/// modifier stack. Returns one of the following values: kRES_INTERNAL_ERROR,
	/// kRES_MOD_NOT_FOUND or kRES_SUCCESS
	virtual ResCode DeleteModifier(INode& node, Modifier& mod) = 0;

	/// Deletes the modifier at the specified index from the node's modifier stack
	/// Returns one of the following values: kRES_INTERNAL_ERROR,
	/// kRES_MOD_NOT_FOUND or kRES_SUCCESS
	virtual ResCode DeleteModifier(INode& node, int modIdx) = 0;

	/// Finds the first occurrence of the specified modifier on a node. 
	/// It returns a pointer to the derived object to which this modifier belongs,
	/// the index of the modifier within this derived object and the index within 
	/// the modifier stack. If the modifier is not found, returns NULL. 
	/// It searches the WS, then the OS part of the geom pipeline of the node.
	virtual IDerivedObject* FindModifier(
		INode& node, 
		Modifier& mod, 
		int& modStackIdx, 
		int& derivedObjIdx) = 0;

	/// Finds the modifier at the specified modstack index on a node. 
	/// It returns a pointer to the derived object to which this modifier belongs, 
	/// the index of the modifier within this derived object, and a pointer to the 
	/// modifier itself. If the modifier is not found, returns NULL. 
	/// It searches both WS and OS part of the geom pipeline of the object
	virtual IDerivedObject* FindModifier(
		INode& node, 
		int modIdx, 
		int& idx, 
		Modifier*& mod) = 0;

	/// Finds the index of a modifier instance in a modstack of a node, when 
	/// the node, the modifier it's corresponding cod context are given.
	/// It returns a pointer to the derived object to which this modifier belongs, 
	/// the index of the modifier within this derived object, and a pointer to the 
	/// modifier itself. If the modifier is not found, returns NULL. It searches 
	/// both WS and OS part of the geom pipeline of the object node, mod and mc 
	/// are input parameters, while modStackIdx and dobjidx are output params
	virtual IDerivedObject* FindModifier(
		INode& node, 
		Modifier& mod, 
		ModContext& mc, 
		int& modStackIdx, 
		int& dobjidx) = 0;

	virtual ResCode DoDeleteModifier(INode& node, IDerivedObject& dobj, int idx) = 0;

	/// A node's reference to its object should be replaced using this method. 
	/// Returns NULL when the operation cannot be completed, such as for Actively 
	/// (File) Linked objects or their user created clone-instances / references.
	virtual Object* GetReplaceableObjRef(INode& node) = 0;

	// --- Render Effects -------------------------------------------------- ///
	virtual void OpenEnvEffectsDialog() = 0;
	virtual void CloseEnvEffectsDialog() = 0;
	virtual BOOL EnvEffectsDialogOpen() = 0;

}; // class Interface7

//! \brief The interface class for max version 8.
/*! This interface should always be 
	retrieved over Interface or Interface7 when programming against version 8
	of the application.  It features new functionality and deprecates obsolete functions. */
class Interface8 : public Interface7
{
public:

	//! \brief The ID for this interface.  Pass this ID to Interface::GetInterface to get an Interface8 pointer.
	CoreExport static const Interface_ID kInterface8InterfaceID;

	// ---------- Rendering ------------
	//! \name Rendering
	//!@{

	//! \brief Launches a Quick Render, as though pressing the Quick Render button
	/*! \param[in] t The time to render the image.
	    Pass TIME_PosInfinity or TIME_NegInfinity to use the current slider time
	    \param[in] rendbm The bitmap to render to, or NULL for default handling.
	    If a bitmap is provided, the caller is responsible for displaying the VFB and saving the file.
	    \param[in] prog The RendProgressCallback is an optional callback.
	    Pass NULL for the default render progress dialog
	    \return The result of the render - Nonzero if success; otherwise 0. */
	virtual int QuickRender( TimeValue t=TIME_PosInfinity, Bitmap* rendbm=NULL, RendProgressCallback* prog=NULL )=0;
	//! \brief Enumerates the frames for rendering, as specified in the Render Dialog
	/*! \param[out] frameNums The frame number list; this is resized and set by the callee */
	virtual void GetRendFrameList(IntTab& frameNums)=0;
	//! \brief Returns the custom progress callback, if any, used for rendering through the UI.
	/*! If NULL, the renderer will use its default progress dialog */
	virtual RendProgressCallback* GetRendProgressCallback()=0;
	//! \brief Sets a custom progress callback, used for rendering through the UI.
	/*! This will be used when performing a render through the UI.
	    If set to NULL, the renderer will use its default progress dialog
	    \param[in] prog The custom progress callback */
	virtual void SetRendProgressCallback( RendProgressCallback* prog )=0;
	//! \brief Returns the custom camera node, if any, used for rendering through the UI.
	/*! If NULL, the renderer will render from a viewport by default */
	virtual INode* GetRendCamNode()=0;
	//! \brief Sets a custom camera node, used for rendering through the UI.
	/*! If NULL, the renderer will render from a viewport by default, but
	    this allows for specifying a perspective other than one of the viewports.
	    \param[in] camNode The custom camera node */
	virtual void SetRendCamNode( INode* camNode )=0;
	//! \brief Returns the "Put Image File List(s) in Output Path(s)" setting in the render dialog.
	virtual BOOL GetRendUseImgSeq()=0;
	//! \brief Sets the "Put Image File List(s) in Output Path(s)" setting in the render dialog.
	/*! When enabled, a sequence file (in .imsq or .ifl format) will be saved for frames stored
	    during the render, including separate sequence files for the frames of each render element
	    \param[in] onOff TRUE to enable writing of sequence files, FALSE to disable */
	virtual void SetRendUseImgSeq(BOOL onOff)=0;
	//! \brief Returns the file format used when the renderer creates image sequence files.
	/*! For .imsq files this is 0, otherwise for .ifl files this is 1 */
	virtual int GetRendImgSeqType()=0;
	//! \brief Sets the file format used when the renderer creates image sequence files.
	/*! \param[in] type The format type. For .imsq files pass 0, otherwise for .ifl files pass 1 */
	virtual void SetRendImgSeqType(int type)=0;
	//! \brief Saves image sequence files (in .imsq or .ifl format) based on the current output settings.
	/*! Equivalent to pressing the "Create Now" button for image sequence files in the render dialog
	    \param[in] cb An optional callback object to be called for each sequence file created */
	virtual void CreateRendImgSeq( CreateRendImgSeqCallback* cb=NULL )=0;
	//! \brief Returns the AssetUser of the Pre-Render Script as indicated in the render dialog
	virtual const MaxSDK::AssetManagement::AssetUser& GetPreRendScriptAsset()=0;
	//! \brief Sets the AssetUser of the Pre-Render Script as shown in the render dialog.
	/*! For non-network rendering, the script is executed once before rendering begins.
	    For network rendering, if the Execute Locally is enabled, the script is executed once, before submission;
	    otherwise the script is executed once on each remote machine the job is assigned to.
	    The script is executed before any pre-render notifications, but after sequence files (.imsq or .ifl) are written
	    \param[in] script The filename to set for the the pre-render script */
	virtual void SetPreRendScriptAsset( const MaxSDK::AssetManagement::AssetUser& script )=0;
	//! \brief Returns the Enable setting for the Pre-Render Script in the render dialog
	virtual BOOL GetUsePreRendScript()=0;
	//! \brief Sets the Enable setting for the Pre-Render Script in the render dialog.
	/*! \param[in] onOff TRUE to enable the pre-render script, FALSE to disable */
	virtual void SetUsePreRendScript( BOOL onOff )=0;
	//! \brief Returns the Execute Locally setting for the Pre-Render Script in the render dialog
	virtual BOOL GetLocalPreRendScript()=0;
	//! \brief Sets the Execute Locally setting for the Pre-Render Script in the render dialog.
	/*! This setting affects network rendering, causing the script to be executed once before submission,
	    instead of once per machine the job is assigned to.
	    \param[in] onOff TRUE to enable local execution of the pre-render script, FALSE to disable */
	virtual void SetLocalPreRendScript( BOOL onOff )=0;
	//! \brief Returns the AssetUser of the Post-Render Script as indicated in the render dialog
	virtual const MaxSDK::AssetManagement::AssetUser& GetPostRendScriptAsset()=0;
	//! \brief Sets the filename of the Post-Render Script as shown in the render dialog.
	/*! The script is executed after any post-render notifications.
	    \param[in] script The filename to set for the the post-render script */
	virtual void SetPostRendScriptAsset( const MaxSDK::AssetManagement::AssetUser& script )=0;
	//! \brief Returns the Enable setting for the Post-Render Script in the render dialog
	virtual BOOL GetUsePostRendScript()=0;
	//! \brief Sets the Enable setting for the Post-Render Script in the render dialog.
	/*! \param[in] onOff TRUE to enable the post-render script, FALSE to disable */
	virtual void SetUsePostRendScript( BOOL onOff )=0;
	//!@}

	// ---------- Animation Preferences ------------

	//! \name Animation
	//!@{
	//! \brief Retrieves whether or not newly created controllers will respect animation range
	/*!	Retrieves the default value of the animation preference which determines 
		whether the active range of parametric controllers will be respected or not.
		This preference does not affect keyable controllers.
		When range is respected, the controller evaluation at a time before the
		activation range start time will return the value at start time and 
		evaluation after activation range end time, the value at end time.
		Users can see the activation range of controllers as a black line in the 
		Track View, in Dope Sheet mode, when Edit Ranges is on.

		\return - TRUE if by default, the animation range of controllers is not respected, FALSE otherwise */
	virtual BOOL GetControllerOverrideRangeDefault() = 0;
	//! \brief Sets whether or not newly created controllers will respect animation range
	/*! Sets the default value of the animation preference which determines whether 
		the active range of parametric controllers will be respected or not
		\see GetControllerOverrideRangeDefault()
		\param override - If FALSE, the active range of parametric controllers will 
		be respected by default, otherwise it won't */
	virtual void SetControllerOverrideRangeDefault(BOOL override) = 0;

	/*! Get the default tangent type for both the "In" and the "Out" tangent.
		This tangent type is the one that gets applied to any new animation key created in Max.
		\param[out] dfltInTangentType - default type for the "In" tangent.
		\param[out] dfltOutTangentType - default type for the "Out" tangent. */
	virtual void GetDefaultTangentType(int &dfltInTangentType, int &dfltOutTangentType) = 0;
	/*! Set the default tangent type for both the "In" and the "Out" tangent.
		This tangent type will get set on any animation key created in Max.
		\param[in] dfltInTangentType - default type for the "In" tangent.
		\param[in] dfltOutTangentType - default type for the "Out" tangent.
		\param[in] writeInCfgFile - TRUE if tangent type values have to be written in the config file, FALSE otherwise. */
	virtual void SetDefaultTangentType(int dfltInTangentType, int dfltOutTangentType, BOOL writeInCfgFile = TRUE) = 0;

	//! \brief Returns whether quick manipulation mode for spring controllers is on
	/*!	Retrieves the animation preference controlling whether spring systems
		used in spring controllers are in quick edit mode.  The default is to
		be OFF.  If turned on, then when something invalidates a spring 
		controller, instead of recomputing the results from start as would be
		required to get the correct results, it resets the system a certain
		number of frames back (see GetSpringRollingStart() below).  This can
		make a big difference in interactivity.
		\return - TRUE if the spring controllers are in quick edit mode */
	virtual BOOL GetSpringQuickEditMode() const = 0;
	//! \brief Sets quick manipulation mode for spring controllers 
	/*!	Sets the animation preference controlling whether spring systems used
		in spring controllers will be accurate at all times.
		\param[in] in_quickEdit - turn on spring quick edit mode. */
	virtual void SetSpringQuickEditMode(BOOL in_quickEdit) = 0;

	//! \brief Sets the rolling start value of the quick manipulation mode for spring controllers 
	/*!	Sets the animation preference controlling how many frames back the 
		spring controllers will use as a rolling starting point for simulation 
		if invalidated, if the Quick Edit option is on (see SetSpringQuickEditMode
		above).
		\param[in] in_start - the number of frames back when restarting.
		\see SetSpringQuickEditMode */
	virtual void SetSpringRollingStart(int in_start) = 0;
	//! \brief Returns the rolling start value of the quick manipulation mode for spring controllers 
	/*!	Returns the animation preference controlling how many frames back the
		spring controllers will use as a rolling starting point for simulation 
		\see GetSpringQuickEditMode */
	virtual int GetSpringRollingStart() const = 0;
	//!@}

	//! \brief This method returns the color corresponding to the id in input

	/*! This function represents the color-id mapping that is done for :
	 \li The Material Id render element
	 \li The Object Id render element
	 \li The Material Effects and Object channels of the rpf format
	 \param[in] id - the id for which we want the color
	 \param[out] c - the color corresponding to the id
	*/
	virtual void ColorById(DWORD id, Color& c) = 0 ;

	//! Register a callback object to be called during max shutdown.
	//! \see ExitMAXCallback2, ExitMAXCallback
	//! \param[in] cb - the ExitMAXCallback2 object
	virtual void RegisterExitMAXCallback(ExitMAXCallback2 *cb)=0;

	//! Unregister a callback object to be called during max shutdown.
	//! \param[in] cb - the ExitMAXCallback2 object
	virtual void UnRegisterExitMAXCallback(ExitMAXCallback2 *cb)=0;

	//! \brief Opens a max-style File Save As dialog for generic file types.

	//! Launches a generic File Save As dialog which supports arbitrary file
	//! types.  The dialog includes 3ds Max specific browsing features such as browse
	//! history. When this function returns it will save the new filter 
	//! index in FilterList's data member m_newFilterIndex.  This
	//! will allow developers to save the new index by calling FilterList::GetNewFilterIndex()
	//! and thus setting the correct filter index (calling FilterList::SetFilterIndex())
	//! when this function is called again.  This makes sure the dialog is showing the correct 
	//! file extension to be used.
	//! \post filename contains the long filename user selection if this function
	//! returns "true"; initialDir contains the path of the file selected, if this
	//! function returns \b true, otherwise it contains the original contents passed-in
	//! \param[in] parentWnd The window handle which should be this dialog's parent window.
	//! \param[in] title The string to be set as the title of this dialog.
	//! \param[in, out] filename As an in parameter, can contain the long or short name
	//! of the file that should be default selection of the dialog.  As an out parameter,
	//! filename contains the long name of the file selected by the user, if this function
	//! returns \b true
	//! \param[in, out] initialDir As an in parameter, contains the initial dialog 
	//! directory.  If the user clicks OK, then this parameter contains the user
	//! selected directory path.
	//! \param[in] extensionList A list of extensions supported by this dialog.  See
	//! FilterList documentation for details.
	//! \return \b true if the user makes an acceptable choice, false if canceled
	virtual bool DoMaxSaveAsDialog(	HWND parentWnd, 
									const MSTR &title, 
									MSTR &filename, 
									MSTR &initialDir, 
									FilterList &extensionList) =0;

	//! \brief Opens a max-style Open dialog for generic file types.

	//! Launches a generic Open dialog which supports arbitrary file
	//! types.  The dialog includes 3ds Max specific browsing features such as browse
	//! history. When this function returns it will save the new filter 
	//! index in FilterList's data member m_newFilterIndex.  This
	//! will allow developers to save the new index by calling FilterList::GetNewFilterIndex()
	//! and thus setting the correct filter index (calling FilterList::SetFilterIndex())
	//! when this function is called again.  This makes sure the dialog is showing the correct 
	//! file extension to be used.
	//! \post filename contains the long filename user selection if this function
	//! returns \b true; initialDir contains the path of the file selected, if this
	//! function returns "true", otherwise it contains the original contents passed-in
	//! \param[in] parentWnd The window handle which should be this dialog's parent window.
	//! \param[in] title The string to be set as the title of this dialog.
	//! \param[in, out] filename As an in parameter, can contain the long or short name
	//! of the file that should be default selection of the dialog.  As an out parameter,
	//! filename contains the long name of the file selected by the user, if this function
	//! returns \b true
	//! \param[in, out] initialDir As an in parameter, contains the initial dialog 
	//! directory.  If the user clicks OK, then this parameter contains the user
	//! selected directory path.
	//! \param[in] extensionList A list of extensions supported by this dialog.  See
	//! FilterList documentation for details.
	//! \return \b true if the user makes an acceptable choice, false if canceled
	virtual bool DoMaxOpenDialog(	HWND parentWnd, 
									const MSTR &title,
									MSTR &filename, 
									MSTR &initialDir, 
									FilterList &extensionList) =0;

	//! \brief Makes a window modeless during a render.

	//! When a render is in progress, window messages to all windows but the virtual frame buffer
	//! and the progress window are suppressed in order to make the render operation modal.
	//! This method may be used to make a window modeless during the render operation.
	//! All messages sent to the given window handle will no longer be suppressed by the render
	//! executer.
	//!
	//! Note: A modeless render dialog should, ideally, only display certain statistics or messages.
	//! It is unsafe to do any complex operation or user interaction from a modeless dialog while
	//! rendering. That is why 3ds max blocks most window messages while rendering by default.
	//!
	//! PS: Be sure to un-register your window with UnRegisterModelessRenderWindow() when
	//! it is destroyed.
	//! \param[in] hWnd Handle to the window to be registered.
	virtual void RegisterModelessRenderWindow(HWND hWnd) =0;
	//! \brief Un-registers a window registered with RegisterModelessRenderWindow().
	//! See the documentation of RegisterModelessRenderWindow() for more details.
	//! \param[in] hWnd Handle to the window to be un-registered.
	virtual void UnRegisterModelessRenderWindow(HWND hWnd) =0;
	/*! \brief Returns true if a window is currently registered with RegisterModelessRenderWindow().
	* See the documentation of RegisterModelessRenderWindow() or UnRegisterModelessRenderWindow()
	* for more details.
	* \param[in] hWnd Handle to the window for which the information is desired.
	*/
	virtual bool IsRegisteredModelessRenderWindow(HWND hWnd) = 0;

	//! \brief Returns whether a file save operation is currently in progress.
	//! \return true if a file save operation is in progress.
	virtual bool IsSavingToFile() = 0;

	//! \brief Flags for LoadFromFile method
	/*! \see Interface8::LoadFromFile
	*/
	enum LoadFromFileFlags {
		/*! When this flag is set, the viewports are redrawn once the file is loaded. 
		*/
		
		kRefreshViewports	= 0x01,
		/*! When this flag is set, messages about file unit mismatch and obsolete 
		file versions will be suppressed.
		*/
		
		kSuppressPrompts = 0x02,
		/*! This flag is only valid in conjunction with kSuppressPrompts. If the current 
			scene's system units differ from those of the scene's being loaded, turning 
			on this flag will cause the system unit scale of the incoming scene to be used 
			(objects that are loaded are not scaled).	Otherwise, the incoming scene's objects 
			will be rescaled to the current scene's system unit.
		*/
		kUseFileUnits	= 0x04,
		
		/*! When this flag is set, the current file path is set to the one of the file 
		that was loaded and the name of the file is displayed in the 3ds Max's title bar.
		Otherwise, the current file path and the title bar are not modified. If the file is
		not loaded successfully, or 3ds Max is working in network rendering mode, 
		the current path and title bar are not modified.
		\note Turning this flag off (not specifying it) has no effect in 3ds Max 2008.
		*/
		kSetCurrentFilePath	= 0x08
	};

	//! \brief Loads the specified scene file.

	//! \see Interface8::LoadFromFileFlags
	//! \param[in] szFilename the file to load
	//! \param[in] lFlags combination of Interface8::LoadFromFileFlags flags.
	//! \return true if success, false if there was an error
	virtual bool LoadFromFile( const MCHAR* szFilename, unsigned long lFlags ) = 0;

	//! \brief Opens a Windows Explorer window pointing to the passed in path

	//! This is a utility function for opening a Windows Explorer window with
	//! a given path.  The path can point to a folder, or it can be a full path
	//! to a file.
	//! If the file path doesn't exist, a warning dialog will open indicating that the path
	//! does not exist and the explorer will not open.
	//! The explorer is executed using a ShellExecute command, and is independent of
	//! the calling application.  In other words, shutting down max will not result in the 
	//! explorer app being shut down.
	//! \pre An absolute path value <= 256 characters.  A zero length path will result in the explorer
	//! opening to the system-default drive (most likely C:\).  An invalid path will result
	//! in a warning dialog, as described above.
	//! \post An explorer window opens to the location specified by the path.
	//! \param[in] path An absolute path for the location used as the default browse location of the explorer.
	//! \return true if shell function returns a success code
	virtual bool RevealInExplorer(const MSTR& path) =0;
}; // Interface8

//! \brief The interface class for max version 9.
/*! This interface should always be 
	retrieved over Interface, Interface7, or Interface8 when programming against version 9
	of the application.  It features new functionality and deprecates obsolete functions. */
class Interface9 : public Interface8	{
public:
	//! \brief The ID for this interface.  Pass this ID to Interface::GetInterface to get an Interface9 pointer.
	CoreExport static const Interface_ID kInterface9InterfaceID;

	//! \brief Opens a application-style Open dialog for generic file types.
	/*!	Launches the standard Browse For Folder dialog in max.
		\param[in] aOwner The handle to the window to whom this browse
		dialog should be a child to.
		\param[in] aInstructions An parameter for specifying user instructions
		which will appear above the main selection area.  These instructions can span
		multiple lines.  If this parameter is an empty string, then default instructions 
		will be specified.
		\param[in, out] aDir This parameter is used to specify both the initial 
		browse directory, and the user selection.  If the user cancels the selection,
		then this variable will not be modified.  If this parameter is an empty string, 
		or if a non-existing directory is passed in, the Windows control will default to
		the My Documents directory.
		\return true if the user accepts a selection, false if the user cancels
	 */
	virtual bool DoMaxBrowseForFolder(
		HWND aOwner, 
		const MSTR& aInstructions,
		MSTR& aDir) = 0;

	//!	\brief Deletes the specified nodes from the scene.
	/*!	Call this method to delete the specific nodes from the scene. The result of
	this operation is the same as deleting the nodes interactively or via maxscript's
	"delete <node>" command. If you need to delete individual nodes, use Interface::DeleteNode.
	\li The children of the deleted nodes will maintain their position. 
	\li Empty group heads resulting from the deletion of the nodes will be deleted. 
	\li If a system node is deleted, the whole system is deleted.
	\li If target light / camera nodes are deleted, their target nodes are also deleted.
	\li If the target of a target light / camera node is deleted, the light / camera node is also deleted.
	\li If a closed group / assembly head node is deleted, the whole group/ assembly 
	is deleted.
	\li The selection state and deletion of the nodes will be undo-able after calling 
	this method, provided that the undo	system was engaged.
	\param [in] aNodes The nodes to be deleted. This array is expanded with other nodes
	based on the criteria described above. After this method returns, the nodes in this 
	array are invalid.
	\param [in] aKeepChildTM If true, the child nodes of each deleted node keep their 
	world transformation. Note that this modifies each child node's transformation matrix.
	If false, the node's transformation matrix is not modified, but that means they'll
	get transformed relative to the world since their parent node is changing.
	\param [in] aRedraw If false the viewports will not be redrawn after the nodes  
	are deleted. This allows client code to optimize the viewport redraw.
	\param [in] overrideDrivenTM If true, this method will delete nodes whose 
	TM controllers implement Control::PreventNodeDeletion() to return TRUE. 
	Control::PreventNodeDeletion() controls whether the node can be deleted from 
	the scene. This parameter allows a driver controller to easily delete driven nodes 
	if it needs to.
	\return The number of nodes that were deleted. 
	*/
	virtual unsigned int DeleteNodes(
		INodeTab& aNodes,			
		bool aKeepChildTM = true,
		bool aRedraw = true,
		bool overrideDrivenTM = false
	) = 0;
}; // Interface9

/*! \brief  Extends Interface9 
Client code should retrieve this interface using GetCOREInterface10
*/
class Interface10 : public Interface9
{
public:
	
	//! \brief The ID for this interface.  Pass this ID to Interface::GetInterface to get an Interface10 pointer.
	CoreExport static const Interface_ID kInterface10InterfaceID;

	// ---------- Trajectory Modes ------------
	//! \name Trajectory
	//!@{

	/*! \brief Get whether or not if we are in the trajectory mode in the motion panel
	\return true if we are in trajectory mode, false otherwise.	
	*/
	virtual bool GetTrajectoryMode()const =0;
	/*! \brief Set whether or not we are in the trajectory mode in the motion panel.
	\param [in] flag If true then we enter the trajectory mode in the motion panel, otherwise if false,
	and we are in the trajectory mode, we leave trajectory mode.
	*/
	virtual void SetTrajectoryMode(bool flag) =0;
	/*! \brief Get whether or not the trajectory is in the key subobject mode so that keys may be edited.
	\return true if the trajectory is in key subobject mode  is on, else false.	
	*/
	virtual bool GetTrajectoryKeySubMode()const =0;
	/*! \brief Set whether or not the trajectory is in the key subobject mode or not.
	\param [in] flag If true then we enter the key subobject mode with trajectories enabled, if not already in that mode. If false and we are in the key SubObject
	mode, then we exit this subobject mode.
	*/
	virtual void SetTrajectoryKeySubMode(bool flag) =0;
	/*! \brief Get whether or not the trajectory is in the add mode of the key subobject mode so that keys may be added by interactively
	pressing left-click on the trajectory.
	\return true if the trajectory is in mode to add keys, else false.	
	*/
	virtual bool GetTrajectoryAddKeyMode() const =0;
	/*! \brief Set whether or not the trajectory is in the add mode of the key subobject mode or not so that keys may be added by interactively
	pressing left-click on the trajectory.
	\param [in] flag If true then we enter the add mode of the key subobject mode with trajectories enabled, if not already in that mode. If false and we are
	in the key subobject mode, then we exit the add mode, if currently in this mode, otherwise nothing occurs.
	*/
	virtual void SetTrajectoryAddKeyMode(bool flag) =0;
	//! \brief Delete the selected trajectory keys.  
	/*! This function will only do something if trajectories are enabled and we are in the key subobject
	mode with selected keys.
	*/
	virtual void DeleteSelectedTrajectoryKey() =0;
	//!@}

	// ---------- Animation Preferences ------------
	//! \name Animation
	//!@{
	/*!	Retrieves the animation preference controlling whether or not a default key is set when animating with Auto Key enabled on a controller with no keys.
	If it returns TRUE then a default key will get set, otherwise it won't.  Note that biped controllers aren't affected by this flag since biped
	controllers never set a default key.
	\return TRUE if a default key will get set when auto keying */
	virtual BOOL GetAutoKeyDefaultKeyOn() const = 0;
	/*!	Sets the animation preference controlling whether or not a default key is set when animating with Auto Key enabled on a controller with no keys.
	Note that biped controllers aren't affected by this flag since biped controllers never get a default key.
	\param[in] setKey Specify whether or a default key will get set when auto keying on a controller that currently has no keys on it. */
	virtual void SetAutoKeyDefaultKeyOn(BOOL setKey) = 0;

	/*!	Returns the animation preference controlling at what time a default key will get set when auto key is on. Note that default key option has to be on
	in order for the default key to get set at the specified time.
	\return The time that the default key will get set when auto keying. 
	\see Interface10::GetAutoKeyDefaultKeyOn */
	virtual TimeValue GetAutoKeyDefaultKeyTime() const = 0;
	/*!	Sets the animation preference controlling at which time a default key will get set when auto key is on. Note that the default key option has to be on
	in order for the default key to get set at the specified time.
	\param[in] t The time at which the default key will get set when auto keying.  Currently this value will be forced to either be frame 0 or frame 1
	\see Interface10::GetAutoKeyDefaultKeyOn */
	virtual void SetAutoKeyDefaultKeyTime(TimeValue t) = 0;
	//!@}

	/*! \brief List of node properties.
	Interface10::FindNodes accepts a combination of these values and uses 
	them to match nodes in the current scene.
	*/
	enum ENodeProperties
	{
		/*! \brief Represents the material property of a node */
		kNodeProp_Material = 1<<0,
		/*! \brief Represents the layer property of a node */
		kNodeProp_Layer = 1<<1,
		/*! \brief Represents all node properties listed above */
		kNodeProp_All = (kNodeProp_Material | kNodeProp_Layer),
	};
	//! \brief Finds nodes in the current scene that are similar to a set of "template" nodes. 
	/*! Two or more nodes are considered similar if they are of the same "type" and 
	their node properties match (see Interface10::ENodeProperties for a list of supported
	node properties)
	Nodes are considered of the same "type" if they are instances of the same 
	ADT Style, or Autocad Block, or if they have the same class and superclass ID.
	Targets of cameras and lights are considered of the same type if their camera or 
	light has the same class and superclass ID.
	\param templateNodes - nodes similar to these ones need to be found
	\param foundNodes - the nodes that were found to be similar to the 
	"template" nodes. This list of nodes contains no duplicates.
	\param nodePropsToMatch - Flag encoding the node properties to be matched. 
	Only nodes whose property specified by this flag matches will be considered similar.
	\see enum ENodePropeties
	\remarks Use Interface::SelectNodeTab to select all the similar nodes
	*/
	virtual void FindNodes(
		const Tab<INode*>& templateNodes, 
		Tab<INode*>& foundNodes,
		const unsigned int nodePropsToMatch = kNodeProp_All) = 0;

	//! \brief Returns the string on the top of the prompt stack.
	virtual MSTR GetPrompt() = 0;

	//! \brief Formats a time value into a string, as shown in the render progress dialog.
	virtual void FormatRenderTime(DWORD msec, MSTR& str, BOOL hundredths=FALSE) = 0; 

}; // Interface 10



/*! \brief  Extends Interface10 
Client code should retrieve this interface using GetCOREInterface11
*/
class Interface11 : public Interface10
{
public:

	//! \brief The ID for this interface.  Pass this ID to Interface::GetInterface to get an Interface11 pointer.
	CoreExport static const Interface_ID kInterface11InterfaceID;

	//! \brief Attempt to open an existing max file as a structured storage file.
	/*! A max file is stored as a structured storage file. While the streams contain binary data not usable by developers,
	property values can be accessed through the standard structured storage property interfaces. These properties include
	those seen in the File/File Properties dialog.
	\param[in] filename A pointer to the path of the null-terminated Unicode string file that contains the storage object.
	This string size cannot exceed MAX_PATH characters.
	\param[out] pIStorage The address of an IStorage pointer variable that receives a pointer for an IStorage interface on 
	the storage object opened; contains NULL if operation failed
	\return  <b>S_OK</b> - Indicates that the storage object was opened successfully.\n\n
	<b>STG_E_INVALIDPOINTER</b> - Indicates an invalid pointer in the pIStorage parameter.\n\n
	<b>E_NOINTERFACE</b> - Indicates that the IStorage interface is not supported.\n\n
	<b>STG_E_INVALIDNAME</b> - Indicates an invalid name in the filename parameter.\n\n
	<b>STG_E_LOCKVIOLATION</b> - Indicates that access was denied because another caller has the file open and locked.\n\n
	<b>STG_E_SHAREVIOLATION</b> - Indicates that access was denied because another caller has the file open and locked.\n\n
	<b>STG_E_ACCESSDENIED</b> - Indicates that the file cannot be opened because the underlying storage device does not allow such access to the current user.\n\n
	This function can also return any file system errors or Win32 errors wrapped in an HRESULT.
	\par Sample Code:
	The following is an example of accessing several properties through the standard structured storage property interfaces.\n\n
	\code
	IStorage*				pIStorage = NULL;
	IPropertySetStorage*	pPropertySetStorage = NULL;
	HRESULT	res = GetCOREInterface11()->OpenMAXStorageFile(wfilename, &pIStorage);
	if (res!=S_OK)
		return;

	// Get the Storage interface
	if (S_OK != pStorage->QueryInterface(IID_IPropertySetStorage, (void**)&pPropertySetStorage)) {
		pStorage->Release();
		return;
		}

	// Get the SummaryInfo property set interface
	if (S_OK == pPropertySetStorage->Open(FMTID_SummaryInformation, STGM_READ|STGM_SHARE_EXCLUSIVE, &pSummaryInfoStorage)) {

		PROPSPEC	PropSpec[3];
		PROPVARIANT	PropVar[3];

		PropSpec[0].ulKind = PRSPEC_PROPID;
		PropSpec[0].propid = PID_TITLE;

		PropSpec[1].ulKind = PRSPEC_PROPID;
		PropSpec[1].propid = PID_SUBJECT;

		PropSpec[2].ulKind = PRSPEC_PROPID;
		PropSpec[2].propid = PID_AUTHOR;

		HRESULT hr = pSummaryInfoStorage->ReadMultiple(3, PropSpec, PropVar);
		if (S_OK == hr) {
			if (PropVar[0].vt == VT_LPSTR)
				SendMessage(GetDlgItem(hPropDialog, IDC_TITLE), WM_SETTEXT, 0, (LPARAM)PropVar[0].pszVal);
			if (PropVar[1].vt == VT_LPSTR)
				SendMessage(GetDlgItem(hPropDialog, IDC_SUBJECT), WM_SETTEXT, 0, (LPARAM)PropVar[1].pszVal);
			if (PropVar[2].vt == VT_LPSTR)
				SendMessage(GetDlgItem(hPropDialog, IDC_AUTHOR), WM_SETTEXT, 0, (LPARAM)PropVar[2].pszVal);
			}

		FreePropVariantArray(3, PropVar);
		pSummaryInfoStorage->Release();
		}
	pStorage->Release();
	return;
	\endcode  */
	virtual HRESULT OpenMAXStorageFile(const WCHAR* filename, IStorage** pIStorage) = 0; 

	//! \brief Returns TRUE if the rendering uses the active viewport, FALSE if locked on a viewport
	/*! This corresponds to the lock button for the viewport dropdown in the render dialog. */
	virtual BOOL GetRendUseActiveView() = 0;
	//! \brief Sets whether the rendering uses the active viewport, or is locked on a viewport
	/*! This corresponds to the lock button for the viewport dropdown in the render dialog.
		\param[in] useActiveView Pass TRUE to use the active viewport, FALSE to lock to a viewport */
	virtual void SetRendUseActiveView( BOOL useActiveView ) = 0;

	//! \brief Returns the number of render presets in the render dialog's most-recently-used render preset list
	virtual int GetRenderPresetMRUListCount() = 0;
	//! \brief Returns the display name of an entry in the render dialog's mostly-recently-used render preset list
	/*!	If an invalid index is passed, it will wrap around to a valid index, since the MRU list contains a fixed number of items.
		\param[in] i The index within the most-recently-used render preset list */
	virtual const MCHAR* GetRenderPresetMRUListDisplayName( int i ) = 0;
	//! \brief Returns the filename associated with an entry in the render dialog's mostly-recently-used render preset list
	/*! If an invalid index is passed, it will wrap around to a valid index, since the MRU list contains a fixed number of items.
		\param[in] i The index within the most-recently-used render preset list */
	virtual const MCHAR* GetRenderPresetMRUListFileName( int i ) = 0;

	// JOHNSON RELEASE SDK

	//! \brief Returns the world transform location of the current transform gizmo.
	/*! Returns the world transform of the current transform gizmo in the active viewport.
		\return The world transform of the transform gizmo in the active viewport.
	*/
	virtual Matrix3 GetTransformGizmoTM() const = 0;

	//! \brief Popup the viewport configuration dialog with specified tab page.
	/*! If an invalid index is passed, the dialog will show the first tab page when it starts up.
		\param[in] i The index of the tab page that to be showed on the dialog starts up */
	virtual void DisplayViewportConfigDialogPage( int i ) = 0;


}; // Interface 11



/*! \brief  Extends Interface11 
 * Client code should retrieve this interface using GetCOREInterface12
 */
class Interface12 : public Interface11
{
public:

	//! \brief The ID for this interface.  Pass this ID to Interface::GetInterface to get an Interface12 pointer.
	CoreExport static const Interface_ID kInterface12InterfaceID;

	/**
	 * \brief Get the coordinates for the area within the main window available
	 * for client UI components.
	 *
	 * The Application Frame reserves a portion of the client area for its own
	 * UI.  Use the coordinates given by this method to determine the available
	 * space for control placement in the main UI.  Absolute positioning of
	 * controls based on (0,0) is inappropriate and will cause controls to paint
	 * under the frame's UI.  Similarly, this method should be used instead of 
	 * GetClientRect, which will return an area including the region under the
	 * frame.
	 *
	 * \return Main window client area, relative to the main window's 
	 * upper left corner.
	 */
	virtual RECT GetMainWindowClientRect() const = 0;
}; // Interface 12


/*! \brief  Extends Interface12 
 * Client code should retrieve this interface using GetCOREInterface13
 */
class Interface13 : public Interface12
{
public:

	//! \brief The ID for this interface.  Pass this ID to Interface::GetInterface to get an Interface12 pointer.
	CoreExport static const Interface_ID kInterface13InterfaceID;

	//! \brief Popup a dialog allowing a user to pick a plug-in class.  Only one class may be picked.
	/*! Displays the names of each class of the given superclass, in a single-pick listbox,
	The list may be filtered using a callback provided by the caller.
		\param[in] hParent The parent window for the dialog
		\param[in] title The title text for the dialog
		\param[in] super The superclass of the class names to display
		\param[in] pPickClassCB The filtering callback, defined by the caller
		\return The class picked by the user */
	virtual ClassDesc *DoPickClassDlg(HWND hParent, const MCHAR *title, SClass_ID super, PickClassCallback *pPickClassCB = NULL) = 0;

	//! \brief Popup a dialog allowing a user to pick multiple plug-in class.
	/*! Displays the names of each class of the given superclass, in a multi-pick listbox.
	The list may be filtered using a callback provided by the caller.
		\param[in] hParent The parent window for the dialog
		\param[in] title The title text for the dialog
		\param[in] super The superclass of the class names to display
		\param[in] pPickClassCB The filtering callback, defined by the caller
		\param[out] classDescTab The list of classes picked by the user, populated by the callee
		\return The number of classes picked by the user */
	virtual int DoMultiplePickClassDlg(HWND hParent, const MCHAR *title, SClass_ID super, PickClassCallback *pPickClassCB, Tab<ClassDesc *> *classDescTab) = 0;

    //! \brief Returns whether rendering uses Iterative mode, when appropriate.
	/*! In Iterative mode, no files are output, no email notifications are sent, and no network or multi frame rendering is performed.
	    Iterative is used only for Quick Renders, and only when this flag is set. */
	virtual BOOL GetRendUseIterative() = 0;
	//! \brief Sets whether rendering will be performed in Iterative mode, when appropriate.
	/*! In Iterative mode, no files are output, no email notifications are sent, and no network or multi frame rendering is performed.
	    Iterative is used only for Quick Renders, and only when this flag is set.
		\param[in] b TRUE to enable Iterative mode, FALSE to disable */
	virtual void SetRendUseIterative( BOOL b ) = 0;

	/*! \brief Saves the current scene into a format compatible with the specified 3ds Max version
	*
	* This method	does not bring up a file save dialog.
	* \param [in] fname - the name of the file the scene is to be saved to
	* \param [in] clearNeedSaveFlag - if true, the scene 'dirty' flag is cleared thus a 
	* subsequent save will do nothing
	* \param [in] useNewFile - if true, the saved file will become the current scene and the file 
	* is added to the "Most Recently Used" list of files
	* \param [in] saveAsVersion - the version of 3ds Max in which the file is to be saved. 
	* Currently it supports saving to the current version of 3ds Max (MAX_RELEASE) and to
	* 3ds Max 2010 (MAX_RELEASE_R12).
	* \return true if the file was saved successfully; otherwise false
	*/
	virtual bool SaveSceneAsVersion(
		const MCHAR* fname, 
		bool clearNeedSaveFlag = true, 
		bool useNewFile = true, 
		unsigned long saveAsVersion = MAX_RELEASE) = 0; 

	/*! \brief Saves the specified nodes into a format compatible with the specified 3ds Max version
	*
	* This method	does not bring up a file save dialog.
	* \param [in] fname - the name of the file the scene is to be saved to
	* \param [in] nodes - list of nodes to save
	* \param [in] saveAsVersion - the version of 3ds Max in which the file is to be saved. 
	* Currently it supports saving to the current version of 3ds Max (MAX_RELEASE) and to
	* 3ds Max 2010 (MAX_RELEASE_R12).
	* \return true if the file was saved successfully; otherwise false
	*/
	virtual bool SaveNodesAsVersion(
		const MCHAR* fname, 
		const INodeTab* nodes,
		unsigned long saveAsVersion = MAX_RELEASE) = 0; 

	/*! \brief Saves the selected nodes into a format compatible with the specified 3ds Max version
	*
	* This method	does not bring up a file save dialog.
	* \param [in] fname - the name of the file the scene is to be saved to
	* \param [in] saveAsVersion - the version of 3ds Max in which the file is to be saved. 
	* Currently it supports saving to the current version of 3ds Max (MAX_RELEASE) and to
	* 3ds Max 2010 (MAX_RELEASE_R12).
	* \return true if the file was saved successfully; otherwise false
	*/
	virtual bool SaveSelectedNodesAsVersion(
		const MCHAR* fname, 
		unsigned long saveAsVersion = MAX_RELEASE) = 0; 

	/*! \brief Archives the current scene into a format compatible with the specified 3ds Max version
	*
	* \param [in] archiveFileName - the fully qualified file name the scene is to be archived to
	* \param [in] saveAsVersion - the version of 3ds Max in which the file is to be saved. Defaults to current version.
	* \return true if the file was archived successfully; otherwise false
	*/
	virtual bool ArchiveSceneFile(
		const MCHAR* archiveFileName,
		unsigned long saveAsVersion = MAX_RELEASE) = 0;

	//! \brief Gets whether the scene file is written compressed
	virtual bool GetSceneFileCompressOnSave() = 0;

	/*! \brief Sets whether the scene file is written compressed
	*
	* \param [in] compress - whether to compress on save
	* \param [in] persist - whether to persist setting across 3ds Max sessions
	*/
	virtual void SetSceneFileCompressOnSave(bool compress, bool persist) = 0;

	/*! \brief Gets a pointer to the ILayerManager */
	virtual ILayerManager* GetLayerManager() = 0;

	/*! \defgroup mtlDlgMode Material Editor Dialog Mode values */
	//!@{
	enum MtlDlgMode {
		mtlDlgMode_Basic = 0,		//!< Basic mode, basic parameter editing of material and textures
		mtlDlgMode_Advanced = 1,	//!< Advanced mode, schematic graph editing of material and texture connections
	};
	//!@}

	//! \brief Returns the Material Editor Dialog Mode, either Basic or Advanced.
	/*! Opening the editor using Interface7::OpenMtlDlg() opens the corresponding dialog; both cannot be open simultaneously.
		\return A mtlDlgMode value
		\see mtlDlgMode */
	virtual int GetMtlDlgMode() = 0;

	//! \brief Sets the Material Editor Dialog Mode, either Basic or Advanced.
	/*! Opening the editor using Interface7::OpenMtlDlg() opens the corresponding dialog; both cannot be open simultaneously.
		\param[in] mode A mtlDlgMode value
		\see mtlDlgMode */
	virtual void SetMtlDlgMode( int mode ) = 0;

	//! \brief Returns whether the given Material Editor is open, either Basic or Advanced.
	/*! \param[in] mode A mtlDlgMode value
		\see mtlDlgMode */
	virtual BOOL IsMtlDlgShowing( int mode ) = 0;

	//! \brief Opens the given Material Editor, either Basic or Advanced.  No effect if already open.
	/*! Opening multiple different editors simultaneously is not supported, and may lead to instability.
		\param[in] mode A mtlDlgMode value
		\see mtlDlgMode */
	virtual void OpenMtlDlg( int mode ) = 0;

	//! \brief Closes the given Material Editor, either Basic or Advanced.  No effect if already closed.
	/*! \param[in] mode A mtlDlgMode value
		\see mtlDlgMode */
	virtual void CloseMtlDlg( int mode ) = 0;

	//! \name Scene Entity Name Suffix Length 
	//!@{
	/** 
	 * \brief Sets the minimum length of the numeric suffix used in scene element names.
	 * The name suffix length set with this method is used when
	 * creating unique node names via Interface::MakeNameUnique, and when 
	 * creating unique names using NameMaker::MakeUniqueName().
	 * The setting remains in effect until it's changed. It's value 
	 * is also written to 3dsmax.ini, thus making it "stick" between 3ds Max sessions.
	 * 
	 * For example, using root name "MyObject" with a a suffix length of three, 
	 * the following names are generated "MyObject001", "MyObject002", etc.
	 * More digits are added as necessary if the suffix is of insufficient length. 
	 * For example, the name following "MyObject999" would be "MyObject1000".
	* 
	 * \param suffixLength The length in digits of the name suffix
	 */
	virtual void SetNameSuffixLength(unsigned short suffixLength) = 0;

	/** 
	 * \brief Returns the length of the numeric prefix used in names.
	 * The default suffix length is 3 digits.
	 */
	virtual unsigned short GetNameSuffixLength() const = 0;
	//!@}

}; // Interface 13

/*! \brief  Extends Interface13 
 * Client code should retrieve this interface using GetCOREInterface14
 */
class Interface14 : public Interface13
{
public:
	/**
	* The ID for this interface.  Pass this ID to Interface::GetInterface to get an Interface14 pointer.
	*/
	CoreExport static const Interface_ID kInterface14InterfaceID;

	/**
	* This method gets a ViewExp interface given a view ID.
	* If the given view ID doesn't match any of the viewport, the
	* ViewExp interface of the current active viewport will be returned.
	* \param[in] viewID the unique ID of a viewport. 
	* The view ID can be retrieved by calling ViewExp::GetViewID().
	* Each view is uniquely identified by an ID which is persisted with the 3ds Max file. 
	* Client code that previously (3ds Max 2012 and earlier) stored a ViewExp*, can now 
	* store the view ID. Note that since views (viewports) can be destroyed as of 3ds Max 2013,
	* plug-ins should not store ViewExp pointers, as they will become invalid when the underlying
	* view (viewport) is destroyed. Instead, plug-ins should store the view's unique ID.
	* \code
	* ViewExp* pViewExp = GetCOREInterface14()->GetViewExpByID(mStoredViewID);
	* // Check the ID against your stored ID to make sure you get the correct ViewExp interface
	* // since you may get the ViewExp interface of current active viewport if the corresponding
	* // viewport got deleted.
	* if (pViewExp && pViewExp->GetID() == mStoredViewID) 
	* {
	*		// your codes below
	* }
	* \endcode
	*/
	virtual ViewExp& GetViewExpByID(int viewID) = 0;

	/**
	* Get the viewport ID used, when the rendering is locked on a viewport, for the current renderer.
	* \return the ID of the viewport to be rendered when the rendering is locked on a viewport.
	* \see Interface14::GetViewExpByID
	*/
	virtual int GetRendViewID() = 0;

	/**
	* Sets the viewport ID used, when the rendering is locked on a viewport, for the current renderer.
	* If an invalid ID is set, attempts to launch a render will fail and an error dialog will be displayed.
	* \param[in] id ID of the viewport to be rendered when the rendering is locked on a viewport.
	* \see Interface14::GetViewExpByID
	*/
	virtual void SetRendViewID( int id ) = 0;

		
	/** 
	* This method is used to access the data used when creating a preview image
	* from the currently active viewport using the real-time (viewport) renderer.
	* \return A copy of the preview parameters 
	*/
	virtual PreviewParams GetPreviewParams() const = 0;

	/*! \brief Set the Play Preview When Done option accessible in the Preferences dialog and the Make Preview dialog
	\param[in] play TRUE to enable the play when done option, FALSE to disable */
	virtual void SetPlayPreviewWhenDone(BOOL play) = 0;

	//! \brief Retrieve Play Preview When Done option accessible in the Preferences dialog and the Make Preview dialog
	virtual BOOL GetPlayPreviewWhenDone() = 0;

	/**
	* if true, don't use language specified in scene file when doing string conversions during scene file load, 
	* instead use value from LanguageToUseForFileIO()
	* In scene files created by 3ds Max 2012, the 3ds Max language ID and active code page are stored 
	* in the file. Normally, string conversions during scene file load would be based on the saved language ID.
	* If this method returns true, the user has specified to do string conversions based on the return value 
	* of LanguageToUseForFileIO() instead. 
	* \return If true, use the language id from LanguageToUseForFileIO()for string conversions while reading scene files
	*/
	virtual bool OverrideLanguageSpecifiedInSceneFile() const = 0;

	/**
	* sets return value from OverrideLanguageSpecifiedInSceneFile()
	* \param[in] overrideFileLangID The value to return from OverrideLanguageSpecifiedInSceneFile()
	*/
	virtual void SetOverrideLanguageSpecifiedInSceneFile(bool overrideFileLangID) = 0;

	/**
	* if true, use the CodePage specified in scene file when doing string conversions during scene 
	* file load instead of using the LANGID in the file or specified by LanguageToUseForFileIO()
	* In scene files created by 3ds Max 2012, the 3ds Max language ID and active code page are stored 
	* in the file. Normally, string conversions during scene file load would be based on the saved language ID.
	* If this method returns true, the user has specified to do string conversions based on the saved active code page
	* instead. 
	* \return If true, use the code page specified in the scene file for string conversions while reading 
	*/
	virtual bool UseCodePageSpecifiedInSceneFile() const = 0;

	/**
	* sets return value from UseCodePageSpecifiedInSceneFile()
	* \param[in] useFileCodePage The value to return from UseCodePageSpecifiedInSceneFile()
	*/
	virtual void SetUseCodePageSpecifiedInSceneFile(bool useFileCodePage) = 0;

	/**
	* The LANGID to use for scene, text, and other binary file io when doing string conversions.
	* This method primarily intended to support loading legacy files from mbcs builds of max 
	* (pre-3ds Max 2013) and for writing non-scene text or binary files where conversion from 
	* wide characters to mbcs characters is required.
	* The LANGID is typically passed to CodePageForLanguage() to get the code page to use for
	* string conversions.
	* \return The LANGID to use for string conversions when doing file io
	*/
	virtual LANGID LanguageToUseForFileIO() const = 0;

	/**
	* sets return value from LanguageToUseForFileIO(). Returns false if LANGID is not valid.
	* This method primarily intended to support loading legacy files from mbcs builds of max 
	* (pre-3ds Max 2013) and for writing non-scene text or binary files where conversion from 
	* wide characters to mbcs characters is required.
	* The valid LANGID values are:
	*   0    (current)
	*   1033 (English)
	*   1031 (German)
	*   1036 (French)
	*   1041 (Japanese)
	*   1042 (Korean)
	*   2052 (Chinese)
	* \param[in] id The language id to return from LanguageToUseForFileIO()
	* \return True if the LANGID was valid
	*/
	virtual bool SetLanguageToUseForFileIO(LANGID id) = 0;

	/**
	* returns the code page corresponding to the specified LANGID. 
	* If the LANGID is 0, the LANGID returned from MaxSDK::Util::GetLanguageID() is used.
	* Returns the English code page if LANGID is not valid.
	* This method primarily intended to support loading legacy files from mbcs builds of max 
	* (pre-3ds Max 2013), Save to Previous to those versions of max, and for writing non-scene
	* text or binary files where conversion from wide characters to mbcs characters is required.
	* The valid LANGID values are:
	*   0    (current)
	*   1033 (English)
	*   1031 (German)
	*   1036 (French)
	*   1041 (Japanese)
	*   1042 (Korean)
	*   2052 (Chinese)
	* \param[in] id The language id to get the code page for. 
	* \return The code page
	*/
	virtual UINT CodePageForLanguage(LANGID id) const = 0;

	/**
	* if true, when saving non-scene legacy files that are readable by 3ds Max, allow use of UTF8 encoding of strings 
	* when it is necessary to convert from wide (Unicode) strings to narrow (ascii/mbcs) strings.
	* If this method returns false, do string conversions based on the return value of LanguageToUseForFileIO() 
	* and CodePageForLanguage() instead. 
	* \code
	* UINT codePage;
	* Interface14 *iface = GetCOREInterface14();
	* if (iface->LegacyFilesCanBeStoredUsingUTF8())
	* {
	*     codePage = CP_UTF8;
	* }
	* else
	* {
	*     LANGID langID = iface->LanguageToUseForFileIO();
	*     codePage = iface->CodePageForLanguage(langID);
	* }
	* \endcode
	* \return If true, use the UTF8 codepage when writing non-scene legacy files
	*/
	virtual bool LegacyFilesCanBeStoredUsingUTF8() const = 0;

	/**
	* sets return value from LegacyFilesCanBeStoredUsingUTF8()
	* \param[in] allowUTF8 The value to return from LegacyFilesCanBeStoredUsingUTF8()
	*/
	virtual void SetLegacyFilesCanBeStoredUsingUTF8(bool allowUTF8) = 0;

	/**
	* Helper method for converting AppDataChunks containing mbcs string data to utf8 encoded string data
	* This method is used primarily in AppDataLoadCallbacks registered using Animatable::RegisterAppDataLoadCallback to
	* convert AppDataChunks containing mbcs string data to utf8 encoded string data. The utf8 encoded string data is
	* prepended with the utf8 BOM ("\xef\xbb\xbf") to signify that it is utf8 encoded.
	* \code
	* void PolygonCounter::UpdateAppDataLoadProc(Animatable *anim, const Class_ID& cid, SClass_ID sid, ILoad* iload, Tab<DWORD> &subIDs)
	* {
	* 	DbgAssert( anim && iload );
	* 	if ( anim == NULL || iload == NULL )
	* 		return;
	* 
	* 	DbgAssert (cid == POLYGON_COUNTER_CLASS_ID);
	* 	DbgAssert (sid == UTILITY_CLASS_ID);
	* 
	* 	Interface14 *iface = GetCOREInterface14();
	* 	iface->ConvertAppDataChunksContainingStringToUTF8(anim, cid, sid, subIDs, iload->CodePage());
	* }
	* \endcode
	* When retrieving the string from the AppDataChunk, check to see if the first 3 chars are the utf8 BOM and
	* convert accordingly.
	* \code
	* MSTR data;
	* const char* raw_string = (const char*)ad->data;
	* if (strncmp(raw_string, "\xef\xbb\xbf", 3) == 0)
	* 	data = MSTR::FromUTF8(raw_string+3);
	* else
	* {
	* 	DbgAssert(!_T("Should always convert appdata in UpdateAppDataLoadProc"));
	* 	Interface14 *iface = GetCOREInterface14();
	* 	LANGID langID = iface->LanguageToUseForFileIO();
	* 	UINT codePage = iface->CodePageForLanguage(langID);
	* 	data = MSTR::FromCP(codePage, raw_string);
	* }
	* \endcode
	* \param[in] anim The Animatable to acquire the AppDataChunk from.
	* \param[in] classID The Class_ID of the owner of the chunk.
	* \param[in] superClassID The SuperClassID of the owner of the chunk.
	* \param[in] subIDs The list of sub-chunk ids
	* \param[in] codePage The code page of the mbcs string data
	*/
	virtual void ConvertAppDataChunksContainingStringToUTF8(Animatable *anim, const Class_ID& classID, SClass_ID superClassID, Tab<DWORD> &subIDs, UINT codePage) = 0;

	/**
	* returns the default code page for Loading aux file
	* This method would be used outside of scene file io, primarily when doing aux file io.
	* If doing scene file load, should be using the result from ILoad::CodePage().
	*/
	virtual UINT DefaultTextLoadCodePage() = 0;

	/**
	* returns the default code page for Saving aux file
	* This method would be used outside of scene file io, primarily when doing aux file io.
	* If doing scene file save, should be using the result from ISave::CodePage().
	* \param[in] allow_utf8 option to save the file in UTF8 encoding
	*/
	virtual UINT DefaultTextSaveCodePage(bool allow_utf8 = false) = 0;

	// Render a 2D bitmap from a procedural texture
	/*! \remarks	This method renders a texmap (or an entire texmap tree) to the specified bitmap.
	\par Parameters:
	<b>Texmap *tex</b>\n\n
	The Texmap to render to a bitmap.\n\n
	<b>Bitmap *bm</b>\n\n
	A pointer to a bitmap to render to. This bitmap must be created at the
	resolution you wish to render to.\n\n
	<b>FBox2* range</b>\n\n
	A pointer to UV range to crop the texmap. When specified, the scale3d parameter won't have any effect.
	If NULL, will retain the UV parameters as specified in the scene at time t.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to render the texmap to the bitmap, defaults to the current frame.
	<b>float scale3d=1.0f</b>\n\n
	This is a scale factor applied to 3D Texmaps. This is the scale of the surface
	in 3d space that is mapped to UV. This controls how much of the texture appears
	in the bitmap representation.\n\n
	<b>BOOL filter=FALSE</b>\n\n
	If TRUE the bitmap is filtered. It is quite a bit slower to rescale bitmaps
	with filtering on.\n\n
	<b>BOOL display=FALSE</b>\n\n
	If TRUE the resulting bitmap is displayed using the virtual frame buffer;
	otherwise it is not.
	<b>bool bake</b>\n\n
	Used to bake the texture to bitmap without including the effects of scale/rotation etc.
	If true, the function will render the unit square of 2D texture maps, by effectively
	disabling the UV Generator during texture rendering, so no settings like scale, rotation,
	tiling etc. have any effect on the result. This baked 2D representation can then later be
	scaled, rotated and tiled, for use in a renderer that may not support the particular
	texture, sent to a game engine, or similar.
	\n\n
	<b>const MCHAR *name=NULL</b>\n\n
	Display Name.
	\n\n
	<b>float z=0.0f</b>\n\n
	The w component of UVW.
	\n\n
	<b>BOOL mono=false</b>\n\n
	Use mono channel. Mono channels are those that don't have a color, but rather a single value.
	\n\n
	<b>disableBitmapProxies=false</b>\n\n
	Used to disable bitmap proxies. bitmap proxies are used to reduce the amount of memory required by the bitmapped textures.
	\n\n

	*/
	virtual void RenderTexmapRange(Texmap *tex, Bitmap *bm, FBox2* range, TimeValue t, float scale3d=1.0f, BOOL filter=FALSE, BOOL display=FALSE, bool bake = false, const MCHAR *name=NULL, float z=0.0f, BOOL mono=false, bool disableBitmapProxies=false) = 0;
};

/*! \brief  Extends Interface14 
 * Client code should retrieve this interface using GetCOREInterface15
 */
class Interface15 : public Interface14
{
public:
	/**
	* The ID for this interface.  Pass this ID to Interface::GetInterface to get an Interface15 pointer.
	*/
	CoreExport static const Interface_ID kInterface15InterfaceID;

	/**
	* returns the main 3ds Max thread id
	*/
	virtual DWORD GetMainThreadID() = 0;

	/**
	* returns the asset path for the template used at scene creation
	*/
	virtual MSTR &GetCurTemplateAssetPath() = 0;
};

/*! \brief  Extends Interface15 
 * Client code should retrieve this interface using GetCOREInterface16
 */
class Interface16 : public Interface15
{
public:
	/**
	* The ID for this interface.  Pass this ID to Interface::GetInterface to get an Interface16 pointer.
	*/
	CoreExport static const Interface_ID kInterface16InterfaceID;

  	/**
	* Get the viewport ID used, when the rendering is locked on a viewport, for the specified renderer.
	* \return the ID of the viewport to be rendered when the rendering is locked on a viewport.
	* \see Interface14::GetViewExpByID
	*/
	virtual int GetRendViewID(const RenderSettingID renderSettingID) = 0;

	/**
	* Sets the viewport ID used, when the rendering is locked on a viewport, for the specified renderer.
	* If an invalid ID is set, attempts to launch a render will fail and an error dialog will be displayed.
	* \param[in] renderSettingID The ID of the renderer.  See \ref renderSettingIDs.
	* \param[in] viewportID ID of the viewport to be rendered when the rendering is locked on a viewport.
	* \see Interface14::GetViewExpByID
	*/
	virtual void SetRendViewID(const RenderSettingID renderSettingID, const int viewportID) = 0;

	/**
	* Returns the view which is currently selected for rendering, in the render settings panel.
	*/
	virtual ViewExp* GetCurrentRenderView() = 0;

	/**
	* sets whether world axis is shown in viewports
	* \param[in] state Whether to show the world axis in viewports
	*/
	virtual void SetShowWorldAxis(BOOL state) = 0;

	/**
	* gets whether world axis is shown in viewports
	*/
	virtual BOOL GetShowWorldAxis() = 0;

	/** Set the override material for an offline renderer.
	/param pOverrideRenderSettingMtl the override material
	*/
	virtual void SetOverrideRenderSettingMtl(MtlBase* pOverrideRenderSettingMtl) = 0;
	/** Get the current override material of the render settings.
	/return the current override material
	*/
	virtual MtlBase* GetOverrideRenderSettingMtl() const = 0;
};

/*! \brief  Extends Interface16 
 * Client code should retrieve this interface using GetCOREInterface17
 */
class Interface17: public Interface16
{
public:
	/**
	* The ID for this interface.  Pass this ID to Interface::GetInterface to get an Interface17 pointer.
	*/
	CoreExport static const Interface_ID kInterface17InterfaceID;

	/**
	 * Set and get the TMCache flag for retaining computed node transforms.
	 * When this flag is active, all node transforms (except those in Biped)
	 * are cached on whole frame numbers (as determined by the current FPS).
	 * These transforms are retained until cleared by BaseNode::InvalidateTM(),
	 * change in FPS or setting SetEnableTMCache(false).
	 *
	 * This makes retrieval of node transforms faster once they have been computed
	 * for the first time. However, please note that this means that invalidation
	 * through notifications (notably REFMSG_CHANGE, REFMSG_OBJECT_CACHE_DUMPED)
	 * are more critical (particularly the interval sent in the notifications)
	 * in cases where the node transform has been invalidated, since the node TM
	 * cache is invalidated only for frames within the interval specified.
	 * (You can always send the interval FOREVER, but this will of course force
	 * all node transforms to be recomputed when first requested.)
	 *
	 * If you are seeing inconsistent results from node transforms you can turn
	 * this flag off to compare results. Turning the flag off temporarily to avoid
	 * using notifications is not recommended.
	 *
	 * By default, EnableTMCache is true.
	 */
	virtual void SetEnableTMCache(bool value, bool commitToInit) = 0;
	virtual bool GetEnableTMCache() const = 0;

	/*!
	Gets whether running in Non Interactive Mode. In Non Interactive mode 3ds Max starts up minimized,
    loads the command line scene file, runs command line and startup scripts, and shuts down. 
	\param[in] checkTestMode Whether to check if in Non Interactive Test Mode. If so, this method returns 
	true even when not in non interactive mode.
	\return True if in non interactive mode.
	\note In non interactive mode the following behavior exists:
	-	No splash screen, no welcome screen
	-	Max starts up in minimized mode, and remains in minimized mode
	-	Autobackup is disabled
	-	Max is shut down after running the -mxs script and the startup utility.
	-	Max window state and size is not loaded at startup, nor saved at shutdown
	-	QuietMode is always on
	-	NoNetworkRendering is always true
	-	Loaded files are not added to recent document list
	-	Floating toolbars are disabled
	-	StartupTemplates are disabled
	-	Ribbon support is disabled
	-	AdWindows is disabled
	-	mxs desktop is not loaded at startup nor saved at shutdown
	*/
	virtual bool InNonInteractiveMode(bool checkTestMode = false) const = 0;

    /**
	* set that we are running in Non Interactive Test Mode. 
	This is used for testing purposes. This does not affect licensing
	or startup behavior, rather the disabling certain functionality
	when running in Non Interactive Mode.
	*/
	virtual void SetInNonInteractiveTestMode() const = 0;

	/**
	security measures include but is not limited to
	- digital signature verifications on plugins/assemblies
	-	certain functionality (such as mxsdotnet, ActiveX, OLE) is disabled or limited
	- prevent blacklisted commands from being used (dos command, registry access)
	- whitelist .NET types and assemblies
	- disable python
	- prevent loading ADSK Store plugins
	- prevent loading user defined MCG assemblies
	*/
	virtual void SetInSecureMode() = 0;
	virtual bool InSecureMode() const = 0;

	/**
	* Gets whether Ribbon support is enabled
	*/
	virtual bool IsRibbonSupportEnabled() const = 0;

	//! \brief Break the tangents of the selected trajectory keys.  
	/*! This function will only do something if trajectories are enabled and we are in the key subobject
	mode with selected keys.
	*/
	virtual void BreakSelectedKeyTangent() = 0;

	//! \brief Unify the tangents of the selected trajectory keys, when unified they will move together.  
	/*! This function will only do something if trajectories are enabled and we are in the key subobject
	mode with selected keys.
	*/
	virtual void UnifySelectedKeyTangent() = 0;

	//! \brief Set the tangents type for the selected trajectory keys.  
	/*! This function will only do something if trajectories are enabled and we are in the key subobject
	mode with selected keys.
	\param[in] type Bezier Tangent Types see /ref keyFlagBits_bezierTangentTypes
	*/
	virtual void SetSelectedKeyTangentType(int type) = 0;

	/*! \defgroup possible Trajectory Path coloring values */
	//!@{
	enum PathColoringType {
		kNoGradation	= 0,	//!< plain line, in the color defined by 'Trajectory' in the Gizmos section of the UI colors (by default red)
		kUIColor		= 1,	//!< from start to finish, progressing from the UI colors 'Trajectory Start' and 'Trajectory End' 
		kWireColor		= 2,	//!< a progression between colors based on the object wire color
		kSpeed			= 3,	//!< color based on the object speed at that point, from blue at trajectory slowest to pure red at the fastest point
	};
	//!@}

	//! \name Trajectory Display options  
	//!@{ 
	//! \brief Returns the colors used for drawing the path of selected nodes.  
	//! \see PathColoringType
	virtual PathColoringType GetTrajectoryPathGradation() const = 0;
	//! \brief Sets the colors used for drawing the path of selected nodes.  
	virtual void SetTrajectoryPathGradation(PathColoringType type) = 0;

	//! \brief Returns whether the trajectory of unselected nodes will show ticks at every frame
	virtual bool GetUnselTrajectoryDrawFrameTicks() const = 0;
	//! \brief Sets whether the trajectory of unselected nodes will show ticks at every frame
	virtual void SetUnselTrajectoryDrawFrameTicks(bool draw) = 0;
	//! \brief Returns whether the trajectory of unselected nodes will show key markers
	virtual bool GetUnselTrajectoryDrawKeys() const = 0;
	//! \brief Sets whether the trajectory of unselected nodes will show key markers
	virtual void SetUnselTrajectoryDrawKeys(bool draw) = 0;
	//! \brief Returns whether the trajectories will be drawn for only a subset of the whole animation duration 
	virtual bool TrimTrajectories() const = 0;
	//! \brief Sets whether the trajectories will be drawn for only a subset of the whole animation duration 
	virtual void SetTrimTrajectories(bool trim) = 0;
	//! \brief Returns whether, if trajectories are trimmed, they are drawn for a fixed range of frames (or a range around the current frame) 
	//! \see TrimTrajectories(), GetTrimStart(), GetTrimEnd()
	virtual bool IsFixedTrimRange() const = 0;
	//! \brief Sets whether, if trajectories are trimmed, they are drawn for a fixed range of frames  
	//! \see TrimTrajectories(), GetTrimStart(), GetTrimEnd()
	virtual void SetFixedTrimRange(bool fixedRange) = 0;
	//! \brief Returns the number of frames before and after the current frame that trajectories will be drawn 
	virtual int  TrimTrajectoriesOffset() const = 0;
	//! \brief Sets the number of frames before and after the current frame that trajectories will be drawn 
	virtual void SetTrimTrajectoriesOffset(int range) = 0;
	//! \brief Returns the start frame of the section of the animation range in which trajectories are drawn 
	virtual int  GetTrimStart() const = 0;
	//! \brief Sets the start frame of the section of the animation range in which trajectories are drawn 
	virtual void SetTrimStart(int startFrame) = 0;
	//! \brief Returns the end frame of the section of the animation range in which trajectories are drawn 
	virtual int  GetTrimEnd() const = 0;
	//! \brief Sets the end frame of the section of the animation range in which trajectories are drawn 
	virtual void SetTrimEnd(int endFrame) = 0;

	//! \brief Returns whether the trajectory of selected nodes will draw tangents for all keys, selected or not
	virtual bool GetSelTrajectoryDrawAllTangents() const = 0;
	//! \brief Sets whether the trajectory of selected nodes will draw tangents for all keys
	virtual void SetSelTrajectoryDrawAllTangents(bool draw) = 0;
	//! \brief Returns whether the trajectory of selected nodes will show ticks at every frame
	virtual bool GetSelTrajectoryDrawFrameTicks() const = 0;
	//! \brief Sets whether the trajectory of selected nodes will show ticks at every frame
	virtual void SetSelTrajectoryDrawFrameTicks(bool draw) = 0;
	//! \brief Returns whether the trajectory of selected nodes will draw ticks in the path color (or in white)
	virtual bool GetSelTrajectoryDrawGradientTicks() const = 0;
	//! \brief Sets whether the trajectory of selected nodes will draw ticks in the path color (or in white)
	virtual void SetSelTrajectoryDrawGradientTicks(bool draw) = 0;
	//! \brief Returns whether the trajectory of selected nodes will draw the key times for each key.
	virtual bool GetTrajectoryDrawKeyTimes() const = 0;
	//! \brief Sets whether the trajectory of selected nodes will draw the key times for each key.
	virtual void SetTrajectoryDrawKeyTimes(bool draw) = 0;
	//! \brief Returns whether the trajectory of selected nodes will draw the key identifier
	virtual bool GetSelTrajectoryDrawKeys() const = 0;
	//! \brief Sets whether the trajectory of selected nodes will draw the key identifier
	virtual void SetSelTrajectoryDrawKeys(bool draw) = 0;

	//!@}

	//! \brief Forces a rebuild of the Scene Material library
	/*! The Scene Material library is not guaranteed to be accurate after some scene operations. For example,
	if you delete a node, the material for that node may still appear in the Scene Material library. In order to 
	force an update of the Scene Material library, call this method.
	*/
	virtual void UpdateSceneMaterialLib() = 0;
};

//! \deprecated This has been deprecated as of 3ds Max 2022, with Interface18 specific methods moved to Interface13
typedef MAX_DEPRECATED Interface17 Interface18;

//! \deprecated This has been deprecated as of 3ds Max 2022, with Interface19 specific methods moved to Interface14
typedef MAX_DEPRECATED Interface17 Interface19;

/// Methods that control an internal optimization related to reference message
/// propagation. The optimization consists of preventing the same ref message
/// from being sent repeatedly to the same refmaker. This optimization improves
/// the performance of interactive manipulation of complex IK rigs, but it is
/// not completely reliable in the presence of some objects such as VIZWalls.
///
/// Call this method to turn the optimization On (value = true) or Off
/// (value = false) at runtime. When 'commitToInit' is true, 'value' is saved
/// to the current market default file as 'DontRepeatRefMsg=\<value\>'
CoreExport void SetDontRepeatRefMsg(bool value, bool commitToInit);
/// Call this method to find out if the optimization is active or not
CoreExport bool DontRepeatRefMsg();
/// In order to apply the optimization to a notification session, the
/// Pre/PostRefNotifyDependents methods have to bracket it.
/// IMPORTANT: The notification session should not contain GetValue calls.
CoreExport void PreRefNotifyDependents();
CoreExport void PostRefNotifyDependents();

class ScopedPrePostRefNotifyDependents : public MaxHeapOperators
{
private:
	bool m_optimizationEnabled;
public:
	ScopedPrePostRefNotifyDependents(bool enableOptimization) : m_optimizationEnabled(enableOptimization) 
	{
		if (enableOptimization)
			PreRefNotifyDependents();
	};
	~ScopedPrePostRefNotifyDependents() 
	{
		if (m_optimizationEnabled)
			PostRefNotifyDependents();
	}
};

/// Methods that control optimization for evaluating the transforms of complex rigs,
/// especially of those with the expose transform helper. Note that this optimization
/// may not work completely reliable with all objects and controllers
CoreExport void SetInvalidateTMOpt(bool value, bool commitToInit);
CoreExport bool GetInvalidateTMOpt();

/// Methods that control optimization for not sending duplicate notifications.
/// Note that this optimization may not work completely reliable with all objects and controllers
CoreExport void SetEnableOptimizeDependentNotifications(bool value, bool commitToInit);
CoreExport bool GetEnableOptimizeDependentNotifications();

CoreExport void GetOptimizeDependentNotificationsStatistics(ULONGLONG &numNotifySkipped, ULONGLONG &numNotifyNotSkipped);
CoreExport void ResetOptimizeDependentNotificationsStatistics();

/*! \brief Set and get the controller validity interval optimization flag for the base controllers.
* When this flag is active, all Linear and Bezier controllers will express validity intervals
* that take into account flat sections in the animation curves that drive them. This allows
* for a performance improvement during scene evaluation because work can be skipped when dependencies
* don't actually change.
*
* By default, ControllerIntervalOptimization is true.
* 
* \param [in] value - Whether or not to enable this optimization
* \param [in] commitToInit - Whether or not to write this setting into the defaults ini file.
*/
CoreExport void SetControllerIntervalOptimization(bool value, bool commitToInit);
CoreExport bool GetControllerIntervalOptimization();
