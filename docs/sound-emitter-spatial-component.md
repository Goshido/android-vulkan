# _SoundEmitterSpatialComponent_

```lua
require "av://engine/sound_emitter_spatial_component.lua"
```

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`GetVolume ()`](#method-get-volume)
- [`SetVolume ( volume )`](#method-set-volume)
- [`IsPlaying ()`](#method-is-playing)
- [`Pause ()`](#method-pause)
- [`Play ()`](#method-play)
- [`SetDistance ( distance )`](#method-set-distance)
- [`SetLocation ( location )`](#method-set-location)
- [`SetSoundAsset ( asset, looped )`](#method-set-sound-asset)
- [`Stop ()`](#method-stop)

## <a id="brief">Brief</a>

Class represents spatial sound emitter entity. It's sound with real position and distance in [physics coordinate system](./rigid-body-component.md#note-physics-coordinate-system).

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

Constructor creates new spatial sound emitter component.

**Parameters:**

- `name` [_required, readonly, string_]: name of the component
- `soundChannel` [_required, readonly, [eSoundChannel](./sound-channel.md)_]: sound channel

**Example:**

```lua
require "av://engine/scene.lua"
require "av://engine/sound_emitter_spatial_component.lua"


local actor = Actor ( "Actor" )
actor:AppendComponent ( SoundEmitterSpatialComponent ( "SFX", eSoundChannel.SFX ) ) )
g_scene:AppendActor ( actor )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-get-volume">`GetVolume ()`</a>

Method returns the volume of the emitter.

**Parameters:**

- none

**Return values:**

- `#1` [_required, number_]: volume in range from `0.0` to `1.0`. The `0.0` value means total silence

**Example:**

```lua
require "av://engine/scene.lua"
require "av://engine/sound_emitter_spatial_component.lua"


local actor = Actor ( "Actor" )

local sfx = SoundEmitterSpatialComponent ( "SFX", eSoundChannel.SFX ) )
actor:AppendComponent ( voice )
g_scene:AppendActor ( actor )

local volume = voice:GetVolume ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-volume">`SetVolume ( volume )`</a>

Method sets the volume of the emitter.

**Parameters:**

- `volume` [_required, readonly, number_]: volume in range from `0.0` to `1.0`. The `0.0` value means total silence

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"
require "av://engine/sound_emitter_spatial_component.lua"


local actor = Actor ( "Actor" )

local sfx = SoundEmitterSpatialComponent ( "SFX", eSoundChannel.SFX ) )
actor:AppendComponent ( sfx )
g_scene:AppendActor ( actor )

sfx:SetVolume ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-is-playing">`IsPlaying ()`</a>

Method tells if the emitter is playing sound or not.

**Parameters:**

- none

**Return values:**

- `#1` [_required, boolean_]: `true` if the emitter is playing sound, otherwise `false`

**Example:**

```lua
require "av://engine/scene.lua"
require "av://engine/sound_emitter_spatial_component.lua"


local actor = Actor ( "Actor" )

local sfx = SoundEmitterSpatialComponent ( "SFX", eSoundChannel.SFX ) )
actor:AppendComponent ( sfx )
g_scene:AppendActor ( actor )

local isPlaying = sfx:IsPlaying ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-pause">`Pause ()`</a>

Method sets the playing of sound on pause.

**Parameters:**

- none

**Return values:**

- `#1` [_required, boolean_]: `true` operation was performed successfully, otherwise `false`

**Example:**

```lua
require "av://engine/scene.lua"
require "av://engine/sound_emitter_spatial_component.lua"


local actor = Actor ( "Actor" )

local sfx = SoundEmitterSpatialComponent ( "SFX", eSoundChannel.SFX ) )
actor:AppendComponent ( sfx )
g_scene:AppendActor ( actor )

sfx:SetSoundAsset ( "sounds/sine.ogg", true )
sfx:Play ()
local success = sfx:Pause ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-play">`Play ()`</a>

Method begins sound playing.

**Parameters:**

- none

**Return values:**

- `#1` [_required, boolean_]: `true` if the operation was performed successfully, otherwise `false`

**Example:**

```lua
require "av://engine/scene.lua"
require "av://engine/sound_emitter_spatial_component.lua"


local actor = Actor ( "Actor" )

local sfx = SoundEmitterSpatialComponent ( "SFX", eSoundChannel.SFX ) )
actor:AppendComponent ( sfx )
g_scene:AppendActor ( actor )

sfx:SetSoundAsset ( "sounds/sine.ogg", true )
local success = sfx:Play ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-distance">`SetDistance ( distance )`</a>

Method sets emitter distance in [physics coordinate system](./rigid-body-component.md#note-physics-coordinate-system) units.

**Parameters:**

- `distance` [_required, readonly, number_]: distance which emitter affects

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"
require "av://engine/sound_emitter_spatial_component.lua"


local actor = Actor ( "Actor" )

local sfx = SoundEmitterSpatialComponent ( "SFX", eSoundChannel.SFX ) )
actor:AppendComponent ( sfx )
g_scene:AppendActor ( actor )

sfx:SetDistance ( 42.0 )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-location">`SetLocation ( location )`</a>

Method sets emitter location in [physics coordinate system](./rigid-body-component.md#note-physics-coordinate-system).

**Parameters:**

- `location` [_required, readonly, [_GXVec3_](./gx-vec3.md)_]: emitter location

**Return values:**

- none

**Example:**

```lua
require "av://engine/scene.lua"
require "av://engine/sound_emitter_spatial_component.lua"


local actor = Actor ( "Actor" )

local sfx = SoundEmitterSpatialComponent ( "SFX", eSoundChannel.SFX ) )
actor:AppendComponent ( sfx )
g_scene:AppendActor ( actor )

local location = GXVec3 ()
location:Init ( 777.0, 3.33, 1.0 )
sfx:SetLocation ( location )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-sound-asset">`SetSoundAsset ( asset, looped )`</a>

Method sets sound asset for playing.

**Parameters:**

- `asset` [_required, readonly, string_]: path to sound asset
- `looped` [_required, readonly, boolean_]: `true` if the asset should be looped, otherwise `false`

**Return values:**

- `#1` [_required, boolean_]: `true` if the operation was performed successfully, otherwise `false`

**Example:**

```lua
require "av://engine/scene.lua"
require "av://engine/sound_emitter_spatial_component.lua"


local actor = Actor ( "Actor" )

local sfx = SoundEmitterSpatialComponent ( "SFX", eSoundChannel.SFX ) )
actor:AppendComponent ( sfx )
g_scene:AppendActor ( actor )

local success = sfx:SetSoundAsset ( "sounds/sine.ogg", true )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-stop">`Stop ()`</a>

Method stops the playing of sound.

**Parameters:**

- none

**Return values:**

- `#1` [_required, boolean_]: `true` if the operation was performed successfully, otherwise `false`

**Example:**

```lua
require "av://engine/scene.lua"
require "av://engine/sound_emitter_spatial_component.lua"


local actor = Actor ( "Actor" )

local sfx = SoundEmitterSpatialComponent ( "SFX", eSoundChannel.SFX ) )
actor:AppendComponent ( sfx )
g_scene:AppendActor ( actor )

sfx:SetSoundAsset ( "sounds/sine.ogg", true )
sfx:Play ()
local success = sfx:Stop ()
```

[↬ table of content ⇧](#table-of-content)
