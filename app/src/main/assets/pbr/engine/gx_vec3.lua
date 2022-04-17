require "av://engine/object.lua"


GXVec3 = {}

-- methods
local function CrossProduct ( self, a, b )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec3,
        [[GXVec3:CrossProduct - Calling not via ":" syntax.]]
    )

    assert ( type ( a ) == "table" and a._type == eObjectType.GXVec3, [[GXVec3:CrossProduct - "a" is not a GXVec3.]] )
    assert ( type ( b ) == "table" and b._type == eObjectType.GXVec3, [[GXVec3:CrossProduct - "b" is not a GXVec3.]] )

    av_GXVec3CrossProduct ( self._handle, a._handle, b._handle )
end

local function Distance ( self, other )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec3,
        [[GXVec3:Distance - Calling not via ":" syntax.]]
    )

    assert ( type ( other ) == "table" and other._type == eObjectType.GXVec3,
        [[GXVec3:Distance - "other" is not a GXVec3.]]
    )

    return av_GXVec3Distance ( self._handle, other._handle )
end

local function DotProduct ( self, other )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec3,
        [[GXVec3:DotProduct - Calling not via ":" syntax.]]
    )

    assert ( type ( other ) == "table" and other._type == eObjectType.GXVec3,
        [[GXVec3:DotProduct - "other" is not a GXVec3.]]
    )

    return av_GXVec3DotProduct ( self._handle, other._handle )
end

local function Init ( self, x, y, z )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec3,
        [[GXVec3:Init - Calling not via ":" syntax.]]
    )

    assert ( type ( x ) == "number", [[GXVec3:Init - "x" is not a number.]] )
    assert ( type ( y ) == "number", [[GXVec3:Init - "y" is not a number.]] )
    assert ( type ( z ) == "number", [[GXVec3:Init - "z" is not a number.]] )

    av_GXVec3Init ( self._handle, x, y, z )
end

local function Length ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec3,
        [[GXVec3:Length - Calling not via ":" syntax.]]
    )

    return av_GXVec3Length ( self._handle )
end

local function Normalize ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec3,
        [[GXVec3:Normalize - Calling not via ":" syntax.]]
    )

    av_GXVec3Normalize ( self._handle )
end

local function Reverse ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec3,
        [[GXVec3:Reverse - Calling not via ":" syntax.]]
    )

    av_GXVec3Reverse ( self._handle )
end

local function SquaredDistance ( self, other )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec3,
        [[GXVec3:SquaredDistance - Calling not via ":" syntax.]]
    )

    assert ( type ( other ) == "table" and other._type == eObjectType.GXVec3,
        [[GXVec3:SquaredDistance - "other" is not a GXVec3.]]
    )

    return av_GXVec3SquaredDistance ( self._handle, other._handle )
end

local function SquaredLength ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec3,
        [[GXVec3:SquaredLength - Calling not via ":" syntax.]]
    )

    return av_GXVec3SquaredLength ( self._handle )
end

local function Subtract ( self, a, b )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec3,
        [[GXVec3:Subtract - Calling not via ":" syntax.]]
    )

    assert ( type ( a ) == "table" and a._type == eObjectType.GXVec3, [[GXVec3:Subtract - "a" is not a GXVec3.]] )
    assert ( type ( b ) == "table" and b._type == eObjectType.GXVec3, [[GXVec3:Subtract - "b" is not a GXVec3.]] )

    av_GXVec3Subtract ( self._handle, a._handle, b._handle )
end

local function Sum ( self, a, b )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec3,
        [[GXVec3:Sum - Calling not via ":" syntax.]]
    )

    assert ( type ( a ) == "table" and a._type == eObjectType.GXVec3, [[GXVec3:Sum - "a" is not a GXVec3.]] )
    assert ( type ( b ) == "table" and b._type == eObjectType.GXVec3, [[GXVec3:Sum - "b" is not a GXVec3.]] )

    av_GXVec3Sum ( self._handle, a._handle, b._handle )
end

local function SumScaled ( self, a, bScale, b )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXVec3,
        [[GXVec3:SumScaled - Calling not via ":" syntax.]]
    )

    assert ( type ( a ) == "table" and a._type == eObjectType.GXVec3, [[GXVec3:SumScaled - "a" is not a GXVec3.]] )
    assert ( type ( bScale ) == "number", [[GXVec3:SumScaled - "bScale" is not a number.]] )
    assert ( type ( b ) == "table" and b._type == eObjectType.GXVec3, [[GXVec3:SumScaled - "b" is not a GXVec3.]] )

    av_GXVec3SumScaled ( self._handle, a._handle, bScale, b._handle )
end

-- metamethods
local mt = {
    __concat = function ( left, right )
        return string.format ( "%s%s", left, right )
    end,

    __gc = function ( self )
        av_GXVec3Destroy ( self._handle )
    end,

    __tostring = function ( self )
        return av_GXVec3ToString ( self._handle )
    end
}

local function Constructor ( self )
    local obj = Object ( eObjectType.GXVec3 )

    -- data
    obj._handle = av_GXVec3Create ()

    -- methods
    obj.CrossProduct = CrossProduct
    obj.Distance = Distance
    obj.DotProduct = DotProduct
    obj.Init = Init
    obj.Length = Length
    obj.Normalize = Normalize
    obj.Reverse = Reverse
    obj.SquaredDistance = SquaredDistance
    obj.SquaredLength = SquaredLength
    obj.Subtract = Subtract
    obj.Sum = Sum
    obj.SumScaled = SumScaled

    return setmetatable ( obj, mt )
end

setmetatable ( GXVec3, { __call = Constructor } )

-- module contract
return nil
