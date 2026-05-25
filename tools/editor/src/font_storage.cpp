#include <precompiled_headers.hpp>
#include <font_storage.hpp>


namespace editor {

FontStorage* FontStorage::_instance = nullptr;

FontStorage::FontStorage ( pbr::FontStorage &fontStorage ) noexcept:
    _fontStorage ( fontStorage )
{
    _instance = this;
}

pbr::FontStorage &FontStorage::Instance () noexcept
{
    return _instance->_fontStorage;
}

} // namespace editor
