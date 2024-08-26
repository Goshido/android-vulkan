#ifndef EDITOR_OS_UTILS_HPP
#define EDITOR_OS_UTILS_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <filesystem>

GX_RESTORE_WARNING_STATE


namespace editor {

class OSUtils final
{
    public:
        OSUtils () = delete;

        OSUtils ( OSUtils const & ) = delete;
        OSUtils &operator = ( OSUtils const & ) = delete;

        OSUtils ( OSUtils && ) = delete;
        OSUtils &operator = ( OSUtils && ) = delete;

        ~OSUtils () = delete;

        [[nodiscard]] static std::filesystem::path ResolvePath ( std::string_view const &file ) noexcept;
        [[nodiscard]] static std::string ToString ( std::filesystem::path const &path ) noexcept;
};

} // namespace editor


#endif // EDITOR_OS_UTILS_HPP
