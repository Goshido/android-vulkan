#ifndef ROTATING_UNIFORM_BUFFER_H
#define ROTATING_UNIFORM_BUFFER_H


#include <renderer.h>

namespace rotating_mesh {

// This class wraps Vulkan creation and update routines of a uniform buffer.
// The class is supposed to used with single uniform buffer object.
class UniformBuffer final
{
    private:
        size_t                          _size;

        VkBuffer                        _buffer;
        VkDeviceMemory                  _bufferMemory;

        VkCommandBuffer                 _commandBuffer;
        VkCommandPool                   _commandPool;

        android_vulkan::Renderer*       _renderer;

        VkPipelineStageFlags            _targetStages;

        VkBuffer                        _transfer;
        VkDeviceMemory                  _transferMemory;

    public:
        UniformBuffer ();
        ~UniformBuffer () = default;

        UniformBuffer ( const UniformBuffer &other ) = delete;
        UniformBuffer& operator = ( const UniformBuffer &other ) = delete;

        void FreeResources ();
        VkBuffer GetBuffer () const;
        size_t GetSize () const;

        // Note this method must be invoked before UniformBuffer::Update.
        // This method must be used only once except situation when user invokes UniformBuffer::FreeResources.
        // "targetStages" is a pipeline stage mask when buffer content will be used.
        // The method returns true if success. Otherwise the method returns false.
        bool Init ( android_vulkan::Renderer &renderer, VkCommandPool commandPool, VkPipelineStageFlags targetStages );

        // Method updates GPU side uniform buffer.
        // Note the method could be used after UniformBuffer::Init method.
        // The method returns true if success. Otherwise the method returns false.
        bool Update ( const uint8_t* data, size_t size );

    private:
        bool InitResources ( size_t size );
};

} // namespace rotating_mesh


#endif // ROTATING_UNIFORM_BUFFER_H