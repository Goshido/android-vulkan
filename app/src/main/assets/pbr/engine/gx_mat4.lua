require "av://engine/av_object.lua"
require "av://engine/gx_vec3.lua"


GXMat4 = {}

-- methods
local function Identity ( self )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:Identity - Calling not via ":" syntax.]]
    )

    av_GXMat4Identity ( self._handle )
end

local function Inverse ( self, sourceMatrix )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:Inverse - Calling not via ":" syntax.]]
    )

    assert ( type ( sourceMatrix ) == "table" and sourceMatrix._type == eAVObjectType.GXMat4,
        [[GXMat4:Inverse - "sourceMatrix" is not a GXMat4.]]
    )

    av_GXMat4Inverse ( self._handle, sourceMatrix._handle )
end

local function Multiply ( self, a, b )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:Multiply - Calling not via ":" syntax.]]
    )

    assert ( type ( a ) == "table" and a._type == eAVObjectType.GXMat4, [[GXMat4:Multiply - "a" is not a GXMat4.]] )
    assert ( type ( b ) == "table" and b._type == eAVObjectType.GXMat4, [[GXMat4:Multiply - "b" is not a GXMat4.]] )

    av_GXMat4Multiply ( self._handle, a._handle, b._handle )
end

local function MultiplyAsNormal ( self, out, v )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:MultiplyAsNormal - Calling not via ":" syntax.]]
    )

    assert ( type ( out ) == "table" and out._type == eAVObjectType.GXVec3,
        [[GXMat4:MultiplyAsNormal - "out" is not a GXVec3.]]
    )

    assert ( type ( v ) == "table" and v._type == eAVObjectType.GXVec3,
        [[GXMat4:MultiplyAsNormal - "v" is not a GXVec3.]]
    )

    av_GXMat4MultiplyAsNormal ( self._handle, out._handle, v._handle )
end

local function MultiplyAsPoint ( self, out, v )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:MultiplyAsPoint - Calling not via ":" syntax.]]
    )

    assert ( type ( out ) == "table" and out._type == eAVObjectType.GXVec3,
        [[GXMat4:MultiplyAsPoint - "out" is not a GXVec3.]]
    )

    assert ( type ( v ) == "table" and v._type == eAVObjectType.GXVec3,
        [[GXMat4:MultiplyAsPoint - "v" is not a GXVec3.]]
    )

    av_GXMat4MultiplyAsPoint ( self._handle, out._handle, v._handle )
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

local function TranslationF ( self, x, y, z )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:TranslationF - Calling not via ":" syntax]]
    )

    assert ( type ( x ) == "number", [[GXMat4:TranslationF - "x" is not a number.]] )
    assert ( type ( y ) == "number", [[GXMat4:TranslationF - "y" is not a number.]] )
    assert ( type ( z ) == "number", [[GXMat4:TranslationF - "z" is not a number.]] )

    av_GXMat4TranslationF ( self._handle, x, y, z )
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
    obj.Inverse = Inverse
    obj.Multiply = Multiply
    obj.MultiplyAsNormal = MultiplyAsNormal
    obj.MultiplyAsPoint = MultiplyAsPoint
    obj.Perspective = Perspective
    obj.RotationX = RotationX
    obj.Scale = Scale
    obj.TranslationF = TranslationF

    return setmetatable ( obj, obj )
end

setmetatable ( GXMat4, { __call = Constructor } )

-- module contract
return nil
