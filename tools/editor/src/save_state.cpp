#include <av_assert.hpp>
#include <save_state.hpp>
#include <logger.hpp>
#include <trace.hpp>
#include <android_vulkan_sdk/primitive_types.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <fstream>
#include <Windows.h>

GX_RESTORE_WARNING_STATE


namespace editor {

namespace {

#pragma pack ( push, 1 )

struct SaveStateInfo final
{
    android_vulkan::UTF8Offset      _bDataOffset = 0U;
};

struct Int8Info final
{
    int8_t      _data = 0;
};

struct UInt8Info final
{
    uint8_t     _data = 0U;
};

struct Int16Info final
{
    int16_t     _data = 0;
};

struct UInt16Info final
{
    uint16_t    _data = 0U;
};

struct Int32Info final
{
    int32_t     _data = 0;
};

struct UInt32Info final
{
    uint32_t    _data = 0U;
};

struct Int64Info final
{
    int64_t     _data = 0;
};

struct UInt64Info final
{
    uint64_t    _data = 0U;
};

struct StringInfo final
{
    android_vulkan::UTF8Offset      _data = 0U;
};

struct FloatInfo final
{
    float       _data = 0.0F;
};

struct DoubleInfo final
{
    double      _data = 0.0;
};

struct BoolInfo final
{
    android_vulkan::Boolean     _data = android_vulkan::AV_FALSE;
};

struct ContainerInfo final
{
    uint64_t    _fields = 0U;
};

struct ArrayInfo final
{
    uint64_t                        _size = 0U;
    SaveState::Container::eType     _type = SaveState::Container::eType::Null;
};

#pragma pack ( pop )

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

SaveState::Container::eType SaveState::Container::GetType () const noexcept
{
    return _type;
}

size_t SaveState::Container::GetArraySize () const noexcept
{
    return _arrayData.size ();
}

SaveState::Container &SaveState::Container::WriteArray ( std::string_view const &key ) noexcept
{
    if ( _type == eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Container has array type. Skipping operation..." );
        static Container null {};
        return null;
    }

    auto const result = _containerData.insert (
        std::make_pair<std::string, Container> ( std::string ( key ), Container ( eType::Array ) )
    );

    return result.first->second;
}

SaveState::Container &SaveState::Container::WriteContainer () noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        static Container null {};
        return null;
    }

    if ( !_arrayData.empty () && _arrayData.back ()._type != eType::Container ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
        static Container null {};
        return null;
    }

    _arrayData.push_back ( Container ( eType::Container ) );
    return _arrayData.back ();
}

SaveState::Container &SaveState::Container::WriteContainer ( std::string_view const &key ) noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected conteiner type. Skipping operation..." );
        static Container null {};
        return null;
    }

    auto const result = _containerData.insert ( 
        std::make_pair<std::string, Container> ( std::string ( key ), Container ( eType::Container ) )
    );

    return result.first->second;
}

void SaveState::Container::Write ( int8_t value ) noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        return;
    }

    if ( _arrayData.empty () || _arrayData.back ()._type == eType::Int8 ) [[likely]]
    {
        _arrayData.push_back ( Container ( value ) );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
}

void SaveState::Container::Write ( uint8_t value ) noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        return;
    }

    if ( _arrayData.empty () || _arrayData.back ()._type == eType::UInt8 ) [[likely]]
    {
        _arrayData.push_back ( Container ( value ) );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
}

void SaveState::Container::Write ( int16_t value ) noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        return;
    }

    if ( _arrayData.empty () || _arrayData.back ()._type == eType::Int16 ) [[likely]]
    {
        _arrayData.push_back ( Container ( value ) );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
}

void SaveState::Container::Write ( uint16_t value ) noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        return;
    }

    if ( _arrayData.empty () || _arrayData.back ()._type == eType::UInt16 ) [[likely]]
    {
        _arrayData.push_back ( Container ( value ) );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
}

void SaveState::Container::Write ( int32_t value ) noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        return;
    }

    if ( _arrayData.empty () || _arrayData.back ()._type == eType::Int32 ) [[likely]]
    {
        _arrayData.push_back ( Container ( value ) );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
}

void SaveState::Container::Write ( uint32_t value ) noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        return;
    }

    if ( _arrayData.empty () || _arrayData.back ()._type == eType::UInt32 ) [[likely]]
    {
        _arrayData.push_back ( Container ( value ) );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
}

void SaveState::Container::Write ( int64_t value ) noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        return;
    }

    if ( _arrayData.empty () || _arrayData.back ()._type == eType::Int64 ) [[likely]]
    {
        _arrayData.push_back ( Container ( value ) );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
}

void SaveState::Container::Write ( uint64_t value ) noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        return;
    }

    if ( _arrayData.empty () || _arrayData.back ()._type == eType::UInt64 ) [[likely]]
    {
        _arrayData.push_back ( Container ( value ) );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
}

void SaveState::Container::Write ( float value ) noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        return;
    }

    if ( _arrayData.empty () || _arrayData.back ()._type == eType::Float ) [[likely]]
    {
        _arrayData.push_back ( Container ( value ) );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
}

void SaveState::Container::Write ( double value ) noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        return;
    }

    if ( _arrayData.empty () || _arrayData.back ()._type == eType::Double ) [[likely]]
    {
        _arrayData.push_back ( Container ( value ) );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
}

