#include <pbr/material_manager.h>
#include <pbr/material_info.h>
#include <pbr/opaque_material.h>


namespace pbr {

MaterialManager* MaterialManager::_instance = nullptr;
std::shared_timed_mutex MaterialManager::_mutex;

MaterialRef MaterialManager::LoadMaterial ( std::string_view const &fileName,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer const* commandBuffers
)
{
    if ( fileName.empty () )
        return std::make_shared<OpaqueMaterial> ();

    return LoadMaterial ( android_vulkan::File ( fileName ), renderer, commandBuffers );
}

MaterialRef MaterialManager::LoadMaterial ( char const* fileName,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer const* commandBuffers
)
{
    if ( !fileName )
        return std::make_shared<OpaqueMaterial> ();

    return LoadMaterial ( android_vulkan::File ( fileName ), renderer, commandBuffers );
}

MaterialManager& MaterialManager::GetInstance ()
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        _instance = new MaterialManager ();

    return *_instance;
}

void MaterialManager::Destroy ( android_vulkan::Renderer &renderer )
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        return;

    _instance->DestroyInternal ( renderer );

    delete _instance;
    _instance = nullptr;
}

void MaterialManager::DestroyInternal ( android_vulkan::Renderer &renderer )
{
    for ( auto &texture : _textureStorage )
        texture.second->FreeResources ( renderer );

    _textureStorage.clear ();
}

MaterialRef MaterialManager::LoadMaterial ( android_vulkan::File &&file,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer const* commandBuffers
)
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );
    MaterialRef material = std::make_shared<OpaqueMaterial> ();

    if ( !file.LoadContent () )
        return material;

    std::vector<uint8_t> const& content = file.GetContent ();
    uint8_t const* data = content.data ();
    auto const* header = reinterpret_cast<MaterialHeader const*> ( data );

    auto* opaqueMaterial = dynamic_cast<OpaqueMaterial*> ( material.get () );

    auto loadTexture = [ & ] ( uint64_t nameOffset, VkFormat format, VkCommandBuffer commandBuffer ) -> Texture2DRef {
        auto const* name = reinterpret_cast<char const*> ( data + nameOffset );
        auto findResult = _textureStorage.find ( name );

        if ( findResult != _textureStorage.cend () )
            return findResult->second;

        Texture2DRef texture = std::make_shared<android_vulkan::Texture2D> ();

        if ( texture->UploadData ( name, format, true, renderer, commandBuffer ) )
            _textureStorage.insert ( std::make_pair ( texture->GetName ().c_str (), texture ) );
        else
            texture = nullptr;

        return texture;
    };

    if ( header->_diffuseOffset != NO_UTF8_OFFSET )
    {
        Texture2DRef texture = loadTexture ( header->_diffuseOffset, VK_FORMAT_R8G8B8A8_SRGB, commandBuffers[ 0U ] );

        if ( !texture )
            return material;

        opaqueMaterial->SetAlbedo ( texture );
    }

    if ( header->_emissionOffset != NO_UTF8_OFFSET )
    {
        Texture2DRef texture = loadTexture ( header->_emissionOffset, VK_FORMAT_R8G8B8A8_SRGB, commandBuffers[ 1U ] );

        if ( !texture )
            return material;

        opaqueMaterial->SetEmission ( texture );
    }

    if ( header->_normalOffset != NO_UTF8_OFFSET )
    {
        Texture2DRef texture = loadTexture ( header->_normalOffset, VK_FORMAT_R8G8B8A8_SRGB, commandBuffers[ 2U ] );

        if ( !texture )
            return material;

        opaqueMaterial->SetNormal ( texture );
    }

    if ( header->_paramOffset != NO_UTF8_OFFSET )
    {
        Texture2DRef texture = loadTexture ( header->_paramOffset, VK_FORMAT_R8G8B8A8_SRGB, commandBuffers[ 3U ] );

        if ( !texture )
            return material;

        opaqueMaterial->SetParam ( texture );
    }

    return material;
}

} // namespace pbr
