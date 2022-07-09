#ifndef PBR_RIGID_BODY_DESC_H
#define PBR_RIGID_BODY_DESC_H


#include "component_desc.h"
#include "primitive_types.h"


namespace pbr {

#pragma pack ( push, 1 )

enum class eShapeTypeDesc : uint8_t
{
    Box = 0U
};

enum class eRigidBodyTypeDesc : uint8_t
{
    Dynamic = 0U,
    Kinematic = 1U
};

struct ShapeDesc
{
    eShapeTypeDesc      _type;
};

struct ShapeBoxDesc final : public ShapeDesc
{
    uint32_t                _formatVersion;
    uint32_t                _collisionGroups;
    android_vulkan::Vec3    _dimensions;
    float                   _friction;
    float                   _mass;
    float                   _restitution;
};

struct RigidBodyComponentDesc final : public ComponentDesc
{
    android_vulkan::Mat4x4      _localMatrix;
    eRigidBodyTypeDesc          _type;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_RIGID_BODY_DESC_H
