//**************************************************************************/
// Copyright (c) 1995-2011 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

/*! \defgroup MAXDirIDs MAX Directory IDs
The following IDs represent a set of application system directories that are
used in various contexts.  The most common usage for these IDs is in the 
IPathConfigMgr::GetDir() and IPathConfigMgr::SetDir(int) methods.
*/
//!@{
/*! The ID used to access the default Font files location.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int) 
*/
#define APP_FONT_DIR	 	      0
/*! The ID used to access the default location where Scene files
	are saved.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int) 
*/
#define APP_SCENE_DIR		      1
/*! The ID used to access the default location where import data
	is browsed from.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_IMPORT_DIR		      2
/*! The ID used to access the default location where export data
	is saved to.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int) 
*/
#define APP_EXPORT_DIR		      3
/*! The ID used to access the system location where help files are stored.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_HELP_DIR		      4
/*! The ID used to access the default location where data files relating
	to the Expression Controller are saved.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_EXPRESSION_DIR	      5
/*! The ID used to access the default location where preview rendering outputs
	are saved.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_PREVIEW_DIR		      6
/*! The ID used to access the default location where all non-material images
	used in scenes are browsed from.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_IMAGE_DIR		      7
/*! The ID used to access the default location where all sound files
	used in scenes are browsed from.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_SOUND_DIR		      8
/*! The ID used to access the system location where all plugin configuration
	files are saved and loaded.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_PLUGCFG_DIR		      9
/*! The ID used to access the system location where the application will look
	for default application startup scene.  Localized plugin configuration files go here.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_MAXSTART_DIR	      10
/*! The ID used to access the default location where all Video Post files
	used in scenes are browsed from.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_VPOST_DIR		      11
/*! The ID used to access the system location where the application will look
	for graphic driver specifc files.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_DRIVERS_DIR		      12
/*! The ID used to access the default location where the autobackup files are saved.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_AUTOBACK_DIR	      13
/*! The ID used to access the default location where the material library files 
	are loaded and saved.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_MATLIB_DIR		      14
/*! The ID used to access the system location where scripts are saved.  This location
	is also included in the maxscript search path.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_SCRIPTS_DIR		      15
/*! The ID used to access the system location where startup scripts are saved.  Scripts
	found in this folder will be run when the application starts up. This location
	is also included in the maxscript search path.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_STARTUPSCRIPTS_DIR    16
/*! The ID used to access the system location where a market defaults set is loaded from.  
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_MARKETDEFAULTS_DIR    17
/*! The ID used to access the default location where render preset files 
	are loaded and saved.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_RENDER_PRESETS_DIR   18
/*! The ID used to access the system location where files relating to the 
	User Interface files are read from.  
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_UI_DIR			     19	
/*! 
	With Windows Vista, plugins  will no longer have direct write access to the 
	application install path.  Application data and settings needs to be written to a user profile path.

	Plugins should follow the following rules from writing files:
	<ul>
	<li>Plugins that need to write localized configuration data should rely on APP_PLUGCFG_DIR.
	<li>Plugins that need to write non-localized configuration data should rely on APP_PLUGCFG_LN_DIR.
	<li>Plugins that need to write miscellaneous data can use APP_MAXDATA_DIR.  This path is
	pointed to a user profile directory and is safe to write to.  Plugins may
	create subdirectories under this path.
	<li>Plugins that need to write out temporary files should rely on APP_TEMP_DIR.
	<li>Finally, plugins that need to dynamically look up the application install root can use the 
	APP_MAX_SYS_ROOT_DIR value.
	</ul>

	The ID used to access the system location where application executable resides.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_MAX_SYS_ROOT_DIR	 20

/*! The ID used to access the default location where render output files are
	saved.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_RENDER_OUTPUT_DIR	 21
/*! The ID used to access the default location where animation files are
	loaded and saved.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_ANIMATION_DIR		 22
/*! The ID used to access the default location where archived scene files are
	saved.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_ARCHIVES_DIR		 23
/*! The ID used to access the default location where photometric data files are
	loaded.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_PHOTOMETRIC_DIR		 24
/*! The ID used to access the default location where render assets (such as 
	mental ray photon maps, etc.) are loaded and saved.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_RENDER_ASSETS_DIR	 25
/*! The ID used to access the default location where a user can safely write or create
	a script file.  This location is also included in the maxscript search path.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_USER_SCRIPTS_DIR	 26
/*! The ID used to access the default location where a user can safely write or create
	a macroscript file.  This is searched at startup to locate custom macroscript actions.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_USER_MACROS_DIR		 27
/*! The ID used to access the default location where a user can safely write or create
	a script file which is run at startup.  
	Scripts found in this folder will be run when the application starts up. This location
	is also included in the maxscript search path.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_USER_STARTUPSCRIPTS_DIR		28
/*! The ID used to access the system location where a temp file can safely be written.  Note 
	that the application does no special action to clean up this folder on exit, that it
	is still the responsibility of the developer to remove temporary files.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_TEMP_DIR					29
/*! The ID used to access the default location where a user can safely create icon files
	that can be used by the Customizable User Interface system.
	This location is included in the search path when the application is attempting to 
	load an icon file by name.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_USER_ICONS_DIR				30
/*! The ID used to access the system location where user specific configuration files
	should be written.  This value was introduced due to support Least Privilege Users.  
	\see APP_MAX_SYS_ROOT_DIR
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_MAXDATA_DIR					31
/*! The ID used to access the root folder of the current system project.  Developers
	should prefer to access this value using the methods named below.
	\see IPathConfigMgr::GetCurrentProjectFolder() 
	\see IPathConfigMgr::SetCurrentProjectFolder(const MaxSDK::Util::Path&)
*/
#define APP_PROJECT_FOLDER_DIR		    32
/*! The ID used to access the user defined location where downloaded files should
	be deposited by default.
	\see IPathConfigMgr::GetCurrentProjectFolder() 
	\see IPathConfigMgr::SetCurrentProjectFolder(const MaxSDK::Util::Path&)
*/
#define APP_DOWNLOAD_DIR		        33
/*! The ID used to access the user defined location where proxies (mainly bitmaps)
	are stored.
	\see IPathConfigMgr::GetCurrentProjectFolder() 
	\see IPathConfigMgr::SetCurrentProjectFolder(const MaxSDK::Util::Path&)
*/
#define APP_PROXIES_DIR		            34
/*! The ID used to access the folder that contains the .NET assemblies that are 
either meant to be plug-ins for the application, or contains components that are
relevant to the application.  
\see IPathConfigMgr::GetCurrentProjectFolder() 
\see IPathConfigMgr::SetCurrentProjectFolder(const MaxSDK::Util::Path&)
*/
#define APP_MANAGED_ASSEMBLIES_DIR		35
/*! The ID used to access the folder that contains bitmap tile pagefiles,
	for the Bitmap Pager.
	\see IBitmapPager::GetPageFilePath() 
	\see IBitmapPager::SetPageFilePath(const MCHAR* path)
*/
#define APP_PAGE_FILE_DIR		        36
/*! The ID used to access the system location where a shader cache file can safely be written. These
are shaders that have been compiled by 3ds Max and stored for later usage.  The compiled shaders can 
live longer than a 3ds Max session, being reused whenever possible.
\see IPathConfigMgr::GetDir() 
\see IPathConfigMgr::SetDir(int)
*/
#define APP_SHADER_CACHE_DIR	        37

