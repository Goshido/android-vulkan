#include <pbr/mesh_manager.hpp>


namespace pbr {

MeshManager* MeshManager::_instance = nullptr;
std::mutex MeshManager::_mutex;

void MeshManager::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( _toFreeTransferResource.empty () )
        return;

    for ( auto* mesh : _toFreeTransferResource )
        mesh->FreeTransferResources ( renderer );

    _toFreeTransferResource.clear ();
}

MeshRef MeshManager::LoadMesh ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    char const* fileName,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    commandBufferConsumed = 0U;
    MeshRef mesh = std::make_shared<android_vulkan::MeshGeometry> ();

    if ( !fileName )
        return mesh;

    std::unique_lock<std::mutex> const lock ( _mutex );
    auto findResult = _meshStorage.find ( fileName );

    if ( findResult != _meshStorage.cend () )
        return findResult->second;

    if ( !mesh->LoadMesh ( fileName, renderer, commandBuffer, fence ) )
    {
        mesh = nullptr;
    }
    else
    {
        _meshStorage.insert ( std::make_pair ( std::string_view ( mesh->GetName () ), mesh ) );
        _toFreeTransferResource.push_back ( mesh.get () );
    }

    commandBufferConsumed = 1U;
    return mesh;
}

MeshManager &MeshManager::GetInstance () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( !_instance )
        _instance = new MeshManager ();

    return *_instance;
}

void MeshManager::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( !_instance )
        return;

    _instance->DestroyInternal ( renderer );

    delete _instance;
    _instance = nullptr;
}

void MeshManager::DestroyInternal ( android_vulkan::Renderer &renderer ) noexcept
{
    for ( auto &mesh : _meshStorage )
        mesh.second->FreeResources ( renderer );

    _meshStorage.clear ();
}

} // namespace pbr
