#ifndef EDITOR_SAVE_STATE_HPP
#define EDITOR_SAVE_STATE_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

GX_RESTORE_WARNING_STATE


namespace editor {

class SaveState;

class SaveState final
{
    public:
        class Container final
        {
            friend class SaveState;

            public:

#pragma pack ( push, 1 )

                enum class eType : uint8_t
                {
                    Null = 0U,
                    Container = 1U,
                    Array = 2U,
                    Int8 = 3U,
                    UInt8 = 4U,
                    Int16 = 5U,
                    UInt16 = 6U,
                    Int32 = 7U,
                    UInt32 = 8U,
                    Int64 = 9U,
                    UInt64 = 10U,
                    String = 11U,
                    Float = 12U,
                    Double = 13U,
                    Bool = 14U
                };

#pragma pack ( pop )

            private:
                union Data
                {
                    int8_t                                      _i8;
                    uint8_t                                     _ui8;
                    int16_t                                     _i16;
                    uint16_t                                    _ui16;
                    int32_t                                     _i32;
                    uint32_t                                    _ui32;
                    int64_t                                     _i64;
                    uint64_t                                    _ui64;
                    float                                       _float;
                    double                                      _double;
                    bool                                        _bool;
                };

            private:
                std::vector<Container>                          _arrayData {};
                std::unordered_map<std::string, Container>      _containerData {};
                Data                                            _data {};
                mutable size_t                                  _idx = 0U;
                std::string                                     _string {};
                eType                                           _type = eType::Null;

            public:
                Container () = default;

                Container ( Container const & ) = delete;
                Container &operator = ( Container const & ) = delete;

                Container ( Container && ) = default;
                Container &operator = ( Container && ) = default;

                ~Container () = default;

                [[nodiscard]] eType GetType () const noexcept;
                [[nodiscard]] size_t GetArraySize () const noexcept;

                [[nodiscard]] Container &WriteArray ( std::string_view const &key ) noexcept;

                [[nodiscard]] Container &WriteContainer () noexcept;
                [[nodiscard]] Container &WriteContainer ( std::string_view const &key ) noexcept;

                void Write ( int8_t value ) noexcept;
                void Write ( uint8_t value ) noexcept;
                void Write ( int16_t value ) noexcept;
                void Write ( uint16_t value ) noexcept;
                void Write ( int32_t value ) noexcept;
                void Write ( uint32_t value ) noexcept;
                void Write ( int64_t value ) noexcept;
                void Write ( uint64_t value ) noexcept;
                void Write ( float value ) noexcept;
                void Write ( double value ) noexcept;
                void Write ( bool value ) noexcept;
                void Write ( std::string_view const &value ) noexcept;

                void Write ( std::string_view const &key, int8_t value ) noexcept;
                void Write ( std::string_view const &key, uint8_t value ) noexcept;
                void Write ( std::string_view const &key, int16_t value ) noexcept;
                void Write ( std::string_view const &key, uint16_t value ) noexcept;
                void Write ( std::string_view const &key, int32_t value ) noexcept;
                void Write ( std::string_view const &key, uint32_t value ) noexcept;
                void Write ( std::string_view const &key, int64_t value ) noexcept;
                void Write ( std::string_view const &key, uint64_t value ) noexcept;
                void Write ( std::string_view const &key, float value ) noexcept;
                void Write ( std::string_view const &key, double value ) noexcept;
                void Write ( std::string_view const &key, bool value ) noexcept;
                void Write ( std::string_view const &key, std::string_view const &value ) noexcept;

                [[nodiscard]] Container const &ReadArray ( std::string_view const &key ) const noexcept;

                [[nodiscard]] Container const &ReadContainer () const noexcept;
                [[nodiscard]] Container const &ReadContainer ( std::string_view const &key ) const noexcept;

                [[nodiscard]] int8_t Read ( int8_t defaultValue ) const noexcept;
                [[nodiscard]] uint8_t Read ( uint8_t defaultValue ) const noexcept;
                [[nodiscard]] int16_t Read ( int16_t defaultValue ) const noexcept;
                [[nodiscard]] uint16_t Read ( uint16_t defaultValue ) const noexcept;
                [[nodiscard]] int32_t Read ( int32_t defaultValue ) const noexcept;
                [[nodiscard]] uint32_t Read ( uint32_t defaultValue ) const noexcept;
                [[nodiscard]] int64_t Read ( int64_t defaultValue ) const noexcept;
                [[nodiscard]] uint64_t Read ( uint64_t defaultValue ) const noexcept;
                [[nodiscard]] float Read ( float defaultValue ) const noexcept;
                [[nodiscard]] double Read ( double defaultValue ) const noexcept;
                [[nodiscard]] bool Read ( bool defaultValue ) const noexcept;
                [[nodiscard]] std::string_view Read ( std::string_view const &defaultValue ) const noexcept;