void SaveState::Container::Write ( bool value ) noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        return;
    }

    if ( _arrayData.empty () || _arrayData.back ()._type == eType::Bool ) [[likely]]
    {
        _arrayData.push_back ( Container ( value ) );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const &value ) noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        AV_ASSERT ( false )
        android_vulkan::LogWarning ( "SaveState: Expected array. Skipping operation..." );
        return;
    }

    if ( _arrayData.empty () || _arrayData.back ()._type == eType::String ) [[likely]]
    {
        _arrayData.push_back ( Container ( value ) );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Changing array type. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const &key, int8_t value ) noexcept
{
    if ( _type == eType::Container ) [[likely]]
    {
        _containerData[ std::string ( key ) ] = Container ( value );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Expected container. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const &key, uint8_t value ) noexcept
{
    if ( _type == eType::Container ) [[likely]]
    {
        _containerData[ std::string ( key ) ] = Container ( value );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Expected container. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const &key, int16_t value ) noexcept
{
    if ( _type == eType::Container ) [[likely]]
    {
        _containerData[ std::string ( key ) ] = Container ( value );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Expected container. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const &key, uint16_t value ) noexcept
{
    if ( _type == eType::Container ) [[likely]]
    {
        _containerData[ std::string ( key ) ] = Container ( value );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Expected container. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const &key, int32_t value ) noexcept
{
    if ( _type == eType::Container ) [[likely]]
    {
        _containerData[ std::string ( key ) ] = Container ( value );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Expected container. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const &key, uint32_t value ) noexcept
{
    if ( _type == eType::Container ) [[likely]]
    {
        _containerData[ std::string ( key ) ] = Container ( value );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Expected container. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const &key, int64_t value ) noexcept
{
    if ( _type == eType::Container ) [[likely]]
    {
        _containerData[ std::string ( key ) ] = Container ( value );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Expected container. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const &key, uint64_t value ) noexcept
{
    if ( _type == eType::Container ) [[likely]]
    {
        _containerData[ std::string ( key ) ] = Container ( value );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Expected container. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const &key, float value ) noexcept
{
    if ( _type == eType::Container ) [[likely]]
    {
        _containerData[ std::string ( key ) ] = Container ( value );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Expected container. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const &key, double value ) noexcept
{
    if ( _type == eType::Container ) [[likely]]
    {
        _containerData[ std::string ( key ) ] = Container ( value );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Expected container. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const& key, bool value ) noexcept
{
    if ( _type == eType::Container ) [[likely]]
    {
        _containerData[ std::string ( key ) ] = Container ( value );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Expected container. Skipping operation..." );
}

void SaveState::Container::Write ( std::string_view const &key, std::string_view const &value ) noexcept
{
    if ( _type == eType::Container ) [[likely]]
    {
        _containerData[ std::string ( key ) ] = Container ( value );
        return;
    }

    AV_ASSERT ( false )
    android_vulkan::LogWarning ( "SaveState: Expected container. Skipping operation..." );
}

SaveState::Container const &SaveState::Container::ReadArray ( std::string_view const &key ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
    {
        static Container const null {};
        return null;
    }

    if ( auto const findResult = _containerData.find ( std::string ( key ) ); findResult != _containerData.cend () )
        return findResult->second;

    static Container const null {};
    return null;
}

SaveState::Container const &SaveState::Container::ReadContainer () const noexcept
{
    if ( _type != eType::Array ) [[unlikely]]
    {
        static Container const null {};
        return null;
    }

    if ( _arrayData.size () <= _idx ) [[unlikely]]
    {
        static Container const null {};
        return null;
    }

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::Container ) [[likely]]
        return c;

    static Container const null {};
    return null;
}

SaveState::Container const &SaveState::Container::ReadContainer ( std::string_view const &key ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
    {
        static Container const null {};
        return null;
    }

    if ( auto const findResult = _containerData.find ( std::string ( key ) ); findResult != _containerData.cend () )
        return findResult->second;

    static Container const null {};
    return null;
}

int8_t SaveState::Container::Read ( int8_t defaultValue ) const noexcept
{
    if ( ( _type != eType::Array ) | ( _arrayData.size () <= _idx ) ) [[unlikely]]
        return defaultValue;

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::Int8 ) [[likely]]
        return c._data._i8;

    return defaultValue;
}

uint8_t SaveState::Container::Read ( uint8_t defaultValue ) const noexcept
{
    if ( ( _type != eType::Array ) | ( _arrayData.size () <= _idx ) ) [[unlikely]]
        return defaultValue;

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::UInt8 ) [[likely]]
        return c._data._ui8;

    return defaultValue;
}

int16_t SaveState::Container::Read ( int16_t defaultValue ) const noexcept
{
    if ( ( _type != eType::Array ) | ( _arrayData.size () <= _idx ) ) [[unlikely]]
        return defaultValue;

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::Int16 ) [[likely]]
        return c._data._i16;

    return defaultValue;
}

uint16_t SaveState::Container::Read ( uint16_t defaultValue ) const noexcept
{
    if ( ( _type != eType::Array ) | ( _arrayData.size () <= _idx ) ) [[unlikely]]
        return defaultValue;

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::UInt16 ) [[likely]]
        return c._data._ui16;

    return defaultValue;
}

int32_t SaveState::Container::Read ( int32_t defaultValue ) const noexcept
{
    if ( ( _type != eType::Array ) | ( _arrayData.size () <= _idx ) ) [[unlikely]]
        return defaultValue;

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::Int32 ) [[likely]]
        return c._data._i32;

    return defaultValue;
}

uint32_t SaveState::Container::Read ( uint32_t defaultValue ) const noexcept
{
    if ( ( _type != eType::Array ) | ( _arrayData.size () <= _idx ) ) [[unlikely]]
        return defaultValue;

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::UInt32 ) [[likely]]
        return c._data._ui32;

    return defaultValue;
}

int64_t SaveState::Container::Read ( int64_t defaultValue ) const noexcept
{
    if ( ( _type != eType::Array ) | ( _arrayData.size () <= _idx ) ) [[unlikely]]
        return defaultValue;

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::Int64 ) [[likely]]
        return c._data._i64;

    return defaultValue;
}

uint64_t SaveState::Container::Read ( uint64_t defaultValue ) const noexcept
{
    if ( ( _type != eType::Array ) | ( _arrayData.size () <= _idx ) ) [[unlikely]]
        return defaultValue;

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::UInt64 ) [[likely]]
        return c._data._ui64;

    return defaultValue;
}

float SaveState::Container::Read ( float defaultValue ) const noexcept
{
    if ( ( _type != eType::Array ) | ( _arrayData.size () <= _idx ) ) [[unlikely]]
        return defaultValue;

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::Float ) [[likely]]
        return c._data._float;

    return defaultValue;
}

double SaveState::Container::Read ( double defaultValue ) const noexcept
{
    if ( ( _type != eType::Array ) | ( _arrayData.size () <= _idx ) ) [[unlikely]]
        return defaultValue;

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::Double ) [[likely]]
        return c._data._double;

    return defaultValue;
}

