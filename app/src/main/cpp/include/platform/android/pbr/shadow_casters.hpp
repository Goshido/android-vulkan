#ifndef PBR_SHADOW_CASTERS_HPP
#define PBR_SHADOW_CASTERS_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

using BatchShadowCaster = std::pair<MeshRef, std::vector<GXMat4>>;

struct ShadowCasters final
{
    std::unordered_map<std::string_view, BatchShadowCaster>     _batches;
    std::vector<MeshRef>                                        _uniques;
};

} // namespace pbr


#endif // PBR_SHADOW_CASTERS_HPP
