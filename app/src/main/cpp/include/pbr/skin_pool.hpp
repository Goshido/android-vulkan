#ifndef PBR_SKIN_POOL_HPP
#define PBR_SKIN_POOL_HPP


#include "skin_descriptor_set_layout.hpp"
#include <buffer_info.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vector>

GX_RESTORE_WARNING_STATE


namespace pbr {

class SkinPool final
{
    private:
        std::vector<VkBufferMemoryBarrier>      _barriers {};
        std::vector<VkDescriptorBufferInfo>     _bufferInfo {};
        VkDescriptorPool                        _descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _descriptorSets {};

        size_t                                  _itemBaseIndex = 0U;
        size_t                                  _itemReadIndex = 0U;
        size_t                                  _itemWriteIndex = 0U;
        size_t                                  _itemWritten = 0U;

        SkinDescriptorSetLayout                 _layout {};
        std::vector<VkWriteDescriptorSet>       _writeSets {};

    public:
        SkinPool () = default;

        SkinPool ( SkinPool const & ) = delete;
        SkinPool &operator = ( SkinPool const & ) = delete;

        SkinPool ( SkinPool && ) = delete;
        SkinPool &operator = ( SkinPool && ) = delete;

        ~SkinPool () = default;

        [[nodiscard]] VkDescriptorSet Acquire () noexcept;

        [[nodiscard]] bool Init ( VkDevice device ) noexcept;
        void Destroy ( VkDevice device ) noexcept;

        void Push ( android_vulkan::BufferInfo pose,
            android_vulkan::BufferInfo skin,
            android_vulkan::MeshBufferInfo referenceMesh,
            VkBuffer skinMeshBuffer
        ) noexcept;

        void SubmitPipelineBarriers ( VkCommandBuffer commandBuffer ) noexcept;
        void UpdateDescriptorSets ( VkDevice device ) const noexcept;
};

} // namespace pbr


#endif // PBR_SKIN_POOL_HPP
