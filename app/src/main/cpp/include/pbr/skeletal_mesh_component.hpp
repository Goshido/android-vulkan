#ifndef PBR_SKELETAL_MESH_COMPONENT_HPP
#define PBR_SKELETAL_MESH_COMPONENT_HPP


#include "actor.hpp"
#include "renderable_component.hpp"
#include "transformable.hpp"


namespace pbr {

class SkeletalMeshComponent final : public RenderableComponent, public Transformable
{
    private:
        struct Usage final
        {
            MeshRef                                                     _skinMesh {};
            std::vector<bool>                                           _frameIds {};
        };

    private:
        Actor*                                                          _actor = nullptr;
        GXColorRGB                                                      _color0;
        GXColorRGB                                                      _color1;
        GXColorRGB                                                      _color2;
        GXColorRGB                                                      _emission;

        MaterialRef                                                     _material;

        GXMat4                                                          _localMatrix;

        GXAABB                                                          _localBounds;
        GXAABB                                                          _worldBounds;

        MeshRef                                                         _referenceMesh;
        Usage                                                           _usage;

        static std::list<Usage>                                         _aboutDelete;
        static size_t                                                   _commandBufferIndex;
        static std::vector<VkCommandBuffer>                             _commandBuffers;
        static VkCommandPool                                            _commandPool;
        static std::vector<VkFence>                                     _fences;
        static android_vulkan::Renderer*                                _renderer;
        static std::unordered_map<Component const*, ComponentRef>       _referenceMeshes;
        static std::list<Usage>                                         _toDelete;

    public:
        SkeletalMeshComponent ( bool &success,
            size_t &commandBufferConsumed,
            char const* mesh,
            char const* skin,
            char const* material,
            VkCommandBuffer const* commandBuffers,
            VkFence const* fences,
            std::string &&name
        ) noexcept;

        SkeletalMeshComponent ( SkeletalMeshComponent const & ) = delete;
        SkeletalMeshComponent &operator = ( SkeletalMeshComponent const & ) = delete;

        SkeletalMeshComponent ( SkeletalMeshComponent && ) = delete;
        SkeletalMeshComponent &operator = ( SkeletalMeshComponent && ) = delete;

        ~SkeletalMeshComponent () override = default;

        void RegisterFromScript ( Actor &actor ) noexcept;
        void Unregister () noexcept;
        [[maybe_unused]] void UpdatePose ( size_t swapchainImageIndex ) noexcept;

        static void FreeUnusedResources ( size_t swapchainImageIndex ) noexcept;
        [[nodiscard]] static bool Init ( lua_State &vm, android_vulkan::Renderer &renderer ) noexcept;
        static void Destroy () noexcept;

    private:
        [[nodiscard]] ComponentRef &GetReference () noexcept override;
        void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept override;
        void Submit ( RenderSession &renderSession ) noexcept override;
        void OnTransform ( GXMat4 const &transformWorld ) noexcept override;

        void SetColor0 ( GXColorRGB const &color ) noexcept;
        void SetColor1 ( GXColorRGB const &color ) noexcept;
        void SetColor2 ( GXColorRGB const &color ) noexcept;
        void SetEmission ( GXColorRGB const &emission ) noexcept;

        [[nodiscard]] static bool AllocateCommandBuffers ( size_t amount ) noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnSetColor0 ( lua_State* state );
        [[nodiscard]] static int OnSetColor1 ( lua_State* state );
        [[nodiscard]] static int OnSetColor2 ( lua_State* state );
        [[nodiscard]] static int OnSetEmission ( lua_State* state );
        [[nodiscard]] static int OnSetMaterial ( lua_State* state );
};

} // namespace pbr


#endif //  PBR_SKELETAL_MESH_COMPONENT_HPP
