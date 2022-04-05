require "av://engine/gx_vec3.lua"


GXMat3 = {}

-- methods
local function GetX ( self, x )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:GetX - Calling not via ":" syntax.]]
    )

    assert ( type ( x ) == "table" and x._type == eAVObjectType.GXVec3, [[GXMat3:GetX - "x" is not a GXVec3.]] )
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

local function Inverse ( self, sourceMatrix )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:Inverse - Calling not via ":" syntax.]]
    )

    assert ( type ( sourceMatrix ) == "table" and sourceMatrix._type == eAVObjectType.GXMat3,
        [[GXMat3:Inverse - "sourceMatrix" is not a GXMat3.]]
    )

    av_GXMat3Inverse ( self._handle, sourceMatrix._handle )
end

local function Multiply ( self, a, b )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:Multiply - Calling not via ":" syntax.]]
    )

    assert ( type ( a ) == "table" and a._type == eAVObjectType.GXMat3, [[GXMat3:Multiply - "a" is not a GXMat3.]] )
    assert ( type ( b ) == "table" and b._type == eAVObjectType.GXMat3, [[GXMat3:Multiply - "b" is not a GXMat3.]] )

    av_GXMat3Multiply ( self._handle, a._handle, b._handle )
end

local function MultiplyMatrixVector ( self, out, v )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:MultiplyMatrixVector - Calling not via ":" syntax.]]
    )

    assert ( type ( out ) == "table" and out._type == eAVObjectType.GXVec3,
        [[GXMat3:MultiplyMatrixVector - "out" is not a GXVec3.]]
    )

    assert ( type ( v ) == "table" and v._type == eAVObjectType.GXVec3,
        [[GXMat3:MultiplyMatrixVector - "v" is not a GXVec3.]]
    )

    av_GXMat3MultiplyMatrixVector ( self._handle, out._handle, v._handle )
end

local function MultiplyVectorMatrix ( self, out, v )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:MultiplyVectorMatrix - Calling not via ":" syntax.]]
    )

    assert ( type ( out ) == "table" and out._type == eAVObjectType.GXVec3,
        [[GXMat3:MultiplyVectorMatrix - "out" is not a GXVec3.]]
    )

    assert ( type ( v ) == "table" and v._type == eAVObjectType.GXVec3,
        [[GXMat3:MultiplyVectorMatrix - "v" is not a GXVec3.]]
    )

    av_GXMat3MultiplyVectorMatrix ( self._handle, out._handle, v._handle )
end

local function SetX ( self, x )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:SetX - Calling not via ":" syntax.]]
    )

    assert ( type ( x ) == "table" and x._type == eAVObjectType.GXVec3, [[GXMat3:SetX - "x" is not a GXVec3.]] )
    av_GXMat3SetX ( self._handle, x._handle )
end

local function SetY ( self, y )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:SetY - Calling not via ":" syntax.]]
    )

    assert ( type ( y ) == "table" and y._type == eAVObjectType.GXVec3, [[GXMat3:SetY - "y" is not a GXVec3.]] )
    av_GXMat3SetY ( self._handle, y._handle )
end

local function SetZ ( self, z )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:SetZ - Calling not via ":" syntax.]]
    )

    assert ( type ( z ) == "table" and z._type == eAVObjectType.GXVec3, [[GXMat3:SetZ - "z" is not a GXVec3.]] )
    av_GXMat3SetZ ( self._handle, z._handle )
end

local function Transpose ( self, sourceMatrix )
    assert ( type ( self ) == "table" and self._type == eAVObjectType.GXMat3,
        [[GXMat3:Transpose - Calling not via ":" syntax.]]
    )

    assert ( type ( sourceMatrix ) == "table" and sourceMatrix._type == eAVObjectType.GXMat3,
        [[GXMat3:Transpose - "sourceMatrix" is not a GXMat3.]]
    )

    av_GXMat3Transpose ( self._handle, sourceMatrix._handle )
end

-- metamethods
local mt = {
    __concat = function ( left, right )
        return string.format ( "%s%s", left, right )
    end,

    __gc = function ( self )
        av_GXMat3Destroy ( self._handle )
    end,

    __tostring = function ( self )
        return av_GXMat3ToString ( self._handle )
    end
}

local function Constructor ( self )
    local obj = AVObject ( eAVObjectType.GXMat3 )

    -- data
    obj._handle = av_GXMat3Create ()

    -- methods
    obj.GetX = GetX
    obj.GetY = GetY
    obj.GetZ = GetZ
    obj.Identity = Identity
    obj.Inverse = Inverse
    obj.Multiply = Multiply
    obj.MultiplyMatrixVector = MultiplyMatrixVector
    obj.MultiplyVectorMatrix = MultiplyVectorMatrix
    obj.SetX = SetX
    obj.SetY = SetY
    obj.SetZ = SetZ
    obj.Transpose = Transpose

    return setmetatable ( obj, mt )
end

setmetatable ( GXMat3, { __call = Constructor } )

-- module contract
return nil
