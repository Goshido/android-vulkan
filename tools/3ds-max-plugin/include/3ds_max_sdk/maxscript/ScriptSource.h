//**************************************************************************/
// Copyright 2020 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise accompanies 
// this software in either electronic or hard copy form.  

#pragma once

/*! \defgroup ScriptSourceTypes Script Source Types
Defines the possible sources of the scripts executed by 3ds Max through the MAXScript engine. Different execution rights apply to scripts 
depending on their source. Execution rights could be defined by 3ds Max's security settings or could be hard coded.
The Script Source type is specified when compiling MAXScript code and specifies whether the source of the MAXScript code was a scene file 
(i.e., a scene file embedded script); a script file, manually evaluated script, or hardcoded script executed via c++; or a script 
executed via c++ where the script includes portions that are not hard coded (for example file names, scene nodes names, scripts read
from configuration files). 
The Script Source type is used when executing the MAXScript code to determine the security rights of the code. For example, based on 
security settings a scene file embedded script may not be able to create most dotnet classes.
\sa class ISceneScriptSecurityManager
\n\n
*/

namespace MAXScript {
	enum class ScriptSource : int
	{
		NotSpecified, 	//!< source of the script was not specified; same security rights as for Embedded scripts will apply
		Embedded,		//!< source of the script was a scene file (i.e., a scene file embedded script), security rights dependent on Safe Scene Script Execution settings
		NonEmbedded,	//!< source of the script was not a scene file, full security rights, i.e. no security restrictions.
		Dynamic			//!< the script is dynamically generated at runtime and could contain input from various sources that are interpreted as scripted commands. Therefore dynamic scripts will always execute with restricted security rights, regardless of Safe Scene Script Execution settings
	};
}