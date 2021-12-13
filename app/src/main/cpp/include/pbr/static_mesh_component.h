#ifndef PBR_STATIC_MESH_COMPONENT_H
#define PBR_STATIC_MESH_COMPONENT_H


#include "component.h"
#include "static_mesh_component_desc.h"
#include "types.h"


namespace pbr {

class StaticMeshComponent final : public Component
{
    private:
        android_vulkan::ColorUnorm      _color0;
        android_vulkan::ColorUnorm      _color1;
        android_vulkan::ColorUnorm      _color2;
        android_vulkan::ColorUnorm      _color3;
        GXMat4                          _localMatrix;
        MaterialRef                     _material;
        MeshRef                         _mesh;
        GXAABB                          _worldBounds;

    public:
        StaticMeshComponent () = delete;

        StaticMeshComponent ( StaticMeshComponent const & ) = delete;
        StaticMeshComponent& operator = ( StaticMeshComponent const & ) = delete;

        StaticMeshComponent ( StaticMeshComponent && ) = delete;
        StaticMeshComponent& operator = ( StaticMeshComponent && ) = delete;

        // "commandBuffer" array MUST contain at least 5 free command buffers.
        explicit StaticMeshComponent ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            StaticMeshComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        // "commandBuffer" array MUST contain at least 5 free command buffers.
        explicit StaticMeshComponent ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            char const* mesh,
            char const* material,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        ~StaticMeshComponent () override = default;

        void Submit ( RenderSession &renderSession ) override;

        [[maybe_unused, nodiscard]] GXAABB const& GetBoundsWorld () const noexcept;

        [[maybe_unused, nodiscard]] android_vulkan::ColorUnorm const& GetColor0 () const noexcept;
        [[maybe_unused]] void SetColor0 ( GXColorRGB const &color ) noexcept;
        void SetColor0 ( android_vulkan::ColorUnorm const &color ) noexcept;

        [[maybe_unused, nodiscard]] android_vulkan::ColorUnorm const& GetColor1 () const noexcept;
        [[maybe_unused]] void SetColor1 ( GXColorRGB const &color ) noexcept;
        [[maybe_unused]] void SetColor1 ( android_vulkan::ColorUnorm const &color ) noexcept;

        [[maybe_unused, nodiscard]] android_vulkan::ColorUnorm const& GetColor2 () const noexcept;
        [[maybe_unused]] void SetColor2 ( GXColorRGB const &color ) noexcept;
        [[maybe_unused]] void SetColor2 ( android_vulkan::ColorUnorm const &color ) noexcept;

        [[maybe_unused, nodiscard]] android_vulkan::ColorUnorm const& GetColor3 () const noexcept;
        [[maybe_unused]] void SetColor3 ( GXColorRGB const &color ) noexcept;
        [[maybe_unused]] void SetColor3 ( android_vulkan::ColorUnorm const &color ) noexcept;

        [[nodiscard]] MaterialRef& GetMaterial () noexcept;
        [[maybe_unused, nodiscard]] MaterialRef const& GetMaterial () const noexcept;

        [[nodiscard]] GXMat4 const& GetTransform () const noexcept;
        void SetTransform ( GXMat4 const &transform ) noexcept;

    private:
        void FreeTransferResources ( VkDevice device ) override;
};

} // namespace pbr


#endif // PBR_STATIC_MESH_COMPONENT_H
