#ifndef EDITOR_TRANSFORM_HPP
#define EDITOR_TRANSFORM_HPP


#include <GXCommon/GXMath.hpp>
#include <vulkan_utils.hpp>


namespace editor {

AV_DX_ALIGNMENT_BEGIN

struct Model final
{
    GXVec3      _x;
    GXVec3      _y;
    GXVec3      _z;
    GXVec3      _w;
};

struct Transform final
{
    Model       _model;
    uint64_t    _normal;
};

AV_DX_ALIGNMENT_END

} // namespace editor


#endif // EDITOR_TRANSFORM_HPP
