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
        using UIBufferResponse = std::optional<UIVertexBuffer>;

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
        GXVec2                  _bottomRight {};
        UIVertexInfo*           _data = nullptr;
        size_t                  _sceneImageVertexIndex = 0U;
        size_t                  _currentVertexIndex = 0U;
        FontStorage             _fontStorage {};

        bool                    _hasChanges = false;
        bool                    _isSceneImageEmbedded = false;

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

        void Commit () noexcept;
        void Execute ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] FontStorage& GetFontStorage () noexcept;
        void RequestEmptyUI () noexcept;
        [[nodiscard]] UIBufferResponse RequestUIBuffer ( size_t neededVertices ) noexcept;
        [[nodiscard]] bool SetResolution ( android_vulkan::Renderer &renderer, VkRenderPass renderPass ) noexcept;

        void SubmitImage () noexcept;
        void SubmitRectangle () noexcept;
        void SubmitText ( size_t usedVertices ) noexcept;

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;

        static void AppendRectangle ( UIVertexInfo* target,
            GXColorRGB const &color,
            GXVec2 const &topLeft,
            GXVec2 const &bottomRight,
            GXVec3 const &glyphTopLeft,
            GXVec3 const &glyphBottomRight,
            GXVec2 const &imageTopLeft,
            GXVec2 const &imageBottomRight
        ) noexcept;

    private:
        [[nodiscard]] bool EmbedSceneImage () noexcept;
};

} // namespace pbr


#endif //  PBR_UI_PASS_H
