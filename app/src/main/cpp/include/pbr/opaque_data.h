#ifndef PBR_OPAQUE_DATA_H
#define PBR_OPAQUE_DATA_H


#include <vulkan_utils.h>
#include <half_types.h>


namespace pbr {

AV_DX_ALIGNMENT_BEGIN

struct OpaqueData final
{
    bool                        _isVisible;
    GXMat4                      _local;
    GXAABB                      _worldBounds;
    android_vulkan::Half4       _color0;
    android_vulkan::Half4       _color1;
    android_vulkan::Half4       _color2;
    android_vulkan::Half4       _color3;
};

AV_DX_ALIGNMENT_END

} // namespace pbr


#endif // PBR_OPAQUE_DATA_H
