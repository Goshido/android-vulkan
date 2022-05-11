require "av://engine/component.lua"


RigidBodyComponent = {}

-- Methods
local function GetLocation ( self, location )
    assert ( type ( self ) == "table" and self._type == eObjectType.RigidBodyComponent,
        [[RigidBodyComponent:GetLocation - Calling not via ":" syntax.]]
    )

    assert ( type ( location ) == "table" and location._type == eObjectType.GXVec3,
        [[RigidBodyComponent:GetLocation - "location" is not a GXVec3.]]
    )

    av_RigidBodyComponentGetLocation ( self._handle, location._handle )
end

-- This function is exported to C++ side.
function RegisterRigidBodyComponent ( handle )
    local obj = Component ( eObjectType.RigidBodyComponent, handle )

    -- Methods
    obj.GetLocation = GetLocation

    return obj
end

-- Metamethods
local function Constructor ( self, name )
    return RegisterRigidBodyComponent ( av_RigidBodyComponentCreate ( name ) )
end

setmetatable ( RigidBodyComponent, { __call = Constructor } )

-- Module contract
return nil
