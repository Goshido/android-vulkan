#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <platform/android/pbr/material_manager.hpp>
#include <platform/android/pbr/opaque_material.hpp>
#include <platform/android/pbr/stipple_material.hpp>


namespace pbr {

[[maybe_unused]] constexpr uint32_t OPAQUE_MATERIAL_FORMAT_VERSION = 1U;
[[maybe_unused]] constexpr uint32_t STIPPLE_MATERIAL_FORMAT_VERSION = 1U;

//----------------------------------------------------------------------------------------------------------------------

class MaterialManager::StaticInitializer final
{
    public:
        StaticInitializer () noexcept;

        StaticInitializer ( StaticInitializer const & ) = delete;
        StaticInitializer &operator = ( StaticInitializer const & ) = delete;

        StaticInitializer ( StaticInitializer && ) = delete;
        StaticInitializer &operator = ( StaticInitializer && ) = delete;

        ~StaticInitializer () = default;
};

MaterialManager::StaticInitializer::StaticInitializer () noexcept
{
    auto &h = MaterialManager::_handlers;

    h[ static_cast<size_t> ( eMaterialTypeDesc::Opaque ) ] = &MaterialManager::CreateOpaqueMaterial;
    h[ static_cast<size_t> ( eMaterialTypeDesc::Stipple ) ] = &MaterialManager::CreateStippleMaterial;
}

[[maybe_unused]] static MaterialManager::StaticInitializer const g_StaticInitializer {};

//----------------------------------------------------------------------------------------------------------------------

MaterialManager::Handler MaterialManager::_handlers[ static_cast<size_t> ( eMaterialTypeDesc::COUNT ) ] = {};
MaterialManager* MaterialManager::_instance = nullptr;
std::mutex MaterialManager::_mutex;

void MaterialManager::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    std::lock_guard const lock ( _mutex );

    if ( _toFreeTransferResource.empty () )
        return;

    for ( auto* texture : _toFreeTransferResource )
        texture->FreeTransferResources ( renderer );

    _toFreeTransferResource.clear ();
}

MaterialRef MaterialManager::LoadMaterial ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    char const* fileName,
    VkCommandBuffer const* commandBuffers,
    VkFence const* fences
) noexcept
{
    commandBufferConsumed = 0U;

    if ( !fileName ) [[unlikely]]
        return std::make_shared<OpaqueMaterial> ();

    std::lock_guard const lock ( _mutex );
    android_vulkan::File file ( fileName );

    if ( !file.LoadContent () ) [[unlikely]]
        return std::make_shared<OpaqueMaterial> ();

    std::vector<uint8_t> const &content = file.GetContent ();
    uint8_t const* data = content.data ();
    auto const &header = *reinterpret_cast<MaterialHeader const*> ( data );

    AV_ASSERT ( header._type < eMaterialTypeDesc::COUNT )
    Handler const handler = _handlers[ static_cast<size_t> ( header._type ) ];

    // C++ calling method by pointer syntax.
    return ( this->*handler ) ( renderer, commandBufferConsumed, header, data, commandBuffers, fences );
}

MaterialManager &MaterialManager::GetInstance () noexcept
{
    std::lock_guard const lock ( _mutex );

    if ( !_instance )
        _instance = new MaterialManager ();

    return *_instance;
}

void MaterialManager::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    std::lock_guard const lock ( _mutex );

    if ( !_instance )
        return;

    _instance->DestroyInternal ( renderer );

    delete _instance;
    _instance = nullptr;
}

MaterialRef MaterialManager::CreateOpaqueMaterial ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    MaterialHeader const &header,
    uint8_t const* data,
    VkCommandBuffer const* commandBuffers,
    VkFence const* fences
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto const &h = static_cast<OpaqueMaterialHeader const &> ( header );

    AV_ASSERT ( h._formatVersion == OPAQUE_MATERIAL_FORMAT_VERSION )
    MaterialRef material = std::make_shared<OpaqueMaterial> ();

    // NOLINTNEXTLINE - downcast.
    auto &opaqueMaterial = static_cast<OpaqueMaterial&> ( *material );

    if ( h._diffuseOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = LoadTexture ( renderer,
            commandBufferConsumed,
            data,
            h._diffuseOffset,
            android_vulkan::eColorSpace::Unorm,
            commandBuffers,
            fences
        );

        if ( !texture )
            return material;

        opaqueMaterial.SetAlbedo ( texture );
    }

    if ( h._emissionOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = LoadTexture ( renderer,
            commandBufferConsumed,
            data,
            h._emissionOffset,
            android_vulkan::eColorSpace::Unorm,
            commandBuffers,
            fences
        );

        if ( !texture )
            return material;

        opaqueMaterial.SetEmission ( texture );
    }

    if ( h._maskOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = LoadTexture ( renderer,
            commandBufferConsumed,
            data,
            h._maskOffset,
            android_vulkan::eColorSpace::Unorm,
            commandBuffers,
            fences
        );

        if ( !texture )
            return material;

        opaqueMaterial.SetMask ( texture );
    }

    if ( h._normalOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = LoadTexture ( renderer,
            commandBufferConsumed,
            data,
            h._normalOffset,
            android_vulkan::eColorSpace::Unorm,
            commandBuffers,
            fences
        );

        if ( !texture )
            return material;

        opaqueMaterial.SetNormal ( texture );
    }

    if ( h._paramOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = LoadTexture ( renderer,
            commandBufferConsumed,
            data,
            h._paramOffset,
            android_vulkan::eColorSpace::Unorm,
            commandBuffers,
            fences
        );

        if ( !texture )
            return material;

        opaqueMaterial.SetParam ( texture );
    }

    return material;
}

