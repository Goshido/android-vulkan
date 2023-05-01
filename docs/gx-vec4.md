# _GXVec4_

```lua
require "av://engine/gx_vec4.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`Init ( x, y, z, w )`](#method-init)

## <a id="brief">Brief</a>

The class represents four component vector of `float32_t` values. The vector itself could be considered as row vector or column vector. It's important for multiplication operations with matrices. Because of that every multiplication operation contains semantic how to perform multiplication. For example:

```lua
require "av://engine/gx_mat4.lua"


local v = GXVec4 ()
v:Init ( -1.0, 777.0, 33.3, 1.0 )

local m = GXMat4 ()
m:RorationX ( math.rad ( 77.7 ) )

-- Performing vRow = v x m.
local vRow = GXVec4 ()
m:MultiplyVectorMatrix ( vRow, v )

-- Performing vColumn = m x v.
local vColumn = GXVec4 ()
m:MultiplyMatrixVector ( vColumn, v )
```

[↬ table of content ⇧](#table-of-content)

## <a id="metamethods">Metamethods</a>

Metamethod | Used
--- | ---
`__add` | ❌
`__band` | ❌
`__bnot` | ❌
`__bor` | ❌
`__bxor` | ❌
`__call` | ✔️
`__close` | ❌
`__concat` | ✔️
`__div` | ❌
`__eq` | ❌
`__gc` | ✔️
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
`__tostring` | ✔️
`__unm` | ❌

[↬ table of content ⇧](#table-of-content)

## <a id="constructor">`Constructor`</a>

Constructor creates new object with undefined initial values.

**Parameters:**

- none

**Example:**

```lua
require "av://engine/gx_vec4.lua"


local v = GXVec4 ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-init">`Init ( x, y, z, w )`</a>

Method initializes the vector with `x`, `y`, `z` and `w` values.

**Parameters:**

- `x` [_required, readonly, number_]: first component
- `y` [_required, readonly, number_]: second component
- `z` [_required, readonly, number_]: third component
- `w` [_required, readonly, number_]: fourth component

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec4.lua"


local v = GXVec4 ()
v:Init ( 777.0, 3.33, 0.0, 1.0 )
```

[↬ table of content ⇧](#table-of-content)
