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

local function SetShapeBox ( self, size, forceAwake )
    assert ( type ( self ) == "table" and self._type == eObjectType.RigidBodyComponent,
        [[RigidBodyComponent:SetShapeBox - Calling not via ":" syntax.]]
    )

    assert ( type ( size ) == "table" and size._type == eObjectType.GXVec3,
        [[RigidBodyComponent:SetShapeBox - "size" is not a GXVec3.]]
    )

    assert ( type ( forceAwake ) == "boolean", [[RigidBodyComponent:SetShapeBox - "forceAwake" is not a boolean.]] )
    av_RigidBodyComponentSetShapeBox ( self._handle, size._handle, forceAwake )
end

local function SetShapeSphere ( self, radius, forceAwake )
    assert ( type ( self ) == "table" and self._type == eObjectType.RigidBodyComponent,
        [[RigidBodyComponent:SetShapeSphere - Calling not via ":" syntax.]]
    )

    assert ( type ( radius ) == "number", [[RigidBodyComponent:SetShapeSphere - "radius" is not a number.]] )
    assert ( type ( forceAwake ) == "boolean", [[RigidBodyComponent:SetShapeSphere - "forceAwake" is not a boolean.]] )
    av_RigidBodyComponentSetShapeSphere ( self._handle, radius, forceAwake )
end

local function GetTransform ( self, transform )
    assert ( type ( self ) == "table" and self._type == eObjectType.RigidBodyComponent,
        [[RigidBodyComponent:GetTransform - Calling not via ":" syntax.]]
    )

    assert ( type ( transform ) == "table" and transform._type == eObjectType.GXMat4,
        [[RigidBodyComponent:GetTransform - "transform" is not a GXMat4.]]
    )

    av_RigidBodyComponentGetTransform ( self._handle, transform._handle )
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

-- Engine event handlers
local function OnDestroy ( self )
    g_Storage[ self._nativeRigidBody ] = nil
    av_RigidBodyComponentDestroy ( self._handle )
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_RigidBodyComponentCollectGarbage ( self._handle )
    end
}

-- This function is exported to C++ side.
function RegisterRigidBodyComponent ( handle, nativeRigidBody )
    local obj = Component ( eObjectType.RigidBodyComponent, handle )

    -- Data
    obj._nativeRigidBody = nativeRigidBody

    -- Methods
    obj.AddForce = AddForce
    obj.GetLocation = GetLocation
    obj.SetLocation = SetLocation
    obj.SetShapeBox = SetShapeBox
    obj.SetShapeSphere = SetShapeSphere
    obj.GetTransform = GetTransform
    obj.GetVelocityLinear = GetVelocityLinear
    obj.SetVelocityLinear = SetVelocityLinear

    -- Engine events
    obj.OnDestroy = OnDestroy

    g_Storage[ nativeRigidBody ] = obj
    return setmetatable ( obj, mt )
end

local function Constructor ( self, name )
    handle, nativeRigidBody = av_RigidBodyComponentCreate ( name )
    return RegisterRigidBodyComponent ( handle, nativeRigidBody )
end

setmetatable ( RigidBodyComponent, { __call = Constructor } )

-- Module contract
return nil
