#ifndef PBR_SCRIPT_COMPONENT_DESC_HPP
#define PBR_SCRIPT_COMPONENT_DESC_HPP


#include "component_desc.hpp"
#include <android_vulkan_sdk/primitive_types.hpp>


namespace pbr {

#pragma pack ( push, 1 )

struct ScriptComponentDesc final : public ComponentDesc
{
    android_vulkan::UTF8Offset      _script;
    android_vulkan::UTF8Offset      _params;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_SCRIPT_COMPONENT_DESC_HPP
