#ifndef PBR_UI_PASS_H
#define PBR_UI_PASS_H


#include "font_storage.h"
#include "ui_program.h"
#include "ui_vertex_info.h"

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace pbr {

class UIPass final
{
    public:
        using VertexBuffer = std::span<UIVertexInfo>;
        using UIBufferResponse = std::optional<VertexBuffer>;

    private:
        struct Buffer final
        {
            VkBuffer            _buffer = VK_NULL_HANDLE;
            VkDeviceMemory      _memory = VK_NULL_HANDLE;
            char const*         _name = nullptr;
            VkDeviceSize        _memoryOffset = 0U;

            [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags memoryProperties,
                char const* name
            ) noexcept;

            void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        };

    private:
        UIVertexInfo*           _data = nullptr;
        size_t                  _currentVertexIndex = 0U;
        FontStorage             _fontStorage {};
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
        [[nodiscard]] bool Execute ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] FontStorage& GetFontStorage () noexcept;

        void RequestEmptyUI () noexcept;
        [[nodiscard]] UIBufferResponse RequestUIBuffer ( size_t neededVertices ) noexcept;

        [[nodiscard]] bool SetResolution ( android_vulkan::Renderer &renderer, VkRenderPass renderPass ) noexcept;

        void SubmitImage () noexcept;
        void SubmitRectangle () noexcept;
        void SubmitText ( size_t usedVertices ) noexcept;
};

} // namespace pbr


#endif //  PBR_UI_PASS_H
