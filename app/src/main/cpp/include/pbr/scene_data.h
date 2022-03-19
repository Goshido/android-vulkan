#ifndef PBR_SCENE_DATA_H
#define PBR_SCENE_DATA_H


#include "geometry_call.h"
#include "geometry_pass_material.h"


namespace pbr {

using SceneData = std::map<GeometryPassMaterial, GeometryCall>;

} // namespace pbr


#endif // PBR_SCENE_DATA_H
