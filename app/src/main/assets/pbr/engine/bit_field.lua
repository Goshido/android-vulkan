require "av://engine/object.lua"


BitField = {}

-- Methods
local function Clone ( self, source )
    assert ( type ( self ) == "table" and self._type == eObjectType.BitField,
        [[BitField:Clone - Calling not via ":" syntax.]]
    )

    assert ( type ( source ) == "table" and source._type == eObjectType.BitField,
        [[BitField:Clone - "source" is not BitField.]]
    )

    av_BitFieldClone ( self._handle, source._handle )
end

local function ResetAllBits ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.BitField,
        [[BitField:ResetAllBits - Calling not via ":" syntax.]]
    )

    av_BitFieldResetAllBits ( self._handle )
end

local function ResetBit ( self, bit )
    assert ( type ( self ) == "table" and self._type == eObjectType.BitField,
        [[BitField:ResetBit - Calling not via ":" syntax.]]
    )

    assert ( type ( bit ) == "number", [[BitField:ResetBit - "bit" is not number.]] )
    av_BitFieldResetBit ( self._handle, bit )
end

local function SetAllBits ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.BitField,
        [[BitField:SetAllBits - Calling not via ":" syntax.]]
    )

    av_BitFieldSetAllBits ( self._handle )
end

local function SetBit ( self, bit )
    assert ( type ( self ) == "table" and self._type == eObjectType.BitField,
        [[BitField:SetBit - Calling not via ":" syntax.]]
    )

    assert ( type ( bit ) == "number", [[BitField:SetBit - "bit" is not number.]] )
    av_BitFieldSetBit ( self._handle, bit )
end

-- Metamethods
local mt = {
    __band = function ( left, right )
        assert ( type ( right ) == "table" and right._type == eObjectType.BitField,
            [[BitField:__band - operand from right is not BitField.]]
        )

        return av_BitFieldAnd ( left._handle, right._handle )
    end,

    __eq = function ( left, right )
        assert ( type ( right ) == "table" and right._type == eObjectType.BitField,
            [[BitField:__eq - operand from right is not BitField.]]
        )

        return av_BitFieldEqual ( left._handle, right._handle )
    end,

    __bnot = function ( self )
        return av_BitFieldNot ( self._handle )
    end,

    __bor = function ( left, right )
        assert ( type ( right ) == "table" and right._type == eObjectType.BitField,
            [[BitField:__bor - operand from right is not BitField.]]
        )

        return av_BitFieldOr ( left._handle, right._handle )
    end,

    __bxor = function ( left, right )
        assert ( type ( right ) == "table" and right._type == eObjectType.BitField,
            [[BitField:__bxor - operand from right is not BitField.]]
        )

        return av_BitFieldXor ( left._handle, right._handle )
    end,

    __gc = function ( self )
        av_BitFieldDestroy ( self._handle )
    end
}

local function Constructor ( self )
    local obj = Object ( eObjectType.BitField )

    -- Data
    obj._handle = av_BitFieldCreate ()

    -- Methods
    obj.Clone = Clone
    obj.ResetAllBits = ResetAllBits
    obj.ResetBit = ResetBit
    obj.SetAllBits = SetAllBits
    obj.SetBit = SetBit

    return setmetatable ( obj, mt )
end

setmetatable ( BitField, { __call = Constructor } )

-- Module contract
return nil
