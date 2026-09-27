#ifndef EDITOR_FONT_STORAGE_HPP
#define EDITOR_FONT_STORAGE_HPP


#include <platform/windows/pbr/font_storage.hpp>


namespace editor {

class FontStorage final
{
    private:
        pbr::FontStorage        &_fontStorage;
        static FontStorage*     _instance;

    public:
        FontStorage () = delete;

        FontStorage ( FontStorage const & ) = delete;
        FontStorage &operator = ( FontStorage const & ) = delete;

        FontStorage ( FontStorage && ) = delete;
        FontStorage &operator = ( FontStorage && ) = delete;

        explicit FontStorage ( pbr::FontStorage &fontStorage ) noexcept;

        ~FontStorage () = default;

        [[nodiscard]] static pbr::FontStorage &Instance () noexcept;
};

} // namespace editor


#endif // EDITOR_FONT_STORAGE_HPP