MaterialRef MaterialManager::CreateStippleMaterial ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    MaterialHeader const &header,
    uint8_t const* data,
    VkCommandBuffer const* commandBuffers,
    VkFence const* fences
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto const &h = static_cast<StippleMaterialHeader const &> ( header );

    AV_ASSERT ( h._formatVersion == STIPPLE_MATERIAL_FORMAT_VERSION )
    MaterialRef material = std::make_shared<StippleMaterial> ();

    // NOLINTNEXTLINE - downcast.
    auto &stippleMaterial = static_cast<StippleMaterial &> ( *material );

    if ( h._diffuseOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = LoadTexture ( renderer,
            commandBufferConsumed,
            data,
            h._diffuseOffset,
            android_vulkan::eColorSpace::sRGB,
            commandBuffers,
            fences
        );

        if ( !texture )
            return material;

        stippleMaterial.SetAlbedo ( texture );
    }

    if ( h._emissionOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = LoadTexture ( renderer,
            commandBufferConsumed,
            data,
            h._emissionOffset,
            android_vulkan::eColorSpace::sRGB,
            commandBuffers,
            fences
        );

        if ( !texture )
            return material;

        stippleMaterial.SetEmission ( texture );
    }

    if ( h._maskOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = LoadTexture ( renderer,
            commandBufferConsumed,
            data,
            h._maskOffset,
            android_vulkan::eColorSpace::sRGB,
            commandBuffers,
            fences
        );

        if ( !texture )
            return material;

        stippleMaterial.SetMask ( texture );
    }

    if ( h._normalOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = LoadTexture ( renderer,
            commandBufferConsumed,
            data,
            h._normalOffset,
            android_vulkan::eColorSpace::Unorm,
            commandBuffers,
            fences
        );

        if ( !texture )
            return material;

        stippleMaterial.SetNormal ( texture );
    }

    if ( h._paramOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = LoadTexture ( renderer,
            commandBufferConsumed,
            data,
            h._paramOffset,
            android_vulkan::eColorSpace::Unorm,
            commandBuffers,
            fences
        );

        if ( !texture )
            return material;

        stippleMaterial.SetParam ( texture );
    }

    return material;
}

void MaterialManager::DestroyInternal ( android_vulkan::Renderer &renderer ) noexcept
{
    for ( auto &texture : _textureStorage )
        texture.second->FreeResources ( renderer );

    _textureStorage.clear ();
}

Texture2DRef MaterialManager::LoadTexture ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    uint8_t const* data,
    uint64_t nameOffset,
    android_vulkan::eColorSpace space,
    VkCommandBuffer const* commandBuffers,
    VkFence const* fences
) noexcept
{
    auto const* name = reinterpret_cast<char const*> ( data + nameOffset );
    auto findResult = _textureStorage.find ( name );

    if ( findResult != _textureStorage.cend () )
        return findResult->second;

    Texture2DRef texture = std::make_shared<android_vulkan::Texture2D> ();
    VkFence fence = fences ? fences[ commandBufferConsumed ] : VK_NULL_HANDLE;

    if ( !texture->UploadData ( renderer, name, space, true, commandBuffers[ commandBufferConsumed ], false, fence ) )
    {
        [[unlikely]]
        texture = nullptr;
    }
    else
    {
        _textureStorage.insert ( std::make_pair ( std::string_view ( texture->GetName () ), texture ) );
        _toFreeTransferResource.push_back ( texture.get () );
    }

    ++commandBufferConsumed;
    return texture;
}

} // namespace pbr
