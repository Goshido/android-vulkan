#ifndef PBR_SCRIPT_COMPONENT_DESC_H
#define PBR_SCRIPT_COMPONENT_DESC_H


#include "component_desc.h"
#include "primitive_types.h"


namespace pbr {

#pragma pack ( push, 1 )

struct ScriptComponentDesc final : public ComponentDesc
{
    android_vulkan::UTF8Offset      _script;
    android_vulkan::UTF8Offset      _params;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_SCRIPT_COMPONENT_DESC_H