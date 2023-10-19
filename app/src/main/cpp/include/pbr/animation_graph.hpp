#ifndef PBR_ANIMATION_GRAPH_HPP
#define PBR_ANIMATION_GRAPH_HPP


#include "command_buffer_count.hpp"
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
            public:
                VkBuffer                                _buffer = VK_NULL_HANDLE;
                VkDeviceMemory                          _memory = VK_NULL_HANDLE;
                VkDeviceSize                            _offset = std::numeric_limits<VkDeviceSize>::max ();

            public:
                Buffer () = default;

                Buffer ( Buffer const& ) = delete;
                Buffer& operator= ( Buffer const& ) = delete;

                Buffer ( Buffer&& ) = delete;
                Buffer& operator= ( Buffer&& ) = delete;

                ~Buffer () = default;

                [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
                    VkDeviceSize size,
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags memoryFlags
                ) noexcept;

                void Destroy ( bool isMapped ) noexcept;
        };

        class BufferSet final
        {
            public:
                Buffer                                  _gpuPoseSkin {};
                Buffer                                  _transferPoseSkin {};
                android_vulkan::BoneJoint*              _poseSkin = nullptr;

            public:
                BufferSet () = default;

                BufferSet ( BufferSet const & ) = delete;
                BufferSet& operator= ( BufferSet const & ) = delete;

                BufferSet ( BufferSet && ) = delete;
                BufferSet& operator= ( BufferSet && ) = delete;

                ~BufferSet () = default;

                [[nodiscard]] bool Init ( VkDeviceSize size ) noexcept;
                void Destroy () noexcept;
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

        BufferSet                                       _bufferSets[ DUAL_COMMAND_BUFFER ];
        std::string                                     _skeletonName {};

        static std::vector<VkBufferMemoryBarrier>       _barriers;
        static size_t                                   _changedGraphCount;
        static Graphs                                   _graphs;
        static size_t                                   _lastCommandBufferIndex;
        static android_vulkan::Renderer*                _renderer;
        static std::list<Reference>                     _toDelete[ DUAL_COMMAND_BUFFER ];

    public:
        AnimationGraph () = delete;

        AnimationGraph ( AnimationGraph const & ) = delete;
        AnimationGraph& operator= ( AnimationGraph const & ) = delete;

        AnimationGraph ( AnimationGraph && ) = delete;
        AnimationGraph& operator= ( AnimationGraph && ) = delete;

        explicit AnimationGraph ( bool &success, std::string &&skeletonFile ) noexcept;

        ~AnimationGraph () = default;

        [[nodiscard]] android_vulkan::BufferInfo GetPoseInfo () const noexcept;
        [[nodiscard]] VkDeviceSize GetPoseRange () const noexcept;
        [[nodiscard]] std::string const &GetSkeletonName () const noexcept;

        static void Init ( lua_State &vm, android_vulkan::Renderer &renderer ) noexcept;
        static void Destroy () noexcept;
        static void Update ( float deltaTime, size_t commandBufferIndex ) noexcept;
        static void UploadGPUData ( VkCommandBuffer commandBuffer,  size_t commandBufferIndex ) noexcept;

    private:
        void FreeResources () noexcept;
        void UpdateInternal ( float deltaTime, size_t commandBufferIndex ) noexcept;

        static void AllocateVulkanStructures ( size_t needed ) noexcept;
        static void FreeUnusedResources ( size_t commandBufferIndex ) noexcept;

        [[nodiscard]] static int OnAwake ( lua_State* state );
        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnSetInput ( lua_State* state );
        [[nodiscard]] static int OnSleep ( lua_State* state );
};

} // namespace pbr


#endif // PBR_ANIMATION_GRAPH_HPP
