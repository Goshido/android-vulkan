require "av://engine/av_object.lua"


GXVec4 = {}

-- methods
local function Init ( self, x, y, z, w )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXVec4,
        [[GXVec4:Init - Calling not via ":" syntax.]]
    )

    assert ( type ( x ) == "number", [[GXVec4:Init - "x" is not a number.]] )
    assert ( type ( y ) == "number", [[GXVec4:Init - "y" is not a number.]] )
    assert ( type ( z ) == "number", [[GXVec4:Init - "z" is not a number.]] )
    assert ( type ( w ) == "number", [[GXVec4:Init - "w" is not a number.]] )

    av_GXVec4Init ( self._handle, x, y, z, w )
end

-- metamethods
local function OnConcat ( left, right )
    return string.format ( "%s%s", left, right )
end

local function OnCG ( self )
    av_GXVec4Destroy ( self._handle )
end

local function OnToString ( self )
    return av_GXVec4ToString ( self._handle )
end

local function Constructor ( self )
    local obj = AVObject ( eAVObjectType.GXVec4 )

    -- data
    obj._handle = av_GXVec4Create ()

    -- metamethods
    obj.__concat = OnConcat
    obj.__gc = OnCG
    obj.__tostring = OnToString

    -- methods
    obj.Init = Init

    return setmetatable ( obj, obj )
end

setmetatable ( GXVec4, { __call = Constructor } )

-- module contract
return nil
