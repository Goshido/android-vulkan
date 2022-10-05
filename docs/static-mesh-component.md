# _StaticMeshComponent_

```lua
require "av://engine/static_mesh_component.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`GetLocal ( localMatrix )`](#method-get-local)
- [`SetLocal ( localMatrix )`](#method-set-local)
- [`SetMaterial ( material )`](#method-set-material)

## <a id="brief">Brief</a>

Class represents static mesh component entity on the scene.

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
`__concat` | ❌
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
`__tostring` | ❌
`__unm` | ❌

## <a id="constructor">`Constructor`</a>

Constructor creates new static mesh component with default properties.

**Parameters:**

- `name` [_required, readonly, string_]: name of the component
- `meshFile` [_required, readonly, string_]: file path to mesh asset

**Example:**

```lua
require "av://engine/scene.lua"


local rock = Actor ( "Rock" )
local mesh = StaticMeshComponent ( "Mesh", "pbr/assets/System/Default.mesh2" )

rock:AppendComponent ( mesh )
g_scene:AppendActor ( rock )
```

## <a id="method-get-local">`GetLocal ( localMatrix )`</a>

Method returns local transform of the component.

**Parameters:**

- `localMatrix` [_required, writeonly, [_GXMat4_](./gx-mat4.md)_]: transform matrix in the world space

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"


local rock = Actor ( "Rock" )
local mesh = StaticMeshComponent ( "Mesh", "pbr/assets/System/Default.mesh2" )

rock:AppendComponent ( mesh )
g_scene:AppendActor ( rock )

local m = GXMat4 ()
mesh:GetLocal ( m )
```

## <a id="method-set-local">`SetLocal ( localMatrix )`</a>

Method sets local transform of the component.

**Parameters:**

- `localMatrix` [_required, readonly, [_GXMat4_](./gx-mat4.md)_]: transform matrix in the world space

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"


local rock = Actor ( "Rock" )
local mesh = StaticMeshComponent ( "Mesh", "pbr/assets/System/Default.mesh2" )

rock:AppendComponent ( mesh )
g_scene:AppendActor ( rock )

local v = GXVec3 ()
v:Init ( 1.0, 777.0, 33.3 )

local m = GXMat4 ()
m:RotationX ( math.rad ( 33.3 ) )
m:SetW ( v )
mesh:SetLocal ( m )
```

## <a id="method-set-material">`SetMaterial ( material )`</a>

Method sets material to component.

**Parameters:**

- `material` [_required, readonly, [_Material_](./material.md)_]: material

**Return values:**

- none

**Example:**

```lua
require "av://engine/material.lua"
require "av://engine/scene.lua"


local rock = Actor ( "Rock" )
local mesh = StaticMeshComponent ( "Mesh", "pbr/assets/System/Default.mesh2" )
mesh:SetMaterial ( Material ( "pbr/assets/System/Default.mtl" ) )

rock:AppendComponent ( mesh )
g_scene:AppendActor ( rock )
```
