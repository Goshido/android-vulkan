#include <pbr/pbr_game.h>
#include <vulkan_utils.h>

namespace pbr {

constexpr static char const* MATERIAL_1_DIFFUSE = "textures/rotating_mesh/sonic-material-1-diffuse.png";
constexpr static char const* MATERIAL_1_MESH = "meshes/rotating_mesh/sonic-material-1.mesh";

constexpr static char const* MATERIAL_2_DIFFUSE = "textures/rotating_mesh/sonic-material-2-diffuse.png";
constexpr static char const* MATERIAL_2_MESH = "meshes/rotating_mesh/sonic-material-2.mesh";
constexpr static char const* MATERIAL_2_NORMAL = "textures/rotating_mesh/sonic-material-2-normal.png";

constexpr static char const* MATERIAL_3_DIFFUSE = "textures/rotating_mesh/sonic-material-3-diffuse.png";
constexpr static char const* MATERIAL_3_MESH = "meshes/rotating_mesh/sonic-material-3.mesh";
constexpr static char const* MATERIAL_3_NORMAL = "textures/rotating_mesh/sonic-material-3-normal.png";

constexpr static size_t const MATERIAL_TEXTURE_COUNT = 5U;
constexpr static size_t const MESH_COUNT = 3U;

constexpr static float const FIELD_OF_VIEW = 60.0F;
constexpr static float const Z_NEAR = 0.1F;
constexpr static float const Z_FAR = 1.0e+3F;

constexpr static uint32_t const RESOLUTION_SCALE_WIDTH = 100U;
constexpr static uint32_t const RESOLUTION_SCALE_HEIGHT = 100U;

//----------------------------------------------------------------------------------------------------------------------

PBRGame::PBRGame ():
    _camera {},
    _commandPool ( VK_NULL_HANDLE ),
    _commandBuffers {},
    _sonicMaterial0 {},
    _sonicMaterial1 {},
    _sonicMaterial2 {},
    _sonicMesh0 {},
    _sonicMesh1 {},
    _sonicMesh2 {}
{
    // NOTHING
}

bool PBRGame::IsReady ()
{
    return static_cast<bool> ( _sonicMesh2 );
}

bool PBRGame::OnInit ( android_vulkan::Renderer &renderer )
{
    VkExtent2D resolution = renderer.GetViewportResolution ();
    resolution.width = resolution.width * RESOLUTION_SCALE_WIDTH / 100U;
    resolution.height = resolution.height * RESOLUTION_SCALE_HEIGHT / 100U;

    if ( !_renderSession.Init ( renderer, resolution ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    if ( !UploadGPUContent ( renderer ) )
    {
        OnDestroy ( renderer );
        return false;
    }

    VkExtent2D const& surfaceResolution = renderer.GetViewportResolution ();

    _camera.SetProjection ( GXDegToRad ( FIELD_OF_VIEW ),
        surfaceResolution.width / static_cast<float> ( surfaceResolution.height ),
        Z_NEAR,
        Z_FAR
    );

    _camera.CaptureInput ();
    return true;
}

bool PBRGame::OnFrame ( android_vulkan::Renderer &renderer, double deltaTime )
{
    _camera.Update ( static_cast<float> ( deltaTime ) );
    _renderSession.Begin ( _camera.GetViewMatrix (), _camera.GetProjectionMatrix () );

    static float angle = 0.0F;
    angle += static_cast<float> ( deltaTime );

    GXMat4 local;
    local.RotationY ( angle );
    local.SetW ( GXVec3 ( 0.0F, -1.0F, 3.0F ) );

    _renderSession.SubmitMesh ( _sonicMesh0, _sonicMaterial0, local );
    _renderSession.SubmitMesh ( _sonicMesh1, _sonicMaterial1, local );
    _renderSession.SubmitMesh ( _sonicMesh2, _sonicMaterial2, local );

    return _renderSession.End ( ePresentTarget::Normal, renderer );
}

bool PBRGame::OnDestroy ( android_vulkan::Renderer &renderer )
{
    const bool result = renderer.CheckVkResult ( vkDeviceWaitIdle ( renderer.GetDevice () ),
        "PBRGame::OnDestroy",
        "Can't wait device idle"
    );

    if ( !result )
        return false;

    _camera.ReleaseInput ();

    DestroyMeshes ( renderer );
    DestroyMaterials ( renderer );
    DestroyCommandPool ( renderer );

    _renderSession.Destroy( renderer );
    return true;
}

bool PBRGame::CreateCommandPool ( android_vulkan::Renderer &renderer )
{
    VkCommandPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0U;
    createInfo.queueFamilyIndex = renderer.GetQueueFamilyIndex ();

    VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult (
        vkCreateCommandPool ( device, &createInfo, nullptr, &_commandPool ),
        "PBRGame::CreateCommandPool",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "PBRGame::_commandPool" )

    constexpr size_t const commandBufferCount = MATERIAL_TEXTURE_COUNT + MESH_COUNT;
    _commandBuffers.resize ( commandBufferCount );

    VkCommandBufferAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.commandBufferCount = static_cast<uint32_t> ( commandBufferCount );
    allocateInfo.commandPool = _commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    result = renderer.CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, _commandBuffers.data () ),
        "PBRGame::CreateCommandPool",
        "Can't allocate command buffers"
    );

    if ( result )
        return true;

    _commandBuffers.clear ();
    return false;
}

void PBRGame::DestroyCommandPool ( android_vulkan::Renderer &renderer )
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( renderer.GetDevice (), _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "PBRGame::_commandPool" )
}

