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
            size_t                                  _commitIndex = 0U;
            size_t                                  _startIndex = 0U;
            size_t                                  _written = 0U;

            std::vector<VkDescriptorSet>            _descriptorSets {};
            UIPassImageDescriptorSetLayout          _layout {};

            std::vector<VkDescriptorImageInfo>      _imageInfo {};
            std::vector<VkWriteDescriptorSet>       _writeSets {};

            VkDescriptorSet                         _transparent = VK_NULL_HANDLE;

            [[nodiscard]] bool Init ( VkDevice device,
                VkDescriptorPool descriptorPool,
                VkImageView transparent
            ) noexcept;

            void Destroy ( VkDevice device ) noexcept;

            void Commit ( VkDevice device ) noexcept;
            void Push ( VkImageView view ) noexcept;
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

        uint32_t                                    _framebufferIndex = std::numeric_limits<uint32_t>::max ();
        std::vector<VkFramebuffer>                  _framebuffers {};

        bool                                        _hasChanges = false;
        bool                                        _isSceneImageEmbedded = false;
        bool                                        _isTransformChanged = false;
        std::vector<Job>                            _jobs {};

        VkPresentInfoKHR                            _presentInfo {};
        UIProgram                                   _program {};
        VkSemaphore                                 _renderEndSemaphore = VK_NULL_HANDLE;
        VkRenderPassBeginInfo                       _renderInfo {};

        VkImageView                                 _scene = VK_NULL_HANDLE;
        VkSubmitInfo                                _submitInfo {};

        Buffer                                      _staging {};
        Buffer                                      _vertex {};

        VkSemaphore                                 _targetAcquiredSemaphore = VK_NULL_HANDLE;
        VkDescriptorSet                             _transformDescriptorSet = VK_NULL_HANDLE;
        [[maybe_unused]] VkImageView                _transparent = VK_NULL_HANDLE;

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

        [[nodiscard]] bool AcquirePresentTarget ( android_vulkan::Renderer &renderer,
            size_t &swapchainImageIndex
        ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            SamplerManager const &samplerManager,
            VkImageView transparent
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool Commit ( VkDevice device ) noexcept;

        [[nodiscard]] bool Execute ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            VkFence fence
        ) noexcept;

        [[nodiscard]] FontStorage& GetFontStorage () noexcept;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer, VkImageView scene ) noexcept;
        void OnSwapchainDestroyed ( VkDevice device ) noexcept;

        void RequestEmptyUI () noexcept;
        [[nodiscard]] UIBufferResponse RequestUIBuffer ( size_t neededVertices ) noexcept;

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

        void UpdateGeometry ( VkCommandBuffer commandBuffer ) noexcept;
        void UpdateTransform ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;
};

} // namespace pbr


#endif //  PBR_UI_PASS_H
