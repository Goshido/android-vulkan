#ifndef PBR_SCENE_DATA_H
#define PBR_SCENE_DATA_H


#include "opaque_call.h"
#include "opaque_material.h"


namespace pbr {

using SceneData = std::map<OpaqueMaterial, OpaqueCall>;

} // namespace pbr


#endif // PBR_SCENE_DATA_H
