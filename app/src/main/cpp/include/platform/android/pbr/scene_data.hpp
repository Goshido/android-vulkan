#ifndef PBR_SCENE_DATA_HPP
#define PBR_SCENE_DATA_HPP


#include "geometry_call.hpp"
#include "geometry_pass_material.hpp"


namespace pbr {

using SceneData = std::map<GeometryPassMaterial, GeometryCall>;

} // namespace pbr


#endif // PBR_SCENE_DATA_HPP
