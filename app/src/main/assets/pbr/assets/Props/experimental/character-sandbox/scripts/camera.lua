require "av://engine/bit_field.lua"
require "av://engine/gx_mat3.lua"
require "av://engine/gx_mat4.lua"
require "av://engine/gx_quat.lua"
require "av://engine/gx_vec3.lua"
require "av://engine/input_event.lua"
require "av://engine/script_component.lua"


local Camera = {}

-- Constants
local TARGET_OFFSET = GXVec3 ()
TARGET_OFFSET:Init ( 0.358, 1.505, 0.0 )

local UP = GXVec3 ()
UP:Init ( 0.0, 1.0, 0.0 )

local RIGHT = GXVec3 ()
RIGHT:Init ( 1.0, 0.0, 0.0 )

local DISTANCE_CLOSE = 2.5

local PITCH_DEAD_ZONE = 0.25
local PITCH_SPEED = 3.777

-- Camera forward direction relative global Y axis.
local PITCH_LOW = math.rad ( 45.0 )
local PITCH_HIGH = math.rad ( 165.0 )

local PITCH_LOW_DOT = math.cos ( PITCH_LOW )
local PITCH_HIGH_DOT = math.cos ( PITCH_HIGH )

local RIGHT_DEAD_ZONE = 0.25
local RIGHT_SPEED = 3.777

local Z_NEAR = 6.4
local Z_FAR = 10000.0
local FOV_Y = math.rad ( 55.0 )

-- Optimization: the value should be negative.
local DELAY_CAMERA_SPEED = -4.0

-- Forward declaration
local OnInput
local OnUpdate

-- Free functions
local function MakePivotOrientation ( right )
    local m = GXMat3 ()
    m:SetX ( right )
    m:SetY ( UP )

    local alpha = GXVec3 ()
    alpha:CrossProduct ( right, UP )
    m:SetZ ( alpha )
    return m
end

-- Methods
local function Activate ( self, playerLocation, playerForward )
    local right = GXVec3 ()
    right:CrossProduct ( UP, playerForward )
    self._right = right

    local forward = GXVec3 ()
    forward:Clone ( playerForward )
    self._forward = forward

    self._pitchDir = 0.0
    self._rightDir = 0.0
    self._playerForward = playerForward
    self._playerLocation = playerLocation
    self._distance = DISTANCE_CLOSE
    self._isLookBack = false
    self._oldLookBack = false

    local hitGroups = BitField ()
    hitGroups:SetAllBits ()
    self._hitGroups = hitGroups

    local r = GXQuat ()
    r:FromAxisAngle ( RIGHT, PITCH_LOW )
    local pitchLowLimit = GXVec3 ()
    r:TransformFast ( pitchLowLimit, UP )
    self._pitchLowLimit = pitchLowLimit

    local pitchHighLimit = GXVec3 ()
    r:FromAxisAngle ( RIGHT, PITCH_HIGH )
    r:TransformFast ( pitchHighLimit, UP )
    self._pitchHighLimit = pitchHighLimit

    local transform = GXMat4 ()
    transform:Identity ()
    transform:SetX ( right )
    transform:SetY ( UP )
    transform:SetZ ( playerForward )
    transform:SetW ( self:GetTarget ( MakePivotOrientation ( right ) ) )

    local location = GXVec3 ()
    self:GetIdealLocation ( location, transform, DISTANCE_CLOSE )
    self._location = location

    self:OnRenderTargetChanged ()
    g_scene:SetActiveCamera ( self._camera )

    self.OnUpdate = OnUpdate
    self:OnUpdate ( 0.0 )

    self.OnInput = OnInput
end

local function GetIdealLocation ( self, idealLocation, targetTransform, distance )
    local negD = -distance
    local d = distance * distance

    local forward = GXVec3 ()
    targetTransform:GetZ ( forward )

    local alpha = GXVec3 ()
    local beta = GXVec3 ()
    local hitGroups = self._hitGroups
    local corners = self._corners

    for i = 1, 4 do
        targetTransform:MultiplyAsPoint ( alpha, corners[ i ] )
        beta:SumScaled ( alpha, negD, forward )

        local hit = g_scene:Raycast ( alpha, beta, hitGroups )

        if hit then
            local newD = alpha:SquaredDistance ( hit._point )

            if d > newD then
                d = newD
            end
        end
    end

    targetTransform:GetW ( alpha )
    idealLocation:SumScaled ( alpha, -math.sqrt ( d ), forward )
end

local function GetRight ( self )
    return self._right
end

local function GetTarget ( self, pivotOrientation )
    local target = GXVec3 ()
    pivotOrientation:MultiplyVectorMatrix ( target, TARGET_OFFSET )
    target:Sum ( target, self._playerLocation )
    return target
end

local function HandleDirection ( self, value, field, deadZone )
    if math.abs ( value ) < deadZone then
        value = 0.0
    end

    self[ field ] = value
end