/*! The ID used to access the system location where the application will look
	for non-localized plugin configuration files.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_PLUGCFG_LN_DIR		        38

/*! The ID used to access the system location where the application will look
	for non-localized UI related data files.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_UI_LN_DIR		              39

/*! The ID used to access the system location where the application will load and save files
	to the cloud (Autodesk 360 synchronization service).
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_AUTODESK_CLOUD_DIR        40

/*! The ID used to access the directory where plug-ins loaded by Plug-in Package Manager are installed for current user.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_PLUGIN_PACKAGE_PRIVATE_DIR 41

/*! The ID used to access the directory where plug-ins loaded by Plug-in Package Manager are installed for all the users.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_PLUGIN_PACKAGE_PUBLIC_DIR 42

/*! The ID used to access the directory where startup templates are read from in projects.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_USER_STARTUP_TEMPLATE_DIR	43

/*! The ID used to access the system location where image files are located on install.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_SYSTEM_IMAGE_DIR	44

/*! The ID used to access the system location where photometric files are located on install.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_SYSTEM_PHOTOMETRIC_DIR	45

/*! The ID used to access the system location where sound files are located on install.
	\see IPathConfigMgr::GetDir() 
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_SYSTEM_SOUND_DIR	46

/*! The ID used to access the directory where fluid simulation files will be placed.
\see IPathConfigMgr::GetDir()
\see IPathConfigMgr::SetDir(int)
*/
#define APP_FLUID_SIMULATION_DIR	47

/*! The ID used to access the directory where user settings files will be placed.
\see IPathConfigMgr::GetDir()
\see IPathConfigMgr::SetDir(int)
*/
#define APP_USER_SETTINGS_DIR  48

/*! The ID used to access the directory where user tools files will be placed.
\see IPathConfigMgr::GetDir()
\see IPathConfigMgr::SetDir(int)
*/
#define APP_USER_TOOLS_DIR  49

/*! (internal only) The ID used to access the system location where system plugin-in data files are located on install.
	\see IPathConfigMgr::GetDir()
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_PLUGINDATA_DIR  50

/*! (internal only) The ID used to access the system location where system presets are located on install.
	\see IPathConfigMgr::GetDir()
	\see IPathConfigMgr::SetDir(int)
*/
#define APP_PRESETS_DIR  51

//!@}
// closes Max Directories group
