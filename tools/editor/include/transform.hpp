#ifndef EDITOR_TRANSFORM_HPP
#define EDITOR_TRANSFORM_HPP


#include <GXCommon/GXMath.hpp>
#include <vulkan_utils.hpp>


namespace editor {

AV_DX_ALIGNMENT_BEGIN

struct Transform final
{
    uint64_t    _rotation;
    GXVec3      _location;
    GXVec3      _scale;
};

AV_DX_ALIGNMENT_END

} // namespace editor


#endif // EDITOR_TRANSFORM_HPP
