#ifndef PBR_TYPES_H
#define PBR_TYPES_H


#include <mesh_geometry.h>
#include <texture2D.h>
#include <textureCube.h>
#include "material.h"
#include "sampler.h"
#include "light.h"


namespace pbr {

class Component;
using ComponentRef = std::shared_ptr<Component>;

using LightRef = std::shared_ptr<Light>;
using MaterialRef = std::shared_ptr<Material>;
using MeshRef = std::shared_ptr<android_vulkan::MeshGeometry>;
using SamplerRef = std::shared_ptr<Sampler>;
using Texture2DRef = std::shared_ptr<android_vulkan::Texture2D>;
using TextureCubeRef = std::shared_ptr<android_vulkan::TextureCube>;

} // namespace pbr


#endif // PBR_TYPES_H
