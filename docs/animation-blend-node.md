# _AnimationBlendNode_

```lua
require "av://engine/animation_blend_node.lua"
```

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`SetBlendFactor ( factor )`](#method-set-blend-factor)
- [`SetInputA ( inputA )`](#method-set-input-a)
- [`SetInputB ( inputB )`](#method-set-input-b)

## <a id="brief">Brief</a>

This class is for skeletal animation system. The class could blend two input animations to produce single animation.

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

Constructor creates new animation blend node.

**Parameters:**

- none

**Example:**

```lua
require "av://engine/animation_blend_node.lua"


local animationBlendNode = AnimationBlendNode ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-blend-factor">`SetBlendFactor ( factor )`</a>

Method set blend factor between input animations. `0.0` factor corresponds input animation _A_. `1.0` factor corresponds input animation _B_. Values between range will produce various mixes between input animations.

**Parameters:**

- `factor` [_required, readonly, number_]: blend factor between animations in range from `0.0` to `1.0`

**Return values:**

- none

**Example:**

```lua
require "av://engine/animation_blend_node.lua"
require "av://engine/animation_player_node.lua"


local runAnimationPlayer = AnimationPlayerNode ()
runAnimationPlayer:LoadAnimation ( "pbr/assets/Props/experimental/character-sandbox/bobby/run.animation" )

local idleAnimationPlayer = AnimationPlayerNode ()
idleAnimationPlayer:LoadAnimation ( "pbr/assets/Props/experimental/character-sandbox/bobby/idle.animation" )

local animationBlend = AnimationBlendNode ()
animationBlend:SetBlendFactor ( 0.5 )
animationBlend:SetInputA ( idleAnimationPlayer )
animationBlend:SetInputB ( runAnimationPlayer )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-input-a">`SetInputA ( inputA )`</a>

Method sets input A node as joint provider for pose calculation.

**Parameters:**

- `inputA` [_required, readonly, [AnimationPlayerNode](./animation-player-node.md) or [AnimationBlendNode](./animation-blend-node.md)_]: input node A

**Return values:**

- none

**Example:**

```lua
require "av://engine/animation_blend_node.lua"
require "av://engine/animation_player_node.lua"


local idleAnimationPlayer = AnimationPlayerNode ()
idleAnimationPlayer:LoadAnimation ( "pbr/assets/Props/experimental/character-sandbox/bobby/idle.animation" )

local animationBlend = AnimationBlendNode ()
animationBlend:SetInputA ( idleAnimationPlayer )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-input-b">`SetInputB ( inputB )`</a>

Method sets input B node as joint provider for pose calculation.

**Parameters:**

- `inputB` [_required, readonly, [AnimationPlayerNode](./animation-player-node.md) or [AnimationBlendNode](./animation-blend-node.md)_]: input node A

**Return values:**

- none

**Example:**

```lua
require "av://engine/animation_blend_node.lua"
require "av://engine/animation_player_node.lua"


local runAnimationPlayer = AnimationPlayerNode ()
runAnimationPlayer:LoadAnimation ( "pbr/assets/Props/experimental/character-sandbox/bobby/run.animation" )

local animationBlend = AnimationBlendNode ()
animationBlend:SetInputB ( runAnimationPlayer )
```

[↬ table of content ⇧](#table-of-content)
