# Logger

```lua
require "av://engine/av_logger.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`AVLogD ( format, ... )`](#function-avlogd)
- [`AVLogE ( format, ... )`](#function-avloge)
- [`AVLogI ( format, ... )`](#function-avlogi)
- [`AVLogW ( format, ... )`](#function-avlogw)

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

## <a id="function-avlogd">`AVLogD ( format, ... )`</a>

Function prints the message to log with _debug_ tag. The function supports formating string [technique](https://en.cppreference.com/w/cpp/io/c/fprintf).

**Parameters:**

- `format` _[required, string]_: message or [formating string](https://en.cppreference.com/w/cpp/io/c/fprintf)
- `...` _[optional, any]_: optional parameters controlled by `format` string

**Return values:**

- none

**Example:**

```lua
require "av://engine/av_logger.lua"


local s = "Hello world"

AVLogD ( "Message from log." )
AVLogD ( "The values: %s, %f", s, 77.7 )
```

## <a id="function-avloge">`AVLogE ( format, ... )`</a>

Function prints the message to log with _error_ tag. The function supports formating string [technique](https://en.cppreference.com/w/cpp/io/c/fprintf).

**Parameters:**

- `format` _[required, string]_: message or [formating string](https://en.cppreference.com/w/cpp/io/c/fprintf)
- `...` _[optional, any]_: optional parameters controlled by `format` string

**Return values:**

- none

**Example:**

```lua
require "av://engine/av_logger.lua"


local s = "Hello world"

AVLogE ( "Message from log." )
AVLogE ( "The values: %s, %f", s, 77.7 )
```

## <a id="function-avlogi">`AVLogI ( format, ... )`</a>

Function prints the message to log with _info_ tag. The function supports formating string [technique](https://en.cppreference.com/w/cpp/io/c/fprintf).

**Parameters:**

- `format` _[required, string]_: message or [formating string](https://en.cppreference.com/w/cpp/io/c/fprintf)
- `...` _[optional, any]_: optional parameters controlled by `format` string

**Return values:**

- none

**Example:**

```lua
require "av://engine/av_logger.lua"


local s = "Hello world"

AVLogI ( "Message from log." )
AVLogI ( "The values: %s, %f", s, 77.7 )
```

## <a id="function-avlogw">`AVLogW ( format, ... )`</a>

Function prints the message to log with _warning_ tag. The function supports formating string [technique](https://en.cppreference.com/w/cpp/io/c/fprintf).

**Parameters:**

- `format` _[required, string]_: message or [formating string](https://en.cppreference.com/w/cpp/io/c/fprintf)
- `...` _[optional, any]_: optional parameters controlled by `format` string

**Return values:**

- none

**Example:**

```lua
require "av://engine/av_logger.lua"


local s = "Hello world"

AVLogW ( "Message from log." )
AVLogW ( "The values: %s, %f", s, 77.7 )
```
