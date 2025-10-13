#ifndef PBR_MATERIAL_MANAGER_HPP
#define PBR_MATERIAL_MANAGER_HPP


#include "types.hpp"
#include <file.hpp>
#include <android_vulkan_sdk/pbr/material_info.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <deque>
#include <mutex>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

class MaterialManager final
{
    public:
        constexpr static char const DEFAULT_MATERIAL[] = "pbr/assets/System/Default.mtl";

    public:
        // Forward declaration. This will init '_handlers' static field.
        class StaticInitializer;

    private:
        using Handler = MaterialRef ( MaterialManager::* ) ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            MaterialHeader const &header,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers,
            VkFence const* fences
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
        MaterialManager &operator = ( MaterialManager const & ) = delete;

        MaterialManager ( MaterialManager && ) = delete;
        MaterialManager &operator = ( MaterialManager && ) = delete;

        void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept;

        // Note commandBuffers must point to at least 4 free command buffers.
        [[nodiscard]] MaterialRef LoadMaterial ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            char const* fileName,
            VkCommandBuffer const* commandBuffers,
            VkFence const* fences
        ) noexcept;

        [[nodiscard]] static MaterialManager &GetInstance () noexcept;
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
            VkCommandBuffer const* commandBuffers,
            VkFence const* fences
        ) noexcept;

        [[nodiscard]] MaterialRef CreateStippleMaterial ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            MaterialHeader const &header,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers,
            VkFence const* fences
        ) noexcept;

        void DestroyInternal ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] Texture2DRef LoadTexture ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            uint8_t const* data,
            uint64_t nameOffset,
            android_vulkan::eColorSpace space,
            VkCommandBuffer const* commandBuffers,
            VkFence const* fences
        ) noexcept;
};

} // namespace pbr


#endif // PBR_MATERIAL_MANAGER_HPP
