# _GXMat4_

```lua
require "av://engine/gx_mat4.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`Identity ()`](#method-identity)
- [`Inverse ( sourceMatrix )`](#method-inverse)
- [`Multiply ( a, b )`](#method-multiply)
- [`MultiplyAsNormal ( out, v )`](#method-multiply-as-normal)
- [`MultiplyAsPoint ( out, v )`](#method-multiply-as-point)
- [`Perspective ( fieldOfViewYRadians, aspectRatio, zNear, zFar )`](#method-perspective)
- [`RotationX ( angle )`](#method-rotation-x)
- [`Scale ( x, y, z )`](#method-scale)
- [`TranslationF ( x, y, z )`](#method-translation-f)

## <a id="brief">Brief</a>

The class represents 4x4 matrix of `float32_t` values. The matrix layout in memory looks like this:

<img src="./images/gx-mat4-layout.png" width="745px"/>

Usually matrix class is used in _3D_ transformations. So by convention the matrix multiplication must be in direct order:

`{vertex} x {transform #1} x {transform #2} x ...`

It's possible bacause internal construction of scale, translation, rotation and projection transformations. For example classical **_model|view|projection_** transform looks like this in _Lua_:

```lua
require "av://engine/gx_mat4.lua"


local m = GXMat4 ()
m:RotationX ( 1.5 )

local camera = GXMat4 ()
camera:TranslationF ( 77.0, 0.0, 0.0 )

local v = GXMat4 ()
v:Inverse ( camera )

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

## <a id="method-inverse">`Inverse ( sourceMatrix )`</a>

Method initializes the matrix as inverse matrix to `sourceMatrix` matrix. `sourceMatrix` matrix is not changed during the operation.

**Parameters:**

- `sourceMatrix` [_required, [GXMat4](./gx-mat4.md)_]: matrix to inverse

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat4.lua"


local m = GXMat4 ()
m:RotationX ( math.rad ( 77.7 ) )

local i = GXMat4 ()
i:Inverse ( m )
```

## <a id="method-multiply">`Multiply ( a, b )`</a>

Method initializes the matrix as a product of the matrix multiplication `a` x `b` according classical [matrix multiplication rules](https://en.wikipedia.org/wiki/Matrix_multiplication#Definition).

**Parameters:**

- `a` [_required, [GXMat4](./gx-mat4.md)_]: left matrix
- `b` [_required, [GXMat4](./gx-mat4.md)_]: right matrix

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat4.lua"


local a = GXMat4 ()
a:RotationX ( math.rad ( 77.7 ) )

local fov = 90.0
local width = 1920.0
local height = 1080.0

local b = GXMat4 ()
b:Perspective ( math.rad ( fov ), width / height, 1.0, 77777.77 )

local m = GXMat4 ()
m:Multiply ( a, b )
```

## <a id="method-multiply-as-normal">`MultiplyAsNormal ( out, v )`</a>

Method applies rotation and scale transforms of current matrix to vector `v` and writes the result to vector `out`. 

**Parameters:**

- `out` [_required, [GXVec3](./gx-vec3.md)_]: result vector
- `v` [_required, [GXVec3](./gx-vec3.md)_]: source vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat4.lua"


local v = GXVec3 ()
v:Init ( 1.0, 777.0, 33.3 )

local r = GXMat4 ()
r:RotationX ( math.rad ( 77.7 ) )

local n = GXVec3 ()
r:MultiplyAsNormal ( n, v )
```
## <a id="method-multiply-as-point">`MultiplyAsPoint ( out, v )`</a>

Method applies rotation, scale and translation transforms of current matrix to vector `v` and writes the result to vector `out`. 

**Parameters:**

- `out` [_required, [GXVec3](./gx-vec3.md)_]: result vector
- `v` [_required, [GXVec3](./gx-vec3.md)_]: source vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat4.lua"


local vLocal = GXVec3 ()
vLocal:Init ( 1.0, 777.0, 33.3 )

local m = GXMat4 ()
m:RotationX ( 1.5 )

local camera = GXMat4 ()
camera:TranslationF ( 77.0, -10.0, 3.33 )

local v = GXMat4 ()
v:Inverse ( camera )

local mv = GXMat4 ()
mv:Multiply ( m, v )

local vView = GXVec3 ()
mv:MultiplyAsPoint ( vView, vLocal )
```

## <a id="method-perspective">`Perspective ( fieldOfViewYRadians, aspectRatio, zNear, zFar )`</a>

Method initializes the matrix with _Vulkan CVV_ perspective transformation.

**Parameters:**

- `fieldOfViewYRadians` [_required, number_]: view angle in vertical direction in radians
- `aspectRatio` [_required, number_]: the aspect ratio of the viewport
- `zNear` [_required, number_]: the distance to the near cutting plane
- `zFar` [_required, number_]: the distance to the far cutting plane

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

- `angle` [_required, number_]: rotation angle about _X_-axis in radians

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

- `x` [_required, number_]: _X_-axis scale value
- `y` [_required, number_]: _Y_-axis scale value
- `z` [_required, number_]: _Z_-axis scale value

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat4.lua"


local s = GXMat4 ()
s:Scale ( 77.7, 3.33, 1.0 )
```

## <a id="method-translation-f">`TranslationF ( x, y, z )`</a>

Method initializes the matrix with translation transformation.

**Parameters:**

- `x` [_required, number_]: _X_-coodinate of the origin
- `y` [_required, number_]: _Y_-coodinate of the origin
- `z` [_required, number_]: _Z_-coodinate of the origin

**Return values:**

- none

**Example:**

```lua
require "av://engine/gx_mat4.lua"


local s = GXMat4 ()
s:TranslationF ( 77.7, 3.33, 1.0 )
```
