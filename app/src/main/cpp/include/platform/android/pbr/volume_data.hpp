#ifndef PBR_VOLUME_DATA_HPP
#define PBR_VOLUME_DATA_HPP


#include <GXCommon/GXMath.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

AV_DX_ALIGNMENT_BEGIN

struct VolumeData final
{
    [[maybe_unused]] GXMat4     _transform;
};

AV_DX_ALIGNMENT_END

} // namespace pbr


#endif // PBR_VOLUME_DATA_HPP