bool SaveState::Container::Read ( bool defaultValue ) const noexcept
{
    if ( ( _type != eType::Array ) | ( _arrayData.size () <= _idx ) ) [[unlikely]]
        return defaultValue;

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::Bool ) [[likely]]
        return c._data._bool;

    return defaultValue;
}

std::string_view SaveState::Container::Read ( std::string_view const &defaultValue ) const noexcept
{
    if ( ( _type != eType::Array ) | ( _arrayData.size () <= _idx ) ) [[unlikely]]
        return defaultValue;

    if ( Container const &c = _arrayData[ _idx++ ]; c.GetType () == eType::String ) [[likely]]
        return c._string;

    return defaultValue;
}

int8_t SaveState::Container::Read ( std::string_view const &key, int8_t defaultValue ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
        return defaultValue;

    auto const findResult = _containerData.find ( std::string ( key ) );

    if ( findResult == _containerData.cend () || findResult->second._type != eType::Int8 ) [[unlikely]]
        return defaultValue;

    return findResult->second._data._i8;
}

uint8_t SaveState::Container::Read ( std::string_view const &key, uint8_t defaultValue ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
        return defaultValue;

    auto const findResult = _containerData.find ( std::string ( key ) );

    if ( findResult == _containerData.cend () || findResult->second._type != eType::UInt8 ) [[unlikely]]
        return defaultValue;

    return findResult->second._data._ui8;
}

int16_t SaveState::Container::Read ( std::string_view const &key, int16_t defaultValue ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
        return defaultValue;

    auto const findResult = _containerData.find ( std::string ( key ) );

    if ( findResult == _containerData.cend () || findResult->second._type != eType::Int16 ) [[unlikely]]
        return defaultValue;

    return findResult->second._data._i16;
}

uint16_t SaveState::Container::Read ( std::string_view const &key, uint16_t defaultValue ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
        return defaultValue;

    auto const findResult = _containerData.find ( std::string ( key ) );

    if ( findResult == _containerData.cend () || findResult->second._type != eType::UInt16 ) [[unlikely]]
        return defaultValue;

    return findResult->second._data._ui16;
}

int32_t SaveState::Container::Read ( std::string_view const &key, int32_t defaultValue ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
        return defaultValue;

    auto const findResult = _containerData.find ( std::string ( key ) );

    if ( findResult == _containerData.cend () || findResult->second._type != eType::Int32 ) [[unlikely]]
        return defaultValue;

    return findResult->second._data._i32;
}

uint32_t SaveState::Container::Read ( std::string_view const &key, uint32_t defaultValue ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
        return defaultValue;

    auto const findResult = _containerData.find ( std::string ( key ) );

    if ( findResult == _containerData.cend () || findResult->second._type != eType::UInt32 ) [[unlikely]]
        return defaultValue;

    return findResult->second._data._ui32;
}

int64_t SaveState::Container::Read ( std::string_view const &key, int64_t defaultValue ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
        return defaultValue;

    auto const findResult = _containerData.find ( std::string ( key ) );

    if ( findResult == _containerData.cend () || findResult->second._type != eType::Int64 ) [[unlikely]]
        return defaultValue;

    return findResult->second._data._i64;
}

uint64_t SaveState::Container::Read ( std::string_view const &key, uint64_t defaultValue ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
        return defaultValue;

    auto const findResult = _containerData.find ( std::string ( key ) );

    if ( findResult == _containerData.cend () || findResult->second._type != eType::UInt64 ) [[unlikely]]
        return defaultValue;

    return findResult->second._data._ui64;
}

