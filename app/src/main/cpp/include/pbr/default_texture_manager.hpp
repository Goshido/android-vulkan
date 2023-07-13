#ifndef PBR_DEFAULT_TEXTURE_MANAGER_HPP
#define PBR_DEFAULT_TEXTURE_MANAGER_HPP


#include "types.hpp"


namespace pbr {

class DefaultTextureManager final
{
    private:
        constexpr static size_t BUFFER_COUNT = 6U;

    private:
        Texture2DRef        _albedo {};
        Texture2DRef        _emission {};
        Texture2DRef        _mask {};
        Texture2DRef        _normal {};
        Texture2DRef        _params {};
        Texture2DRef        _transparent {};

        VkCommandBuffer     _commandBuffers[ BUFFER_COUNT ];

    public:
        DefaultTextureManager () = default;

        DefaultTextureManager ( DefaultTextureManager const & ) = delete;
        DefaultTextureManager &operator = ( DefaultTextureManager const & ) = delete;

        DefaultTextureManager ( DefaultTextureManager && ) = delete;
        DefaultTextureManager &operator = ( DefaultTextureManager && ) = delete;

        ~DefaultTextureManager () = default;

        void FreeTransferResources ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer) noexcept;

        [[nodiscard]] Texture2DRef const &GetAlbedo () const noexcept;
        [[nodiscard]] Texture2DRef const &GetEmission () const noexcept;
        [[nodiscard]] Texture2DRef const &GetMask () const noexcept;
        [[nodiscard]] Texture2DRef const &GetNormal () const noexcept;
        [[nodiscard]] Texture2DRef const &GetParams () const noexcept;
        [[nodiscard]] Texture2DRef const &GetTransparent () const noexcept;
};

} // namespace pbr


#endif // PBR_DEFAULT_TEXTURE_MANAGER_HPP
