#ifndef EDITOR_PROPERTY_HPP
#define EDITOR_PROPERTY_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <functional>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace editor {

class Property
{
    public:
        using Handler = std::move_only_function<void ()>;
        using Connection = uint32_t;

    private:
        std::unordered_map<uint32_t, Handler>       _connections {};
        uint32_t                                    _id = 0U;

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
