//**************************************************************************/
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: File Resolution Manager => resolves the physical location of
// an asset file
// AUTHOR: Peter Ochodo
// DATE: 2008-06-04 (YYYY-MM-DD) 
//***************************************************************************/

#pragma once

namespace MaxSDK
{
	namespace AssetManagement
	{
		enum AssetType { 
			//NB: it is important to leave kOtherAsset at the beginning of this list and kPredefinedAssetTypeCount
			// at the bottom of this list because some code depends on these properties.
			kOtherAsset,		/*!< A value to use if the asset type doesn't fit into any of the pre-defined types below. */	
			kBitmapAsset,		/*!< Any image or shader file. */
			kXRefAsset,			/*!< Any XRef asset. */
			kPhotometricAsset,  /*!< Any photometric info file. */
			kAnimationAsset,	/*!< Any animation file. */
			kVideoPost,			/*!< Any file used by a VideoPost plugin. */
			kBatchRender,		/*!< Any file used by the Batch Renderer. */
			kExternalLink,		/*!< Any file linked to externally (i.e. FileLink material). */	
			kRenderOutput,		/*!< The file that is output after a render. */
			kPreRenderScript,	/*!< A script file that runs before rendering. */
			kPostRenderScript,	/*!< A script flle that runs after rendering. */
			kSoundAsset,		/*!< Any sound file. */
			kContainerAsset,	/*!< Any container file. */
			kMaxCreationGraph,	/*!< Any Max Creation Graph file. */
			kScript,			/*!< Any script file other than a pre or post render script file. */
			kPredefinedAssetTypeCount		/*!< Total number of predefined asset types. */
		};
	}
}

