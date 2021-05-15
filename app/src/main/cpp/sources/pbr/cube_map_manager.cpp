#include <pbr/cube_map_manager.h>


namespace pbr {

CubeMapID::Hasher::Hasher () noexcept:
    _hashServer {}
{
    // NOTHING
}

size_t CubeMapID::Hasher::operator () ( CubeMapID const &me ) const noexcept
{
    // Hash function is based on Boost implementation:
    // https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine

    size_t hash = 0U;

    auto hashCombine = [ & ] ( std::string_view const &string )
    {
        constexpr size_t const magic = 0x9e3779b9U;
        hash ^= _hashServer ( string ) + magic + ( hash << 6U ) + ( hash >> 2U );
    };

    hashCombine ( me._xPlusFile );
    hashCombine ( me._xMinusFile );
    hashCombine ( me._yPlusFile );
    hashCombine ( me._yMinusFile );
    hashCombine ( me._zPlusFile );
    hashCombine ( me._zMinusFile );

    return hash;
}

[[nodiscard]] bool CubeMapID::operator == ( CubeMapID const &other ) const noexcept
{
    if ( _xPlusFile != other._xPlusFile )
        return false;

    if ( _xMinusFile != other._xMinusFile )
        return false;

    if ( _yPlusFile != other._yPlusFile )
        return false;

    if ( _yMinusFile != other._yMinusFile )
        return false;

    if ( _zPlusFile != other._zPlusFile )
        return false;

    return _zMinusFile == other._zPlusFile;
}

//----------------------------------------------------------------------------------------------------------------------

CubeMapManager* CubeMapManager::_instance = nullptr;
std::shared_timed_mutex CubeMapManager::_mutex;

TextureCubeRef CubeMapManager::LoadCubeMap ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    android_vulkan::TextureCubeData const &data,
    VkCommandBuffer commandBuffer
)
{
    commandBufferConsumed = 0U;
    TextureCubeRef textureCube = std::make_shared<android_vulkan::TextureCube> ();

    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    CubeMapID const id
    {
        ._xPlusFile = GetStringView ( data._xPlusFile ),
        ._xMinusFile = GetStringView ( data._xMinusFile ),
        ._yPlusFile = GetStringView ( data._yPlusFile ),
        ._yMinusFile = GetStringView ( data._yMinusFile ),
        ._zPlusFile = GetStringView ( data._zPlusFile ),
        ._zMinusFile = GetStringView ( data._zMinusFile )
    };

    auto findResult = _cubeMaps.find ( id );

    if ( findResult != _cubeMaps.cend () )
        return findResult->second;

    if ( !textureCube->UploadData ( renderer, data, commandBuffer ) )
        textureCube = nullptr;
    else
        _cubeMaps.insert ( std::make_pair ( id, textureCube ) );

    commandBufferConsumed = 1U;
    return textureCube;
}

CubeMapManager& CubeMapManager::GetInstance ()
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        _instance = new CubeMapManager ();

    return *_instance;
}

void CubeMapManager::Destroy ( VkDevice device )
{
    std::unique_lock<std::shared_timed_mutex> const lock ( _mutex );

    if ( !_instance )
        return;

    _instance->DestroyInternal ( device );

    delete _instance;
    _instance = nullptr;
}

void CubeMapManager::DestroyInternal ( VkDevice device )
{
    for ( auto& cubeMap : _cubeMaps )
        cubeMap.second->FreeResources ( device );

    _cubeMaps.clear ();
    _knownFiles.clear ();
    _stringStorage.clear ();
}

std::string_view CubeMapManager::GetStringView ( char const* string )
{
    auto const findResult = _knownFiles.find ( string );

    if ( findResult != _knownFiles.cend () )
        return *findResult;

    auto const& newString = _stringStorage.emplace_back ( string );
    std::string_view const view ( newString );
    _knownFiles.insert ( view );

    return view;
}

} // namespace pbr
