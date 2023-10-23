require "av://engine/component.lua"
require "av://engine/gx_mat4.lua"


TransformComponent = {}

-- Methods
local function GetTransform ( self, transform )
    assert ( type ( self ) == "table" and self._type == eObjectType.TransformComponent,
        [[TransformComponent:GetTransform - Calling not via ":" syntax.]]
    )

    assert ( type ( transform ) == "table" and transform._type == eObjectType.GXMat4,
        [[TransformComponent:GetTransform - "transform" is not GXMat4.]]
    )

    av_TransformComponentGetTransform ( self._handle, transform._handle )
end

-- This function is exported to C++ side.
function RegisterTransformComponent ( handle )
    local obj = Component ( eObjectType.TransformComponent, handle )

    -- Methods
    obj.GetTransform = GetTransform

    return obj
end

-- Metamethods
local function Constructor ( self, name )
    return RegisterTransformComponent ( av_TransformComponentCreate ( name ) )
end

setmetatable ( TransformComponent, { __call = Constructor } )

-- Module contract
return nil
