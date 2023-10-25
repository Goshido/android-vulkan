# _BitField_

```lua
require "av://engine/bit_field.lua"
```

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`Clone ( source )`](#method-clone)
- [`ResetAllBits ()`](#method-reset-all-bits)
- [`ResetBit ( bit )`](#method-reset-bit)
- [`SetAllBits ()`](#method-set-all-bits)
- [`SetBit ( bit )`](#method-set-bit)

## <a id="brief">Brief</a>

Class represents 32 bit field. The least significant bit has index `0`. The most significant bit has index `31`. Bitwise operations are performed via overloaded math operators:

- `&` (binary, _AND_)
- `|` (binary, _OR_)
- `~` (binary, _XOR_)
- `~` (unary, _NOT_)
- `==` (binary, _EQUAL_)

[↬ table of content ⇧](#table-of-content)

## <a id="metamethods">Metamethods</a>

Metamethod | Used
--- | ---
`__add` | ❌
`__band` | ✔️
`__bnot` | ✔️
`__bor` | ✔️
`__bxor` | ✔️
`__call` | ✔️
`__close` | ❌
`__concat` | ❌
`__div` | ❌
`__eq` | ✔️
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

Constructor creates new object. All bits are reset.

**Parameters:**

- none

**Example:**

```lua
require "av://engine/bit_field.lua"


local bitField = BitField ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-clone">`Clone ( source )`</a>

Method clones content of bit field to the current bit field.

**Parameters:**

- `source` [_required, readonly, [BitField](#brief)_]: bit field to clone

**Return values:**

- none

**Example:**

```lua
require "av://engine/bit_field.lua"


local srcBitField = BitField ()
srcBitField:SetBit ( 7 )

local dstBitField = BitField ()
dstBitField:Clone ( srcBitField )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-reset-all-bits">`ResetAllBits ()`</a>

Method resets every bit.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/bit_field.lua"


local bitField = BitField ()
bitField:SetBit ( 7 )
bitField:ResetAllBits ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-reset-bit">`ResetBit ( bit )`</a>

Method resets bit by index.

**Parameters:**

- `bit` [_required, readonly, number_]: bit index. Must be in range from `0` to `31`

**Return values:**

- none

**Example:**

```lua
require "av://engine/bit_field.lua"


local bitField = BitField ()
bitField = ~bitField
bitField:ResetBit ( 7 )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-all-bits">`SetAllBits ()`</a>

Method sets every bit.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/bit_field.lua"


local bitField = BitField ()
bitField:SetAllBits ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-bit">`SetBit ( bit )`</a>

Method sets bit by index.

**Parameters:**

- `bit` [_required, readonly, number_]: bit index. Must be in range from `0` to `31`

**Return values:**

- none

**Example:**

```lua
require "av://engine/bit_field.lua"


local bitField = BitField ()
bitField:SetBit ( 7 )
```

[↬ table of content ⇧](#table-of-content)
