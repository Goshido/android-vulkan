#include <pbr/image_ui_element.h>
#include <av_assert.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

constexpr size_t ALLOCATE_COMMAND_BUFFERS = 8U;
constexpr size_t COMMAND_BUFFERS_PER_TEXTURE = 1U;
constexpr size_t INITIAL_COMMAND_BUFFERS = 32U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

size_t ImageUIElement::_commandBufferIndex = 0U;
std::vector<VkCommandBuffer> ImageUIElement::_commandBuffers {};
VkCommandPool ImageUIElement::_commandPool = VK_NULL_HANDLE;
std::vector<VkFence> ImageUIElement::_fences {};
android_vulkan::Renderer* ImageUIElement::_renderer = nullptr;
std::unordered_map<std::string, Texture2DRef> ImageUIElement::_textures {};

ImageUIElement::ImageUIElement ( bool &success,
    UIElement const* parent,
    lua_State &vm,
    int errorHandlerIdx,
    std::string &&asset,
    CSSComputedValues &&css
) noexcept:
    UIElement ( true, parent ),
    _asset ( std::move ( asset ) ),
    _css ( css ),
    _isAutoWidth ( css._width.GetType () == LengthValue::eType::Auto ),
    _isAutoHeight ( css._height.GetType () == LengthValue::eType::Auto ),
    _isInlineBlock ( css._display == DisplayProperty::eValue::InlineBlock )
{
    if ( success = lua_checkstack ( &vm, 2 ); !success )
    {
        android_vulkan::LogError ( "pbr::ImageUIElement::ImageUIElement - Stack is too small." );
        return;
    }

    if ( success = lua_getglobal ( &vm, "RegisterImageUIElement" ) == LUA_TFUNCTION; !success )
    {
        android_vulkan::LogError ( "pbr::ImageUIElement::ImageUIElement - Can't find register function." );
        return;
    }

    lua_pushlightuserdata ( &vm, this );

    if ( success = lua_pcall ( &vm, 1, 1, errorHandlerIdx ) == LUA_OK; !success )
    {
        android_vulkan::LogWarning ( "pbr::ImageUIElement::ImageUIElement - Can't append element inside Lua VM." );
    }

    if ( _commandBuffers.size () - _commandBufferIndex < COMMAND_BUFFERS_PER_TEXTURE )
    {
        if ( success = AllocateCommandBuffers ( ALLOCATE_COMMAND_BUFFERS ); !success )
        {
            return;
        }
    }

    if ( auto const findResult = _textures.find ( _asset ); findResult != _textures.cend () )
    {
        _texture = findResult->second;
        return;
    }

    _texture = std::make_shared<android_vulkan::Texture2D> ();

    success = _texture->UploadData ( *_renderer,
        _asset,
        android_vulkan::eFormat::sRGB,
        true,
        _commandBuffers[ _commandBufferIndex ],
        _fences[ _commandBufferIndex ]
    );

    if ( !success )
    {
        _visible = false;
        _texture = nullptr;
        return;
    }

    _commandBufferIndex += COMMAND_BUFFERS_PER_TEXTURE;
    _textures.emplace ( _asset, _texture );
}

bool ImageUIElement::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _renderer = &renderer;

    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &createInfo, nullptr, &_commandPool ),
        "pbr::ImageUIElement::OnInitDevice",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "pbr::ImageUIElement::_commandPool" )
    return AllocateCommandBuffers ( INITIAL_COMMAND_BUFFERS );
}

void ImageUIElement::OnDestroyDevice () noexcept
{
    if ( !_textures.empty () )
    {
        android_vulkan::LogWarning ( "pbr::ImageUIElement::OnDestroyDevice - Memory leak." );
        AV_ASSERT ( false )
    }

    _textures.clear ();

    VkDevice device = _renderer->GetDevice ();

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
        AV_UNREGISTER_COMMAND_POOL ( "pbr::ImageUIElement::_commandPool" )
    }

    auto const clean = [] ( auto &v ) noexcept {
        v.clear ();
        v.shrink_to_fit ();
    };

    clean ( _commandBuffers );

    for ( auto fence : _fences )
    {
        vkDestroyFence ( device, fence, nullptr );
        AV_UNREGISTER_FENCE ( "pbr::ImageUIElement::_fences" )
    }

    clean ( _fences );
    _renderer = nullptr;
}

bool ImageUIElement::SyncGPU () noexcept
{
    if ( !_commandBufferIndex )
        return true;

    VkDevice device = _renderer->GetDevice ();
    auto const fenceCount = static_cast<uint32_t> ( _commandBufferIndex );
    VkFence* fences = _fences.data ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, fenceCount, fences, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "pbr::ImageUIElement::SyncGPU",
        "Can't wait fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, fenceCount, fences ),
        "pbr::ImageUIElement::SyncGPU",
        "Can't reset fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( device, _commandPool, 0U ),
        "pbr::ImageUIElement::SyncGPU",
        "Can't reset command pool"
    );

    if ( !result )
        return false;

    _commandBufferIndex = 0U;
    return true;
}