                [[nodiscard]] int8_t Read ( std::string_view const &key, int8_t defaultValue ) const noexcept;
                [[nodiscard]] uint8_t Read ( std::string_view const &key, uint8_t defaultValue ) const noexcept;
                [[nodiscard]] int16_t Read ( std::string_view const &key, int16_t defaultValue ) const noexcept;
                [[nodiscard]] uint16_t Read ( std::string_view const &key, uint16_t defaultValue ) const noexcept;
                [[nodiscard]] int32_t Read ( std::string_view const &key, int32_t defaultValue ) const noexcept;
                [[nodiscard]] uint32_t Read ( std::string_view const &key, uint32_t defaultValue ) const noexcept;
                [[nodiscard]] int64_t Read ( std::string_view const &key, int64_t defaultValue ) const noexcept;
                [[nodiscard]] uint64_t Read ( std::string_view const &key, uint64_t defaultValue ) const noexcept;
                [[nodiscard]] float Read ( std::string_view const &key, float defaultValue ) const noexcept;
                [[nodiscard]] double Read ( std::string_view const &key, double defaultValue ) const noexcept;
                [[nodiscard]] bool Read ( std::string_view const &key, bool defaultValue ) const noexcept;

                [[nodiscard]] std::string_view Read ( std::string_view const &key,
                    std::string_view const &defaultValue
                ) const noexcept;

            private:
                explicit Container ( eType type ) noexcept;
                explicit Container ( int8_t value ) noexcept;
                explicit Container ( uint8_t value ) noexcept;
                explicit Container ( int16_t value ) noexcept;
                explicit Container ( uint16_t value ) noexcept;
                explicit Container ( int32_t value ) noexcept;
                explicit Container ( uint32_t value ) noexcept;
                explicit Container ( int64_t value ) noexcept;
                explicit Container ( uint64_t value ) noexcept;
                explicit Container ( float value ) noexcept;
                explicit Container ( double value ) noexcept;
                explicit Container ( bool value ) noexcept;
                explicit Container ( std::string_view const &value ) noexcept;
        };

    private:
        using Binary = std::vector<uint8_t>;

    private:
        Container                                               _container { Container::eType::Container };

    public:
        SaveState () = default;

        SaveState ( SaveState const & ) = delete;
        SaveState &operator = ( SaveState const & ) = delete;

        SaveState ( SaveState && ) = delete;
        SaveState &operator = ( SaveState && ) = delete;

        ~SaveState () = default;

        [[nodiscard]] bool Load ( std::string_view const &file ) noexcept;
        [[nodiscard]] bool Save ( std::string_view const &file ) noexcept;

        [[nodiscard]] Container &GetContainer () noexcept;
        [[nodiscard]] Container const &GetContainer () const noexcept;

    private:
        static void SaveContainer ( Binary &aData,
            Binary &bData,
            Container const &container
        ) noexcept;

        static void EncodeArray ( Binary &aData, Binary &bData, Container const &container ) noexcept;

        static void EncodeContainer ( Binary &aData,
            Binary &bData,
            Container const &container,
            bool writeType
        ) noexcept;

        static void EncodeInt8 ( Binary &aData, Container const &container, bool writeType ) noexcept;
        static void EncodeUInt8 ( Binary &aData, Container const &container, bool writeType ) noexcept;
        static void EncodeInt16 ( Binary &aData, Container const &container, bool writeType ) noexcept;
        static void EncodeUInt16 ( Binary &aData, Container const &container, bool writeType ) noexcept;
        static void EncodeInt32 ( Binary &aData, Container const &container, bool writeType ) noexcept;
        static void EncodeUInt32 ( Binary &aData, Container const &container, bool writeType ) noexcept;
        static void EncodeInt64 ( Binary &aData, Container const &container, bool writeType ) noexcept;
        static void EncodeUInt64 ( Binary &aData, Container const &container, bool writeType ) noexcept;
        static void EncodeString ( Binary &aData, Binary &bData, Container const &container, bool writeType ) noexcept;
        static void EncodeFloat ( Binary &aData, Container const &container, bool writeType ) noexcept;
        static void EncodeDouble ( Binary &aData, Container const &container, bool writeType ) noexcept;
        static void EncodeBool ( Binary &aData, Container const &container, bool writeType ) noexcept;
        [[nodiscard]] static size_t EncodeType ( Binary &aData, size_t offset, Container::eType type ) noexcept;
        static void EncodeKey ( Binary &aData, Binary &bData, std::string const &key ) noexcept;

        [[nodiscard]] static std::filesystem::path ResolvePath ( std::string_view const &file ) noexcept;
};

} // namespace editor


#endif // EDITOR_SAVE_STATE_HPP
