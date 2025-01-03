#ifndef PBR_UMA_UNIFORM_BUFFER_HPP
#define PBR_UMA_UNIFORM_BUFFER_HPP


#include "descriptor_set_layout.hpp"
#include "uniform_size.hpp"


namespace pbr {

class UMAUniformBuffer final
{
    public:
        struct BufferInfo final
        {
            VkBuffer            _buffer = VK_NULL_HANDLE;
            VkDeviceMemory      _memory = VK_NULL_HANDLE;
            VkDeviceSize        _offset = 0U;
            size_t              _stepSize = 0U;
        };

    private:
        BufferInfo              _bufferInfo {};
        uint8_t*                _data = nullptr;
        size_t                  _index = 0U;
        size_t                  _itemCount = 0U;

    public:
        explicit UMAUniformBuffer () = default;

        UMAUniformBuffer ( UMAUniformBuffer const & ) = delete;
        UMAUniformBuffer &operator = ( UMAUniformBuffer const & ) = delete;

        UMAUniformBuffer ( UMAUniformBuffer && ) = delete;
        UMAUniformBuffer &operator = ( UMAUniformBuffer && ) = delete;

        ~UMAUniformBuffer () = default;

        [[nodiscard]] size_t GetAvailableItemCount () const noexcept;
        void Push ( void const* item, size_t size ) noexcept;
        [[nodiscard]] BufferInfo const &GetBufferInfo () noexcept;

        // The method returns all items to the pool.
        void Reset () noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            eUniformSize size,
            size_t itemSize,
            char const* name
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_UMA_UNIFORM_BUFFER_HPP
