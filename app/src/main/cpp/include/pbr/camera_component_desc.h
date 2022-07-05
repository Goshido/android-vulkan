#ifndef PBR_CAMERA_COMPONENT_DESC_H
#define PBR_CAMERA_COMPONENT_DESC_H


#include "component_desc.h"
#include "primitive_types.h"


namespace pbr {

#pragma pack ( push, 1 )

struct CameraComponentDesc final : public ComponentDesc
{
    float                       _fieldOfViewRadians;
    android_vulkan::Mat4x4      _localMatrix;
    float                       _zFar;
    float                       _zNear;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_CAMERA_COMPONENT_DESC_H
