# _Scene_

```lua
-- No need to require anything. g_scene is an object instance of the Scene class and it's created globally.
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`AppendActor ( actor )`](#method-append-actor)
- [`FindActor ( name )`](#method-find-actor)
- [`FindActors ( name )`](#method-find-actors)
- [`GetPhysicsToRenderScaleFactor ()`](#method-get-physics-to-renderer-scale-factor)
- [`GetRenderToPhysicsScaleFactor ()`](#method-get-renderer-to-physics-scale-factor)

## <a id="brief">Brief</a>

Class represents scene entity which is used as foundation for scriptable logic. Scene holds active [actors](./actor.md) and allows to manage them via various methods. There is only one instance of [_Scene_](./scene.md) class called `g_scene` which is accessible from everywhere.

## <a id="metamethods">Metamethods</a>

Metamethod | Used
--- | ---
`__add` | ❌
`__band` | ❌
`__bnot` | ❌
`__bor` | ❌
`__bxor` | ❌
`__call` | ❌
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

_N/A_

## <a id="method-append-actor">`AppendActor ( actor )`</a>

Method appends `actor` to scene.

**Parameters:**

- `actor` [_required, readonly, [Actor](./actor.md)_]: [_Actor_](./actor.md) to append

**Return values:**

- none

**Example:**

```lua
require "av://engine/actor.lua"
require "av://engine/point_light_component.lua"


local sun = Actor ( "Sun" )
sun:AppendComponent ( PointLightComponent ( "Light" ) )

g_scene:AppendActor ( sun )
```

## <a id="method-find-actor">`FindActor ( name )`</a>

Method returns first existing [_Actor_](./actor.md) with `name`. Otherwise method returns `nil`.

**Parameters:**

- `name` [_required, readonly, string_]: name of [_Actor_](./actor.md) for search

**Return values:**

- `#1` [_required, [Actor](./actor.md) or nil_]: [_Actor_](./actor.md) with `name` if it exists. Otherwise `nil` if there is no any [_Actor_](./actor.md) objects with `name`

**Example:**

```lua
require "av://engine/actor.lua"
require "av://engine/point_light_component.lua"


local sun = Actor ( "Sun" )
sun:AppendComponent ( PointLightComponent ( "Light" ) )
g_scene:AppendActor ( sun )

local lightSource = g_scene:FindComponent ( "Sun" )
```

## <a id="method-find-actors">`FindActors ( name )`</a>

Method returns an array of [_Actors_](./actor.md) with `name`. Otherwise method returns `nil`.

**Parameters:**

- `name` [_required, readonly, string_]: name of [_Actor_](./actor.md) for search

**Return values:**

- `#1` [_required, array of [Actors](./actor.md) or nil_]: array of [_Actors_](./actor.md) objects with `name` if they exists. Otherwise `nil` if there is no any [_Actor_](./actor.md) objects with `name`

**Example:**

```lua
require "av://engine/actor.lua"
require "av://engine/point_light_component.lua"


local lamp0 = Actor ( "Lamp" )
lamp0:AppendComponent ( PointLightComponent ( "Light" ) )
g_scene:AppenActor ( lamp0 )

local lamp1 = Actor ( "Lamp" )
lamp1:AppendComponent ( PointLightComponent ( "Light" ) )
g_scene:AppenActor ( lamp1 )

local lamps = g_scene:FindActors ( "Lamp" )
```

## <a id="method-get-physics-to-renderer-scale-factor">`GetPhysicsToRendererScaleFactor ()`</a>

Method returns scale factor to convert [physics coordinate system](./rigid-body-component.md#note-physics-coordinate-system) to render coordinate system.

**Parameters:**

- none

**Return values:**

- `#1` [_required, number_]: scale factor

**Example:**

```lua
require "av://engine/scene.lua"


local actor = Actor ( "Box" )
local body = RigidBodyComponent ( "RigidBody" )
actor:AppendComponent ( body )
g_scene:AppendActor ( actor )

local locationPhysics = GXVec3 ()
body:GetLocation ( locationPhysics )

local locationRender = GXVec3 ()
locationRender:MultiplyScalar ( locationPhysics, g_scene:GetPhysicsToRenderScaleFactor () )
```

## <a id="method-get-renderer-to-physics-scale-factor">`GetRendererToPhysicsScaleFactor ()`</a>

Method returns scale factor to convert render coordinate system to [physics coordinate system](./rigid-body-component.md#note-physics-coordinate-system).

**Parameters:**

- none

**Return values:**

- `#1` [_required, number_]: scale factor

**Example:**

```lua
require "av://engine/scene.lua"


local locationRender = GXVec3 ()
locationRender:Init ( 777.0, 3.33, 1.0 )

local locationPhysics = GXVec3 ()
locationPhysics:MultiplyScalar ( locationRender, g_scene:GetRendererToPhysicsScaleFactor () )
```
