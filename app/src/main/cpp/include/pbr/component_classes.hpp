#ifndef PBR_COMPONENT_CLASSES_HPP
#define PBR_COMPONENT_CLASSES_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>

GX_RESTORE_WARNING_STATE


namespace pbr {

enum class ClassID : uint64_t
{
    Unknown = 0U,
    StaticMesh = 1U,
    PointLight = 2U,
    Reflection = 3U,
    Script = 4U,
    RigidBody = 5U,
    Camera = 6U,
    Transform = 7U,
    SoundEmitterGlobal = 8U,
    SoundEmitterSpatial = 9U,

    COUNT = 10U
};

} // namespace pbr


#endif // PBR_COMPONENT_CLASSES_HPP
