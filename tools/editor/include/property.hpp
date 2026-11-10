#ifndef EDITOR_PROPERTY_HPP
#define EDITOR_PROPERTY_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <functional>
#include <vector>

GX_RESTORE_WARNING_STATE


namespace editor {

class Property
{
    public:
        using Handler = std::move_only_function<void ()>;
        using Connection = size_t;

    private:
        std::vector<Handler>    _connections {};

    public:
        Property ( Property const & ) = delete;
        Property &operator = ( Property const & ) = delete;

        Property ( Property && ) = delete;
        Property &operator = ( Property && ) = delete;

        virtual ~Property () = default;

        [[nodiscard]] Connection Connect ( Handler &&handler ) noexcept;
        void Disconnect ( Connection connection ) noexcept;

    protected:
        explicit Property () = default;

        void OnUpdated () noexcept;
};

} // namespace editor


#endif // EDITOR_PROPERTY_HPP