void ImageUIElement::ApplyLayout ( ApplyLayoutInfo &info ) noexcept
{
    if ( !_visible )
        return;

    GXVec2 const& canvasSize = info._canvasSize;
    CSSUnitToDevicePixel const& units = *info._cssUnits;

    GXVec2 marginTopLeft ( ResolvePixelLength ( _css._marginLeft, canvasSize._data[ 0U ], false, units ),
        ResolvePixelLength ( _css._marginTop, canvasSize._data[ 1U ], true, units )
    );

    GXVec2 const paddingTopLeft ( ResolvePixelLength ( _css._paddingLeft, canvasSize._data[ 0U ], false, units ),
        ResolvePixelLength ( _css._paddingTop, canvasSize._data[ 1U ], true, units )
    );

    GXVec2 marginBottomRight ( ResolvePixelLength ( _css._marginRight, canvasSize._data[ 0U ], false, units ),
        ResolvePixelLength ( _css._marginBottom, canvasSize._data[ 1U ], true, units )
    );

    GXVec2 const paddingBottomRight ( ResolvePixelLength ( _css._paddingRight, canvasSize._data[ 0U ], false, units ),
        ResolvePixelLength ( _css._paddingBottom, canvasSize._data[ 1U ], true, units )
    );

    GXVec2 alpha {};
    alpha.Sum ( marginTopLeft, paddingTopLeft );

    GXVec2 penLocation {};
    float const& parentLeft = info._parentTopLeft._data[ 0U ];
    float const& parentWidth = canvasSize._data[ 0U ];
    float const& paddingRight = paddingBottomRight._data[ 0U ];
    float const& marginRight = marginBottomRight._data[ 0U ];

    auto const newLine = [ & ] () noexcept {
        GXVec2 beta {};
        beta.Sum ( alpha, GXVec2 ( parentLeft, info._currentLineHeight ) );

        penLocation._data[ 0U ] = beta._data[ 0U ];
        penLocation._data[ 1U ] += beta._data[ 1U ];
    };

    switch ( _css._position )
    {
        case PositionProperty::eValue::Absolute:
            penLocation.Sum ( info._parentTopLeft, alpha );
        break;

        case PositionProperty::eValue::Static:
        {
            if ( !_isInlineBlock )
            {
                newLine ();
                break;
            }

            penLocation.Sum ( info._penLocation, alpha );
            float const beta = penLocation._data[ 0U ] + marginRight + paddingRight;

            if ( parentWidth + parentLeft - beta <= 0.0F )
            {
                newLine ();
            }
        }
        break;

        default:
            AV_ASSERT ( false )
        return;
    }

    GXVec2 const imageArea = ResolveSize ( canvasSize, units );

    if ( _css._position == PositionProperty::eValue::Absolute )
    {
        GXVec2 topLeftOffset {};
        _topLeft = penLocation;
        _bottomRight.Sum ( _topLeft, imageArea );
        return;
    }

    // 'static' position territory

    GXVec2 beta {};
    beta.Sum ( imageArea, alpha );

    GXVec2 gamma {};
    gamma.Sum ( marginBottomRight, paddingBottomRight );

    GXVec2 blockSize {};
    blockSize.Sum ( beta, gamma );

    if ( ( blockSize._data[ 0U ] <= 0.0F ) | ( blockSize._data[ 1U ] <= 0.0F ) )
        return;

    info._vertices += UIPass::GetVerticesPerRectangle ();

    auto const computeVisibleBounds = [ & ] () noexcept {
        // Reusing "marginBottomRight" and "marginTopLeft" variables. They will be not used anyway.
        marginTopLeft.Reverse ();
        marginBottomRight.Reverse ();

        GXVec2 yotta {};
        yotta.Sum ( blockSize, GXVec2 ( marginTopLeft._data[ 0U ], marginBottomRight._data[ 1U ] ) );

        _topLeft.Subtract ( info._penLocation, GXVec2 ( yotta._data[ 0U ], -marginTopLeft._data[ 1U ] ) );
        _bottomRight.Sum ( info._penLocation, GXVec2 ( marginBottomRight._data[ 0U ], yotta._data[ 1U ] ) );
    };

    if ( _css._display == DisplayProperty::eValue::Block )
    {
        // Block starts from new line and consumes whole parent block line.
        float const cases[] = { blockSize._data[ 1U ], info._currentLineHeight };
        auto const s = static_cast<size_t> ( info._currentLineHeight != 0.0F );

        info._currentLineHeight = cases[ s ];
        info._newLines = s;
        info._newLineHeight = blockSize._data[ 1U ];
        info._penLocation._data[ 0U ] = parentLeft + blockSize._data[ 0U ];
        info._penLocation._data[ 1U ] = info._parentTopLeft._data[ 1U ];

        computeVisibleBounds ();
        return;
    }

    // 'inline-block' territory.

    bool const firstBlock = info._penLocation.IsEqual ( info._parentTopLeft );
    float const rest = parentWidth + parentLeft - info._penLocation._data[ 0U ];
    bool const blockCanFit = rest >= blockSize._data[ 0U ];

    if ( firstBlock | blockCanFit )
    {
        info._currentLineHeight = std::max ( info._currentLineHeight, blockSize._data[ 1U ] );
        info._newLines = 0U;
        info._penLocation._data[ 0U ] += blockSize._data[ 0U ];

        computeVisibleBounds ();
        return;
    }

    // Block goes to the new line of parent block.
    info._newLines = 1U;
    info._newLineHeight = blockSize._data[ 1U ];
    info._penLocation._data[ 0U ] = parentLeft + blockSize._data[ 0U ];
    info._penLocation._data[ 1U ] += info._currentLineHeight;

    computeVisibleBounds ();
}

