#include <pbr/mesh_manager.h>


namespace pbr {

MeshManager* MeshManager::_instance = nullptr;
std::shared_timed_mutex MeshManager::_mutex;

MeshRef MeshManager::LoadMesh ( std::string_view const &fileName,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    return fileName.empty () ?
        std::make_shared<android_vulkan::MeshGeometry> () :
        LoadMesh ( std::string ( fileName ), renderer, commandBuffer );
}

MeshRef MeshManager::LoadMesh ( char const* fileName,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    return fileName ?
        std::make_shared<android_vulkan::MeshGeometry> () :
        LoadMesh ( std::string ( fileName ), renderer, commandBuffer );
}

MeshManager& MeshManager::GetInstance ()
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        _instance = new MeshManager ();

    return *_instance;
}

void MeshManager::Destroy ( android_vulkan::Renderer &renderer )
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        return;

    _instance->DestroyInternal ( renderer );

    delete _instance;
    _instance = nullptr;
}

void MeshManager::DestroyInternal ( android_vulkan::Renderer &renderer )
{
    for ( auto &mesh : _meshStorage )
        mesh.second->FreeResources ( renderer );

    _meshStorage.clear ();
}

MeshRef MeshManager::LoadMesh ( std::string &&fileName,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );
    MeshRef mesh = std::make_shared<android_vulkan::MeshGeometry> ();

    auto findResult = _meshStorage.find ( fileName );

    if ( findResult != _meshStorage.cend () )
        return findResult->second;

    if ( mesh->LoadMesh ( std::move ( fileName ), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, renderer, commandBuffer ) )
        _meshStorage.insert ( std::make_pair ( mesh->GetName (), mesh ) );
    else
        mesh = nullptr;

    return mesh;
}

} // namespace pbr
