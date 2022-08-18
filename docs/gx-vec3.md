# _GXVec3_

```lua
require "av://engine/gx_vec3.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`Clone ( other )`](#method-clone)
- [`CrossProduct ( a, b )`](#method-cross-product)
- [`Distance ( other )`](#method-distance)
- [`DotProduct ( other )`](#method-dot-product)
- [`GetX ()`](#method-get-x)
- [`GetY ()`](#method-get-y)
- [`GetZ ()`](#method-get-z)
- [`Init ( x, y, z )`](#method-init)
- [`Length ()`](#method-length)
- [`MultiplyScalar ( a, scale )`](#method-multiply-scalar)
- [`MultiplyVector ( a, b )`](#method-multiply-vector)
- [`Normalize ()`](#method-normalize)
- [`Reverse ()`](#method-reverse)
- [`SetX ( x )`](#method-set-x)
- [`SetY ( y )`](#method-set-y)
- [`SetZ ( z )`](#method-set-z)
- [`SquaredDistance ( other )`](#method-squared-distance)
- [`SquaredLength ()`](#method-squared-length)
- [`Subtract ( a, b )`](#method-subtract)
- [`Sum ( a, b )`](#method-sum)
- [`SumScaled ( a, bScale, b )`](#method-sum-scaled)

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

## <a id="method-clone">`Clone ( other )`</a>

Method clones content of vector to the current vector.

**Parameters:**

- `other` [_required, readonly, [GXVec3](./gx-vec3.md)_]: vector to clone

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local vector = GXVec3 ()
vector:Init ( 777.0, 3.33, 1.0 )

local vectorCopy = GXVec3 ()
vectorCopy:Clone ( vector )
```

## <a id="method-cross-product">`CrossProduct ( a, b )`</a>

Method saves the result of [cross product](https://en.wikipedia.org/wiki/Cross_product) of two vectors `a` and `b` to the current vector. The cross product order is: `a` x `b`.

**Parameters:**

- `a` [_required, readonly, [GXVec3](./gx-vec3.md)_]: left vector
- `b` [_required, readonly, [GXVec3](./gx-vec3.md)_]: right vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v0 = GXVec3 ()
v0:Init ( 777.0, 3.33, 1.0 )

local v1 = GXVec3 ()
v1:Init ( 0.0, -33.3, 77.7 )

local cross = GXVec3 ()
cross:CrossProduct ( v0, v1 )
```

## <a id="method-distance">`Distance ( other )`</a>

Method returns distance between current point and `other` point.

**Parameters:**

- `other` [_required, readonly, [GXVec3](./gx-vec3.md)_]: coordinate of another point

**Return values:**

- `#1` [_required, number_]: distance between two points

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v0 = GXVec3 ()
v0:Init ( 777.0, 3.33, 1.0 )

local v1 = GXVec3 ()
v1:Init ( 0.0, -33.3, 77.7 )

local d = v0:Distance ( v1 )
```

## <a id="method-dot-product">`DotProduct ( other )`</a>

Method returns the result of [dot product](https://en.wikipedia.org/wiki/Dot_product) operation between current vector and `other` vector.

**Parameters:**

- `other` [_required, readonly, [GXVec3](./gx-vec3.md)_]: another vector

**Return values:**

- `#1` [_required, number_]: the result of [dot product](https://en.wikipedia.org/wiki/Dot_product) operation

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v0 = GXVec3 ()
v0:Init ( 777.0, 3.33, 1.0 )

local v1 = GXVec3 ()
v1:Init ( 0.0, -33.3, 77.7 )

local dot = v0:DotProduct ( v1 )
```

## <a id="method-get-x">`GetX ()`</a>

Method returns first component of the current vector.

**Parameters:**

- none

**Return values:**

- `#1` [_required, number_]: first component of the current vector

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )

local x = v:GetX ()
```

## <a id="method-get-y">`GetY ()`</a>

Method returns second component of the current vector.

**Parameters:**

- none

**Return values:**

- `#1` [_required, number_]: second component of the current vector

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )

local y = v:GetY ()
```

## <a id="method-get-z">`GetZ ()`</a>

Method returns third component of the current vector.

**Parameters:**

- none

**Return values:**

- `#1` [_required, number_]: third component of the current vector

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )

local z = v:GetZ ()
```

## <a id="method-init">`Init ( x, y, z )`</a>

Method initializes the vector with `x`, `y` and `z` values.

**Parameters:**

- `x` [_required, readonly, number_]: first component
- `y` [_required, readonly, number_]: second component
- `z` [_required, readonly, number_]: third component

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )
```

## <a id="method-length">`Length ()`</a>

Method returns vector length.

**Parameters:**

- none

**Return values:**

- `#1` [_required, number_]: length of the vector

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )
local l = v:Length ()
```

## <a id="method-multiply-scalar">`MultiplyScalar ( a, scale )`</a>

Method multiplies each component of vector `a` by `scale` value. The result is written to current vector.

**Note:** It's perfectly fine to pass current vector as `a` parameter.

**Parameters:**

- `a` [_required, readonly, [GXVec3](./gx-vec3.md)_]: vector
- `scale` [_required, readonly, number_]: scalar

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )

local res = GXVec3 ()
res:MultiplyScalar ( v, -7.77 )
```

