# _AnimationPlayerNode_

```lua
require "av://engine/animation_player_node.lua"
```

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`LoadAnimation ( animation )`](#method-load-animation)
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
