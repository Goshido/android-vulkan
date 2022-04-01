require "av://engine/av_object.lua"


GXVec3 = {}

-- methods
local function Init ( self, x, y, z )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXVec3,
        [[GXVec3:Init - Calling not via ":" syntax.]]
    )

    assert ( type ( x ) == "number", [[GXVec3:Init - "x" is not a number.]] )
    assert ( type ( y ) == "number", [[GXVec3:Init - "y" is not a number.]] )
    assert ( type ( z ) == "number", [[GXVec3:Init - "z" is not a number.]] )

    av_GXVec3Init ( self._handle, x, y, z )
end

-- metamethods
local function OnConcat ( left, right )
    return string.format ( "%s%s", left, right )
end

local function OnCG ( self )
    av_GXVec3Destroy ( self._handle )
end

local function OnToString ( self )
    return av_GXVec3ToString ( self._handle )
end

local function Constructor ( self )
    local obj = AVObject ( eAVObjectType.GXVec3 )

    -- data
    obj._handle = av_GXVec3Create ()

    -- metamethods
    obj.__concat = OnConcat
    obj.__gc = OnCG
    obj.__tostring = OnToString

    -- methods
    obj.Init = Init

    return setmetatable ( obj, obj )
end

setmetatable ( GXVec3, { __call = Constructor } )

-- module contract
return nil
