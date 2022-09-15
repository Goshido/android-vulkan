#ifndef PBR_MATERIAL_MANAGER_H
#define PBR_MATERIAL_MANAGER_H


#include "material_info.h"
#include "types.h"
#include <file.h>

GX_DISABLE_COMMON_WARNINGS

#include <deque>
#include <mutex>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

class MaterialManager final
{
    public:
        // Forward declaration. This will init '_handlers' static field.
        class StaticInitializer;

    private:
        using Handler = MaterialRef ( MaterialManager::* ) ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            MaterialHeader const &header,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        using Storage = std::unordered_map<std::string_view, Texture2DRef>;

    private:
        Storage                                     _textureStorage {};
        std::deque<android_vulkan::Texture2D*>      _toFreeTransferResource {};

        static Handler                              _handlers[ static_cast<size_t> ( eMaterialTypeDesc::COUNT ) ];
        static MaterialManager*                     _instance;
        static std::mutex                           _mutex;

    public:
        MaterialManager ( MaterialManager const & ) = delete;
        MaterialManager& operator = ( MaterialManager const & ) = delete;

        MaterialManager ( MaterialManager && ) = delete;
        MaterialManager& operator = ( MaterialManager && ) = delete;

        // Note commandBuffers must point to at least 4 free command buffers.
        [[nodiscard]] MaterialRef LoadMaterial ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            char const* fileName,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] static MaterialManager& GetInstance () noexcept;
        static void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] constexpr static uint32_t MaxCommandBufferPerMaterial () noexcept
        {
            return 4U;
        }

    private:
        MaterialManager() = default;
        ~MaterialManager () = default;

        [[nodiscard]] MaterialRef CreateOpaqueMaterial ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            MaterialHeader const &header,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        [[nodiscard]] MaterialRef CreateStippleMaterial ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            MaterialHeader const &header,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        void DestroyInternal ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] Texture2DRef LoadTexture ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            uint8_t const* data,
            uint64_t nameOffset,
            android_vulkan::eFormat format,
            VkCommandBuffer const* commandBuffers
        ) noexcept;
};

} // namespace pbr


#endif // PBR_MATERIAL_MANAGER_H
