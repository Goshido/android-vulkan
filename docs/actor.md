# _Actor_

```lua
require "av://engine/actor.lua"
```

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`AppendComponent ( component )`](#method-append-component)
- [`Destroy ()`](#method-destroy)
- [`FindComponent ( name )`](#method-find-component)
- [`FindComponents ( name )`](#method-find-components)
- [`GetName ()`](#method-get-name)

## <a id="brief">Brief</a>

Class represents actor entity which is used as building block for game scene. Actor is extended via component entities which actually do all job.

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

[↬ table of content ⇧](#table-of-content)

## <a id="constructor">`Constructor`</a>

Constructor creates new object.

**Note:** Name unique constraint is not mandatory, but recommended.

**Parameters:**

- `name` [_required, readonly, string_]: name of the actor

**Example:**

```lua
require "av://engine/actor.lua"


local actor = Actor ( "Actor #1" )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-append-component">`AppendComponent ( component )`</a>

Method appends `component` to current actor.

**Parameters:**

- `component` [_required, readonly, Component_]: component to append

**Return values:**

- none

**Example:**

```lua
require "av://engine/actor.lua"
require "av://engine/point_light_component.lua"


local actor = Actor ( "Actor" )
actor:AppendComponent ( PointLightComponent ( "Light" ) )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-destroy">`Destroy ()`</a>

Method destroys actor with all attached components.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/actor.lua"


local actor = Actor ( "Actor" )
g_scene:AppendActor ( actor )

actor:Destroy ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-find-component">`FindComponent ( name )`</a>

Method returns first existing component with `name`. Otherwise method returns `nil`.

**Parameters:**

- `name` [_required, readonly, string_]: name of component for search

**Return values:**

- `#1` [_required, Component or nil_]: _Component_ with `name` if it exists. Otherwise `nil` if there is no any _Component_ objects with `name`

**Example:**

```lua
require "av://engine/actor.lua"
require "av://engine/point_light_component.lua"


local actor = Actor ( "Actor" )
actor:AppendComponent ( PointLightComponent ( "Light #1" ) )
actor:AppendComponent ( PointLightComponent ( "Light #2" ) )

local light = actor:FindComponent ( "Light #1" )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-find-components">`FindComponents ( name )`</a>

Method returns an array of components with `name`. Otherwise method returns `nil`.

**Parameters:**

- `name` [_required, readonly, string_]: name of component for search

**Return values:**

- `#1` [_required, array of Component or nil_]: array of _Component_ objects with `name` if they exists. Otherwise `nil` if there is no any _Component_ objects with `name`

**Example:**

```lua
require "av://engine/actor.lua"
require "av://engine/point_light_component.lua"


local actor = Actor ( "Actor" )
actor:AppendComponent ( PointLightComponent ( "Light" ) )
actor:AppendComponent ( PointLightComponent ( "Light" ) )
actor:AppendComponent ( PointLightComponent ( "Another light" ) )

local lights = actor:FindComponents ( "Light" )
local secondLight = lights[ 2 ]
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-get-name">`GetName ()`</a>

Method returns name of the actor.

**Parameters:**

- none

**Return values:**

- `#1` [_required, string_]: name of the actor

**Example:**

```lua
require "av://engine/actor.lua"


local actor = Actor ( "Sun" )
local name = actor:GetName ()
```

[↬ table of content ⇧](#table-of-content)
