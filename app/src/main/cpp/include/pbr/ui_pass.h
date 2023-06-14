#ifndef PBR_UI_PASS_H
#define PBR_UI_PASS_H


#include "ui_program.h"
#include "ui_vertex_info.h"


namespace pbr {

class UIPass final
{
    private:
        struct Buffer final
        {
            VkBuffer            _buffer = VK_NULL_HANDLE;
            VkDeviceMemory      _memory = VK_NULL_HANDLE;
            char const*         _name = nullptr;
            VkDeviceSize        _memoryOffset = 0U;
            size_t              _writeOffset = 0U;

            [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags memoryProperties,
                char const* name
            ) noexcept;

            void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        };

    private:
        UIVertexInfo*           _data = nullptr;
        UIProgram               _program {};

        VkExtent2D              _resolution
        {
            .width = 0U,
            .height = 0U
        };

        Buffer                  _staging {};
        Buffer                  _vertex {};

    public:
        UIPass () = default;

        UIPass ( UIPass const & ) = delete;
        UIPass& operator = ( UIPass const & ) = delete;

        UIPass ( UIPass && ) = delete;
        UIPass& operator = ( UIPass && ) = delete;

        ~UIPass () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool SetResolution ( android_vulkan::Renderer &renderer, VkRenderPass renderPass ) noexcept;
};

} // namespace pbr


#endif //  PBR_UI_PASS_H
