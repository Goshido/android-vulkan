#ifndef PBR_ANIMATION_GRAPH_HPP
#define PBR_ANIMATION_GRAPH_HPP


#include "fif_count.hpp"
#include "joint_provider_node.hpp"
#include "node_link.hpp"
#include <buffer_info.hpp>
#include <renderer.hpp>
#include <android_vulkan_sdk/bone_joint.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

#include <string>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

class AnimationGraph final : public NodeLink
{
    public:
        // Making sure that array will be tightly packed. It's important for upload to GPU side.
        using Joints = std::vector<android_vulkan::BoneJoint>;

    private:
        using Reference = std::unique_ptr<AnimationGraph>;
        using Graphs = std::unordered_map<AnimationGraph const*, Reference>;

        class Buffer final
        {
            private:
                VkMappedMemoryRange                     _range
                {
                    .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                    .pNext = nullptr,
                    .memory = VK_NULL_HANDLE,
                    .offset = 0U,
                    .size = 0U
                };

            public:
                VkBuffer                                _buffer = VK_NULL_HANDLE;
                android_vulkan::BoneJoint*              _poseSkin = nullptr;

            public:
                explicit Buffer () = default;

                Buffer ( Buffer const& ) = delete;
                Buffer& operator= ( Buffer const& ) = delete;

                Buffer ( Buffer&& ) = delete;
                Buffer& operator= ( Buffer&& ) = delete;

                ~Buffer () = default;

                [[nodiscard]] bool Init ( VkDeviceSize size, size_t frameInFlightIndex ) noexcept;
                void Destroy () noexcept;
                [[nodiscard]] bool Flush () noexcept;
        };

    private:
        JointProviderNode*                              _inputNode = nullptr;
        Joints                                          _inverseBindTransforms {};
        bool                                            _isSleep = true;
        VkDeviceSize                                    _jointSize = 0U;
        std::vector<std::string>                        _names {};
        Joints                                          _referenceTransforms {};
        std::vector<int32_t>                            _parents {};
        Joints                                          _poseLocal {};
        Joints                                          _poseGlobal {};

        Buffer                                          _buffers[ FIF_COUNT ];
        std::string                                     _skeletonName {};

        static size_t                                   _changedGraphCount;
        static Graphs                                   _graphs;
        static size_t                                   _lastCommandBufferIndex;
        static android_vulkan::Renderer*                _renderer;
        static std::list<Reference>                     _toDelete[ FIF_COUNT ];

    public:
        AnimationGraph () = delete;

        AnimationGraph ( AnimationGraph const & ) = delete;
        AnimationGraph& operator= ( AnimationGraph const & ) = delete;

        AnimationGraph ( AnimationGraph && ) = delete;
        AnimationGraph& operator= ( AnimationGraph && ) = delete;

        explicit AnimationGraph ( bool &success, std::string &&skeletonFile ) noexcept;

        ~AnimationGraph () override = default;

        [[nodiscard]] android_vulkan::BufferInfo GetPoseInfo () const noexcept;
        [[nodiscard]] VkDeviceSize GetPoseRange () const noexcept;
        [[nodiscard]] std::string const &GetSkeletonName () const noexcept;

        static void Init ( lua_State &vm, android_vulkan::Renderer &renderer ) noexcept;
        static void Destroy () noexcept;
        static void Update ( lua_State &vm, float deltaTime, size_t commandBufferIndex ) noexcept;
        [[nodiscard]] static bool UploadGPUData ( size_t commandBufferIndex ) noexcept;

    private:
        void FreeResources () noexcept;
        void UpdateInternal ( lua_State &vm, float deltaTime, size_t commandBufferIndex ) noexcept;

        static void FreeUnusedResources ( size_t commandBufferIndex ) noexcept;

        [[nodiscard]] static int OnAwake ( lua_State* state );
        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnSetInput ( lua_State* state );
        [[nodiscard]] static int OnSleep ( lua_State* state );
};

} // namespace pbr


#endif // PBR_ANIMATION_GRAPH_HPP
