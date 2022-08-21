# _CameraComponent_

```lua
require "av://engine/camera_component.lua"
```

## Table of content

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`SetAspectRatio ( aspectRatio )`](#method-set-aspect-ratio)
- [`SetLocal ( localMatrix )`](#method-set-local)
- [`SetProjection ( fieldOfViewRadians, aspectRatio, zNear, zFar )`](#method-set-projection)

## <a id="brief">Brief</a>

Class represents perspective camera which could be used for scene rendering. There is only one active camera for render session. The [_Scene:SetActiveCamera_](./scene.md#method-set-active-camera) _API_ should be used for switching the cameras.

```lua
require "av://engine/scene.lua"


local mainCamera = Actor ( "Main Camera" )
local cameraComponent = CameraComponent ( "Camera" )

mainCamera:AppendComponent ( cameraComponent )
g_scene:AppendActor ( mainCamera )

g_scene:SetActiveCamera ( cameraComponent )
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


local mainCamera = Actor ( "Main Camera" )
local cameraComponent = CameraComponent ( "Camera" )

mainCamera:AppendComponent ( cameraComponent )
g_scene:AppendActor ( mainCamera )
```

## <a id="method-set-aspect-ratio">`SetAspectRatio ( aspectRatio )`</a>

Method changes camera projection transformation.

**Parameters:**

- `aspectRatio` [_required, readonly, number_]: viewport aspect ratio, i.e. width divided by height

**Return values:**

- none

```lua
require "av://engine/scene.lua"


local mainCamera = Actor ( "Main Camera" )
local cameraComponent = CameraComponent ( "Camera" )

mainCamera:AppendComponent ( cameraComponent )
g_scene:AppendActor ( mainCamera )

local width = 1920.0
local height = 1080.0
cameraComponent:SetAspectRatio ( width / height )
```

## <a id="method-set-local">`SetLocal ( localMatrix )`</a>

Method changes local camera transformation, i.e. orientation and location.

**Parameters:**

- `localMatrix` [_required, readonly, [_GXMat4_](./gx-mat4.md)_]: composite camera transformation

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"


local mainCamera = Actor ( "Main Camera" )
local cameraComponent = CameraComponent ( "Camera" )

mainCamera:AppendComponent ( cameraComponent )
g_scene:AppendActor ( mainCamera )

local axis = GXVec3 ()
axis:Init ( 7.77, 3.33, 1.0 )
axis:Normalize ()

local rotation = GXQuat ()
rotation:FromAxisAngle ( axis, math.rad ( 77.7 ) )

local origin = GXVec3 ()
origin:Init ( 1.0, 333.0, 0.0 )

local transform = GXMat4 ()
transform:FromFast ( rotation, origin )

cameraComponent:SetLocal ( transform )
```

## <a id="method-set-projection">`SetProjection ( fieldOfViewRadians, aspectRatio, zNear, zFar )`</a>

Method changes camera projection transformation.

**Parameters:**

- `fieldOfViewRadians` [_required, readonly, number_]: field of view angle of _Y_ axis in [radians](https://en.wikipedia.org/wiki/Radian)
- `aspectRatio` [_required, readonly, number_]: viewport aspect ratio, i.e. width divided by height
- `zNear` [_required, readonly, number_]: the distance to near clipping plane
- `zFar` [_required, readonly, number_]: the distance to far clipping plane

**Return values:**

- none

```lua
require "av://engine/scene.lua"


local mainCamera = Actor ( "Main Camera" )
local cameraComponent = CameraComponent ( "Camera" )

mainCamera:AppendComponent ( cameraComponent )
g_scene:AppendActor ( mainCamera )

local width = 1920.0
local height = 1080.0
cameraComponent:SetProjection ( math.rad ( 60.0 ), width / height, 1.0e-1, 1.0e+4 )
```
