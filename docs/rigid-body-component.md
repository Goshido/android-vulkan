# _RigidBodyComponent_

```lua
require "av://engine/rigid_body_component.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`AddForce ( force, point, forceAwake )`](#method-add-force)
- [`GetLocation ( location )`](#method-get-location)
- [`GetName ()`](#method-get-name)
- [`GetVelocityLinear ( velocity )`](#method-get-velocity-linear)
- [`SetVelocityLinear ( velocity, forceAwake )`](#method-set-velocity-linear)

## <a id="brief">Brief</a>

Class represents rigid body entity which is one of the main building blocks of the physics simulation. There two main category for rigid bodies: *_kinematic_* and *_dynamic_*. The kinematic body has infinite mass and it is the best tool for describing level geometry. The dynamic body is simulated according physics laws. Dynamic body also controls the transfromation of every transformable components which are attached to the current [_Actor_](./actor.md).

**Important rule:** There must be no more that one [_RigidBodyComponent_](./rigid-body-component.md) attached to current [_Actor_](./actor.md). This rule helps to avoid ambiguity between physics simulation and graphics representation.

<a id="note-physics-coordinate-system">**Importan note:**</a> Physics coordinate system is different from renderer coordinate system in term of unit length. The following table describes physics coordinate system properties:

Property | Description
--- | ---
_X_ | from left to right
_Y_ | from bottom to top
_Z_ | viewer sight direction
_Unit length_ | 1 [meter](https://en.wikipedia.org/wiki/Metre)

See [`Scene:GetPhysicsToRenderScaleFactor`](./scene.md#method-get-physics-to-renderer-scale-factor) and [`Scene:GetRenderToPhysicsScaleFactor`](./scene.md#method-get-renderer-to-physics-scale-factor)

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

Constructor creates new rigid body component with default properties.

**Parameters:**

- `name` [_required, readonly, string_]: name of the component

**Example:**

```lua
require "av://engine/scene.lua"


local actor = Actor ( "Box" )

actor:AppendComponent ( RigidBodyComponent ( "RigidBody" ) )
g_scene:AppendActor ( actor )
```

## <a id="method-add-force">`AddForce ( force, point, forceAwake )`</a>

Method applies force to body.

**Note:** This method uses force vector and apply point in [physics coordinate system](#note-physics-coordinate-system).

**Parameters:**

- `force` [_required, readonly, [_GXVec3_](./gx-vec3.md)_]: force vector in world space
- `point` [_required, readonly, [_GXVec3_](./gx-vec3.md)_]: point to apply force in world space
- `forceAwake` [_required, readonly, boolean_]: awake rigid body or not

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"


local actor = Actor ( "Box" )
local body = RigidBodyComponent ( "RigidBody" )
actor:AppendComponent ( body )
g_scene:AppendActor ( actor )

local force = GXVec3 ()
force:Init ( 0.0, 77.7, 0.0 )

local point = GXVec3 ()
point:Init ( 0.0, 0.0, 0.0 )

body:AddForce ( force, point, true )
```

## <a id="method-get-location">`GetLocation ( location )`</a>

Method writes the body location in world space to the supplied `location` vector of the [_GXVec3_](./gx-vec3.md) type.

**Note:** This method returns location in [physics coordinate system](#note-physics-coordinate-system).

**Parameters:**

- `location` [_required, writeonly, [_GXVec3_](./gx-vec3.md)_]: target vector

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"


local actor = Actor ( "Box" )
local body = RigidBodyComponent ( "RigidBody" )
actor:AppendComponent ( body )
g_scene:AppendActor ( actor )

local location = GXVec3 ()
body:GetLocation ( location )
```

## <a id="method-get-name">`GetName ()`</a>

Method returns name of the component.

**Parameters:**

- none

**Return values:**

- `#1` [_required, string_]: name of the component

**Example:**

```lua
require "av://engine/scene.lua"


local actor = Actor ( "Box" )

actor:AppendComponent ( RigidBodyComponent ( "RigidBody" ) )
g_scene:AppendActor ( actor )

local rigidBody = actor:FindComponent ( "RigidBody" )
local name = rigidBody:GetName ()
```

## <a id="method-get-velocity-linear">`GetVelocityLinear ( velocity )`</a>

Method returns current linear velocity of the body.

**Note:** This method returns velocity in [physics coordinate system](#note-physics-coordinate-system).

**Parameters:**

- `velocity` [_required, writeonly, [_GXVec3_](./gx-vec3.md)_]: velocity vector in world space

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"


local actor = Actor ( "Box" )
local body = RigidBodyComponent ( "RigidBody" )
actor:AppendComponent ( body )
g_scene:AppendActor ( actor )

local velocity = GXVec3 ()
body:GetVelocityLinear ( velocity )
```

## <a id="method-set-velocity-linear">`SetVelocityLinear ( velocity, forceAwake )`</a>

Method sets linear velocity to the body.

**Note:** This method uses [physics coordinate system](#note-physics-coordinate-system) for velocity vector.

**Parameters:**

- `velocity` [_required, readonly, [_GXVec3_](./gx-vec3.md)_]: velocity vector in world space
- `forceAwake` [_required, readonly, boolean_]: awake rigid body or not

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"


local actor = Actor ( "Box" )
local body = RigidBodyComponent ( "RigidBody" )
actor:AppendComponent ( body )
g_scene:AppendActor ( actor )

local velocity = GXVec3 ()
body:GetVelocityLinear ( velocity )
```