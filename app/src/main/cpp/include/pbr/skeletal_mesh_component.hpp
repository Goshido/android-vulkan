#ifndef PBR_SKELETAL_MESH_COMPONENT_HPP
#define PBR_SKELETAL_MESH_COMPONENT_HPP


#include "actor.hpp"
#include "command_buffer_count.hpp"
#include "renderable_component.hpp"
#include "transformable.hpp"
#include <skin_data.hpp>


namespace pbr {

class SkeletalMeshComponent final : public RenderableComponent, public Transformable
{
    private:
        struct Usage final
        {
            android_vulkan::SkinData        _skinData {};
            MeshRef                         _skinMesh {};
        };

        struct CommandBufferInfo final
        {
            std::vector<VkCommandBuffer>    _buffers {};
            VkCommandPool                   _commandPool = VK_NULL_HANDLE;
            size_t                          _index = 0U;
            std::vector<VkFence>            _fences {};
        };

        using SkeletalMeshes = std::unordered_map<Component const*, ComponentRef>;

    private:
        Actor*                              _actor = nullptr;
        GXColorRGB                          _color0;
        GXColorRGB                          _color1;
        GXColorRGB                          _color2;
        GXColorRGB                          _emission;

        MaterialRef                         _material;
        GXMat4                              _localMatrix;
        GXAABB                              _worldBounds;

        MeshRef                             _referenceMesh;
        Usage                               _usage;

        static CommandBufferInfo            _cbInfo;
        static size_t                       _lastCommandBufferIndex;
        static android_vulkan::Renderer*    _renderer;
        static SkeletalMeshes               _skeletalMeshes;
        static std::list<Usage>             _toDelete[ DUAL_COMMAND_BUFFER ];
        static std::deque<Usage*>           _transferQueue;

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
            char const* material,
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

        void SetColor0 ( GXColorRGB const &color ) noexcept;
        void SetColor1 ( GXColorRGB const &color ) noexcept;
        void SetColor2 ( GXColorRGB const &color ) noexcept;
        void SetEmission ( GXColorRGB const &emission ) noexcept;

        void SetTransform ( GXMat4 const &transform ) noexcept;

        [[nodiscard]] static bool AllocateCommandBuffers ( size_t amount ) noexcept;
        static void FreeUnusedResources ( size_t commandBufferIndex ) noexcept;
        [[nodiscard]] static bool WaitGPUUploadComplete () noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnSetColor0 ( lua_State* state );
        [[nodiscard]] static int OnSetColor1 ( lua_State* state );
        [[nodiscard]] static int OnSetColor2 ( lua_State* state );
        [[nodiscard]] static int OnSetEmission ( lua_State* state );
        [[nodiscard]] static int OnSetLocal ( lua_State* state );
        [[nodiscard]] static int OnSetMaterial ( lua_State* state );
};

} // namespace pbr


#endif //  PBR_SKELETAL_MESH_COMPONENT_HPP
