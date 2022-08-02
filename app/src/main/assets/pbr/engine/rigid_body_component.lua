require "av://engine/component.lua"


RigidBodyComponent = {}

-- C++ internal
local g_Storage = {}

function FindRigidBodyComponent ( nativeRigidBody )
    return g_Storage[ nativeRigidBody ]
end

-- Methods
local function AddForce ( self, force, point, forceAwake )
    assert ( type ( self ) == "table" and self._type == eObjectType.RigidBodyComponent,
        [[RigidBodyComponent:AddForce - Calling not via ":" syntax.]]
    )

    assert ( type ( force ) == "table" and force._type == eObjectType.GXVec3,
        [[RigidBodyComponent:AddForce - "force" is not a GXVec3.]]
    )

    assert ( type ( point ) == "table" and point._type == eObjectType.GXVec3,
        [[RigidBodyComponent:AddForce - "point" is not a GXVec3.]]
    )

    assert ( type ( forceAwake ) == "boolean", [[RigidBodyComponent:AddForce - "forceAwake" is not a boolean.]] )
    av_RigidBodyComponentAddForce ( self._handle, force._handle, point._handle, forceAwake )
end

local function GetLocation ( self, location )
    assert ( type ( self ) == "table" and self._type == eObjectType.RigidBodyComponent,
        [[RigidBodyComponent:GetLocation - Calling not via ":" syntax.]]
    )

    assert ( type ( location ) == "table" and location._type == eObjectType.GXVec3,
        [[RigidBodyComponent:GetLocation - "location" is not a GXVec3.]]
    )

    av_RigidBodyComponentGetLocation ( self._handle, location._handle )
end

local function SetLocation ( self, location )
    assert ( type ( self ) == "table" and self._type == eObjectType.RigidBodyComponent,
        [[RigidBodyComponent:SetLocation - Calling not via ":" syntax.]]
    )

    assert ( type ( location ) == "table" and location._type == eObjectType.GXVec3,
        [[RigidBodyComponent:SetLocation - "location" is not a GXVec3.]]
    )

    av_RigidBodyComponentSetLocation ( self._handle, location._handle )
end

local function GetVelocityLinear ( self, velocity )
    assert ( type ( self ) == "table" and self._type == eObjectType.RigidBodyComponent,
        [[RigidBodyComponent:GetVelocityLinear - Calling not via ":" syntax.]]
    )

    assert ( type ( velocity ) == "table" and velocity._type == eObjectType.GXVec3,
        [[RigidBodyComponent:GetVelocityLinear - "velocity" is not a GXVec3.]]
    )

    av_RigidBodyComponentGetVelocityLinear ( self._handle, velocity._handle )
end

local function SetVelocityLinear ( self, velocity, forceAwake )
    assert ( type ( self ) == "table" and self._type == eObjectType.RigidBodyComponent,
        [[RigidBodyComponent:SetVelocityLinear - Calling not via ":" syntax.]]
    )

    assert ( type ( velocity ) == "table" and velocity._type == eObjectType.GXVec3,
        [[RigidBodyComponent:SetVelocityLinear - "velocity" is not a GXVec3.]]
    )

    assert ( type ( forceAwake ) == "boolean",
        [[RigidBodyComponent:SetVelocityLinear - "forceAwake" is not a boolean.]]
    )

    av_RigidBodyComponentSetVelocityLinear ( self._handle, velocity._handle, forceAwake )
end

-- This function is exported to C++ side.
function RegisterRigidBodyComponent ( handle, nativeRigidBody )
    local obj = Component ( eObjectType.RigidBodyComponent, handle )

    -- Methods
    obj.AddForce = AddForce
    obj.GetLocation = GetLocation
    obj.SetLocation = SetLocation
    obj.GetVelocityLinear = GetVelocityLinear
    obj.SetVelocityLinear = SetVelocityLinear

    g_Storage[ nativeRigidBody ] = obj

    return obj
end

-- Metamethods
local function Constructor ( self, name )
    handle, nativeRigidBody = av_RigidBodyComponentCreate ( name )
    return RegisterRigidBodyComponent ( handle, nativeRigidBody )
end

setmetatable ( RigidBodyComponent, { __call = Constructor } )

-- Module contract
return nil
