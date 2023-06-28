# _ImageUIElement_

```lua
require "av://engine/image_ui_element.lua"
```


## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`Hide ()`](#method-hide)
- [`Show ()`](#method-show)

## <a id="brief">Brief</a>

Class allows to manipulate [_IMG_](https://developer.mozilla.org/en-US/docs/Web/HTML/Element/img) element inside [_UI_ system](ui-system.md) assets.

```lua
require "av://engine/image_ui_element.lua"
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )
local image = uiLayer:Find ( "coin" )
```

[↬ table of content ⇧](#table-of-content)

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

_N/A_

[↬ table of content ⇧](#table-of-content)

## <a id="method-hide">`Hide ()`</a>

Method hides element.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/image_ui_element.lua"
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )

local image = uiLayer:Find ( "coin" )
image:Hide ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-show">`Show ()`</a>

Method shows element.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/image_ui_element.lua"
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )

local image = uiLayer:Find ( "coin" )
image:Show ()
```

[↬ table of content ⇧](#table-of-content)
