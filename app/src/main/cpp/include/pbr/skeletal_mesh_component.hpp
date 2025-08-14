#ifndef PBR_SKELETAL_MESH_COMPONENT_HPP
#define PBR_SKELETAL_MESH_COMPONENT_HPP


#include "actor.hpp"
#include "animation_graph.hpp"
#include "fif_count.hpp"
#include "renderable_component.hpp"
#include "skin_pool.hpp"
#include "skin_program.hpp"
#include "transformable.hpp"
#include <skin_data.hpp>


namespace pbr {

class SkeletalMeshComponent final : public RenderableComponent, public Transformable
{
    private:
        struct Usage final
        {
            android_vulkan::SkinData                        _skinData {};
            MeshRef                                         _skinMesh {};
        };

        struct CommandBufferInfo final
        {
            std::vector<VkCommandBuffer>                    _buffers {};
            VkCommandPool                                   _commandPool = VK_NULL_HANDLE;
            size_t                                          _index = 0U;
            std::vector<VkFence>                            _fences {};
        };

        using SkeletalMeshes = std::unordered_map<Component const*, ComponentRef>;

    private:
        Actor*                                              _actor = nullptr;
        AnimationGraph*                                     _animationGraph = nullptr;

        GXColorUNORM                                        _color0;
        GXColorUNORM                                        _color1;
        GXColorUNORM                                        _color2;
        std::optional<GeometryPassProgram::ColorData>       _colorData = std::nullopt;
        GXColorUNORM                                        _emission { 0U, 0U, 0U, 0U };
        float                                               _emissionIntensity = 1.0F;

        VkExtent3D                                          _dispatch {};

        MaterialRef                                         _material;
        GXMat4                                              _localMatrix;
        GXAABB                                              _worldBounds;

        MeshRef                                             _referenceMesh;
        Usage                                               _usage;

        static CommandBufferInfo                            _cbInfo;
        static size_t                                       _lastCommandBufferIndex;
        static SkinProgram                                  _program;
        static android_vulkan::Renderer*                    _renderer;
        static SkeletalMeshes                               _skeletalMeshes;
        static SkinPool                                     _skinPool;
        static std::list<Usage>                             _toDelete[ FIF_COUNT ];
        static std::deque<Usage*>                           _transferQueue;

    public:
        SkeletalMeshComponent () = delete;

        SkeletalMeshComponent ( SkeletalMeshComponent const & ) = delete;
        SkeletalMeshComponent &operator = ( SkeletalMeshComponent const & ) = delete;

        SkeletalMeshComponent ( SkeletalMeshComponent && ) = delete;
        SkeletalMeshComponent &operator = ( SkeletalMeshComponent && ) = delete;

        SkeletalMeshComponent ( bool &success,
            size_t &commandBufferConsumed,
            char const* mesh,
            char const* skin,
            char const* skeleton,
            VkCommandBuffer const* commandBuffers,
            VkFence const* fences,
            std::string &&name
        ) noexcept;

        ~SkeletalMeshComponent () override = default;

        void RegisterFromScript ( Actor &actor ) noexcept;
        void Unregister () noexcept;

        [[nodiscard]] static bool ApplySkin ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept;
        [[nodiscard]] static bool Init ( lua_State &vm, android_vulkan::Renderer &renderer ) noexcept;
        static void Destroy () noexcept;

    private:
        [[nodiscard]] ComponentRef &GetReference () noexcept override;
        void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept override;
        void Submit ( RenderSession &renderSession ) noexcept override;
        void OnTransform ( GXMat4 const &transformWorld ) noexcept override;

        [[nodiscard]] bool SetAnimationGraph ( AnimationGraph &animationGraph ) noexcept;

        void SetColor0 ( GXColorUNORM color ) noexcept;
        void SetColor1 ( GXColorUNORM color ) noexcept;
        void SetColor2 ( GXColorUNORM color ) noexcept;
        void SetEmission ( GXColorUNORM color, float intensity ) noexcept;

        void SetTransform ( GXMat4 const &transform ) noexcept;
        void UpdateColorData () noexcept;

        [[nodiscard]] static bool AllocateCommandBuffers ( size_t amount ) noexcept;
        static void FreeUnusedResources ( size_t commandBufferIndex ) noexcept;
        [[nodiscard]] static bool WaitGPUUploadComplete () noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnSetAnimationGraph ( lua_State* state );
        [[nodiscard]] static int OnSetColor0 ( lua_State* state );
        [[nodiscard]] static int OnSetColor1 ( lua_State* state );
        [[nodiscard]] static int OnSetColor2 ( lua_State* state );
        [[nodiscard]] static int OnSetEmission ( lua_State* state );
        [[nodiscard]] static int OnGetLocal ( lua_State* state );
        [[nodiscard]] static int OnSetLocal ( lua_State* state );
        [[nodiscard]] static int OnSetMaterial ( lua_State* state );
};

} // namespace pbr


#endif //  PBR_SKELETAL_MESH_COMPONENT_HPP
