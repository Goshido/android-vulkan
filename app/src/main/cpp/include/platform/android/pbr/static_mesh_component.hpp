#ifndef PBR_STATIC_MESH_COMPONENT_HPP
#define PBR_STATIC_MESH_COMPONENT_HPP


#include "actor.hpp"
#include "renderable_component.hpp"
#include "transformable.hpp"
#include <android_vulkan_sdk/pbr/static_mesh_component_desc.hpp>


namespace pbr {

class StaticMeshComponent final : public RenderableComponent, public Transformable
{
    private:
        Actor*                                                          _actor = nullptr;
        GXColorUNORM                                                    _color0;
        GXColorUNORM                                                    _color1;
        GXColorUNORM                                                    _color2;
        std::optional<GeometryPassProgram::ColorData>                   _colorData = std::nullopt;
        GXColorUNORM                                                    _emission { 0U, 0U, 0U, 0U };
        float                                                           _emissionIntensity = 1.0F;
        GXMat4                                                          _localMatrix;
        MaterialRef                                                     _material;
        MeshRef                                                         _mesh;
        GXAABB                                                          _worldBounds;

        static size_t                                                   _commandBufferIndex;
        static std::vector<VkCommandBuffer>                             _commandBuffers;
        static VkCommandPool                                            _commandPool;
        static std::vector<VkFence>                                     _fences;
        static int                                                      _registerComponentIndex;
        static android_vulkan::Renderer*                                _renderer;
        static std::unordered_map<Component const*, ComponentRef>       _staticMeshes;

    public:
        StaticMeshComponent () = delete;

        StaticMeshComponent ( StaticMeshComponent const & ) = delete;
        StaticMeshComponent &operator = ( StaticMeshComponent const & ) = delete;

        StaticMeshComponent ( StaticMeshComponent && ) = delete;
        StaticMeshComponent &operator = ( StaticMeshComponent && ) = delete;

        // "commandBuffer" array MUST contain at least 5 free command buffers.
        explicit StaticMeshComponent ( android_vulkan::Renderer &renderer,
            bool &success,
            size_t &commandBufferConsumed,
            StaticMeshComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers,
            VkFence const* fences
        ) noexcept;

        // "commandBuffer" array MUST contain at least 5 free command buffers.
        explicit StaticMeshComponent ( android_vulkan::Renderer &renderer,
            bool &success,
            size_t &commandBufferConsumed,
            char const* mesh,
            char const* material,
            VkCommandBuffer const* commandBuffers,
            VkFence const* fences,
            std::string &&name
        ) noexcept;

        explicit StaticMeshComponent ( android_vulkan::Renderer &renderer,
            bool &success,
            char const* mesh,
            MaterialRef &material
        ) noexcept;

        ~StaticMeshComponent () override = default;

        void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept override;
        void Submit ( RenderSession &renderSession ) noexcept override;

        void SetColor0 ( GXColorUNORM color ) noexcept;
        void SetColor1 ( GXColorUNORM color ) noexcept;
        void SetColor2 ( GXColorUNORM color ) noexcept;
        void SetEmission ( GXColorUNORM color, float intensity ) noexcept;

        [[nodiscard]] MaterialRef &GetMaterial () noexcept;

        [[nodiscard]] GXMat4 const &GetTransform () const noexcept;
        void SetTransform ( GXMat4 const &transform ) noexcept;

        [[nodiscard]] bool RegisterFromNative ( lua_State &vm, Actor &actor ) noexcept;
        void RegisterFromScript ( Actor &actor ) noexcept;

        [[nodiscard]] static bool Init ( lua_State &vm, android_vulkan::Renderer &renderer ) noexcept;
        static void Destroy () noexcept;

        // Waiting until all mesh data will be uploaded to GPU.
        [[nodiscard]] static bool Sync () noexcept;

    private:
        [[nodiscard]] ComponentRef &GetReference () noexcept override;
        void OnTransform ( GXMat4 const &transformWorld ) noexcept override;

        void UpdateColorData () noexcept;

        [[nodiscard]] static bool AllocateCommandBuffers ( size_t amount ) noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnSetColor0 ( lua_State* state );
        [[nodiscard]] static int OnSetColor1 ( lua_State* state );
        [[nodiscard]] static int OnSetColor2 ( lua_State* state );
        [[nodiscard]] static int OnSetEmission ( lua_State* state );
        [[nodiscard]] static int OnGetLocal ( lua_State* state );
        [[nodiscard]] static int OnSetLocal ( lua_State* state );
        [[nodiscard]] static int OnSetMaterial ( lua_State* state );
};

} // namespace pbr


#endif // PBR_STATIC_MESH_COMPONENT_HPP
