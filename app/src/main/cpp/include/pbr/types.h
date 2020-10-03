#ifndef PBR_TYPES_H
#define PBR_TYPES_H


#include <mesh_geometry.h>
#include <texture2D.h>
#include "material.h"
#include "sampler.h"


namespace pbr {

class Component;
using ComponentRef = std::shared_ptr<Component>;

using MaterialRef = std::shared_ptr<Material>;
using MeshRef = std::shared_ptr<android_vulkan::MeshGeometry>;
using SamplerRef = std::shared_ptr<Sampler>;
using Texture2DRef = std::shared_ptr<android_vulkan::Texture2D>;

} // namespace pbr


#endif // PBR_TYPES_H
