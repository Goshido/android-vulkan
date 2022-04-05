# _GXMat3_

```lua
require "av://engine/gx_mat3.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`GetX ( x )`](#method-get-x)
- [`GetY ( y )`](#method-get-y)
- [`GetZ ( z )`](#method-get-z)
- [`Identity ()`](#method-identity)
- [`Multiply ( a, b )`](#method-multiply)
- [`MultiplyMatrixVector ( out, v )`](#method-multiply-matrix-vector)
- [`MultiplyVectorMatrix ( out, v )`](#method-multiply-vector-matrix)
- [`SetX ( x )`](#method-set-x)
- [`SetY ( y )`](#method-set-y)
- [`SetZ ( z )`](#method-set-z)

## <a id="brief">Brief</a>

The class represents 3x3 matrix of `float32_t` values. The matrix layout in memory looks like this:

<img src="./images/gx-mat3-layout.png"/>

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
require "av://engine/gx_mat3.lua"


local m = GXMat3 ()
```

## <a id="method-get-x">`GetX ( x )`</a>

Method writes the [_X_ row](#brief) of current matrix to the supplied `x` vector of the [_GXVec3_](./gx-vec3.md) type.

**Parameters:**

- `x` [_required, writeonly, [_GXVec3_](./gx-vec3.md)_]: target vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat3.lua"


local m = GXMat3 ()
m:Identity ()

local v = GXVec3 ()
m:GetX ( v )
```

## <a id="method-get-y">`GetY ( y )`</a>

Method writes the [_Y_ row](#brief) of current matrix to the supplied `y` vector of the [_GXVec3_](./gx-vec3.md) type.

**Parameters:**

- `y` [_required, writeonly, [_GXVec3_](./gx-vec3.md)_]: target vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat3.lua"


local m = GXMat3 ()
m:Identity ()

local v = GXVec3 ()
m:GetY ( v )
```

## <a id="method-get-z">`GetZ ( z )`</a>

Method writes the [_Z_ row](#brief) of current matrix to the supplied `z` vector of the [_GXVec3_](./gx-vec3.md) type.

**Parameters:**

- `z` [_required, writeonly, [_GXVec3_](./gx-vec3.md)_]: target vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat4.lua"


local m = GXMat3 ()
m:Identity ()

local v = GXVec3 ()
m:GetZ ( v )
```

## <a id="method-identity">`Identity ()`</a>

Method initializes the matrix as [identity matrix](https://en.wikipedia.org/wiki/Identity_matrix).

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat3.lua"


local m = GXMat3 ()
m:Identity ()
```

## <a id="method-inverse">`Inverse ( sourceMatrix )`</a>

Method initializes the matrix as [inverse matrix](https://en.wikipedia.org/wiki/Invertible_matrix) to `sourceMatrix` matrix. `sourceMatrix` matrix is not changed during the operation.

**Parameters:**

- `sourceMatrix` [_required, readonly, [GXMat3](./gx-mat3.md)_]: matrix to inverse

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat3.lua"


local x = GXVec3 ()
x:Init ( 77.7, 1.0, 7.77 )

local y = GXVec3 ()
y:Init ( -0.333, 77.7, 1.11 )

local z = GXVec3 ()
z:Init ( 3.33, -1.333, 77.7 )

local m = GXMat3 ()
m:SetX ( x )
m:SetY ( y )
m:SetZ ( z )

local i = GXMat3 ()
i:Inverse ( m )
```

## <a id="method-multiply">`Multiply ( a, b )`</a>

Method initializes the matrix as a product of the matrix multiplication `a` x `b` according classical [matrix multiplication rules](https://en.wikipedia.org/wiki/Matrix_multiplication#Definition).

**Parameters:**

- `a` [_required, readonly, [GXMat3](./gx-mat4.md)_]: left matrix
- `b` [_required, readonly, [GXMat3](./gx-mat4.md)_]: right matrix

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat3.lua"


local x0 = GXVec3 ()
x0:Init ( 1.0, 0.0, 0.0 )

local y0 = GXVec3 ()
y0:Init ( 0.0, 0.499999672, 0.866025626 )

local z0 = GXVec3 ()
z0:Init ( 0.0, -0.866025626, 0.499999672 )

local a = GXMat3 ()
a:SetX ( x0 )
a:SetY ( y0 )
a:SetZ ( z0 )

local x1 = GXVec3 ()
x1:Init ( 0.835807204, 0.549023032, 0.0 )

local y1 = GXVec3 ()
y1:Init ( -0.549023032, 0.835807204, 0.0 )

local z1 = GXVec3 ()
z1:Init ( 0.0, 0.0, 1.0 )

local b = GXMat3 ()
b:SetX ( x1 )
b:SetY ( y1 )
b:SetZ ( z1 )

local m = GXMat3 ()
m:Multiply ( a, b )
```

## <a id="method-multiply-matrix-vector">`MultiplyMatrixVector ( out, v )`</a>

Method performs matrix-vector multiplication of current matrix `self` and `v` [column-vector](https://en.wikipedia.org/wiki/Row_and_column_vectors): `self` x `v`. The result is written to the `out` [column-vector](https://en.wikipedia.org/wiki/Row_and_column_vectors).

**Parameters:**

- `out` [_required, writeonly, [GXVec3](./gx-vec3.md)_]: result [column-vector](https://en.wikipedia.org/wiki/Row_and_column_vectors)
- `v` [_required, readonly, [GXVec3](./gx-vec3.md)_]: source [column-vector](https://en.wikipedia.org/wiki/Row_and_column_vectors)

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat3.lua"


local x = GXVec3 ()
x:Init ( 1.0, 0.0, 0.0 )

local y = GXVec3 ()
y:Init ( 0.0, 0.499999672, 0.866025626 )

local z = GXVec3 ()
z:Init ( 0.0, -0.866025626, 0.499999672 )

local m = GXMat3 ()
m:SetX ( x )
m:SetY ( y )
m:SetZ ( z )

local v0 = GXVec3 ()
v0:Init ( 1.0, 777.0, 33.3 )

local v1 = GXVec3 ()
m:MultiplyMatrixVector ( v1, v0 )
```

## <a id="method-multiply-vector-matrix">`MultiplyVectorMatrix ( out, v )`</a>

Method performs vector-matrix multiplication of `v` [row-vector](https://en.wikipedia.org/wiki/Row_and_column_vectors) and current matrix `self`: `v` x `self`. The result is written to the `out` [row-vector](https://en.wikipedia.org/wiki/Row_and_column_vectors).

**Parameters:**

- `out` [_required, writeonly, [GXVec3](./gx-vec3.md)_]: result [row-vector](https://en.wikipedia.org/wiki/Row_and_column_vectors)
- `v` [_required, readonly, [GXVec3](./gx-vec3.md)_]: source [row-vector](https://en.wikipedia.org/wiki/Row_and_column_vectors)

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat3.lua"


local x = GXVec3 ()
x:Init ( 1.0, 0.0, 0.0 )

local y = GXVec3 ()
y:Init ( 0.0, 0.499999672, 0.866025626 )

local z = GXVec3 ()
z:Init ( 0.0, -0.866025626, 0.499999672 )

local m = GXMat3 ()
m:SetX ( x )
m:SetY ( y )
m:SetZ ( z )

local v0 = GXVec3 ()
v0:Init ( 1.0, 777.0, 33.3 )

local v1 = GXVec3 ()
m:MultiplyVectorMatrix ( v1, v0 )
```

## <a id="method-set-x">`SetX ( x )`</a>

Method sets the [_X_ row](#brief) of current matrix from the supplied `x` vector of the [_GXVec3_](./gx-vec3.md) type.

**Parameters:**

- `x` [_required, readonly, [_GXVec3_](./gx-vec3.md)_]: source vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat3.lua"


local v = GXVec3 ()
v:Init ( 1.0, 777.0, 33.3 )

local m = GXMat3 ()
m:Identity ()
m:SetX ( v )
```

## <a id="method-set-y">`SetY ( y )`</a>

Method sets the [_Y_ row](#brief) of current matrix from the supplied `y` vector of the [_GXVec3_](./gx-vec3.md) type.

**Parameters:**

- `y` [_required, readonly, [_GXVec3_](./gx-vec3.md)_]: source vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat3.lua"


local v = GXVec3 ()
v:Init ( 1.0, 777.0, 33.3 )

local m = GXMat3 ()
m:Identity ()
m:SetY ( v )
```

## <a id="method-set-z">`SetZ ( z )`</a>

Method sets the [_Z_ row](#brief) of current matrix from the supplied `z` vector of the [_GXVec3_](./gx-vec3.md) type.

**Parameters:**

- `z` [_required, readonly, [_GXVec3_](./gx-vec3.md)_]: source vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat3.lua"


local v = GXVec3 ()
v:Init ( 1.0, 777.0, 33.3 )

local m = GXMat3 ()
m:Identity ()
m:SetZ ( v )
```

## <a id="method-transpose">`Transpose ( sourceMatrix )`</a>

Method initializes the matrix as [transpose matrix](https://en.wikipedia.org/wiki/Transpose) to `sourceMatrix` matrix. `sourceMatrix` matrix is not changed during the operation.

**Parameters:**

- `sourceMatrix` [_required, readonly, [GXMat3](./gx-mat3.md)_]: matrix to transpose

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat3.lua"


local x = GXVec3 ()
x:Init ( 77.7, 1.0, 7.77 )

local y = GXVec3 ()
y:Init ( -0.333, 77.7, 1.11 )

local z = GXVec3 ()
z:Init ( 3.33, -1.333, 77.7 )

local m = GXMat3 ()
m:SetX ( x )
m:SetY ( y )
m:SetZ ( z )

local t = GXMat3 ()
t:Transpose ( m )
```
