#include <pbr/mesh_manager.h>


namespace pbr {

MeshManager* MeshManager::_instance = nullptr;
std::shared_timed_mutex MeshManager::_mutex;

MeshRef MeshManager::LoadMesh ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    char const* fileName,
    VkCommandBuffer commandBuffer
) noexcept
{
    commandBufferConsumed = 0U;
    MeshRef mesh = std::make_shared<android_vulkan::MeshGeometry> ();

    if ( !fileName )
        return mesh;

    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );
    auto findResult = _meshStorage.find ( fileName );

    if ( findResult != _meshStorage.cend () )
        return findResult->second;

    if ( !mesh->LoadMesh ( fileName, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, renderer, commandBuffer ) )
        mesh = nullptr;
    else
        _meshStorage.insert ( std::make_pair ( std::string_view ( mesh->GetName () ), mesh ) );

    commandBufferConsumed = 1U;
    return mesh;
}

MeshManager& MeshManager::GetInstance () noexcept
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        _instance = new MeshManager ();

    return *_instance;
}

void MeshManager::Destroy ( VkDevice device ) noexcept
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        return;

    _instance->DestroyInternal ( device );

    delete _instance;
    _instance = nullptr;
}

void MeshManager::DestroyInternal ( VkDevice device ) noexcept
{
    for ( auto& mesh : _meshStorage )
        mesh.second->FreeResources ( device );

    _meshStorage.clear ();
}

} // namespace pbr