bool PBRGame::CreateMaterials ( android_vulkan::Renderer &renderer )
{
    auto loadTexture = [ & ] ( Texture2DRef &texture,
        char const* file,
        VkFormat format,
        VkCommandBuffer commandBuffer
    ) -> bool {
        texture = std::make_shared<android_vulkan::Texture2D> ();

        if ( texture->UploadData ( file, format, true, renderer, commandBuffer ) )
            return true;

        texture = nullptr;
        return false;
    };

    Texture2DRef texture;
    _sonicMaterial0 = std::make_shared<OpaqueMaterial> ();
    auto* opaqueMaterial = dynamic_cast<OpaqueMaterial*> ( _sonicMaterial0.get () );

    if ( !loadTexture ( texture, MATERIAL_1_DIFFUSE, VK_FORMAT_R8G8B8A8_SRGB, _commandBuffers[ 0U ] ) )
    {
        _sonicMaterial0 = nullptr;
        return false;
    }

    opaqueMaterial->SetAlbedo ( texture );

    _sonicMaterial1 = std::make_shared<OpaqueMaterial> ();
    opaqueMaterial = dynamic_cast<OpaqueMaterial*> ( _sonicMaterial1.get () );

    if ( !loadTexture ( texture, MATERIAL_2_DIFFUSE, VK_FORMAT_R8G8B8A8_SRGB, _commandBuffers[ 1U ] ) )
    {
        _sonicMaterial1 = nullptr;
        return false;
    }

    opaqueMaterial->SetAlbedo ( texture );

    if ( !loadTexture ( texture, MATERIAL_2_NORMAL, VK_FORMAT_R8G8B8A8_UNORM, _commandBuffers[ 2U ] ) )
    {
        _sonicMaterial1 = nullptr;
        return false;
    }

    opaqueMaterial->SetNormal ( texture );

    _sonicMaterial2 = std::make_shared<OpaqueMaterial> ();
    opaqueMaterial = dynamic_cast<OpaqueMaterial*> ( _sonicMaterial2.get () );

    if ( !loadTexture ( texture, MATERIAL_3_DIFFUSE, VK_FORMAT_R8G8B8A8_SRGB, _commandBuffers[ 3U ] ) )
    {
        _sonicMaterial2 = nullptr;
        return false;
    }

    opaqueMaterial->SetAlbedo ( texture );

    if ( !loadTexture ( texture, MATERIAL_3_NORMAL, VK_FORMAT_R8G8B8A8_UNORM, _commandBuffers[ 4U ] ) )
    {
        _sonicMaterial2 = nullptr;
        return false;
    }

    opaqueMaterial->SetNormal ( texture );
    return true;
}

