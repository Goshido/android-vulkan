require "av://engine/av_object.lua"


GXMat4 = {}

-- methods
local function Identity ( self )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:Identity - Calling not via ":" syntax.]]
    )

    av_GXMat4Identity ( self._handle )
end

local function Perspective ( self, fieldOfViewYRadians, aspectRatio, zNear, zFar )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:Perspective - Calling not via ":" syntax.]]
    )

    assert ( type ( fieldOfViewYRadians ) == "number",
        [[GXMat4:Perspective - "fieldOfViewYRadians"" is not a number.]]
    )

    assert ( type ( aspectRatio ) == "number", [[GXMat4:Perspective - "aspectRatio" is not a number.]] )
    assert ( type ( zNear ) == "number", [[GXMat4:Perspective - "zNear" is not a number.]] )
    assert ( type ( zFar ) == "number", [[GXMat4:Perspective - "zFar" is not a number.]] )

    av_GXMat4Perspective ( self._handle, fieldOfViewYRadians, aspectRatio, zNear, zFar )
end

local function RotationX ( self, angle )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:RotationX - Calling not via ":" syntax.]]
    )

    assert ( type ( angle ) == "number", [[GXMat4:RotationX - "angle" is not a number.]] )
    av_GXMat4RotationX ( self._handle, angle )
end

local function Scale ( self, x, y, z )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:Scale - Calling not via ":" syntax]]
    )

    assert ( type ( x ) == "number", [[GXMat4:Scale - "x" is not a number.]] )
    assert ( type ( y ) == "number", [[GXMat4:Scale - "y" is not a number.]] )
    assert ( type ( z ) == "number", [[GXMat4:Scale - "z" is not a number.]] )

    av_GXMat4Scale ( self._handle, x, y, z )
end

-- metamethods
local function OnConcat ( left, right )
    return string.format ( "%s%s", left, right )
end

local function OnCG ( self )
    av_GXMat4Destroy ( self._handle )
end

local function OnToString ( self )
    return av_GXMat4ToString ( self._handle )
end

local function Constructor ( self )
    local obj = AVObject ( eAVObjectType.GXMat4 )

    -- data
    obj._handle = av_GXMat4Create ()

    -- metamethods
    obj.__concat = OnConcat
    obj.__gc = OnCG
    obj.__tostring = OnToString

    -- methods
    obj.Identity = Identity
    obj.Perspective = Perspective
    obj.RotationX = RotationX
    obj.Scale = Scale

    return setmetatable ( obj, obj )
end

setmetatable ( GXMat4, { __call = Constructor } )

-- module contract
return nil