float SaveState::Container::Read ( std::string_view const &key, float defaultValue ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
        return defaultValue;

    auto const findResult = _containerData.find ( std::string ( key ) );

    if ( findResult == _containerData.cend () || findResult->second._type != eType::Float ) [[unlikely]]
        return defaultValue;

    return findResult->second._data._float;
}

double SaveState::Container::Read ( std::string_view const &key, double defaultValue ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
        return defaultValue;

    auto const findResult = _containerData.find ( std::string ( key ) );

    if ( findResult == _containerData.cend () || findResult->second._type != eType::Double ) [[unlikely]]
        return defaultValue;

    return findResult->second._data._double;
}

bool SaveState::Container::Read ( std::string_view const &key, bool defaultValue ) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
        return defaultValue;

    auto const findResult = _containerData.find ( std::string ( key ) );

    if ( findResult == _containerData.cend () || findResult->second._type != eType::Bool ) [[unlikely]]
        return defaultValue;

    return findResult->second._data._bool;
}

std::string_view SaveState::Container::Read ( std::string_view const &key,
    std::string_view const &defaultValue
) const noexcept
{
    if ( _type != eType::Container ) [[unlikely]]
        return defaultValue;

    auto const findResult = _containerData.find ( std::string ( key ) );

    if ( findResult == _containerData.cend () || findResult->second._type != eType::String ) [[unlikely]]
        return defaultValue;

    return findResult->second._string;
}

SaveState::Container::Container ( eType type ) noexcept:
    _type ( type )
{
    // NOTHING
}

SaveState::Container::Container ( int8_t value ) noexcept:
    _data (
        Data
        {
            ._i8 = value
        }
    ),
    _type ( eType::Int8 )
{
    // NOTHING
}

SaveState::Container::Container ( uint8_t value ) noexcept:
    _data (
        Data
        {
            ._ui8 = value
        }
    ),
    _type ( eType::UInt8 )
{
    // NOTHING
}

SaveState::Container::Container ( int16_t value ) noexcept:
    _data (
        Data
        {
            ._i16 = value
        }
    ),
    _type ( eType::Int16 )
{
    // NOTHING
}

SaveState::Container::Container ( uint16_t value ) noexcept:
    _data (
        Data
        {
            ._ui16 = value
        }
    ),
    _type ( eType::UInt16 )
{
    // NOTHING
}

SaveState::Container::Container ( int32_t value ) noexcept:
    _data (
        Data
        {
            ._i32 = value
        }
    ),
    _type ( eType::Int32 )
{
    // NOTHING
}

SaveState::Container::Container ( uint32_t value ) noexcept:
    _data (
        Data
        {
            ._ui32 = value
        }
    ),
    _type ( eType::UInt32 )
{
    // NOTHING
}

SaveState::Container::Container ( int64_t value ) noexcept:
    _data (
        Data
        {
            ._i64 = value
        }
    ),
    _type ( eType::Int64 )
{
    // NOTHING
}

SaveState::Container::Container ( uint64_t value ) noexcept:
    _data (
        Data
        {
            ._ui64 = value
        }
    ),
    _type ( eType::UInt64 )
{
    // NOTHING
}

SaveState::Container::Container ( float value ) noexcept:
    _data (
        Data
        {
            ._float = value
        }
    ),
    _type ( eType::Float )
{
    // NOTHING
}

SaveState::Container::Container ( double value ) noexcept:
    _data (
        Data
        {
            ._double = value
        }
    ),
    _type ( eType::Double )
{
    // NOTHING
}

SaveState::Container::Container ( bool value ) noexcept:
    _data (
        Data
        {
            ._bool = value
        }
    ),
    _type ( eType::Bool )
{
    // NOTHING
}

SaveState::Container::Container ( std::string_view const &value ) noexcept:
    _string ( value ),
    _type ( eType::String )
{
    // NOTHING
}

//----------------------------------------------------------------------------------------------------------------------

bool SaveState::Load ( std::string_view const &file, bool silent ) noexcept
{
    AV_TRACE ( "Loading state" )

    std::ifstream stream ( ResolvePath ( file ), std::ios::binary | std::ios::ate | std::ios::in );

    if ( !stream.is_open () ) [[unlikely]]
    {
        if ( silent ) [[likely]]
            return true;

        android_vulkan::LogError ( "SaveState: Can't open file %s", file.data () );
        return false;
    }

    auto const size = stream.tellg ();
    stream.seekg ( 0, std::ios::beg );

    Binary content {};
    content.resize ( static_cast<size_t> ( size ) );
    uint8_t* dst = content.data ();
    stream.read ( reinterpret_cast<char*> ( dst ), size );

    uint8_t const* aData = dst;
    auto const& saveStatInfo = *reinterpret_cast<SaveStateInfo const*> ( aData );
    uint8_t const* bData = aData + saveStatInfo._bDataOffset;
    aData += sizeof ( SaveStateInfo ) + sizeof ( Container::eType );

    DecodeContainer ( _container, aData, bData );
    return true;
}

