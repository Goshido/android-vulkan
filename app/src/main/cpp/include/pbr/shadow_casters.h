#ifndef PBR_SHADOW_CASTERS_H
#define PBR_SHADOW_CASTERS_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <unordered_map>

GX_RESTORE_WARNING_STATE

#include "batch_shadow_caster.h"
#include "unique_shadow_caster.h"


namespace pbr {

struct ShadowCasters final
{
    [[maybe_unused]] std::unordered_map<std::string_view, BatchShadowCaster>    _batches;
    [[maybe_unused]] std::vector<UniqueShadowCaster>                            _uniques;
};

} // namespace pbr


#endif // PBR_SHADOW_CASTERS_H
