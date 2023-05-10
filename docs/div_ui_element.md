# _DIVUIElement_

```lua
require "av://engine/div_ui_element.lua"
```


## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`GetTextElement ()`](#method-get-text-element)
- [`Hide ()`](#method-hide)
- [`Show ()`](#method-show)

## <a id="brief">Brief</a>

Class allows to manipulate [_DIV_](https://developer.mozilla.org/en-US/docs/Web/HTML/Element/div) element inside [_UI_ system](ui-system.md) assets.

**Note:** The _HTML_ body element could be treated as [_DIV_](https://developer.mozilla.org/en-US/docs/Web/HTML/Element/div) element.

```lua
require "av://engine/text_ui_element.lua"
require "av://engine/div_ui_element.lua"


local uiLayer = UILayer ( "example/ui/index.html" )
local div = uiLayer:Find ( "main-widget" )
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

_N/A_

[↬ table of content ⇧](#table-of-content)

## <a id="method-get-text-element">`GetTextElement ()`</a>

Methods returns **FIRST** child [_TextUIElement_](./text-ui-element.md) of current [_DIV_](https://developer.mozilla.org/en-US/docs/Web/HTML/Element/div) element. _HTML_ doesn't provide any ways to create named text elements. So this method is the way to overcome that rule.

**Parameters:**

- none

**Return values:**

- `#1` [_required, [_TextUIElement_](./text-ui-element.md) or nil_]: first found [_TextUIElement_](./text-ui-element.md) if it exists. Otherwise `nil`.

**Example:**

```lua
require "av://engine/div_ui_element.lua"
require "av://engine/text_ui_element.lua"
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )

local div = uiLayer:Find ( "main-widget" )
local text = div:GetTextElement ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-hide">`Hide ()`</a>

Method hides element.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/div_ui_element.lua"
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )

local div = uiLayer:Find ( "main-widget" )
div:Hide ()
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
require "av://engine/div_ui_element.lua"
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )

local div = uiLayer:Find ( "main-widget" )
div:Show ()
```

[↬ table of content ⇧](#table-of-content)
