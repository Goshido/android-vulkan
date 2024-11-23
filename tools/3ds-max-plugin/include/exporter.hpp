#ifndef AVP_EXPORTER_HPP
#define AVP_EXPORTER_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <fstream>
#include <optional>
#include <Windows.h>
#include <IGame/IGame.h>

GX_RESTORE_WARNING_STATE


namespace avp {

class Exporter
{
    public:
        constexpr static int FACE_CORNERS = 3;

    protected:
        struct Attributes final
        {
            class Hasher final
            {
                private:
                    std::hash<uint64_t> const       _hashServer {};

                public:
                    Hasher () = default;

                    Hasher ( Hasher const & ) = default;
                    Hasher &operator = ( Hasher const & ) = delete;

                    Hasher ( Hasher && ) = delete;
                    Hasher &operator = ( Hasher && ) = delete;

                    ~Hasher () = default;

                    [[nodiscard]] size_t operator () ( Attributes const &item ) const noexcept;
            };

            int                                     _normal;
            int                                     _position;
            int                                     _tangentBitangent;
            int                                     _uv;

            [[nodiscard]] bool operator == ( Attributes const &other ) const noexcept;
        };

        class AutoReleaseIGameScene final
        {
            private:
                IGameScene*                         _scene = nullptr;

            public:
                AutoReleaseIGameScene () = default;

                AutoReleaseIGameScene ( AutoReleaseIGameScene const & ) = delete;
                AutoReleaseIGameScene &operator = ( AutoReleaseIGameScene const & ) = delete;

                AutoReleaseIGameScene ( AutoReleaseIGameScene && ) = delete;
                AutoReleaseIGameScene &operator = ( AutoReleaseIGameScene && ) = delete;

                ~AutoReleaseIGameScene () noexcept;

                [[nodiscard]] IGameScene &GetScene () noexcept;
                [[nodiscard]] bool Init ( HWND parent ) noexcept;
        };

        class AutoReleaseIGameNode final
        {
            private:
                IGameObject*                        _object = nullptr;
                IGameNode*                          _node = nullptr;

            public:
                AutoReleaseIGameNode () = default;

                AutoReleaseIGameNode ( AutoReleaseIGameNode const & ) = delete;
                AutoReleaseIGameNode &operator = ( AutoReleaseIGameNode const & ) = delete;

                AutoReleaseIGameNode ( AutoReleaseIGameNode && ) = delete;
                AutoReleaseIGameNode &operator = ( AutoReleaseIGameNode && ) = delete;

                ~AutoReleaseIGameNode () noexcept;

                [[nodiscard]] IGameObject &GetGameObject () noexcept;
                [[nodiscard]] bool Init ( HWND parent, IGameScene &scene, INode &node ) noexcept;
        };

    public:
        Exporter () = delete;

        Exporter ( Exporter const & ) = delete;
        Exporter &operator = ( Exporter const & ) = delete;

        Exporter ( Exporter && ) = delete;
        Exporter &operator = ( Exporter && ) = delete;

        ~Exporter () = delete;

    protected:
        [[nodiscard]] static std::optional<std::ofstream> OpenFile ( HWND parent, MSTR const &path ) noexcept;
};

} // namespace avp


#endif // AVP_EXPORTER_HPP