bool SaveState::Save ( std::string_view const &file ) noexcept
{
    AV_TRACE ( "Saving state" )

    Binary aData {};
    Binary bData {};

    constexpr size_t initialSize = 32U * 1024U * 1024U;
    aData.reserve ( initialSize );

    bData.reserve ( initialSize );
    aData.resize ( sizeof ( SaveStateInfo ) );

    EncodeContainer ( aData, bData, _container, true );

    auto& saveStateInfo = *reinterpret_cast<SaveStateInfo*> ( aData.data () );
    saveStateInfo._bDataOffset = static_cast<android_vulkan::UTF8Offset> ( aData.size () );

    std::filesystem::path const path = ResolvePath ( file );
    std::filesystem::create_directories ( path.parent_path () );

    std::ofstream stream ( path, std::ios::binary | std::ios::trunc | std::ios::out );

    if ( !stream.is_open () ) [[unlikely]]
    {
        android_vulkan::LogError ( "SaveState: Can't write file %s", path.c_str () );
        return false;
    }

    stream.write ( reinterpret_cast<char const*> ( &saveStateInfo ), static_cast<std::streamsize> ( aData.size () ) );
    stream.write ( reinterpret_cast<char const*> ( bData.data () ), static_cast<std::streamsize> ( bData.size () ) );
    return true;
}

SaveState::Container &SaveState::GetContainer () noexcept
{
    return _container;
}

SaveState::Container const &SaveState::GetContainer () const noexcept
{
    return _container;
}

void SaveState::EncodeArray ( Binary &aData, Binary &bData, Container const &container ) noexcept
{
    size_t const offset = EncodeType ( aData, aData.size (), Container::eType::Array );

    std::vector<Container> const &arrayData = container._arrayData;
    aData.resize ( offset + sizeof ( ArrayInfo ) );
    size_t const size = arrayData.size ();

    *reinterpret_cast<ArrayInfo*> ( aData.data () + offset ) = ArrayInfo
    {
        ._size = static_cast<uint64_t> ( size ),
        ._type = size == 0U ? Container::eType::Null : arrayData.front ().GetType ()
    };

    for ( Container const &value : arrayData )
    {
        switch ( value.GetType () )
        {
            case Container::eType::Container:
                EncodeContainer ( aData, bData, value, false );
            break;

            case Container::eType::Int8:
                EncodeInt8 ( aData, value, false );
            break;

            case Container::eType::UInt8:
                EncodeUInt8 ( aData, value, false );
            break;

            case Container::eType::Int16:
                EncodeInt16 ( aData, value, false );
            break;

            case Container::eType::UInt16:
                EncodeUInt16 ( aData, value, false );
            break;

            case Container::eType::Int32:
                EncodeInt32 ( aData, value, false );
            break;

            case Container::eType::UInt32:
                EncodeUInt32 ( aData, value, false );
            break;

            case Container::eType::Int64:
                EncodeInt64 ( aData, value, false );
            break;

            case Container::eType::UInt64:
                EncodeUInt64 ( aData, value, false );
            break;

            case Container::eType::String:
                EncodeString ( aData, bData, value, false );
            break;

            case Container::eType::Float:
                EncodeFloat ( aData, value, false );
            break;

            case Container::eType::Double:
                EncodeDouble ( aData, value, false );
            break;

            case Container::eType::Bool:
                EncodeBool ( aData, value, false );
            break;

            case Container::eType::Array:
                [[fallthrough]];

            case Container::eType::Null:
                [[fallthrough]];

            default:
                AV_ASSERT ( false )
            break;
        }
    }
}

void SaveState::EncodeContainer ( Binary &aData, Binary &bData, Container const &container, bool writeType ) noexcept
{
    size_t aOffset = aData.size ();

    if ( writeType )
        aOffset = EncodeType ( aData, aOffset, Container::eType::Container );

    std::unordered_map<std::string, Container> const &containerData = container._containerData;
    aData.resize ( aOffset + sizeof ( ContainerInfo ) );

    *reinterpret_cast<ContainerInfo*> ( aData.data () + aOffset ) = ContainerInfo
    {
        ._fields = static_cast<uint64_t> ( containerData.size () )
    };

    for ( auto const &[key, value] : containerData )
    {
        switch ( value.GetType () )
        {
            case Container::eType::Array:
                EncodeKey ( aData, bData, key );
                EncodeArray ( aData, bData, value );
            break;

            case Container::eType::Container:
                EncodeKey ( aData, bData, key );
                EncodeContainer ( aData, bData, value, true );
            break;

            case Container::eType::Int8:
                EncodeKey ( aData, bData, key );
                EncodeInt8 ( aData, value, true );
            break;

            case Container::eType::UInt8:
                EncodeKey ( aData, bData, key );
                EncodeUInt8 ( aData, value, true );
            break;

            case Container::eType::Int16:
                EncodeKey ( aData, bData, key );
                EncodeInt16 ( aData, value, true );
            break;

            case Container::eType::UInt16:
                EncodeKey ( aData, bData, key );
                EncodeUInt16 ( aData, value, true );
            break;

            case Container::eType::Int32:
                EncodeKey ( aData, bData, key );
                EncodeInt32 ( aData, value, true );
            break;

            case Container::eType::UInt32:
                EncodeKey ( aData, bData, key );
                EncodeUInt32 ( aData, value, true );
            break;

            case Container::eType::Int64:
                EncodeKey ( aData, bData, key );
                EncodeInt64 ( aData, value, true );
            break;

            case Container::eType::UInt64:
                EncodeKey ( aData, bData, key );
                EncodeUInt64 ( aData, value, true );
            break;

            case Container::eType::String:
                EncodeKey ( aData, bData, key );
                EncodeString ( aData, bData, value, true );
            break;

            case Container::eType::Float:
                EncodeKey ( aData, bData, key );
                EncodeFloat ( aData, value, true );
            break;

            case Container::eType::Double:
                EncodeKey ( aData, bData, key );
                EncodeDouble ( aData, value, true );
            break;

            case Container::eType::Bool:
                EncodeKey ( aData, bData, key );
                EncodeBool ( aData, value, true );
            break;

            case Container::eType::Null:
                [[fallthrough]];

            default:
                AV_ASSERT ( false )
            break;
        }
    }
}

