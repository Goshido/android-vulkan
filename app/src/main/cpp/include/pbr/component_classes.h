#ifndef PBR_COMPONENT_CLASSES_H
#define PBR_COMPONENT_CLASSES_H


#include <GXCommon/GXWarning.h>

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
    Script = 4U
};

} // namespace pbr


#endif // PBR_COMPONENT_CLASSES_H
