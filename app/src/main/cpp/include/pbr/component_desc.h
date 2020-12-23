#ifndef PBR_COMPONENT_DESC_H
#define PBR_COMPONENT_DESC_H


#include "component_classes.h"


namespace pbr {

#pragma pack ( push, 1 )

struct ComponentDesc
{
    ClassID     _classID;
    uint32_t    _formatVersion;
};

#pragma pack ( pop )

} // namespace pbr


#endif // PBR_COMPONENT_DESC_H
