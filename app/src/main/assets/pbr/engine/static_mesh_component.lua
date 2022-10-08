require "av://engine/component.lua"
require "av://engine/gx_mat4.lua"


StaticMeshComponent = {}

-- Methods
local function SetColor0 ( self, color0 )
    assert ( type ( self ) == "table" and self._type == eObjectType.StaticMeshComponent,
        [[StaticMeshComponent:SetColor0 - Calling not via ":" syntax.]]
    )

    assert ( type ( color0 ) == "table" and color0._type == eObjectType.GXVec4,
        [[StaticMeshComponent:SetColor0 - "color0" is not GXVec4.]]
    )

    av_StaticMeshComponentSetColor0 ( self._handle, color0._handle )
end

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

local function SetMaterial ( self, material )
    assert ( type ( self ) == "table" and self._type == eObjectType.StaticMeshComponent,
        [[StaticMeshComponent:SetMaterial - Calling not via ":" syntax.]]
    )

    assert ( type ( material ) == "table" and material._type == eObjectType.Material,
        [[StaticMeshComponent:SetMaterial - "material" is not a Material entity.]]
    )

    av_StaticMeshComponentSetMaterial ( self._handle, material._handle )
end

-- Engine event handlers
local function OnDestroy ( self )
    av_StaticMeshComponentDestroy ( self._handle )
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_StaticMeshComponentCollectGarbage ( self._handle )
    end
}

-- This function is exported to C++ side.
function RegisterStaticMeshComponent ( handle )
    local obj = Component ( eObjectType.StaticMeshComponent, handle )

    -- Methods
    obj.SetColor0 = SetColor0
    obj.GetLocal = GetLocal
    obj.SetLocal = SetLocal
    obj.SetMaterial = SetMaterial

    -- Engine events
    obj.OnDestroy = OnDestroy

    return setmetatable ( obj, mt )
end

local function Constructor ( self, name, meshFile )
    return RegisterStaticMeshComponent ( av_StaticMeshComponentCreate ( name, meshFile ) )
end

setmetatable ( StaticMeshComponent, { __call = Constructor } )

-- Module contract
return nil
