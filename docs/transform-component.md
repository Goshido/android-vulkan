# _TransformComponent_

```lua
require "av://engine/transform_component.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`GetTransform ( transform )`](#method-get-transform)

## <a id="brief">Brief</a>

The class holds transformation in world space. Usually the class is used as building block for game logic.

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
`__gc` | ❌
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

Constructor creates new object.

**Note:** Name unique constraint is not mandatory, but recommended.

**Parameters:**

- `name` [_required, readonly, string_]: name of the component

**Example:**

```lua
require "av://engine/scene.lua"


local platform = Actor ( "Platform" )
local transform = TransformComponent ( "Transform" )

platform:AppendComponent ( transform )
g_scene:AppendActor ( platform )
```

## <a id="method-get-transform">`GetTransform ( transform )`</a>

Method returns transformation in world space.

**Parameters:**

- `transform` [_required, writeonly, [GXMat4](./gx-mat4.md)_]: matrix to write transformation

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"


local platform = Actor ( "Platform" )
local transform = TransformComponent ( "Transform" )

platform:AppendComponent ( transform )
g_scene:AppendActor ( platform )

local t = GXMat4 ()
transform:GetTransform ( t )
```
