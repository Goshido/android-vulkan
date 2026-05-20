#ifndef EDITOR_TEXTURE_2D_STORAGE_HPP
#define EDITOR_TEXTURE_2D_STORAGE_HPP


#include "texture2D_upload_info.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <deque>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace editor {

class Texture2DStorage final
{
    private:
        struct Item final
        {
            Texture2DRef                                                    _texture {};
            size_t                                                          _references = 0U;
        };

    private:
        std::unordered_map<std::string, Item>                               _storage {};
        std::unordered_map<std::string, std::deque<Texture2DLoadResult>>    _tasks {};

        static Texture2DStorage*                                            _instance;

    public:
        explicit Texture2DStorage () noexcept;

        Texture2DStorage ( Texture2DStorage const & ) = delete;
        Texture2DStorage &operator = ( Texture2DStorage const & ) = delete;

        Texture2DStorage ( Texture2DStorage && ) = delete;
        Texture2DStorage &operator = ( Texture2DStorage && ) = delete;

        ~Texture2DStorage () = default;

        // Note load result will be called from IO thread.
        void Load ( std::string_view asset, Texture2DLoadResult &&result ) noexcept;
        void Unload ( Texture2DRef &&texture ) noexcept;

        [[nodiscard]] static Texture2DStorage &Instance () noexcept;
};

} // namespace editor


#endif // EDITOR_TEXTURE_2D_STORAGE_HPP
