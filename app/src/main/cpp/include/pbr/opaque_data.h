#ifndef PBR_OPAQUE_DATA_H
#define PBR_OPAQUE_DATA_H


#include <color32.h>
#include <vulkan_utils.h>


namespace pbr {

AV_DX_ALIGNMENT_BEGIN

struct OpaqueData final
{
    bool                        _isVisible;
    GXMat4                      _local;
    GXAABB                      _worldBounds;
    android_vulkan::Color32     _color0;
    android_vulkan::Color32     _color1;
    android_vulkan::Color32     _color2;
    android_vulkan::Color32     _color3;
};

AV_DX_ALIGNMENT_END

} // namespace pbr


#endif // PBR_OPAQUE_DATA_H
