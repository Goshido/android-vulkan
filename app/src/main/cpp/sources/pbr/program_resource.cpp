#include <pbr/program_resource.h>


namespace pbr {

ProgramResource::ProgramResource ( VkDescriptorType type, uint32_t count ):
    _type ( type ),
    _count ( count )
{
    // NOTHING
}

} // namespace pbr
