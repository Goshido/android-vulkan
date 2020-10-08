#ifndef PBR_OPAQUE_DATA_H
#define PBR_OPAQUE_DATA_H


#include <vulkan_utils.h>


namespace pbr {

AV_DX_ALIGNMENT_BEGIN

struct OpaqueData final
{
    GXMat4          _local;
    GXColorRGB      _color0;
    GXColorRGB      _color1;
    GXColorRGB      _color2;
    GXColorRGB      _color3;
};

AV_DX_ALIGNMENT_END

} // namespace pbr


#endif // PBR_OPAQUE_DATA_H
