#include <precompiled_headers.hpp>
#include <texture2D_storage.hpp>


namespace editor {

Texture2DStorage* Texture2DStorage::_instance = nullptr;

Texture2DStorage::Texture2DStorage () noexcept
{
    _instance = this;
}

void Texture2DStorage::Load ( std::string_view /*asset*/, Texture2DLoadResult &&/*result*/ ) noexcept
{
    // FUCK
}

void Texture2DStorage::Unload ( Texture2DRef &&/*texture*/ ) noexcept
{
    // FUCK
}

Texture2DStorage &Texture2DStorage::Instance () noexcept
{
    return *_instance;
}

} // namespace editor