## <a id="method-multiply-vector">`MultiplyVector ( a, b )`</a>

Method performs per component vector multiplication. The result is written to current vector.

**Note:** It's perfectly fine to pass current vector as `a`, `b` parameters.

**Parameters:**

- `a` [_required, readonly, [GXVec3](./gx-vec3.md)_]: first vector
- `b` [_required, readonly, [GXVec3](./gx-vec3.md)_]: second vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v0 = GXVec3 ()
v0:Init ( 777.0, 3.33, 1.0 )

local v1 = GXVec3 ()
v1:Init ( -3.0, 1.0, -77.7 )

local res = GXVec3 ()
res:MultiplyScalar ( v0, v1 )
```

## <a id="method-normalize">`Normalize ()`</a>

Method normalizes the vector.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )
v:Normalize ()
```

## <a id="method-reverse">`Reverse ()`</a>

Method makes the vector in oposite direction to current vector. The length of the result vector equals the lenght of original vector.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )
v:Reverse ()
```

## <a id="method-set-x">`SetX ( x )`</a>

Method sets first component of the current vector.

**Parameters:**

- `x` [_required, readonly, number_]: first component

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )

v:SetX ( 0.0 )
```

## <a id="method-set-y">`SetY ( y )`</a>

Method sets second component of the current vector.

**Parameters:**

- `y` [_required, readonly, number_]: second component

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )

v:SetY ( 0.0 )
```

## <a id="method-set-z">`SetZ ( z )`</a>

Method sets third component of the current vector.

**Parameters:**

- `z` [_required, readonly, number_]: third component

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )

v:SetZ ( 0.0 )
```

## <a id="method-squared-distance">`SquaredDistance ( other )`</a>

Method returns squared distance between current point and `other` point. This method is faster that normal [GXVec3:Distance](#method-distance). Method could be useful in fast comparisons.

**Parameters:**

- `other` [_required, readonly, [GXVec3](./gx-vec3.md)_]: coordinate of another point

**Return values:**

- `#1` [_required, number_]: squared distance between two points

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v0 = GXVec3 ()
v0:Init ( 777.0, 3.33, 1.0 )

local v1 = GXVec3 ()
v1:Init ( 0.0, -33.3, 77.7 )

local sd = v0:SquaredDistance ( v1 )
```

## <a id="method-squared-length">`SquaredLength ()`</a>

Method returns squared length of the vector. This method is faster that normal [GXVec3:Length](#method-length). Method could be useful in fast comparisons.

**Parameters:**

- none

**Return values:**

- `#1` [_required, number_]: squared length of the vector

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v = GXVec3 ()
v:Init ( 777.0, 3.33, 1.0 )
local sl = v:SquaredLength ()
```

## <a id="method-subtract">`Subtract ( a, b )`</a>

Method performs subtract operation: `a` - `b`. The result is written to current vector.

**Note:** It's perfectly fine to set `a`, `b` vectors as current vector.

**Parameters:**

- `a` [_required, *readonly, [GXVec3](./gx-vec3.md)_]: first vector
- `b` [_required, *readonly, [GXVec3](./gx-vec3.md)_]: second vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v0 = GXVec3 ()
v0:Init ( 777.0, 3.33, 1.0 )

local v1 = GXVec3 ()
v1:Init ( 0.0, -33.3, 77.7 )

local s = GXVec3 ()
s:Subtract ( v0, v1 )
```

## <a id="method-sum">`Sum ( a, b )`</a>

Method sums `a`, `b` and writes the result in current vector.

**Note:** It's perfectly fine to set `a`, `b` vectors as current vector.

**Parameters:**

- `a` [_required, *readonly, [GXVec3](./gx-vec3.md)_]: first vector
- `b` [_required, *readonly, [GXVec3](./gx-vec3.md)_]: second vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v0 = GXVec3 ()
v0:Init ( 777.0, 3.33, 1.0 )

local v1 = GXVec3 ()
v1:Init ( 0.0, -33.3, 77.7 )

local sum = GXVec3 ()
sum:Sum ( v0, v1 )
```

## <a id="method-sum-scaled">`SumScaled ( a, bScale, b )`</a>

Method performs the operation: `a` + `bScale` x `b`. The result is written to current vector.

**Note:** It's perfectly fine to set `a`, `b` vectors as current vector.

**Parameters:**

- `a` [_required, *readonly, [GXVec3](./gx-vec3.md)_]: first vector
- `bScale` [_required, readonly, number_]: scale factor for second vector
- `b` [_required, *readonly, [GXVec3](./gx-vec3.md)_]: second vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_vec3.lua"


local v0 = GXVec3 ()
v0:Init ( 777.0, 3.33, 1.0 )

local v1 = GXVec3 ()
v1:Init ( 0.0, -33.3, 77.7 )

local s = GXVec3 ()
s:SumScaled ( v0, 3.14, v1 )
```