void SaveState::EncodeInt8 ( Binary &aData, Container const &container, bool writeType ) noexcept
{
    size_t offset = aData.size ();

    if ( writeType )
        offset = EncodeType ( aData, offset, Container::eType::Int8 );

    aData.resize ( offset + sizeof ( Int8Info ) );

    *reinterpret_cast<Int8Info*> ( aData.data () + offset ) = Int8Info
    {
        ._data = container._data._i8
    };
}

void SaveState::EncodeUInt8 ( Binary &aData, Container const &container, bool writeType ) noexcept
{
    size_t offset = aData.size ();

    if ( writeType )
        offset = EncodeType ( aData, offset, Container::eType::UInt8 );

    aData.resize ( offset + sizeof ( UInt8Info ) );

    *reinterpret_cast<UInt8Info*> ( aData.data () + offset ) = UInt8Info
    {
        ._data = container._data._ui8
    };
}

void SaveState::EncodeInt16 ( Binary &aData, Container const &container, bool writeType ) noexcept
{
    size_t offset = aData.size ();

    if ( writeType )
        offset = EncodeType ( aData, offset, Container::eType::Int16 );

    aData.resize ( offset + sizeof ( Int16Info ) );

    *reinterpret_cast<Int16Info*> ( aData.data () + offset ) = Int16Info
    {
        ._data = container._data._i16
    };
}

void SaveState::EncodeUInt16 ( Binary &aData, Container const &container, bool writeType ) noexcept
{
    size_t offset = aData.size ();

    if ( writeType )
        offset = EncodeType ( aData, offset, Container::eType::UInt16 );

    aData.resize ( offset + sizeof ( UInt16Info ) );

    *reinterpret_cast<UInt16Info*> ( aData.data () + offset ) = UInt16Info
    {
        ._data = container._data._ui16
    };
}

void SaveState::EncodeInt32 ( Binary &aData, Container const &container, bool writeType ) noexcept
{
    size_t offset = aData.size ();

    if ( writeType )
        offset = EncodeType ( aData, offset, Container::eType::Int32 );

    aData.resize ( offset + sizeof ( Int32Info ) );

    *reinterpret_cast<Int32Info*> ( aData.data () + offset ) = Int32Info
    {
        ._data = container._data._i32
    };
}

void SaveState::EncodeUInt32 ( Binary &aData, Container const &container, bool writeType ) noexcept
{
    size_t offset = aData.size ();

    if ( writeType )
        offset = EncodeType ( aData, offset, Container::eType::UInt32 );

    aData.resize ( offset + sizeof ( UInt32Info ) );

    *reinterpret_cast<UInt32Info*> ( aData.data () + offset ) = UInt32Info
    {
        ._data = container._data._ui16
    };
}

void SaveState::EncodeInt64 ( Binary &aData, Container const &container, bool writeType ) noexcept
{
    size_t offset = aData.size ();

    if ( writeType )
        offset = EncodeType ( aData, offset, Container::eType::Int64 );

    aData.resize ( offset + sizeof ( Int64Info ) );

    *reinterpret_cast<Int64Info*> ( aData.data () + offset ) = Int64Info
    {
        ._data = container._data._i64
    };
}

void SaveState::EncodeUInt64 ( Binary &aData, Container const &container, bool writeType ) noexcept
{
    size_t offset = aData.size ();

    if ( writeType )
        offset = EncodeType ( aData, offset, Container::eType::UInt64 );

    aData.resize ( offset + sizeof ( UInt64Info ) );

    *reinterpret_cast<UInt64Info*> ( aData.data () + offset ) = UInt64Info
    {
        ._data = container._data._ui64
    };
}

void SaveState::EncodeString ( Binary &aData, Binary &bData, Container const &container, bool writeType ) noexcept
{
    size_t aOffset = aData.size ();

    if ( writeType )
        aOffset = EncodeType ( aData, aOffset, Container::eType::String );

    aData.resize ( aOffset + sizeof ( StringInfo ) );
    size_t const bOffset = bData.size ();

    *reinterpret_cast<StringInfo*> ( aData.data () + aOffset ) = StringInfo
    {
        ._data = static_cast<android_vulkan::UTF8Offset> ( bOffset )
    };

    std::string const &s = container._string;
    size_t const len = s.size () + 1U;
    bData.resize ( bOffset + len );
    std::memcpy ( bData.data () + bOffset, s.c_str (), len );
}