-- Engine events
local function OnActorConstructed ( self, actor )
    local camera = actor:FindComponent ( "Camera" )
    self._camera = camera

    camera:SetProjection ( FOV_Y, g_scene:GetRenderTargetAspectRatio (), Z_NEAR, Z_FAR )

    local corners = {}
    table.insert ( corners, GXVec3 () )
    table.insert ( corners, GXVec3 () )
    table.insert ( corners, GXVec3 () )
    table.insert ( corners, GXVec3 () )
    self._corners = corners
end

OnInput = function ( self, inputEvent )
    local t = inputEvent._type
    local down = t == eEventType.KeyDown

    if ( down or t == eEventType.KeyUp ) and inputEvent._key == eKey.RightStick then
        self._isLookBack = down
        return
    end

    if t ~= eEventType.RightStick then
        return
    end

    self:HandleDirection ( -inputEvent._y, "_pitchDir", PITCH_DEAD_ZONE )
    self:HandleDirection ( inputEvent._x, "_rightDir", RIGHT_DEAD_ZONE )
end

local function OnInputIdle ( self, inputEvent )
    -- NOTHING
end

local function OnRenderTargetChanged ( self )
    local aspect = g_scene:GetRenderTargetAspectRatio ()
    self._camera:SetAspectRatio ( aspect )

    local dY = Z_NEAR * math.tan ( FOV_Y * 0.5 ) * g_scene:GetRendererToPhysicsScaleFactor ()
    local dX = dY * aspect

    local corners = self._corners
    corners[ 1 ]:Init ( -dX, -dY, 0.0 )
    corners[ 2 ]:Init ( dX, -dY, 0.0 )
    corners[ 3 ]:Init ( dX, dY, 0.0 )
    corners[ 4 ]:Init ( -dX, dY, 0.0 )
end

OnUpdate = function ( self, deltaTime )
    local right = self._right
    local isLookBack = self._isLookBack
    local isLookBackChanged = isLookBack ~= self._oldLookBack

    if isLookBackChanged then
        if isLookBack then
            right:CrossProduct ( self._playerForward, UP )
        else
            right:CrossProduct ( UP, self._playerForward )
        end
    end

    local target = self:GetTarget ( MakePivotOrientation ( right ) )

    local transform = GXMat4 ()
    transform:Identity ()
    transform:SetW ( target )

    local forward = self._forward

    local alpha = GXVec3 ()
    alpha:SumScaled ( target, -self._distance, forward )
    local hit = g_scene:Raycast ( target, alpha, self._hitGroups )

    if hit then
        alpha:Clone ( hit._point )
    end

    local location = self._location

    if isLookBackChanged then
        location:Clone ( alpha )
        self._oldLookBack = isLookBack
    else
        alpha:Subtract ( alpha, location )
        location:SumScaled ( location, 1.0 - math.exp ( DELAY_CAMERA_SPEED * deltaTime ), alpha )
    end

    alpha:Subtract ( target, location )
    local distance = alpha:Length ()
    alpha:MultiplyScalar ( alpha, 1.0 / distance )

    local dYaw = GXQuat ()
    dYaw:FromAxisAngle ( UP, self._rightDir * deltaTime * RIGHT_SPEED )

    local dPitch = GXQuat ()
    dPitch:FromAxisAngle ( right, self._pitchDir * deltaTime * PITCH_SPEED )

    local r = GXQuat ()
    r:Multiply ( dPitch, dYaw )
    r:TransformFast ( forward, alpha )

    right:CrossProduct ( UP, forward )
    right:Normalize ()
    transform:SetX ( right )

    local dot = forward:DotProduct ( UP )

    if dot < PITCH_HIGH_DOT then
        transform:SetY ( UP )
        alpha:CrossProduct ( right, UP )
        transform:SetZ ( alpha )
        transform:MultiplyAsNormal ( forward, self._pitchHighLimit )
    elseif dot > PITCH_LOW_DOT then
        transform:SetY ( UP )
        alpha:CrossProduct ( right, UP )
        transform:SetZ ( alpha )
        transform:MultiplyAsNormal ( forward, self._pitchLowLimit )
    end

    transform:SetZ ( forward )

    alpha:CrossProduct ( forward, right )
    transform:SetY ( alpha )

    self:GetIdealLocation ( location, transform, distance )
    transform:SetW ( location )

    g_scene:SetSoundListenerTransform ( transform )

    alpha:MultiplyScalar ( location, g_scene:GetPhysicsToRendererScaleFactor () )
    transform:SetW ( alpha )
    self._camera:SetLocal ( transform )
end

local function OnUpdateIdle ( self, deltaTime )
    -- NOTHING
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Methods
    obj.Activate = Activate
    obj.GetIdealLocation = GetIdealLocation
    obj.GetRight = GetRight
    obj.GetTarget = GetTarget
    obj.HandleDirection = HandleDirection

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInputIdle
    obj.OnRenderTargetChanged = OnRenderTargetChanged
    obj.OnUpdate = OnUpdateIdle

    return obj
end

setmetatable ( Camera, { __call = Constructor } )

-- Module function: fabric callable for Camera class.
return Camera
