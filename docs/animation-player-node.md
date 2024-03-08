# _AnimationPlayerNode_

```lua
require "av://engine/animation_player_node.lua"
```

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`LoadAnimation ( animation )`](#method-load-animation)
- [`SetEvent ( context, frameIndex, callback )`](#method-set-event)
- [`SetPlaybackSpeed ( speed )`](#method-set-playback-speed)

## <a id="brief">Brief</a>

Class is a node for skeletal animation system. Main job of the class is to play animation track from `*.animation` assets which could be created via [_3ds Max plugin_](./3ds-max-exporter.md). Animation track is considered to be looped from timeline perspective.

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

Constructor creates new animation player node.

**Parameters:**

- none

**Example:**

```lua
require "av://engine/animation_player_node.lua"


local animationPlayerNode = AnimationPlayerNode ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-load-animation">`LoadAnimation ( animation )`</a>

Method loads animation track.

**Parameters:**

- `animation` [_required, readonly, string_]: file path to `*.animation` asset which could be created via [_3ds Max plugin_](./3ds-max-exporter.md)

**Return values:**

- `#1` [_required, boolean_]: `true` if animation track was loaded, otherwise `false`

**Example:**

```lua
require "av://engine/animation_player_node.lua"


local animationPlayerNone = AnimationPlayerNode ()
local success = animationPlayerNone:LoadAnimation ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/walk.animation" )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-event">`SetEvent ( context, frameIndex, callback )`</a>

Methods sets event on frame with `frameIndex`. Only single event could be set for each frame index.

`callback` MUST be function with <a id="set-event-callabck">signature</a>:

```lua
function Callback ( context, frameIndex )
```

`context` is the same parameter which was passed via `SetEvent`.

`frameIndex` is zero based event frame index which is provided via `SetEvent`. **Note** this index **IS NOT** real animation position. Often real frame index is slightly bigger that event frame index. For example in situations when animation speed is high and hardware frame rate is low.

Callback function **MUST NOT** return.

According to [event handler calling order](./script-component.md#event-calling-order) animation event will be called during _Animation update_ step.

**Parameters:**

- `context` [_required, readonly, any_]: user provided data which will be passed to callback when ever occurs
- `frameIndex` [_required, readonly, number_]: zero based frame index which must be in range from `0` to `animation frames - 1`
- `callback` [_required, readonly, function_] callback function with signature [above](#set-event-callabck)

**Return values:**

- `#1` [_required, boolean_]: `true` if event is set successfully, otherwise `false`

**Example:**

```lua
require "av://engine/animation_player_node.lua"
require "av://engine/logger.lua"


local function OnStepEvent ( context, frameIndex )
    LogD ( "Footstep event happened." )
end

local animationPlayerNone = AnimationPlayerNode ()
animationPlayerNone:LoadAnimation ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/walk.animation" )
animationPlayerNone:SetEvent ( animationPlayerNode, 42, OnStepEvent )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-playback-speed">`SetPlaybackSpeed ( speed )`</a>

Method sets playback speed.

**Note:** By default playback speed is `1.0`.

**Parameters:**

- `speed` [_required, readonly, number_]: animation speed of track

**Return values:**

- none

**Example:**

```lua
require "av://engine/animation_player_node.lua"


local animationPlayerNone = AnimationPlayerNode ()
animationPlayerNone:SetPlaybackSpeed ( -3.33 )
```

[↬ table of content ⇧](#table-of-content)