void SaveState::EncodeFloat ( Binary &aData, Container const &container, bool writeType ) noexcept
{
    size_t offset = aData.size ();

    if ( writeType )
        offset = EncodeType ( aData, offset, Container::eType::Float );

    aData.resize ( offset + sizeof ( FloatInfo ) );

    *reinterpret_cast<FloatInfo*> ( aData.data () + offset ) = FloatInfo
    {
        ._data = container._data._float
    };
}

void SaveState::EncodeDouble ( Binary &aData, Container const &container, bool writeType ) noexcept
{
    size_t offset = aData.size ();

    if ( writeType )
        offset = EncodeType ( aData, offset, Container::eType::Double );

    aData.resize ( offset + sizeof ( DoubleInfo ) );

    *reinterpret_cast<DoubleInfo*> ( aData.data () + offset ) = DoubleInfo
    {
        ._data = container._data._double
    };
}

void SaveState::EncodeBool ( Binary &aData, Container const &container, bool writeType ) noexcept
{
    size_t offset = aData.size ();

    if ( writeType )
        offset = EncodeType ( aData, offset, Container::eType::Bool );

    aData.resize ( offset + sizeof ( BoolInfo ) );

    *reinterpret_cast<BoolInfo*> ( aData.data () + offset ) = BoolInfo
    {
        ._data = container._data._bool ? android_vulkan::AV_TRUE : android_vulkan::AV_FALSE
    };
}

size_t SaveState::EncodeType ( Binary& aData, size_t offset, Container::eType type ) noexcept
{
    aData.resize ( offset + sizeof ( Container::eType ) );
    *reinterpret_cast<Container::eType*> ( aData.data () + offset ) = type;
    return offset + sizeof ( Container::eType );
}

void SaveState::EncodeKey ( Binary &aData, Binary &bData, std::string const &key ) noexcept
{
    size_t aOffset = aData.size ();
    aData.resize ( aOffset + sizeof ( StringInfo ) );
    size_t const bOffset = bData.size ();

    *reinterpret_cast<StringInfo*> ( aData.data () + aOffset ) = StringInfo
    {
        ._data = static_cast<android_vulkan::UTF8Offset> ( bOffset )
    };

    size_t const len = key.size () + 1U;
    bData.resize ( bOffset + len );
    std::memcpy ( bData.data () + bOffset, key.c_str (), len );
}

void SaveState::DecodeArray ( Container &root, uint8_t const* &aData, uint8_t const* bData ) noexcept
{
    auto const &info = *reinterpret_cast<ArrayInfo const*> ( aData );
    aData += sizeof ( ArrayInfo );
    uint64_t const count = info._size;

    switch ( info._type )
    {
        case Container::eType::Container:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                DecodeContainer ( root.WriteContainer (), aData, bData );
            }
        break;

        case Container::eType::Int8:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                root.Write ( DecodeInt8 ( aData ) );
            }
        break;

        case Container::eType::UInt8:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                root.Write ( DecodeUInt8 ( aData ) );
            }
        break;

        case Container::eType::Int16:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                root.Write ( DecodeInt16 ( aData ) );
            }
        break;

        case Container::eType::UInt16:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                root.Write ( DecodeUInt16 ( aData ) );
            }
        break;

        case Container::eType::Int32:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                root.Write ( DecodeInt32 ( aData ) );
            }
        break;

        case Container::eType::UInt32:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                root.Write ( DecodeUInt32 ( aData ) );
            }
        break;

        case Container::eType::Int64:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                root.Write ( DecodeInt64 ( aData ) );
            }
        break;

        case Container::eType::UInt64:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                root.Write ( DecodeUInt64 ( aData ) );
            }
        break;

        case Container::eType::String:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                root.Write ( DecodeString ( aData, bData ) );
            }
        break;

        case Container::eType::Float:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                root.Write ( DecodeFloat ( aData ) );
            }
        break;

        case Container::eType::Double:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                root.Write ( DecodeDouble ( aData ) );
            }
        break;

        case Container::eType::Bool:
            for ( uint64_t i = 0U; i < count; ++i )
            {
                root.Write ( DecodeBool ( aData ) );
            }
        break;

        case Container::eType::Array:
            [[fallthrough]];

        case Container::eType::Null:
            [[fallthrough]];

        default:
            AV_ASSERT ( false )
        break;
    }
}

