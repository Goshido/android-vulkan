#ifndef PBR_CUBE_MAP_MANAGER_H
#define PBR_CUBE_MAP_MANAGER_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <list>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

GX_RESTORE_WARNING_STATE

#include "types.h"


namespace pbr {

struct CubeMapID final
{
    std::string_view                        _xPlusFile;
    std::string_view                        _xMinusFile;

    std::string_view                        _yPlusFile;
    std::string_view                        _yMinusFile;

    std::string_view                        _zPlusFile;
    std::string_view                        _zMinusFile;

    class Hasher final
    {
        private:
            std::hash<std::string_view>     _hashServer {};

        public:
            Hasher () = default;

            Hasher ( Hasher const & ) = default;
            Hasher& operator = ( Hasher const & ) = default;

            Hasher ( Hasher && ) = default;
            Hasher &operator = ( Hasher && ) = default;

            ~Hasher () = default;

            // hash function. std::unordered_map requirement.
            [[nodiscard]] size_t operator () ( CubeMapID const &me ) const noexcept;
    };

    // Explicitly specifying all 5 constructors are prohibited to conformance with syntax of designated initializers.
    ~CubeMapID () = default;

    // std::unordered_map requirement.
    [[nodiscard]] bool operator == ( CubeMapID const &other ) const noexcept;
};

class CubeMapManager final
{
    private:
        std::unordered_map<CubeMapID, TextureCubeRef, CubeMapID::Hasher>    _cubeMaps {};
        std::unordered_set<std::string_view>                                _knownFiles {};
        std::list<std::string>                                              _stringStorage {};

        static CubeMapManager*                                              _instance;
        static std::shared_timed_mutex                                      _mutex;

    public:
        CubeMapManager ( CubeMapManager const & ) = delete;
        CubeMapManager& operator = ( CubeMapManager const & ) = delete;

        CubeMapManager ( CubeMapManager && ) = delete;
        CubeMapManager& operator = ( CubeMapManager && ) = delete;

        [[nodiscard]] TextureCubeRef LoadCubeMap ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            android_vulkan::TextureCubeData const &data,
            VkCommandBuffer commandBuffer
        ) noexcept;

        [[nodiscard]] static CubeMapManager& GetInstance () noexcept;
        static void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

    private:
        CubeMapManager () = default;
        ~CubeMapManager () = default;

        void DestroyInternal ( android_vulkan::Renderer &renderer ) noexcept;
        [[nodiscard]] std::string_view GetStringView ( char const* string ) noexcept;
};

} // namespace pbr


#endif // PBR_CUBE_MAP_MANAGER_H