void ImageUIElement::Submit ( SubmitInfo &info ) noexcept
{
    if ( !_visible )
        return;

    constexpr GXColorRGB white ( 1.0F, 1.0F, 1.0F, 1.0F );
    constexpr GXVec2 imageTopLeft ( 0.0F, 0.0F );
    constexpr GXVec2 imageBottomRight ( 1.0F, 1.0F );

    FontStorage::GlyphInfo const& g = info._fontStorage->GetTransparentGlyphInfo ();
    UIVertexBuffer& vertexBuffer = info._vertexBuffer;

    UIPass::AppendRectangle ( vertexBuffer.data (),
        white,
        _topLeft,
        _bottomRight,
        g._topLeft,
        g._bottomRight,
        imageTopLeft,
        imageBottomRight
    );

    vertexBuffer = vertexBuffer.subspan ( UIPass::GetVerticesPerRectangle () );
    info._uiPass->SubmitImage ( _texture );
}

GXVec2 ImageUIElement::ResolveSize ( GXVec2 const& parentCanvasSize, CSSUnitToDevicePixel const& units ) noexcept
{
    if ( !_isAutoWidth & _isAutoHeight )
        return ResolveSizeByWidth ( parentCanvasSize._data[ 0U ], units );

    if ( _isAutoWidth & !_isAutoHeight )
        return ResolveSizeByHeight ( parentCanvasSize._data[ 1U ], units );

    return GXVec2 ( ResolvePixelLength ( _css._width, parentCanvasSize._data[ 0U ], false, units ),
        ResolvePixelLength ( _css._height, parentCanvasSize._data[ 1U ], true, units )
    );
}

GXVec2 ImageUIElement::ResolveSizeByWidth ( float parentWidth, CSSUnitToDevicePixel const &units ) noexcept
{
    VkExtent2D const &r = _texture->GetResolution ();

    GXVec2 result {};
    result._data[ 0U ] = ResolvePixelLength ( _css._width, parentWidth, false, units );
    result._data[ 1U ] = result._data[ 0U ] * static_cast<float> ( r.height ) / static_cast<float> ( r.width );

    return result;
}

GXVec2 ImageUIElement::ResolveSizeByHeight ( float parentHeight, CSSUnitToDevicePixel const &units ) noexcept
{
    VkExtent2D const &r = _texture->GetResolution ();

    GXVec2 result {};
    result._data[ 1U ] = ResolvePixelLength ( _css._height, parentHeight, true, units );
    result._data[ 0U ] = result._data[ 1U ] * static_cast<float> ( r.width ) / static_cast<float> ( r.height );

    return result;
}

bool ImageUIElement::AllocateCommandBuffers ( size_t amount ) noexcept
{
    size_t const current = _commandBuffers.size ();
    size_t const size = current + amount;
    _commandBuffers.resize ( size );

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( amount )
    };

    VkDevice device = _renderer->GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, &_commandBuffers[ current ] ),
        "pbr::ImageUIElement::AllocateCommandBuffers",
        "Can't allocate command buffer"
    );

    if ( !result )
        return false;

    _fences.resize ( size );

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    VkFence* fences = _fences.data ();

    for ( size_t i = current; i < size; ++i )
    {
        result = android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, fences + i ),
            "pbr::ImageUIElement::AllocateCommandBuffers",
            "Can't create fence"
        );

        if ( !result )
            return false;

        AV_REGISTER_FENCE ( "pbr::ImageUIElement::_fences" )
    }

    return true;
}

} // namespace pbr
