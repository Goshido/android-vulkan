require "av://engine/gx_vec3.lua"
require "av://engine/gx_vec4.lua"


GXMat4 = {}

-- methods
local function FromFast ( self, unitQuaternion, origin )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:FromFast - Calling not via ":" syntax.]]
    )

    assert ( type ( unitQuaternion ) == "table" and unitQuaternion._type == eAVObjectType.GXQuat,
        [[GXMat4:FromFast - "unitQuaternion" is not a GXQuat.]]
    )

    assert ( type ( origin ) == "table" and origin._type == eAVObjectType.GXVec3,
        [[GXMat4:FromFast - "origin" is not a GXVec3.]]
    )

    av_GXMat4FromFast ( self._handle, unitQuaternion._handle, origin._handle )
end

local function GetX ( self, x )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:GetX - Calling not via ":" syntax.]]
    )

    assert ( type ( x ) == "table" and x._type == eAVObjectType.GXVec3, [[GXMat4:GetX - "x" is not a GXVec3.]] )
    av_GXMat4GetX ( self._handle, x._handle )
end

local function GetY ( self, y )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:GetY - Calling not via ":" syntax.]]
    )

    assert ( type ( y ) == "table" and y._type == eAVObjectType.GXVec3, [[GXMat4:GetY - "y" is not a GXVec3.]] )
    av_GXMat4GetY ( self._handle, y._handle )
end

local function GetZ ( self, z )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:GetZ - Calling not via ":" syntax.]]
    )

    assert ( type ( z ) == "table" and z._type == eAVObjectType.GXVec3, [[GXMat4:GetZ - "z" is not a GXVec3.]] )
    av_GXMat4GetZ ( self._handle, z._handle )
end

local function GetW ( self, w )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:GetW - Calling not via ":" syntax.]]
    )

    assert ( type ( w ) == "table" and w._type == eAVObjectType.GXVec3, [[GXMat4:GetW - "w" is not a GXVec3.]] )
    av_GXMat4GetW ( self._handle, w._handle )
end

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

local function MultiplyMatrixVector ( self, out, v )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:MultiplyMatrixVector - Calling not via ":" syntax.]]
    )

    assert ( type ( out ) == "table" and out._type == eAVObjectType.GXVec4,
        [[GXMat4:MultiplyMatrixVector - "out" is not a GXVec4.]]
    )

    assert ( type ( v ) == "table" and v._type == eAVObjectType.GXVec4,
        [[GXMat4:MultiplyMatrixVector - "v" is not a GXVec4.]]
    )

    av_GXMat4MultiplyMatrixVector ( self._handle, out._handle, v._handle )
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

local function MultiplyVectorMatrix ( self, out, v )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:MultiplyVectorMatrix - Calling not via ":" syntax.]]
    )

    assert ( type ( out ) == "table" and out._type == eAVObjectType.GXVec4,
        [[GXMat4:MultiplyVectorMatrix - "out" is not a GXVec4.]]
    )

    assert ( type ( v ) == "table" and v._type == eAVObjectType.GXVec4,
        [[GXMat4:MultiplyVectorMatrix - "v" is not a GXVec4.]]
    )

    av_GXMat4MultiplyVectorMatrix ( self._handle, out._handle, v._handle )
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

local function RotationY ( self, angle )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:RotationY - Calling not via ":" syntax.]]
    )

    assert ( type ( angle ) == "number", [[GXMat4:RotationY - "angle" is not a number.]] )
    av_GXMat4RotationY ( self._handle, angle )
end

local function RotationZ ( self, angle )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:RotationZ - Calling not via ":" syntax.]]
    )

    assert ( type ( angle ) == "number", [[GXMat4:RotationZ - "angle" is not a number.]] )
    av_GXMat4RotationZ ( self._handle, angle )
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

local function SetX ( self, x )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:SetX - Calling not via ":" syntax.]]
    )

    assert ( type ( x ) == "table" and x._type == eAVObjectType.GXVec3, [[GXMat4:SetX - "x" is not a GXVec3.]] )
    av_GXMat4SetX ( self._handle, x._handle )
end

local function SetY ( self, y )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:SetY - Calling not via ":" syntax.]]
    )

    assert ( type ( y ) == "table" and y._type == eAVObjectType.GXVec3, [[GXMat4:SetY - "y" is not a GXVec3.]] )
    av_GXMat4SetY ( self._handle, y._handle )
end

local function SetZ ( self, z )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:SetZ - Calling not via ":" syntax.]]
    )

    assert ( type ( z ) == "table" and z._type == eAVObjectType.GXVec3, [[GXMat4:SetZ - "z" is not a GXVec3.]] )
    av_GXMat4SetZ ( self._handle, z._handle )
end

local function SetW ( self, w )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:SetW - Calling not via ":" syntax.]]
    )

    assert ( type ( w ) == "table" and w._type == eAVObjectType.GXVec3, [[GXMat4:SetW - "w" is not a GXVec3.]] )
    av_GXMat4SetW ( self._handle, w._handle )
end

local function Translation ( self, x, y, z )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat4,
        [[GXMat4:Translation - Calling not via ":" syntax]]
    )

    assert ( type ( x ) == "number", [[GXMat4:Translation - "x" is not a number.]] )
    assert ( type ( y ) == "number", [[GXMat4:Translation - "y" is not a number.]] )
    assert ( type ( z ) == "number", [[GXMat4:Translation - "z" is not a number.]] )

    av_GXMat4Translation ( self._handle, x, y, z )
end

-- metamethods
local mt = {
    __concat = function ( left, right )
        return string.format ( "%s%s", left, right )
    end,

    __gc = function ( self )
        av_GXMat4Destroy ( self._handle )
    end,

    __tostring = function ( self )
        return av_GXMat4ToString ( self._handle )
    end
}

local function Constructor ( self )
    local obj = AVObject ( eAVObjectType.GXMat4 )

    -- data
    obj._handle = av_GXMat4Create ()

    -- methods
    obj.FromFast = FromFast
    obj.GetX = GetX
    obj.GetY = GetY
    obj.GetZ = GetZ
    obj.GetW = GetW
    obj.Identity = Identity
    obj.Inverse = Inverse
    obj.Multiply = Multiply
    obj.MultiplyMatrixVector = MultiplyMatrixVector
    obj.MultiplyAsNormal = MultiplyAsNormal
    obj.MultiplyAsPoint = MultiplyAsPoint
    obj.MultiplyVectorMatrix = MultiplyVectorMatrix
    obj.Perspective = Perspective
    obj.RotationX = RotationX
    obj.RotationY = RotationY
    obj.RotationZ = RotationZ
    obj.Scale = Scale
    obj.SetX = SetX
    obj.SetY = SetY
    obj.SetZ = SetZ
    obj.SetW = SetW
    obj.Translation = Translation

    return setmetatable ( obj, mt )
end

setmetatable ( GXMat4, { __call = Constructor } )

-- module contract
return nil