void PBRGame::DestroyMaterials ( android_vulkan::Renderer &renderer )
{
    auto wipeMaterial = [ & ] ( MaterialRef &material ) {
        if ( !material )
            return;

        auto* m = dynamic_cast<OpaqueMaterial*> ( material.get () );

        if ( m->GetAlbedo () )
            m->GetAlbedo ()->FreeResources ( renderer );

        if ( m->GetEmission () )
            m->GetEmission ()->FreeResources ( renderer );

        if ( m->GetNormal () )
            m->GetNormal ()->FreeResources ( renderer );

        if ( m->GetParam () )
            m->GetParam ()->FreeResources ( renderer );

        material = nullptr;
    };

    wipeMaterial ( _sonicMaterial2 );
    wipeMaterial ( _sonicMaterial1 );
    wipeMaterial ( _sonicMaterial0 );
}

bool PBRGame::CreateMeshes ( android_vulkan::Renderer &renderer )
{
    auto loadMesh = [ & ] ( MeshRef &mesh,
        char const* file,
        VkCommandBuffer commandBuffer
    ) -> bool {
        mesh = std::make_shared<android_vulkan::MeshGeometry> ();

        if ( mesh->LoadMesh ( file, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, renderer, commandBuffer ) )
            return true;

        mesh = nullptr;
        return false;
    };

    VkCommandBuffer* meshCommandBuffers = _commandBuffers.data () + MATERIAL_TEXTURE_COUNT;

    if ( !loadMesh ( _sonicMesh0, MATERIAL_1_MESH, meshCommandBuffers[ 0U ] ) )
        return false;

    if ( !loadMesh ( _sonicMesh1, MATERIAL_2_MESH, meshCommandBuffers[ 1U ] ) )
        return false;

    return loadMesh ( _sonicMesh2, MATERIAL_3_MESH, meshCommandBuffers[ 2U ] );
}

void PBRGame::DestroyMeshes ( android_vulkan::Renderer &renderer )
{
    auto wipeMesh = [ & ] ( MeshRef &mesh ) {
        if ( !mesh )
            return;

        mesh->FreeResources ( renderer );
        mesh = nullptr;
    };

    wipeMesh ( _sonicMesh2 );
    wipeMesh ( _sonicMesh1 );
    wipeMesh ( _sonicMesh0 );
}

bool PBRGame::UploadGPUContent ( android_vulkan::Renderer& renderer )
{
    if ( !CreateCommandPool ( renderer ) ) {
        return false;
    }

    if ( !CreateMaterials ( renderer ) ) {
        return false;
    }

    if ( !CreateMeshes ( renderer ) ) {
        return false;
    }

    bool const result = renderer.CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "PBRGame::UploadGPUContent",
        "Can't run upload commands"
    );

    if ( !result )
        return false;

    _sonicMesh2->FreeTransferResources ( renderer );
    _sonicMesh1->FreeTransferResources ( renderer );
    _sonicMesh0->FreeTransferResources ( renderer );

    auto wipeMaterial = [ & ] ( MaterialRef &material ) {
        if ( !material )
            return;

        auto* m = dynamic_cast<OpaqueMaterial*> ( material.get () );

        if ( m->GetAlbedo () )
            m->GetAlbedo ()->FreeTransferResources ( renderer );

        if ( m->GetEmission () )
            m->GetEmission ()->FreeTransferResources ( renderer );

        if ( m->GetNormal () )
            m->GetNormal ()->FreeTransferResources ( renderer );

        if ( !m->GetParam () )
            return;

        m->GetParam ()->FreeTransferResources ( renderer );
    };

    wipeMaterial ( _sonicMaterial2 );
    wipeMaterial ( _sonicMaterial1 );
    wipeMaterial ( _sonicMaterial0 );

    return true;
}

} // namespace pbr
