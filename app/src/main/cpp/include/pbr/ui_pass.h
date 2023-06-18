#ifndef PBR_UI_PASS_H
#define PBR_UI_PASS_H


#include "font_storage.h"
#include "sampler_manager.h"
#include "types.h"
#include "ui_program.h"
#include "ui_vertex_info.h"
#include "uniform_buffer_pool_manager.h"

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace pbr {

class UIPass final
{
    public:
        using UIBufferResponse = std::optional<UIVertexBuffer>;

    private:
        struct CommonDescriptorSet final
        {
            VkDescriptorSet                         _descriptorSet = VK_NULL_HANDLE;
            VkDescriptorImageInfo                   _imageInfo {};
            UIPassCommonDescriptorSetLayout         _layout {};
            VkWriteDescriptorSet                    _write {};

            [[nodiscard]] bool Init ( VkDevice device,
                VkDescriptorPool descriptorPool,
                SamplerManager const &samplerManager
            ) noexcept;

            void Destroy ( VkDevice device ) noexcept;

            void Update ( VkDevice device, VkImageView currentAtlas ) noexcept;
        };

        struct Buffer final
        {
            VkBuffer                                _buffer = VK_NULL_HANDLE;
            VkDeviceMemory                          _memory = VK_NULL_HANDLE;
            char const*                             _name = nullptr;
            VkDeviceSize                            _memoryOffset = 0U;

            [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags memoryProperties,
                char const* name
            ) noexcept;

            void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        };

        struct ImageDescriptorSets final
        {
            std::vector<VkDescriptorSet>            _descriptorSets {};
            UIPassImageDescriptorSetLayout          _layout {};

            std::vector<VkDescriptorImageInfo>      _imageInfo {};
            std::vector<VkWriteDescriptorSet>       _writeSets {};

            [[maybe_unused]] size_t                 _readIndex = 0U;
            [[maybe_unused]] size_t                 _writeIndex = 0U;

            [[nodiscard]] bool Init ( VkDevice device, VkDescriptorPool descriptorPool ) noexcept;
            void Destroy ( VkDevice device ) noexcept;
        };

        struct Job final
        {
            Texture2DRef                            _texture {};
            uint32_t                                _vertices {};
        };

    private:
        GXVec2                                      _bottomRight {};
        UIPassTransformDescriptorSetLayout          _transformLayout {};

        UIVertexInfo*                               _data = nullptr;

        CommonDescriptorSet                         _commonDescriptorSet {};
        VkDescriptorPool                            _descriptorPool = VK_NULL_HANDLE;
        ImageDescriptorSets                         _imageDescriptorSets {};

        size_t                                      _currentVertexIndex = 0U;
        size_t                                      _sceneImageVertexIndex = 0U;

        FontStorage                                 _fontStorage {};

        [[maybe_unused]] uint32_t                   _framebufferIndex = std::numeric_limits<uint32_t>::max ();
        std::vector<VkFramebuffer>                  _framebuffers {};

        bool                                        _hasChanges = false;
        bool                                        _isSceneImageEmbedded = false;
        std::vector<Job>                            _jobs {};

        [[maybe_unused]] VkPresentInfoKHR           _presentInfo {};
        UIProgram                                   _program {};
        VkSemaphore                                 _renderEndSemaphore = VK_NULL_HANDLE;
        VkRenderPassBeginInfo                       _renderInfo {};
        VkRenderPass                                _renderPass = VK_NULL_HANDLE;

        [[maybe_unused]] VkImage                    _sceneImage = VK_NULL_HANDLE;
        [[maybe_unused]] VkImageView                _sceneView = VK_NULL_HANDLE;
        [[maybe_unused]] VkSubmitInfo               _submitInfo {};

        Buffer                                      _staging {};
        Buffer                                      _vertex {};

        VkSemaphore                                 _targetAcquiredSemaphore = VK_NULL_HANDLE;

        UniformBufferPoolManager                    _uniformPool
        {
            eUniformPoolSize::Nanoscopic_64KB,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
        };

    public:
        UIPass () = default;

        UIPass ( UIPass const & ) = delete;
        UIPass& operator = ( UIPass const & ) = delete;

        UIPass ( UIPass && ) = delete;
        UIPass& operator = ( UIPass && ) = delete;

        ~UIPass () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, SamplerManager const &samplerManager ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool Commit () noexcept;
        void Execute ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] FontStorage& GetFontStorage () noexcept;

        void RequestEmptyUI () noexcept;
        [[nodiscard]] UIBufferResponse RequestUIBuffer ( size_t neededVertices ) noexcept;

        [[nodiscard]] bool SetPresentationInfo ( android_vulkan::Renderer &renderer,
            android_vulkan::Texture2D const &scene
        ) noexcept;

        [[maybe_unused]] void SubmitImage ( Texture2DRef const &texture ) noexcept;
        void SubmitRectangle () noexcept;
        void SubmitText ( size_t usedVertices ) noexcept;

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] constexpr static size_t GetVerticesPerRectangle () noexcept
        {
            return 6U;
        }

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
        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer,
            VkExtent2D const &resolution
        ) noexcept;

        void DestroyFramebuffers ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyRenderPass ( VkDevice device ) noexcept;

        void InitCommonStructures () noexcept;
        void SubmitNonImage ( size_t usedVertices ) noexcept;
};

} // namespace pbr


#endif //  PBR_UI_PASS_H
