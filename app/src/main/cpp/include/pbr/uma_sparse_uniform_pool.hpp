#ifndef PBR_UMA_SPARSE_UNIFORM_POOL_HPP
#define PBR_UMA_SPARSE_UNIFORM_POOL_HPP


#include "descriptor_set_layout.hpp"
#include "uniform_pool_size.hpp"


namespace pbr {

class UMASparseUniformPool final
{
    private:
        VkBuffer                                _buffer = VK_NULL_HANDLE;
        VkDescriptorPool                        _pool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>            _sets {};
        std::vector<VkMappedMemoryRange>        _ranges {};
        VkDeviceMemory                          _memory = VK_NULL_HANDLE;
        VkDeviceSize                            _offset = 0U;
        uint8_t*                                _data = nullptr;
        size_t                                  _nonCoherentAtomSize = 0U;

        size_t                                  _baseIndex = 0U;
        size_t                                  _readIndex = 0U;
        size_t                                  _writeIndex = 0U;
        size_t                                  _written = 0U;

    public:
        explicit UMASparseUniformPool () = default;

        UMASparseUniformPool ( UMASparseUniformPool const & ) = delete;
        UMASparseUniformPool &operator = ( UMASparseUniformPool const & ) = delete;

        UMASparseUniformPool ( UMASparseUniformPool && ) = delete;
        UMASparseUniformPool &operator = ( UMASparseUniformPool && ) = delete;

        ~UMASparseUniformPool () = default;

        [[nodiscard]] VkDescriptorSet Acquire () noexcept;
        void Commit () noexcept;
        [[nodiscard]] bool IssueSync ( VkDevice device ) const noexcept;
        void Push ( void const* item, size_t size ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            DescriptorSetLayout const &descriptorSetLayout,
            eUniformPoolSize size,
            size_t itemSize,
            uint32_t bind,
            char const* name
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_UMA_SPARSE_UNIFORM_POOL_HPP
