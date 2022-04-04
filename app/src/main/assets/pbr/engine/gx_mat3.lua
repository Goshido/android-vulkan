require "av://engine/gx_vec3.lua"


GXMat3 = {}

-- methods
local function GetX ( self, x )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:GetX - Calling not via ":" syntax.]]
    )

    assert ( type ( x ) == "table" and x._type == eAVObjectType.GXVec3, [[GXMat4:GetX - "x" is not a GXVec3.]] )
    av_GXMat3GetX ( self._handle, x._handle )
end

local function GetY ( self, y )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:GetY - Calling not via ":" syntax.]]
    )

    assert ( type ( y ) == "table" and y._type == eAVObjectType.GXVec3, [[GXMat3:GetY - "y" is not a GXVec3.]] )
    av_GXMat3GetY ( self._handle, y._handle )
end

local function GetZ ( self, z )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:GetZ - Calling not via ":" syntax.]]
    )

    assert ( type ( z ) == "table" and z._type == eAVObjectType.GXVec3, [[GXMat3:GetZ - "z" is not a GXVec3.]] )
    av_GXMat3GetZ ( self._handle, z._handle )
end

local function Identity ( self )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:Identity - Calling not via ":" syntax.]]
    )

    av_GXMat3Identity ( self._handle )
end

-- metamethods
local function OnConcat ( left, right )
    return string.format ( "%s%s", left, right )
end

local function OnCG ( self )
    av_GXMat3Destroy ( self._handle )
end

local function OnToString ( self )
    return av_GXMat3ToString ( self._handle )
end

local function Constructor ( self )
    local obj = AVObject ( eAVObjectType.GXMat3 )

    -- data
    obj._handle = av_GXMat3Create ()

    -- metamethods
    obj.__concat = OnConcat
    obj.__gc = OnCG
    obj.__tostring = OnToString

    -- methods
    obj.GetX = GetX
    obj.GetY = GetY
    obj.GetZ = GetZ
    obj.Identity = Identity

    return setmetatable ( obj, obj )
end

setmetatable ( GXMat3, { __call = Constructor } )

-- module contract
return nil
