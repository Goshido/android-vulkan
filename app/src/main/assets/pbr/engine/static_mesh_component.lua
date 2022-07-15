require "av://engine/component.lua"
require "av://engine/gx_mat4.lua"


StaticMeshComponent = {}

-- Methods
local function GetLocal ( self, localMatrix )
    assert ( type ( self ) == "table" and self._type == eObjectType.StaticMeshComponent,
        [[StaticMeshComponent:GetLocal - Calling not via ":" syntax.]]
    )

    assert ( type ( localMatrix ) == "table" and localMatrix._type == eObjectType.GXMat4,
        [[StaticMeshComponent:GetLocal - "localMatrix" is not a GXMat4.]]
    )

    av_StaticMeshComponentGetLocal ( self._handle, localMatrix._handle )
end

local function SetLocal ( self, localMatrix )
    assert ( type ( self ) == "table" and self._type == eObjectType.StaticMeshComponent,
        [[StaticMeshComponent:SetLocal - Calling not via ":" syntax.]]
    )

    assert ( type ( localMatrix ) == "table" and localMatrix._type == eObjectType.GXMat4,
        [[StaticMeshComponent:SetLocal - "localMatrix" is not a GXMat4.]]
    )

    av_StaticMeshComponentSetLocal ( self._handle, localMatrix._handle )
end

-- This function is exported to C++ side.
function RegisterStaticMeshComponent ( handle )
    local obj = Component ( eObjectType.StaticMeshComponent, handle )

    -- Methods
    obj.GetLocal = GetLocal
    obj.SetLocal = SetLocal

    return obj
end

-- Metamethods
local function Constructor ( self, name )
    return RegisterStaticMeshComponent ( av_StaticMeshComponentCreate ( name ) )
end

setmetatable ( StaticMeshComponent, { __call = Constructor } )

-- Module contract
return nil
