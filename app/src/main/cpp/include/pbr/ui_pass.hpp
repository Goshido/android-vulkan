#ifndef PBR_UI_PASS_HPP
#define PBR_UI_PASS_HPP


#include "font_storage.hpp"
#include "sampler_manager.hpp"
#include "types.hpp"
#include "ui_program.hpp"
#include "ui_vertex_info.hpp"
#include "uniform_buffer_pool_manager.hpp"

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

            VkDescriptorSet                         _scene = VK_NULL_HANDLE;
            VkDescriptorSet                         _transparent = VK_NULL_HANDLE;

            [[nodiscard]] bool Init ( VkDevice device,
                VkDescriptorPool descriptorPool,
                VkImageView transparent
            ) noexcept;

            void Destroy ( VkDevice device ) noexcept;

            void Commit ( VkDevice device ) noexcept;
            void Push ( VkImageView view ) noexcept;
            void UpdateScene ( VkDevice device, VkImageView scene ) noexcept;
        };

        struct InUseImageTracker final
        {
            using Entry = std::unordered_map<Texture2DRef, size_t>;

            Entry                                   _registry[ DUAL_COMMAND_BUFFER ];

            void Destroy () noexcept;

            void CollectGarbage ( size_t commandBufferIndex ) noexcept;
            void MarkInUse ( Texture2DRef const &texture, size_t commandBufferIndex );
        };

        struct Job final
        {
            Texture2DRef const*                     _texture = nullptr;
            uint32_t                                _vertices {};
        };

    private:
        GXVec2                                      _bottomRight {};
        VkBufferMemoryBarrier                       _bufferBarrier {};
        float                                       _brightnessBalance = 0.0F;

        VkExtent2D                                  _currentResolution
        {
            .width = 0U,
            .height = 0U
        };

        UIPassTransformDescriptorSetLayout          _transformLayout {};

        UIVertexInfo*                               _data = nullptr;

        CommonDescriptorSet                         _commonDescriptorSet {};
        VkDescriptorPool                            _descriptorPool = VK_NULL_HANDLE;
        ImageDescriptorSets                         _imageDescriptorSets {};
        InUseImageTracker                           _inUseImageTracker {};

        size_t                                      _readVertexIndex = 0U;
        size_t                                      _writeVertexIndex = 0U;

        FontStorage                                 _fontStorage {};

        bool                                        _hasChanges = false;
        bool                                        _isTransformChanged = false;
        std::vector<Job>                            _jobs {};

        UIProgram                                   _program {};
        Buffer                                      _staging {};
        Buffer                                      _vertex {};

        VkDescriptorSet                             _transformDescriptorSet = VK_NULL_HANDLE;

        UniformBufferPoolManager                    _uniformPool
        {
            eUniformPoolSize::Nanoscopic_64KB,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
        };

    public:
        UIPass () = default;

        UIPass ( UIPass const & ) = delete;
        UIPass &operator = ( UIPass const & ) = delete;

        UIPass ( UIPass && ) = delete;
        UIPass &operator = ( UIPass && ) = delete;

        ~UIPass () = default;

        [[nodiscard]] bool Execute ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept;

        [[nodiscard]] FontStorage &GetFontStorage () noexcept;
        [[nodiscard]] size_t GetUsedVertexCount () const noexcept;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer,
            SamplerManager const &samplerManager,
            VkImageView transparent
        ) noexcept;

        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkImageView scene
        ) noexcept;

        void OnSwapchainDestroyed () noexcept;

        void RequestEmptyUI () noexcept;
        [[nodiscard]] UIBufferResponse RequestUIBuffer ( size_t neededVertices ) noexcept;

        [[nodiscard]] bool SetBrightness ( android_vulkan::Renderer &renderer,
            VkRenderPass renderPass,
            uint32_t subpass,
            float brightnessBalance
        ) noexcept;

        void SubmitImage ( Texture2DRef const &texture ) noexcept;
        void SubmitRectangle () noexcept;
        void SubmitText ( size_t usedVertices ) noexcept;

        [[nodiscard]] bool UploadGPUData ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer,
            size_t framebufferIndex
        ) noexcept;

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

        static void ReleaseImage ( Texture2DRef const &image ) noexcept;
        [[nodiscard]] static std::optional<Texture2DRef const> RequestImage ( std::string const &asset ) noexcept;

    private:
        void SubmitNonImage ( size_t usedVertices ) noexcept;
        void UpdateGeometry ( VkCommandBuffer commandBuffer ) noexcept;
        void UpdateTransform ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;
};

} // namespace pbr


#endif //  PBR_UI_PASS_HPP
