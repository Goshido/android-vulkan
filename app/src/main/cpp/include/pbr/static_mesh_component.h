#ifndef PBR_STATIC_MESH_COMPONENT_H
#define PBR_STATIC_MESH_COMPONENT_H


#include "actor.h"
#include "renderable_component.h"
#include "static_mesh_component_desc.h"
#include "transformable.h"


namespace pbr {

class StaticMeshComponent final : public RenderableComponent, public Transformable
{
    private:
        Actor*          _actor = nullptr;
        GXColorRGB      _color0;
        GXColorRGB      _color1;
        GXColorRGB      _color2;
        GXColorRGB      _emission;
        GXMat4          _localMatrix;
        MaterialRef     _material;
        MeshRef         _mesh;
        GXAABB          _worldBounds;

        static int      _registerStaticMeshComponentIndex;

    public:
        StaticMeshComponent () = delete;

        StaticMeshComponent ( StaticMeshComponent const & ) = delete;
        StaticMeshComponent& operator = ( StaticMeshComponent const & ) = delete;

        StaticMeshComponent ( StaticMeshComponent && ) = delete;
        StaticMeshComponent& operator = ( StaticMeshComponent && ) = delete;

        // "commandBuffer" array MUST contain at least 5 free command buffers.
        explicit StaticMeshComponent ( android_vulkan::Renderer &renderer,
            bool &success,
            size_t &commandBufferConsumed,
            StaticMeshComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        // "commandBuffer" array MUST contain at least 5 free command buffers.
        explicit StaticMeshComponent ( android_vulkan::Renderer &renderer,
            bool &success,
            size_t &commandBufferConsumed,
            char const* mesh,
            char const* material,
            VkCommandBuffer const* commandBuffers,
            std::string &&name
        ) noexcept;

        // "commandBuffer" array MUST contain at least 1 free command buffers.
        explicit StaticMeshComponent ( android_vulkan::Renderer &renderer,
            bool &success,
            size_t &commandBufferConsumed,
            char const* mesh,
            MaterialRef &material,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        ~StaticMeshComponent () override = default;

        void FreeTransferResources ( VkDevice device ) noexcept override;
        void Submit ( RenderSession &renderSession ) noexcept override;

        [[maybe_unused, nodiscard]] GXAABB const& GetBoundsWorld () const noexcept;

        [[maybe_unused, nodiscard]] GXColorRGB const& GetColor0 () const noexcept;
        [[maybe_unused]] void SetColor0 ( GXColorRGB const &color ) noexcept;

        [[maybe_unused, nodiscard]] GXColorRGB const& GetColor1 () const noexcept;
        [[maybe_unused]] void SetColor1 ( GXColorRGB const &color ) noexcept;

        [[maybe_unused, nodiscard]] GXColorRGB const& GetColor2 () const noexcept;
        [[maybe_unused]] void SetColor2 ( GXColorRGB const &color ) noexcept;

        [[maybe_unused, nodiscard]] GXColorRGB const& GetEmission () const noexcept;
        void SetEmission ( GXColorRGB const &emission ) noexcept;

        [[nodiscard]] MaterialRef& GetMaterial () noexcept;
        [[maybe_unused, nodiscard]] MaterialRef const& GetMaterial () const noexcept;

        [[maybe_unused, nodiscard]] GXMat4 const& GetTransform () const noexcept;
        void SetTransform ( GXMat4 const &transform ) noexcept;

        [[nodiscard]] bool Register ( lua_State &vm, Actor &actor ) noexcept;

        [[nodiscard]] static bool Init ( lua_State &vm ) noexcept;

    private:
        void OnTransform ( GXMat4 const &transformWorld ) noexcept override;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGetLocal ( lua_State* state );
        [[nodiscard]] static int OnSetLocal ( lua_State* state );
};

} // namespace pbr


#endif // PBR_STATIC_MESH_COMPONENT_H
