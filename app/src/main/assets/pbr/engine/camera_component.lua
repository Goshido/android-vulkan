require "av://engine/component.lua"
require "av://engine/gx_mat4.lua"


CameraComponent = {}

-- Methods
local function SetAspectRatio ( self, aspectRatio )
    assert ( type ( self ) == "table" and self._type == eObjectType.CameraComponent,
        [[CameraComponent:SetAspectRatio - Calling not via ":" syntax.]]
    )

    assert ( type ( aspectRatio ) == "number", [[CameraComponent:SetAspectRatio - "aspectRatio" is not a number.]] )
    av_CameraComponentSetAspectRatio ( self._handle, aspectRatio )
end

local function SetLocal ( self, localMatrix )
    assert ( type ( self ) == "table" and self._type == eObjectType.CameraComponent,
        [[CameraComponent:SetLocal - Calling not via ":" syntax.]]
    )

    assert ( type ( localMatrix ) == "table" and localMatrix._type == eObjectType.GXMat4,
        [[CameraComponent:SetLocal - "localMatrix" is not a GXMat4.]]
    )

    av_CameraComponentSetLocal ( self._handle, localMatrix._handle )
end

local function SetProjection ( self, fieldOfViewRadians, aspectRatio, zNear, zFar )
    assert ( type ( self ) == "table" and self._type == eObjectType.CameraComponent,
        [[CameraComponent:SetProjection - Calling not via ":" syntax.]]
    )

    assert ( type ( fieldOfViewRadians ) == "number",
        [[CameraComponent:SetProjection - "fieldOfViewRadians" is not a number.]]
    )

    assert ( type ( aspectRatio ) == "number", [[CameraComponent:SetProjection - "aspectRatio" is not a number.]] )
    assert ( type ( zNear ) == "number", [[CameraComponent:SetProjection - "zNear" is not a number.]] )
    assert ( type ( zFar ) == "number", [[CameraComponent:SetProjection - "zFar" is not a number.]] )

    av_CameraComponentSetProjection ( self._handle, fieldOfViewRadians, aspectRatio, zNear, zFar )
end

-- Engine event handlers
local function OnDestroy ( self )
    -- TODO
end

-- This function is exported to C++ side.
function RegisterCameraComponent ( handle )
    local obj = Component ( eObjectType.CameraComponent, handle )

    -- Methods
    obj.SetAspectRatio = SetAspectRatio
    obj.SetLocal = SetLocal
    obj.SetProjection = SetProjection

    -- Engine events
    obj.OnDestroy = OnDestroy

    return obj
end

-- Metamethods
local function Constructor ( self, name )
    return RegisterCameraComponent ( av_CameraComponentCreate ( name ) )
end

setmetatable ( CameraComponent, { __call = Constructor } )

-- Module contract
return nil
