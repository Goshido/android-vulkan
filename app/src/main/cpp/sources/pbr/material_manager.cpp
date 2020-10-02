#include <pbr/material_manager.h>
#include <pbr/material_info.h>
#include <pbr/opaque_material.h>
#include <file.h>


namespace pbr {

MaterialManager* MaterialManager::_instance = nullptr;
std::shared_timed_mutex MaterialManager::_mutex;

MaterialRef MaterialManager::LoadMaterial ( std::string_view const &fileName,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer const* commandBuffers
)
{
    android_vulkan::File file ( fileName );
    MaterialRef material = std::make_shared<OpaqueMaterial> ();

    if ( !file.LoadContent () )
        return material;

    std::vector<uint8_t> const& content = file.GetContent ();
    uint8_t const* data = content.data ();
    auto const* header = reinterpret_cast<MaterialHeader const*> ( data );

    auto* opaqueMaterial = dynamic_cast<OpaqueMaterial*> ( material.get () );

    auto loadTexture = [ & ] ( uint64_t nameOffset, VkFormat format, VkCommandBuffer commandBuffer ) -> Texture2DRef {
        Texture2DRef texture = std::make_shared<android_vulkan::Texture2D> ();

        bool const result = texture->UploadData (
            std::string_view ( reinterpret_cast<char const *> ( data + nameOffset ) ),
            format,
            true,
            renderer,
            commandBuffer
        );

        if ( !result )
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

MaterialManager& MaterialManager::GetInstance ()
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        _instance = new MaterialManager ();

    return *_instance;
}

void MaterialManager::Destroy ()
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        return;

    delete _instance;
    _instance = nullptr;
}

} // namespace pbr