void SaveState::DecodeContainer ( Container &root, uint8_t const* &aData, uint8_t const* bData ) noexcept
{
    auto const &info = *reinterpret_cast<ContainerInfo const*> ( aData );
    aData += sizeof ( ContainerInfo );
    uint64_t const count = info._fields;

    for ( uint64_t i = 0U; i < count; ++i )
    {
        std::string_view const key = DecodeKey ( aData, bData );

        switch ( DecodeType ( aData ) )
        {
            case Container::eType::Array:
                DecodeArray ( root.WriteArray ( key ), aData, bData );
            break;

            case Container::eType::Container:
                DecodeContainer ( root.WriteContainer ( key ), aData, bData );
            break;

            case Container::eType::Int8:
                root.Write ( key, DecodeInt8 ( aData ) );
            break;

            case Container::eType::UInt8:
                root.Write ( key, DecodeUInt8 ( aData ) );
            break;

            case Container::eType::Int16:
                root.Write ( key, DecodeInt16 ( aData ) );
            break;

            case Container::eType::UInt16:
                root.Write ( key, DecodeUInt16 ( aData ) );
            break;

            case Container::eType::Int32:
                root.Write ( key, DecodeInt32 ( aData ) );
            break;

            case Container::eType::UInt32:
                root.Write ( key, DecodeUInt32 ( aData ) );
            break;

            case Container::eType::Int64:
                root.Write ( key, DecodeInt64 ( aData ) );
            break;

            case Container::eType::UInt64:
                root.Write ( key, DecodeUInt64 ( aData ) );
            break;

            case Container::eType::String:
                root.Write ( key, DecodeString ( aData, bData ) );
            break;

            case Container::eType::Float:
                root.Write ( key, DecodeFloat ( aData ) );
            break;

            case Container::eType::Double:
                root.Write ( key, DecodeDouble ( aData ) );
            break;

            case Container::eType::Bool:
                root.Write ( key, DecodeBool ( aData ) );
            break;

            case Container::eType::Null:
                [[fallthrough]];

            default:
                AV_ASSERT ( false )
            break;
        }
    }
}

int8_t SaveState::DecodeInt8 ( uint8_t const* &aData ) noexcept
{
    int8_t const result = reinterpret_cast<Int8Info const*> ( aData )->_data;
    aData += sizeof ( Int8Info );
    return result;
}

uint8_t SaveState::DecodeUInt8 ( uint8_t const* &aData ) noexcept
{
    uint8_t const result = reinterpret_cast<UInt8Info const*> ( aData )->_data;
    aData += sizeof ( UInt8Info );
    return result;
}

int16_t SaveState::DecodeInt16 ( uint8_t const* &aData ) noexcept
{
    int16_t const result = reinterpret_cast<Int16Info const*> ( aData )->_data;
    aData += sizeof ( Int16Info );
    return result;
}

uint16_t SaveState::DecodeUInt16 ( uint8_t const* &aData ) noexcept
{
    uint16_t const result = reinterpret_cast<UInt16Info const*> ( aData )->_data;
    aData += sizeof ( UInt16Info );
    return result;
}

int32_t SaveState::DecodeInt32 ( uint8_t const* &aData ) noexcept
{
    int32_t const result = reinterpret_cast<Int32Info const*> ( aData )->_data;
    aData += sizeof ( Int32Info );
    return result;
}

uint32_t SaveState::DecodeUInt32 ( uint8_t const* &aData ) noexcept
{
    uint32_t const result = reinterpret_cast<UInt32Info const*> ( aData )->_data;
    aData += sizeof ( UInt32Info );
    return result;
}

int64_t SaveState::DecodeInt64 ( uint8_t const* &aData ) noexcept
{
    int64_t const result = reinterpret_cast<Int64Info const*> ( aData )->_data;
    aData += sizeof ( Int64Info );
    return result;
}

uint64_t SaveState::DecodeUInt64 ( uint8_t const* &aData ) noexcept
{
    uint64_t const result = reinterpret_cast<UInt64Info const*> ( aData )->_data;
    aData += sizeof ( UInt64Info );
    return result;
}

std::string_view SaveState::DecodeString ( uint8_t const* &aData, uint8_t const* bData ) noexcept
{
    std::string_view result =
        reinterpret_cast<char const*> ( bData + reinterpret_cast<StringInfo const*> ( aData )->_data );

    aData += sizeof ( StringInfo );
    return result;
}

float SaveState::DecodeFloat ( uint8_t const* &aData ) noexcept
{
    float const result = reinterpret_cast<FloatInfo const*> ( aData )->_data;
    aData += sizeof ( FloatInfo );
    return result;
}

double SaveState::DecodeDouble ( uint8_t const* &aData ) noexcept
{
    double const result = reinterpret_cast<DoubleInfo const*> ( aData )->_data;
    aData += sizeof ( DoubleInfo );
    return result;
}

bool SaveState::DecodeBool ( uint8_t const* &aData ) noexcept
{
    bool const result = reinterpret_cast<BoolInfo const*> ( aData )->_data == android_vulkan::AV_TRUE;
    aData += sizeof ( BoolInfo );
    return result;
}

SaveState::Container::eType SaveState::DecodeType ( uint8_t const* &aData ) noexcept
{
    auto const result = *reinterpret_cast<Container::eType const*> ( aData );
    aData += sizeof ( Container::eType );
    return result;
}

std::string_view SaveState::DecodeKey ( uint8_t const* &aData, uint8_t const* bData ) noexcept
{
    std::string_view result =
        reinterpret_cast<char const*> ( bData + reinterpret_cast<StringInfo const*> ( aData )->_data );

    aData += sizeof ( StringInfo );
    return result;
}

std::filesystem::path SaveState::ResolvePath ( std::string_view const &file ) noexcept
{
    AV_TRACE ( "Resolving path" )
    char const* f = file.data ();
    DWORD const len = ExpandEnvironmentStringsA ( f, nullptr, 0U );
    std::string path {};
    path.resize ( static_cast<size_t> ( len ) );
    ExpandEnvironmentStringsA ( f, path.data (), len );
    return std::filesystem::path ( std::move ( path ) );
}

} // namespace editor
