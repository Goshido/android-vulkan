#ifndef PBR_GEOMETRY_DATA_HPP
#define PBR_GEOMETRY_DATA_HPP


#include <vulkan_utils.h>
#include <GXCommon/GXMath.h>


namespace pbr {

AV_DX_ALIGNMENT_BEGIN

struct GeometryData final
{
    bool            _isVisible;
    GXMat4          _local;
    GXAABB          _worldBounds;
    GXColorRGB      _color0;
    GXColorRGB      _color1;
    GXColorRGB      _color2;
    GXColorRGB      _emission;
};

AV_DX_ALIGNMENT_END

} // namespace pbr


#endif // PBR_GEOMETRY_DATA_HPP
