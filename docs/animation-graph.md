# _Animation Graph_

```lua
require "av://engine/animation_graph.lua"
```

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`Awake ()`](#method-awake)
- [`SetInput ( inputNode )`](#method-set-input)
- [`Sleep ()`](#method-sleep)

## <a id="brief">Brief</a>

_Animation graph_ is the central part of the skeletal animation system. _Animation graph_ job is to compute pose for skeletal meshes. _Animation graph_ could be attached to multiple [_SkeletalMeshComponents_](./skeletal-mesh-component.md). _Animation graph_ has single input which must provide joints for pose computation. The following entities could be inputs for _Animation graph_:

- [_AnimationPlayerNode_](./animation-player-node.md)

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

Constructor creates new object.

**Parameters:**

- `skeletonFile` [_required, readonly, string_]: file path to `*.skeleton` asset which could be created via [_3ds Max plugin_](./3ds-max-exporter.md)

**Example:**

```lua
require "av://engine/animation_graph.lua"


local animationGraph = AnimationGraph ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton" )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-awake">`Awake ()`</a>

Method forces graph to compute new pose each frame.

**Note:** Animation graph is in sleep mode by default.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/animation_graph.lua"
require "av://engine/animation_player_node.lua"


local animationGraph = AnimationGraph ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton" )

local walkAnimationPlayer = AnimationPlayerNode ()
walkAnimationPlayer:LoadAnimation ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/walk.animation" )

animationGraph:SetInput ( walkAnimationPlayer )
animationGraph:Awake ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-input">`SetInput ( inputNode )`</a>

Method sets input node as joint provider for pose calculation.

**Parameters:**

- `inputNode` [_required, readonly, [AnimationPlayerNode](./animation-player-node.md) or [AnimationBlendNode](./animation-blend-node.md)_]: input node

**Return values:**

- none

**Example:**

```lua
require "av://engine/animation_graph.lua"
require "av://engine/animation_player_node.lua"


local animationGraph = AnimationGraph ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton" )

local walkAnimationPlayer = AnimationPlayerNode ()
walkAnimationPlayer:LoadAnimation ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/walk.animation" )

animationGraph:SetInput ( walkAnimationPlayer )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-sleep">`Sleep ()`</a>

Method puts animation graph in sleep mode. This means that animation graph will not compute new pose each frame.

**Note:** Animation graph is in sleep mode by default.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/animation_graph.lua"
require "av://engine/animation_player_node.lua"


local animationGraph = AnimationGraph ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/human.skeleton" )

local walkAnimationPlayer = AnimationPlayerNode ()
walkAnimationPlayer:LoadAnimation ( "pbr/assets/Props/experimental/skeletal-mesh-sandbox/human/walk.animation" )

animationGraph:SetInput ( walkAnimationPlayer )
animationGraph:Sleep ()
```

[↬ table of content ⇧](#table-of-content)
