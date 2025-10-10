#ifndef PBR_GEOMETRY_DATA_HPP
#define PBR_GEOMETRY_DATA_HPP


#include "geometry_pass_program.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

struct GeometryData final
{
    bool                                _isVisible;
    GXMat4                              _local;
    GXAABB                              _worldBounds;
    GeometryPassProgram::ColorData      _colorData;
};

} // namespace pbr


#endif // PBR_GEOMETRY_DATA_HPP
