# _Material_

```lua
require "av://engine/material.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)

## <a id="brief">Brief</a>

Class allows to load materials from `*.mtl` assets. Later material instance could be assigned to renderable entity. For example to [_StaticMeshComponent_](./static-mesh-component.md).

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

Constructor creates new material instance from `*.mtl` asset.

**Parameters:**

- `materialFile` [_required, readonly, string_]: file path to material asset

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
