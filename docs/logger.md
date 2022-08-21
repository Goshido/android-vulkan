# Logger

```lua
require "av://engine/logger.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`LogD ( format, ... )`](#function-logd)
- [`LogE ( format, ... )`](#function-loge)
- [`LogI ( format, ... )`](#function-logi)
- [`LogW ( format, ... )`](#function-logw)

## <a id="brief">Brief</a>

The module provides log utilities for _Lua_ scripts.

## <a id="metamethods">Metamethods</a>

Metamethod | Used
--- | ---
`__add` | ❌
`__band` | ❌
`__bnot` | ❌
`__bor` | ❌
`__bxor` | ❌
`__call` | ❌
`__close` | ❌
`__concat` | ❌
`__div` | ❌
`__eq` | ❌
`__gc` | ❌
`__idiv` | ❌
`__index` | ❌
`__le` | ❌
`__len` | ❌
`__lt` | ❌
`__mod` | ❌
`__mode` | ❌
`__mul` | ❌
`__name` | ❌
`__newindex` | ❌
`__pow` | ❌
`__shl` | ❌
`__shr` | ❌
`__sub` | ❌
`__tostring` | ❌
`__unm` | ❌

## <a id="function-logd">`LogD ( format, ... )`</a>

Function prints the message to log with _debug_ tag. The function supports formatting string [technique](https://en.cppreference.com/w/cpp/io/c/fprintf).

**Parameters:**

- `format` [_required, readonly, string_]: message or [formatting string](https://en.cppreference.com/w/cpp/io/c/fprintf)
- `...` [_optional, readonly, any_]: optional parameters controlled by `format` string

**Return values:**

- none

**Example:**

```lua
require "av://engine/logger.lua"


local s = "Hello world"

LogD ( "Message from log." )
LogD ( "The values: %s, %f", s, 77.7 )
```

## <a id="function-loge">`LogE ( format, ... )`</a>

Function prints the message to log with _error_ tag. The function supports formatting string [technique](https://en.cppreference.com/w/cpp/io/c/fprintf).

**Parameters:**

- `format` [_required, readonly, string_]: message or [formatting string](https://en.cppreference.com/w/cpp/io/c/fprintf)
- `...` [_optional, readonly, any_]: optional parameters controlled by `format` string

**Return values:**

- none

**Example:**

```lua
require "av://engine/logger.lua"


local s = "Hello world"

LogE ( "Message from log." )
LogE ( "The values: %s, %f", s, 77.7 )
```

## <a id="function-logi">`LogI ( format, ... )`</a>

Function prints the message to log with _info_ tag. The function supports formatting string [technique](https://en.cppreference.com/w/cpp/io/c/fprintf).

**Parameters:**

- `format` [_required, readonly, string_]: message or [formatting string](https://en.cppreference.com/w/cpp/io/c/fprintf)
- `...` [_optional, readonly, any_]: optional parameters controlled by `format` string

**Return values:**

- none

**Example:**

```lua
require "av://engine/logger.lua"


local s = "Hello world"

LogI ( "Message from log." )
LogI ( "The values: %s, %f", s, 77.7 )
```

## <a id="function-logw">`LogW ( format, ... )`</a>

Function prints the message to log with _warning_ tag. The function supports formatting string [technique](https://en.cppreference.com/w/cpp/io/c/fprintf).

**Parameters:**

- `format` [_required, readonly, string_]: message or [formatting string](https://en.cppreference.com/w/cpp/io/c/fprintf)
- `...` [_optional, readonly, any_]: optional parameters controlled by `format` string

**Return values:**

- none

**Example:**

```lua
require "av://engine/logger.lua"


local s = "Hello world"

LogW ( "Message from log." )
LogW ( "The values: %s, %f", s, 77.7 )
```
