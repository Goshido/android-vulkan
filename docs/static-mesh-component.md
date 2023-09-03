# _StaticMeshComponent_

```lua
require "av://engine/static_mesh_component.lua"
```

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`SetColor0 ( color )`](#method-set-color-0)
- [`SetColor1 ( color )`](#method-set-color-1)
- [`SetColor2 ( color )`](#method-set-color-2)
- [`SetEmission ( color )`](#method-set-emission)
- [`GetLocal ( localMatrix )`](#method-get-local)
- [`SetLocal ( localMatrix )`](#method-set-local)
- [`SetMaterial ( material )`](#method-set-material)

## <a id="brief">Brief</a>

Class represents static mesh component entity on the scene.

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

[↬ table of content ⇧](#table-of-content)

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

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-color-0">`SetColor0 ( color )`</a>

Method sets material's [_Color0_](./material.md#blend-mask) value.

**Parameters:**

- `color` [_required, readonly, [_GXVec4_](./gx-vec4.md)_]: color value

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"


local rock = Actor ( "Rock" )
local mesh = StaticMeshComponent ( "Mesh", "pbr/assets/System/Default.mesh2" )

rock:AppendComponent ( mesh )
g_scene:AppendActor ( rock )

local color = GXVec4 ()
color:Init ( 0.45, 0.72, 0.0, 1.0 )
mesh:SetColor0 ( color )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-color-1">`SetColor1 ( color )`</a>

Method sets material's [_Color1_](./material.md#blend-mask) value.

**Parameters:**

- `color` [_required, readonly, [_GXVec3_](./gx-vec3.md)_]: color value

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"


local rock = Actor ( "Rock" )
local mesh = StaticMeshComponent ( "Mesh", "pbr/assets/System/Default.mesh2" )

rock:AppendComponent ( mesh )
g_scene:AppendActor ( rock )

local color = GXVec3 ()
color:Init ( 0.45, 0.72, 0.0 )
mesh:SetColor1 ( color )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-color-2">`SetColor2 ( color )`</a>

Method sets material's [_Color2_](./material.md#blend-mask) value.

**Parameters:**

- `color` [_required, readonly, [_GXVec3_](./gx-vec3.md)_]: color value

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"


local rock = Actor ( "Rock" )
local mesh = StaticMeshComponent ( "Mesh", "pbr/assets/System/Default.mesh2" )

rock:AppendComponent ( mesh )
g_scene:AppendActor ( rock )

local color = GXVec3 ()
color:Init ( 0.45, 0.72, 0.0 )
mesh:SetColor2 ( color )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-emission">`SetEmission ( emission )`</a>

Method sets material's [_Emission_](./material.md#emisson) value.

**Parameters:**

- `emission` [_required, readonly, [_GXVec3_](./gx-vec3.md)_]: emission value

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"


local rock = Actor ( "Rock" )
local mesh = StaticMeshComponent ( "Mesh", "pbr/assets/System/Default.mesh2" )

rock:AppendComponent ( mesh )
g_scene:AppendActor ( rock )

local emission = GXVec3 ()
emission:Init ( 0.45, 0.72, 0.0 )
mesh:SetEmission ( emission )
```

[↬ table of content ⇧](#table-of-content)

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

[↬ table of content ⇧](#table-of-content)

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

[↬ table of content ⇧](#table-of-content)

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

[↬ table of content ⇧](#table-of-content)
