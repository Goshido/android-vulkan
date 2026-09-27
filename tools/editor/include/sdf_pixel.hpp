#ifndef EDITOR_SDF_PIXEL_HPP
#define EDITOR_SDF_PIXEL_HPP


#include <GXCommon/GXMath.hpp>
#include <vulkan_utils.hpp>


namespace editor {

AV_DX_ALIGNMENT_BEGIN

struct SDFPixel final
{
    GXVec4      _sdfParams;
    GXVec3      _cameraLocationSDF;
    GXVec3      _viSDF;
};

AV_DX_ALIGNMENT_END

} // namespace editor


#endif // EDITOR_SDF_PIXEL_HPP
