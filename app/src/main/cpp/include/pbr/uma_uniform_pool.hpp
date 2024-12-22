#ifndef PBR_UMA_UNIFORM_POOL_HPP
#define PBR_UMA_UNIFORM_POOL_HPP


#include "descriptor_set_layout.hpp"
#include "uniform_size.hpp"


namespace pbr {

class UMAUniformPool final
{
    private:
        VkBuffer                        _buffer = VK_NULL_HANDLE;
        VkDescriptorPool                _pool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet>    _sets {};

        VkMappedMemoryRange             _ranges[ 2U ]{};
        size_t                          _rangeIndex = 0U;

        uint8_t*                        _data = nullptr;
        size_t                          _size = 0U;
        size_t                          _itemSize = 0U;
        size_t                          _stepSize = 0U;

        size_t                          _readIndex = 0U;
        size_t                          _writeIndex = 0U;
        bool                            _written = false;

    public:
        explicit UMAUniformPool () = default;

        UMAUniformPool ( UMAUniformPool const & ) = delete;
        UMAUniformPool &operator = ( UMAUniformPool const & ) = delete;

        UMAUniformPool ( UMAUniformPool && ) = delete;
        UMAUniformPool &operator = ( UMAUniformPool && ) = delete;

        ~UMAUniformPool () = default;

        [[nodiscard]] VkDescriptorSet Acquire () noexcept;
        [[nodiscard]] bool IssueSync ( VkDevice device ) noexcept;
        void Push ( void const* item ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            DescriptorSetLayout const &descriptorSetLayout,
            eUniformSize size,
            size_t itemSize,
            uint32_t bind,
            char const* name
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_UMA_UNIFORM_POOL_HPP
