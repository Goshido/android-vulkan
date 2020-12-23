#ifndef PBR_STATIC_MESH_COMPONENT_H
#define PBR_STATIC_MESH_COMPONENT_H


#include "component.h"
#include "static_mesh_component_desc.h"
#include "types.h"


namespace pbr {

class StaticMeshComponent final : public Component
{
    private:
        android_vulkan::Half4       _color0;
        android_vulkan::Half4       _color1;
        android_vulkan::Half4       _color2;
        android_vulkan::Half4       _color3;
        GXMat4                      _localMatrix;
        MaterialRef                 _material;
        MeshRef                     _mesh;

    public:
        StaticMeshComponent () = delete;

        StaticMeshComponent ( StaticMeshComponent const & ) = delete;
        StaticMeshComponent& operator = ( StaticMeshComponent const & ) = delete;

        StaticMeshComponent ( StaticMeshComponent && ) = delete;
        StaticMeshComponent& operator = ( StaticMeshComponent && ) = delete;

        // "commandBuffer" array MUST contain at least 5 free command buffers.
        explicit StaticMeshComponent ( size_t &commandBufferConsumed,
            StaticMeshComponentDesc const &desc,
            uint8_t const *data,
            android_vulkan::Renderer &renderer,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        ~StaticMeshComponent () override = default;

    private:
        void FreeTransferResources ( android_vulkan::Renderer &renderer ) override;
        void Submit ( RenderSession &renderSession ) override;
};

} // namespace pbr


#endif // PBR_STATIC_MESH_COMPONENT_H
