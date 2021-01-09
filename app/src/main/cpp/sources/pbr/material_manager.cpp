#include <pbr/material_manager.h>
#include <pbr/material_info.h>
#include <pbr/opaque_material.h>


namespace pbr {

MaterialManager* MaterialManager::_instance = nullptr;
std::shared_timed_mutex MaterialManager::_mutex;

MaterialRef MaterialManager::LoadMaterial ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    char const* fileName,
    VkCommandBuffer const* commandBuffers
)
{
    commandBufferConsumed = 0U;

    if ( !fileName )
        return std::make_shared<OpaqueMaterial> ();

    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );
    MaterialRef material = std::make_shared<OpaqueMaterial> ();
    android_vulkan::File file ( fileName );

    if ( !file.LoadContent () )
        return material;

    std::vector<uint8_t> const& content = file.GetContent ();
    uint8_t const* data = content.data ();
    auto const* header = reinterpret_cast<MaterialHeader const*> ( data );

    // Note it's safe to cast like that here. "NOLINT" is clang-tidy control comment.
    auto* opaqueMaterial = static_cast<OpaqueMaterial*> ( material.get () ); // NOLINT

    auto loadTexture = [ & ] ( uint64_t nameOffset, android_vulkan::eFormat format ) -> Texture2DRef {
        auto const* name = reinterpret_cast<char const*> ( data + nameOffset );
        auto findResult = _textureStorage.find ( name );

        if ( findResult != _textureStorage.cend () )
            return findResult->second;

        Texture2DRef texture = std::make_shared<android_vulkan::Texture2D> ();

        if ( !texture->UploadData ( name, format, true, renderer, commandBuffers[ commandBufferConsumed ] ) )
            texture = nullptr;
        else
            _textureStorage.insert ( std::make_pair ( std::string_view ( texture->GetName () ), texture ) );

        ++commandBufferConsumed;
        return texture;
    };

    if ( header->_diffuseOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = loadTexture ( header->_diffuseOffset, android_vulkan::eFormat::sRGB );

        if ( !texture )
            return material;

        opaqueMaterial->SetAlbedo ( texture );
    }

    if ( header->_emissionOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = loadTexture ( header->_emissionOffset, android_vulkan::eFormat::Unorm );

        if ( !texture )
            return material;

        opaqueMaterial->SetEmission ( texture );
    }

    if ( header->_maskOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = loadTexture ( header->_maskOffset, android_vulkan::eFormat::Unorm );

        if ( !texture )
            return material;

        opaqueMaterial->SetMask ( texture );
    }

    if ( header->_normalOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = loadTexture ( header->_normalOffset, android_vulkan::eFormat::Unorm );

        if ( !texture )
            return material;

        opaqueMaterial->SetNormal ( texture );
    }

    if ( header->_paramOffset != android_vulkan::NO_UTF8_OFFSET )
    {
        Texture2DRef texture = loadTexture ( header->_paramOffset, android_vulkan::eFormat::Unorm );

        if ( !texture )
            return material;

        opaqueMaterial->SetParam ( texture );
    }

    return material;
}

MaterialManager& MaterialManager::GetInstance ()
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        _instance = new MaterialManager ();

    return *_instance;
}

void MaterialManager::Destroy ( VkDevice device )
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        return;

    _instance->DestroyInternal ( device );

    delete _instance;
    _instance = nullptr;
}

void MaterialManager::DestroyInternal ( VkDevice device )
{
    for ( auto &texture : _textureStorage )
        texture.second->FreeResources ( device );

    _textureStorage.clear ();
}

} // namespace pbr
