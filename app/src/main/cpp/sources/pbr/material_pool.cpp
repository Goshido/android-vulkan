#include <pbr/material_pool.hpp>
#include <pbr/geometry_pass_texture_descriptor_set_layout.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr size_t FRAMES = 5U;
constexpr size_t MATERIALS_PER_FRAME = 16384U;
constexpr size_t BIND_PER_SET = 5U;
constexpr size_t MATERIALS = FRAMES * MATERIALS_PER_FRAME;

constexpr size_t SLOT_DIFFUSE = 0U;
constexpr size_t SLOT_EMISSION = 1U;
constexpr size_t SLOT_MASK = 2U;
constexpr size_t SLOT_NORMAL = 3U;
constexpr size_t SLOT_PARAM = 4U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

VkDescriptorSet MaterialPool::Acquire () noexcept
{
    VkDescriptorSet set = _descriptorSets[ _itemReadIndex ];
    _itemReadIndex = ( _itemReadIndex + 1U ) % MATERIALS;
    return set;
}

void MaterialPool::Commit () noexcept
{
    _itemBaseIndex = _itemWriteIndex;
    _itemReadIndex = _itemWriteIndex;
    _itemWritten = 0U;
}

bool MaterialPool::Init ( VkDevice device, DefaultTextureManager const &defaultTextureManager ) noexcept
{
    _defaultDiffuse = defaultTextureManager.GetAlbedo ()->GetImageView ();
    _defaultEmission = defaultTextureManager.GetEmission ()->GetImageView ();
    _defaultMask = defaultTextureManager.GetMask ()->GetImageView ();
    _defaultNormal = defaultTextureManager.GetNormal ()->GetImageView ();
    _defaultParam = defaultTextureManager.GetParams ()->GetImageView ();

    constexpr size_t totalImages = MATERIALS * BIND_PER_SET;

    VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<uint32_t> ( totalImages )
        }
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( MATERIALS ),
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::MaterialPool::Init",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::MaterialPool::_descriptorPool" )

    _descriptorSets.resize ( MATERIALS );

    std::vector<VkDescriptorSetLayout> const layouts ( MATERIALS,
        GeometryPassTextureDescriptorSetLayout ().GetLayout ()
    );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, _descriptorSets.data () ),
        "pbr::MaterialPool::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    // Initialize all immutable constant fields.

    VkDescriptorImageInfo const image
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    _imageInfo.resize ( totalImages, image );

    constexpr VkWriteDescriptorSet writeSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = 0U,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    _writeSets.resize ( totalImages, writeSet );

    for ( size_t i = 0U; i < MATERIALS; ++i )
    {
        size_t idx = BIND_PER_SET * i;
        VkDescriptorSet set = _descriptorSets[ i ];

        VkWriteDescriptorSet &diffuse = _writeSets[ idx ];
        diffuse.dstSet = set;
        diffuse.dstBinding = static_cast<uint32_t> ( SLOT_DIFFUSE );
        diffuse.pImageInfo = &_imageInfo[ idx++ ];

        VkWriteDescriptorSet &emission = _writeSets[ idx ];
        emission.dstSet = set;
        emission.dstBinding = static_cast<uint32_t> ( SLOT_EMISSION );
        emission.pImageInfo = &_imageInfo[ idx++ ];

        VkWriteDescriptorSet &mask = _writeSets[ idx ];
        mask.dstSet = set;
        mask.dstBinding = static_cast<uint32_t> ( SLOT_MASK );
        mask.pImageInfo = &_imageInfo[ idx++ ];

        VkWriteDescriptorSet &normal = _writeSets[ idx ];
        normal.dstSet = set;
        normal.dstBinding = static_cast<uint32_t> ( SLOT_NORMAL );
        normal.pImageInfo = &_imageInfo[ idx++ ];

        VkWriteDescriptorSet &param = _writeSets[ idx ];
        param.dstSet = set;
        param.dstBinding = static_cast<uint32_t> ( SLOT_PARAM );
        param.pImageInfo = &_imageInfo[ idx ];
    }

    // Now all what is needed to do is to init "_imageInfo::imageView".
    // Then to invoke vkUpdateDescriptorSets.

    _itemBaseIndex = 0U;
    _itemReadIndex = 0U;
    _itemWriteIndex = 0U;
    _itemWritten = 0U;

    return true;
}

void MaterialPool::Destroy ( VkDevice device ) noexcept
{
    if ( _descriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::MaterialPool::_descriptorPool" )
    }

    auto const clean = [] ( auto &vector ) noexcept {
        vector.clear ();
        vector.shrink_to_fit ();
    };

    clean ( _descriptorSets );
    clean ( _imageInfo );
    clean ( _writeSets );
}

void MaterialPool::IssueSync ( VkDevice device ) const noexcept
{
    size_t const idx = _itemBaseIndex + _itemWritten;
    size_t const cases[] = { 0U, idx - MATERIALS };
    size_t const more = cases[ static_cast<size_t> ( idx > MATERIALS ) ];
    size_t const available = _itemWritten - more;

    VkWriteDescriptorSet const* writeSets = _writeSets.data ();

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( BIND_PER_SET * available ),
        writeSets + BIND_PER_SET * _itemBaseIndex,
        0U,
        nullptr
    );

    if ( more > 0U )
    {
        vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( BIND_PER_SET * more ), writeSets, 0U, nullptr );
    }
}

void MaterialPool::Push ( GeometryPassMaterial &material ) noexcept
{
    VkDescriptorImageInfo* images = _imageInfo.data ();

    Texture2DRef const &diffuse = material.GetAlbedo ();
    Texture2DRef const &emission = material.GetEmission ();
    Texture2DRef const &mask = material.GetMask ();
    Texture2DRef const &normal = material.GetNormal ();
    Texture2DRef const &param = material.GetParam ();

    size_t const base = _itemWriteIndex * BIND_PER_SET;
    images[ base ].imageView = diffuse ? diffuse->GetImageView () : _defaultDiffuse;
    images[ base + SLOT_EMISSION ].imageView = emission ? emission->GetImageView () : _defaultEmission;
    images[ base + SLOT_MASK ].imageView = mask ? mask->GetImageView () : _defaultMask;
    images[ base + SLOT_NORMAL ].imageView = normal ? normal->GetImageView () : _defaultNormal;
    images[ base + SLOT_PARAM ].imageView = param ? param->GetImageView () : _defaultParam;

    _itemWriteIndex = ( _itemWriteIndex + 1U ) % MATERIALS;
    ++_itemWritten;
}

} // namespace pbr
