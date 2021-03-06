#ifndef ROTATING_MESH_DRAWCALL_H
#define ROTATING_MESH_DRAWCALL_H


#include "mesh_geometry.h"
#include <texture2D.h>


namespace rotating_mesh {

struct Drawcall final
{
    VkDescriptorSet                 _descriptorSet;

    android_vulkan::Texture2D       _diffuse;
    VkSampler                       _diffuseSampler;

    android_vulkan::MeshGeometry    _mesh;

    android_vulkan::Texture2D       _normal;
    VkSampler                       _normalSampler;
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_DRAWCALL_H
