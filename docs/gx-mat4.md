# _GXMat4_

```lua
require "av://engine/gx_mat4.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`Identity ()`](#method-identity)
- [`Perspective ( fieldOfViewYRadians, aspectRatio, zNear, zFar )`](#method-perspective)
- [`RotationX ( angle )`](#method-rotation-x)
- [`Scale ( x, y, z )`](#method-scale)


## <a id="brief">Brief</a>

The class represents 4x4 matrix of `float32_t` values. The matrix layout in memory looks like this:

<img src="./images/gx-mat4-layout.png" width="745px"/>

Usually matrix class is used in _3D_ transformations. So by convention the matrix multiplication must be in direct order:

`{vertex} x {transform #1} x {transform #2} x ...`

For example classical **_model|view|projection_** transform looks like this in _Lua_:

```lua
require "av://engine/gx_math.lua"


local m = GXMat4 ()
m:RotateX ( 1.5 )

local v = GXMat4 ()
v:Translation ( 77.0, 0.0, 0.0 )
v:Inverse ()

local p = GXMat4 ()
p:Perspective ( 0.4, 16.0 / 9.0, 1.0, 77777.0 )

local mv = GXMat4 ()
mv:Multiply ( m, v )

local mvp = GXMat4 ()
mvp:Multiply ( mv, p )

local vertexLocal = GXVec4 ( 7.0, 3.3, 0.0, 1.0 )
local vertexCVV = GXVec4 ()

mvp:MultiplyVectorMatrix ( vertexCVV, vertexLocal )
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
require "av://engine/gx_mat4.lua"


local m = GXMat4 ()
```

## <a id="method-identity">`Identity ()`</a>

Method initializes the matrix as [identity matrix](https://en.wikipedia.org/wiki/Identity_matrix).

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat4.lua"


local p = GXMat4 ()
p:Identity ()
```

## <a id="method-perspective">`Perspective ( fieldOfViewYRadians, aspectRatio, zNear, zFar )`</a>

Method initializes the matrix with _Vulkan CVV_ perspective transformation.

**Parameters:**

- `fieldOfViewYRadians` _[required, number]_: view angle in vertical direction in radians
- `aspectRatio` _[required, number]_: the aspect ratio of the viewport
- `zNear` _[required, number]_: the distance to the near cutting plane
- `zFar` _[required, number]_: the distance to the far cutting plane

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat4.lua"


local fov = 90.0
local width = 1920.0
local height = 1080.0

local p = GXMat4 ()
p:Perspective ( math.rad ( fov ), width / height, 1.0, 77777.77 )
```

## <a id="method-rotation-x">`RotationX ( angle )`</a>

Method initializes the matrix as rotation transform about _X_-axis.

**Parameters:**

- `angle` _[required, number]_: rotation angle about _X_-axis in radians

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat4.lua"


local r = GXMat4 ()
r:RotationX ( 1.15 )
```

## <a id="method-scale">`Scale ( x, y, z )`</a>

Method initializes the matrix with scale transformation.

**Parameters:**

- `x` _[required, number]_: _X_-axis scale value
- `y` _[required, number]_: _Y_-axis scale value
- `z` _[required, number]_: _Z_-axis scale value

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat4.lua"


local s = GXMat4 ()
s:Scale ( 77.7, 3.33, 1.0 )
```
