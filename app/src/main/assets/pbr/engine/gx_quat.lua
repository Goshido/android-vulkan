require "av://engine/gx_vec3.lua"


GXQuat = {}

-- Methods
local function FromAxisAngle ( self, axis, angle )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXQuat,
        [[GXQuat:FromAxisAngle - Calling not via ":" syntax.]]
    )

    assert ( type ( axis ) == "table" and axis._type == eObjectType.GXVec3,
        [[GXQuat:FromAxisAngle - "axis" is not a GXVec3.]]
    )

    assert ( type ( angle ) == "number", [[GXVec3:FromAxisAngle - "angle" is not number.]] )

    av_GXQuatFromAxisAngle ( self._handle, axis._handle, angle )
end

local function Inverse ( self, q )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXQuat,
        [[GXQuat:Inverse - Calling not via ":" syntax.]]
    )

    assert ( type ( q ) == "table" and q._type == eObjectType.GXQuat, [[GXQuat:Inverse - "q" is not a GXQuat.]] )

    av_GXQuatInverse ( self._handle, q._handle )
end

local function InverseFast ( self, unitQuaternion )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXQuat,
        [[GXQuat:InverseFast - Calling not via ":" syntax.]]
    )

    assert ( type ( unitQuaternion ) == "table" and unitQuaternion._type == eObjectType.GXQuat,
        [[GXQuat:InverseFast - "unitQuaternion" is not a GXQuat.]]
    )

    av_GXQuatInverseFast ( self._handle, unitQuaternion._handle )
end

local function Multiply ( self, a, b )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXQuat,
        [[GXQuat:Multiply - Calling not via ":" syntax.]]
    )

    assert ( type ( a ) == "table" and a._type == eObjectType.GXQuat, [[GXQuat:Multiply - "a" is not a GXQuat.]] )
    assert ( type ( b ) == "table" and b._type == eObjectType.GXQuat, [[GXQuat:Multiply - "b" is not a GXQuat.]] )

    av_GXQuatMultiply ( self._handle, a._handle, b._handle )
end

local function Normalize ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXQuat,
        [[GXQuat:Normalize - Calling not via ":" syntax.]]
    )

    av_GXQuatNormalize ( self._handle )
end

local function SphericalLinearInterpolation ( self, start, finish, interpolationFactor )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXQuat,
        [[GXQuat:SphericalLinearInterpolation - Calling not via ":" syntax.]]
    )

    assert ( type ( start ) == "table" and start._type == eObjectType.GXQuat,
        [[GXQuat:SphericalLinearInterpolation - "start" is not a GXQuat.]]
    )

    assert ( type ( finish ) == "table" and finish._type == eObjectType.GXQuat,
        [[GXQuat:SphericalLinearInterpolation - "finish" is not a GXQuat.]]
    )

    assert ( type ( interpolationFactor ) == "number",
        [[GXVec3:SphericalLinearInterpolation - "interpolationFactor" is not number.]]
    )

    av_GXQuatSphericalLinearInterpolation ( self._handle, start._handle, finish._handle, interpolationFactor );
end

local function TransformFast ( self, out, v )
    assert ( type ( self ) == "table" and self._type == eObjectType.GXQuat,
        [[GXQuat:TransformFast - Calling not via ":" syntax.]]
    )

    assert ( type ( out ) == "table" and out._type == eObjectType.GXVec3,
        [[GXQuat:TransformFast - "out" is not a GXVec3.]]
    )

    assert ( type ( v ) == "table" and v._type == eObjectType.GXVec3,
        [[GXQuat:TransformFast - "v" is not a GXVec3.]]
    )

    av_GXQuatTransformFast ( self._handle, out._handle, v._handle )
end

-- Metamethods
local mt = {
    __concat = function ( left, right )
        return string.format ( "%s%s", left, right )
    end,

    __gc = function ( self )
        av_GXQuatDestroy ( self._handle )
    end,

    __tostring = function ( self )
        return av_GXQuatToString ( self._handle )
    end
}

local function Constructor ( self )
    local obj = Object ( eObjectType.GXQuat )

    -- Data
    obj._handle = av_GXQuatCreate ()

    -- Methods
    obj.FromAxisAngle = FromAxisAngle
    obj.Inverse = Inverse
    obj.InverseFast = InverseFast
    obj.Multiply = Multiply
    obj.Normalize = Normalize
    obj.SphericalLinearInterpolation = SphericalLinearInterpolation
    obj.TransformFast = TransformFast

    return setmetatable ( obj, mt )
end

setmetatable ( GXQuat, { __call = Constructor } )

-- Module contract
return nil
