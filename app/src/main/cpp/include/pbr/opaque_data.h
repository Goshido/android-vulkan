#ifndef PBR_OPAQUE_DATA_H
#define PBR_OPAQUE_DATA_H


#include <vulkan_utils.h>
#include <GXCommon/GXMath.h>


namespace pbr {

AV_DX_ALIGNMENT_BEGIN

struct OpaqueData final
{
    bool            _isVisible;
    GXMat4          _local;
    GXAABB          _worldBounds;
    GXColorRGB      _color0;
    GXColorRGB      _color1;
    GXColorRGB      _color2;
    GXColorRGB      _color3;
};

AV_DX_ALIGNMENT_END

} // namespace pbr


#endif // PBR_OPAQUE_DATA_H
