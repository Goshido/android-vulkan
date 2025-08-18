// FUCK - windows and android separation

#ifndef PBR_UI_PASS_HPP
#define PBR_UI_PASS_HPP


#include "font_storage.hpp"
#include "sampler_manager.hpp"
#include "types.hpp"

// FUCK - use relative path
#include <platform/android/pbr/ui_program.hpp>
#include <platform/android/pbr/ui_vertex_info.hpp>

#include "uniform_pool.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace pbr {

class UIPass final
{
    public:
        // FUCK - remove namespace
        using UIBufferResponse = std::optional<android::UIVertexBuffer>;

    private:
        class CommonDescriptorSet final
        {
            public:
                VkDescriptorSet                         _descriptorSet = VK_NULL_HANDLE;
                VkDescriptorImageInfo                   _imageInfo {};
                UIPassCommonDescriptorSetLayout         _layout {};
                VkWriteDescriptorSet                    _write {};
                VkCommandPool                           _pool = VK_NULL_HANDLE;
                VkFence                                 _fence = VK_NULL_HANDLE;
                android_vulkan::Texture2D               _textLUT {};

            public:
                CommonDescriptorSet () = default;

                CommonDescriptorSet ( CommonDescriptorSet const & ) = delete;
                CommonDescriptorSet &operator = ( CommonDescriptorSet const & ) = delete;

                CommonDescriptorSet ( CommonDescriptorSet && ) = delete;
                CommonDescriptorSet &operator = ( CommonDescriptorSet && ) = delete;

                ~CommonDescriptorSet () = default;

                [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
                    VkDescriptorPool descriptorPool,
                    SamplerManager const &samplerManager
                ) noexcept;

                void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
                [[nodiscard]] bool Update ( android_vulkan::Renderer &renderer, VkImageView currentAtlas ) noexcept;

            private:
                [[nodiscard]] bool FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept;
        };

        struct Buffer final
        {
            VkBuffer                                    _buffer = VK_NULL_HANDLE;
            VkDeviceMemory                              _memory = VK_NULL_HANDLE;
            char const*                                 _name = nullptr;
            VkDeviceSize                                _memoryOffset = 0U;

            [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
                size_t size,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags memoryProperties,
                char const* name
            ) noexcept;

            void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        };

        class BufferStream final
        {
            private:
                VkBufferMemoryBarrier                   _barrier
                {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .buffer = VK_NULL_HANDLE,
                    .offset = 0U,
                    .size = 0U
                };

                uint8_t*                                _data = nullptr;
                size_t const                            _elementSize = 0U;

                Buffer                                  _staging {};
                Buffer                                  _vertex {};

            public:
                BufferStream () = delete;

                BufferStream ( BufferStream const & ) = delete;
                BufferStream &operator = ( BufferStream const & ) = delete;

                BufferStream ( BufferStream && ) = delete;
                BufferStream &operator = ( BufferStream && ) = delete;

                explicit BufferStream ( size_t elementSize ) noexcept;

                ~BufferStream () = default;

                [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
                    char const *vertexName,
                    char const *stagingName
                ) noexcept;

                void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

                [[nodiscard]] VkBuffer GetBuffer () const noexcept;
                [[nodiscard]] void *GetData ( size_t startIndex ) const noexcept;
                void UpdateGeometry ( VkCommandBuffer commandBuffer, size_t readIdx, size_t writeIdx ) noexcept;
        };

        class ImageDescriptorSets final
        {
            public:
                size_t                                  _commitIndex = 0U;
                size_t                                  _startIndex = 0U;
                size_t                                  _written = 0U;

                std::vector<VkDescriptorSet>            _descriptorSets {};
                UIPassImageDescriptorSetLayout          _layout {};

                std::vector<VkDescriptorImageInfo>      _imageInfo {};
                std::vector<VkWriteDescriptorSet>       _writeSets {};

                VkDescriptorSet                         _transparent = VK_NULL_HANDLE;

            public:
                ImageDescriptorSets () = default;

                ImageDescriptorSets ( ImageDescriptorSets const & ) = delete;
                ImageDescriptorSets &operator = ( ImageDescriptorSets const & ) = delete;

                ImageDescriptorSets ( ImageDescriptorSets && ) = delete;
                ImageDescriptorSets &operator = ( ImageDescriptorSets && ) = delete;

                ~ImageDescriptorSets () = default;

                [[nodiscard]] bool Init ( VkDevice device,
                    VkDescriptorPool descriptorPool,
                    VkImageView transparent
                ) noexcept;

                void Destroy ( VkDevice device ) noexcept;

                void Commit ( VkDevice device ) noexcept;
                void Push ( VkImageView view ) noexcept;
        };

        struct InUseImageTracker final
        {
            using Entry = std::unordered_map<Texture2DRef, size_t>;

            Entry                                       _registry[ FIF_COUNT ];

            void Destroy () noexcept;

            void CollectGarbage ( size_t commandBufferIndex ) noexcept;
            void MarkInUse ( Texture2DRef const &texture, size_t commandBufferIndex );
        };

        struct Job final
        {
            Texture2DRef const*                         _texture = nullptr;
            uint32_t                                    _vertices {};
        };

    private:
        GXVec2                                          _bottomRight {};
        float                                           _brightnessBalance = 0.0F;

        VkExtent2D                                      _currentResolution
        {
            .width = 0U,
            .height = 0U
        };

        UIPassTransformDescriptorSetLayout              _transformLayout {};

        CommonDescriptorSet                             _commonDescriptorSet {};
        VkDescriptorPool                                _descriptorPool = VK_NULL_HANDLE;
        ImageDescriptorSets                             _imageDescriptorSets {};
        InUseImageTracker                               _inUseImageTracker {};

        size_t                                          _readVertexIndex = 0U;
        size_t                                          _writeVertexIndex = 0U;

        FontStorage                                     _fontStorage {};

        bool                                            _hasChanges = false;
        bool                                            _isTransformChanged = false;
        std::vector<Job>                                _jobs {};

        BufferStream                                    _positions { sizeof ( GXVec2 ) };

        // FUCK - remove namespace
        BufferStream                                    _rest { sizeof ( android::UIVertex ) };

        // FUCK - remove namespace
        android::UIProgram                              _program {};

        VkDescriptorSet                                 _transformDescriptorSet = VK_NULL_HANDLE;

        UniformPool                                     _uniformPool
        {
            eUniformSize::Nanoscopic_64KB,
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
            uint32_t subpass
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

        static void AppendImage ( GXVec2* targetPositions,

            // FUCK - remove namespace
            android::UIVertex* targetVertices,

            GXColorUNORM color,
            GXVec2 const &topLeft,
            GXVec2 const &bottomRight
        ) noexcept;

        static void AppendRectangle ( GXVec2* targetPositions,

            // FUCK - remove namespace
            android::UIVertex* targetVertices,

            GXColorUNORM color,
            GXVec2 const &topLeft,
            GXVec2 const &bottomRight
        ) noexcept;

        static void AppendText ( GXVec2* targetPositions,

            // FUCK - remove namespace
            android::UIVertex* targetVertices,

            GXColorUNORM color,
            GXVec2 const &topLeft,
            GXVec2 const &bottomRight,
            android_vulkan::Half2 const &glyphTopLeft,
            android_vulkan::Half2 const &glyphBottomRight,
            uint8_t atlasLayer
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
