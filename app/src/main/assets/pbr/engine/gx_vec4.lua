require "av://engine/object.lua"


GXVec4 = {}

-- Methods
local function Init ( self, x, y, z, w )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec4,
        [[GXVec4:Init - Calling not via ":" syntax.]]
    )

    assert ( type ( x ) == "number", [[GXVec4:Init - "x" is not number.]] )
    assert ( type ( y ) == "number", [[GXVec4:Init - "y" is not number.]] )
    assert ( type ( z ) == "number", [[GXVec4:Init - "z" is not number.]] )
    assert ( type ( w ) == "number", [[GXVec4:Init - "w" is not number.]] )

    av_GXVec4Init ( self._handle, x, y, z, w )
end

-- Metamethods
local mt = {
    __concat = function ( left, right )
        return string.format ( "%s%s", left, right )
    end,

    __gc = function ( self )
        av_GXVec4Destroy ( self._handle )
    end,

    __tostring = function ( self )
        return av_GXVec4ToString ( self._handle )
    end
}

local function Constructor ( self )
    local obj = Object ( eObjectType.GXVec4 )

    -- Data
    obj._handle = av_GXVec4Create ()

    -- Methods
    obj.Init = Init

    return setmetatable ( obj, mt )
end

setmetatable ( GXVec4, { __call = Constructor } )

-- Module contract
return nil
