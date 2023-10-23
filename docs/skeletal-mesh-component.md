# _SkeletalMeshComponent_

```lua
require "av://engine/skeletal_mesh_component.lua"
```

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`SetAnimationGraph ( animationGraph )`](#method-set-animation-graph)
- [`SetColor0 ( color )`](#method-set-color-0)
- [`SetColor1 ( color )`](#method-set-color-1)
- [`SetColor2 ( color )`](#method-set-color-2)
- [`SetEmission ( color )`](#method-set-emission)
- [`GetLocal ( localMatrix )`](#method-get-local)
- [`SetLocal ( localMatrix )`](#method-set-local)
- [`SetMaterial ( material )`](#method-set-material)

## <a id="brief">Brief</a>

Class represents skeletal mesh component entity on the scene.

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

Constructor creates new skeletal mesh component.

**Parameters:**

- `name` [_required, readonly, string_]: name of the component
- `meshFile` [_required, readonly, string_]: file path to `*.mesh2` asset which could be created via [_3ds Max plugin_](./3ds-max-exporter.md)
- `skinFile` [_required, readonly, string_]: file path to `*.skin` asset which could be created via [_3ds Max plugin_](./3ds-max-exporter.md)
- `skeletonFile`[_required, readonly, string_]: file path to `*.skeleton` asset which could be created via [_3ds Max plugin_](./3ds-max-exporter.md)

**Example:**

```lua
require "av://engine/actor.lua"
require "av://engine/skeletal_mesh_component.lua"


local player = Actor ( "Player" )

local mesh = SkeletalMeshComponent ( "Mesh",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.mesh2",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skin",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton"
)

player:AppendComponent ( mesh )
g_scene:AppendActor ( player )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-animation-graph">`SetAnimationGraph ( animationGraph )`</a>

Method sets [_AnimationGraph_](./animation-graph.md).

**Parameters:**

- `animationGraph` [_required, readonly, [AnimationGraph](./animation-graph.md)_]: animation graph

**Return values:**

- none

**Example:**

```lua
require "av://engine/animation_graph.lua"
require "av://engine/skeletal_mesh_component.lua"


local player = Actor ( "Player" )

local mesh = SkeletalMeshComponent ( "Mesh",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.mesh2",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skin",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton"
)

local animationGraph = AnimationGraph ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton" )

mesh:SetAnimationGraph ( animationGraph )

-- Making sure that 'animationGraph' will be not garbage collected.
mesh._animationGraph = animationGraph

player:AppendComponent ( mesh )
g_scene:AppendActor ( player )
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
require "av://engine/actor.lua"
require "av://engine/gx_vec4.lua"
require "av://engine/skeletal_mesh_component.lua"


local player = Actor ( "Player" )

local mesh = SkeletalMeshComponent ( "Mesh",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.mesh2",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skin",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton"
)

local color = GXVec4 ()
color:Init ( 0.45, 0.72, 0.0, 1.0 )
mesh:SetColor0 ( color )

player:AppendComponent ( mesh )
g_scene:AppendActor ( player )
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
require "av://engine/actor.lua"
require "av://engine/gx_vec3.lua"
require "av://engine/skeletal_mesh_component.lua"


local player = Actor ( "Player" )

local mesh = SkeletalMeshComponent ( "Mesh",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.mesh2",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skin",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton"
)

local color = GXVec3 ()
color:Init ( 0.45, 0.72, 0.0 )
mesh:SetColor1 ( color )

player:AppendComponent ( mesh )
g_scene:AppendActor ( player )
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
require "av://engine/actor.lua"
require "av://engine/gx_vec3.lua"
require "av://engine/skeletal_mesh_component.lua"


local player = Actor ( "Player" )

local mesh = SkeletalMeshComponent ( "Mesh",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.mesh2",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skin",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton"
)

local color = GXVec3 ()
color:Init ( 0.45, 0.72, 0.0 )
mesh:SetColor2 ( color )

player:AppendComponent ( mesh )
g_scene:AppendActor ( player )
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
require "av://engine/actor.lua"
require "av://engine/gx_vec3.lua"
require "av://engine/skeletal_mesh_component.lua"


local player = Actor ( "Player" )

local mesh = SkeletalMeshComponent ( "Mesh",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.mesh2",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skin",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton"
)

local emission = GXVec3 ()
emission:Init ( 0.45, 0.72, 0.0 )
mesh:SetEmission ( emission )

player:AppendComponent ( mesh )
g_scene:AppendActor ( player )
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
require "av://engine/actor.lua"
require "av://engine/gx_mat4.lua"
require "av://engine/gx_vec3.lua"
require "av://engine/skeletal_mesh_component.lua"


local player = Actor ( "Player" )

local mesh = SkeletalMeshComponent ( "Mesh",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.mesh2",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skin",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton"
)

local v = GXVec3 ()
v:Init ( 1.0, 777.0, 33.3 )

local m = GXMat4 ()
m:RotationX ( math.rad ( 33.3 ) )
m:SetW ( v )
mesh:SetLocal ( m )

player:AppendComponent ( mesh )
g_scene:AppendActor ( player )
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
require "av://engine/actor.lua"
require "av://engine/gx_mat4.lua"
require "av://engine/skeletal_mesh_component.lua"


local player = Actor ( "Player" )

local mesh = SkeletalMeshComponent ( "Mesh",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.mesh2",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skin",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton"
)

player:AppendComponent ( mesh )
g_scene:AppendActor ( player )

local m = GXMat4 ()
mesh:GetLocal ( m )
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
require "av://engine/actor.lua"
require "av://engine/material.lua"
require "av://engine/skeletal_mesh_component.lua"


local player = Actor ( "Player" )

local mesh = SkeletalMeshComponent ( "Mesh",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.mesh2",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skin",
    "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton"
)

mesh:SetMaterial ( Material ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.mtl" ) )

player:AppendComponent ( mesh )
g_scene:AppendActor ( player )
```

[↬ table of content ⇧](#table-of-content)
