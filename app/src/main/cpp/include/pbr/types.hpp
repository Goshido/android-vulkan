#ifndef PBR_TYPES_HPP
#define PBR_TYPES_HPP


#include <platform/android/mesh_geometry.hpp>
#include <texture2D.hpp>
#include <texture_cube.hpp>
#include "material.hpp"
#include "sampler.hpp"
#include "light.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <list>

GX_RESTORE_WARNING_STATE


namespace pbr {

class Actor;
using ActorRef = std::shared_ptr<Actor>;

class Component;
using ComponentRef = std::shared_ptr<Component>;
using ComponentList = std::list<std::reference_wrapper<Component>>;

class Transformable;
using TransformableList = std::list<std::reference_wrapper<Transformable>>;

class UILayer;
using UILayerList = std::list<std::reference_wrapper<UILayer>>;

using LightRef = std::shared_ptr<Light>;
using MaterialRef = std::shared_ptr<Material>;
using MeshRef = std::shared_ptr<android_vulkan::android::MeshGeometry>;
using Texture2DRef = std::shared_ptr<android_vulkan::Texture2D>;
using TextureCubeRef = std::shared_ptr<android_vulkan::TextureCube>;

} // namespace pbr


#endif // PBR_TYPES_HPP
