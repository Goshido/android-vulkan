# _GXVec3_

```lua
require "av://engine/gx_vec3.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`Init ( x, y, z )`](#method-init)

## <a id="brief">Brief</a>

The class represents three component vector of `float32_t` values. The vector itself could be considered as row vector or column vector. It's important for multiplication operations with matrices. Because of that every multiplication operation contains semantic how to perform multiplication. For example:

```lua
require "av://engine/gx_mat3.lua"


local v = GXVec3 ()
v:Init ( 1.0, 777.0, 33.3 )

local m = GXMat3 ()
m:SetX ( 0.0, 3.33, 0.0 )
m:SetY ( 7.77, 0.0, 0.0 )
m:SetZ ( 0.0, 0.0, -1.0 )

-- Performing vRow = v x m.
local vRow = GXVec3 ()
m:MultiplyVectorMatrix ( vRow, v );

-- Performing vColumn = m x v.
local vColumn = GXVec3 ()
m:MultiplyMatrixVector ( vColumn, v );
```

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

## <a id="constructor">`Constructor`</a>

Constructor creates new object with undefined initial values.

**Parameters:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
```

## <a id="method-init">`Init ( x, y, z )`</a>

Method initializes the vector with `x`, `y` and `z` values.

**Parameters:**

- `x` [_required, number_]: first component
- `y` [_required, number_]: second component
- `z` [_required, number_]: third component

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )
```
