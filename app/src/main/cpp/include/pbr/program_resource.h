#ifndef PBR_PROGRAM_RESOURCE_H
#define PBR_PROGRAM_RESOURCE_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <vector>
#include <vulkan_wrapper.h>

GX_RESTORE_WARNING_STATE


namespace pbr {

struct ProgramResource final
{
    VkDescriptorType    _type;
    uint32_t            _count;

    ProgramResource () = delete;
    explicit ProgramResource ( VkDescriptorType type, uint32_t count );

    ProgramResource ( ProgramResource const & ) = default;
    ProgramResource& operator = ( ProgramResource const & ) = default;

    ProgramResource ( ProgramResource && ) = default;
    ProgramResource& operator = ( ProgramResource && ) = default;
};

using DescriptorSetInfo = std::vector<ProgramResource>;

} // namespace pbr


#endif // PBR_PROGRAM_RESOURCE_H
