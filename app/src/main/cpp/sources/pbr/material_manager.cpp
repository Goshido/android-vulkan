#include <pbr/material_manager.h>
#include <pbr/opaque_material.h>
#include <pbr/stipple_material.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

[[maybe_unused]] constexpr uint32_t OPAQUE_MATERIAL_FORMAT_VERSION = 1U;
[[maybe_unused]] constexpr uint32_t STIPPLE_MATERIAL_FORMAT_VERSION = 1U;

//----------------------------------------------------------------------------------------------------------------------

class MaterialManager::StaticInitializer final
{
    public:
        StaticInitializer () noexcept;

        StaticInitializer ( StaticInitializer const & ) = delete;
        StaticInitializer& operator = ( StaticInitializer const & ) = delete;

        StaticInitializer ( StaticInitializer && ) = delete;
        StaticInitializer& operator = ( StaticInitializer && ) = delete;

        ~StaticInitializer () = default;
};

MaterialManager::StaticInitializer::StaticInitializer () noexcept
{
    auto& h = MaterialManager::_handlers;

    h[ static_cast<size_t> ( eMaterialTypeDesc::Opaque ) ] = &MaterialManager::CreateOpaqueMaterial;
    h[ static_cast<size_t> ( eMaterialTypeDesc::Stipple ) ] = &MaterialManager::CreateStippleMaterial;
}

[[maybe_unused]] static MaterialManager::StaticInitializer const g_StaticInitializer {};

//----------------------------------------------------------------------------------------------------------------------

MaterialManager::Handler MaterialManager::_handlers[ static_cast<size_t> ( eMaterialTypeDesc::COUNT ) ] = {};
MaterialManager* MaterialManager::_instance = nullptr;
std::shared_timed_mutex MaterialManager::_mutex;

MaterialRef MaterialManager::LoadMaterial ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    char const* fileName,
    VkCommandBuffer const* commandBuffers
) noexcept
{
    commandBufferConsumed = 0U;

    if ( !fileName )
        return std::make_shared<OpaqueMaterial> ();

    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );
    android_vulkan::File file ( fileName );

    if ( !file.LoadContent () )
        return std::make_shared<OpaqueMaterial> ();

    std::vector<uint8_t> const& content = file.GetContent ();
    uint8_t const* data = content.data ();
    auto const& header = *reinterpret_cast<MaterialHeader const*> ( data );

    assert ( header._type < eMaterialTypeDesc::COUNT );
    Handler const handler = _handlers[ static_cast<size_t> ( header._type ) ];

    // C++ calling method by pointer syntax.
    return ( this->*handler ) ( renderer, commandBufferConsumed, header, data, commandBuffers );
}

MaterialManager& MaterialManager::GetInstance () noexcept
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        _instance = new MaterialManager ();

    return *_instance;
}

void MaterialManager::Destroy ( VkDevice device ) noexcept
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        return;

    _instance->DestroyInternal ( device );

    delete _instance;
    _instance = nullptr;
}

MaterialRef MaterialManager::CreateOpaqueMaterial ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    MaterialHeader const &header,
    uint8_t const* data,
    VkCommandBuffer const* commandBuffers
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto const& h = static_cast<OpaqueMaterialHeader const &> ( header );

    assert ( h._formatVersion == OPAQUE_MATERIAL_FORMAT_VERSION );
    MaterialRef material = std::make_shared<OpaqueMaterial> ();

    // NOLINTNEXTLINE - downcast.
    auto& opaqueMaterial = static_cast<OpaqueMaterial&> ( *material );

    if ( h._diffuseOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = LoadTexture ( renderer,
            commandBufferConsumed,
            data,
            h._diffuseOffset,
            android_vulkan::eFormat::Unorm,
            commandBuffers
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
            android_vulkan::eFormat::Unorm,
            commandBuffers
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
            android_vulkan::eFormat::Unorm,
            commandBuffers
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
            android_vulkan::eFormat::Unorm,
            commandBuffers
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
            android_vulkan::eFormat::Unorm,
            commandBuffers
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
    VkCommandBuffer const* commandBuffers
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto const& h = static_cast<StippleMaterialHeader const&> ( header );

    assert ( h._formatVersion == STIPPLE_MATERIAL_FORMAT_VERSION );
    MaterialRef material = std::make_shared<StippleMaterial> ();

    // NOLINTNEXTLINE - downcast.
    auto& stippleMaterial = static_cast<StippleMaterial&> ( *material );

    if ( h._diffuseOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = LoadTexture ( renderer,
            commandBufferConsumed,
            data,
            h._diffuseOffset,
            android_vulkan::eFormat::Unorm,
            commandBuffers
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
            android_vulkan::eFormat::Unorm,
            commandBuffers
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
            android_vulkan::eFormat::Unorm,
            commandBuffers
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
            android_vulkan::eFormat::Unorm,
            commandBuffers
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
            android_vulkan::eFormat::Unorm,
            commandBuffers
        );

        if ( !texture )
            return material;

        stippleMaterial.SetParam ( texture );
    }

    return material;
}

void MaterialManager::DestroyInternal ( VkDevice device ) noexcept
{
    for ( auto& texture : _textureStorage )
        texture.second->FreeResources ( device );

    _textureStorage.clear ();
}

Texture2DRef MaterialManager::LoadTexture ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    uint8_t const* data,
    uint64_t nameOffset,
    android_vulkan::eFormat format,
    VkCommandBuffer const* commandBuffers
) noexcept
{
    auto const* name = reinterpret_cast<char const*> ( data + nameOffset );
    auto findResult = _textureStorage.find ( name );

    if ( findResult != _textureStorage.cend () )
        return findResult->second;

    Texture2DRef texture = std::make_shared<android_vulkan::Texture2D> ();

    if ( !texture->UploadData ( renderer, name, format, true, commandBuffers[ commandBufferConsumed ] ) )
        texture = nullptr;
    else
        _textureStorage.insert ( std::make_pair ( std::string_view ( texture->GetName () ), texture ) );

    ++commandBufferConsumed;
    return texture;
}

} // namespace pbr
