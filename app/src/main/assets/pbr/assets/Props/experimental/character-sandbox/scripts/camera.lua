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

local DISTANCES = {}
table.insert ( DISTANCES, 2.5 )
table.insert ( DISTANCES, 4.5 )
table.insert ( DISTANCES, 6.5 )
local DISTANCE_COUNT = #DISTANCES
local DISTANCE_INDEX = 1

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

-- Forward declarations
local OnInput
local OnUpdate

-- Methods
local function Activate ( self, playerLocation, playerForward )
    local right = self._right
    right:CrossProduct ( UP, playerForward )
    self._logicalRight:Clone ( right )

    self._forward:Clone ( playerForward )

    self._playerForward = playerForward
    self._playerLocation = playerLocation

    local r = GXQuat ()
    r:FromAxisAngle ( RIGHT, PITCH_LOW )
    r:TransformFast ( self._pitchLowLimit, UP )

    r:FromAxisAngle ( RIGHT, PITCH_HIGH )
    r:TransformFast ( self._pitchHighLimit, UP )

    local transform = GXMat4 ()
    transform:Identity ()
    transform:SetX ( right )
    transform:SetY ( UP )
    transform:SetZ ( playerForward )
    transform:SetW ( self:GetTarget () )
    self:GetIdealLocation ( self._location, transform, self._distance )

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
    return self._logicalRight
end

local function GetTarget ( self )
    local right = self._right

    local m = GXMat3 ()
    m:SetX ( right )
    m:SetY ( UP )

    local alpha = GXVec3 ()
    alpha:CrossProduct ( right, UP )
    m:SetZ ( alpha )

    m:MultiplyVectorMatrix ( alpha, TARGET_OFFSET )
    alpha:Sum ( alpha, self._playerLocation )
    return alpha
end

local function HandleCamera ( self, eventType, inputEvent )
    if eventType ~= eEventType.RightStick then
        return false
    end

    self:HandleDirection ( -inputEvent._y, "_pitchDir", PITCH_DEAD_ZONE )
    self:HandleDirection ( inputEvent._x, "_rightDir", RIGHT_DEAD_ZONE )
    return true
end

local function HandleDirection ( self, value, field, deadZone )
    if math.abs ( value ) < deadZone then
        value = 0.0
    end

    self[ field ] = value
end

local function HandleDistance ( self, eventType, key )
    local down = eventType == eEventType.KeyDown

    if not down or key ~= eKey.View then
        return false
    end

    local idx = 1 + self._distanceIndex % DISTANCE_COUNT
    self._distanceIndex = idx
    self._distance = DISTANCES[ idx ]
    return true
end

local function HandleLookBack ( self, eventType, key )
    local down = eventType == eEventType.KeyDown

    if ( not down and eventType ~= eEventType.KeyUp ) or key ~= eKey.RightStick then
        return false
    end

    self._isLookBack = down
    return true
end

-- Engine events
local function OnActorConstructed ( self, actor )
    local camera = actor:FindComponent ( "Camera" )
    self._camera = camera

    camera:SetProjection ( FOV_Y, g_scene:GetRenderTargetAspectRatio (), Z_NEAR, Z_FAR )
end

OnInput = function ( self, inputEvent )
    local eventType = inputEvent._type
    local key = inputEvent._key

    local shortCircuitEvaluation = self:HandleCamera ( eventType, inputEvent ) or
        self:HandleLookBack ( eventType, key ) or
        self:HandleDistance ( eventType, key )
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
    local forward = self._forward
    local isLookBack = self._isLookBack
    local isLookBackChanged = isLookBack ~= self._oldLookBack

    if isLookBackChanged then
        if isLookBack then
            right:CrossProduct ( self._playerForward, UP )
        else
            right:CrossProduct ( UP, self._playerForward )
        end

        forward:CrossProduct ( right, UP )
    end

    local target = self:GetTarget ()

    local transform = GXMat4 ()
    transform:Identity ()
    transform:SetW ( target )

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

    local logicalRight = self._logicalRight
    logicalRight:Clone ( right )

    if isLookBack then
        logicalRight:Reverse ()
    end

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

    -- Data
    obj._forward = GXVec3 ()
    obj._location = GXVec3 ()

    obj._right = GXVec3 ()
    obj._logicalRight = GXVec3 ()
    obj._rightDir = 0.0

    obj._pitchLowLimit = GXVec3 ()
    obj._pitchHighLimit = GXVec3 ()
    obj._pitchDir = 0.0

    obj._distance = DISTANCES[ DISTANCE_INDEX ]
    obj._distanceIndex = DISTANCE_INDEX

    obj._isLookBack = false
    obj._oldLookBack = false

    local hitGroups = BitField ()
    hitGroups:SetAllBits ()
    obj._hitGroups = hitGroups

    local corners = {}
    table.insert ( corners, GXVec3 () )
    table.insert ( corners, GXVec3 () )
    table.insert ( corners, GXVec3 () )
    table.insert ( corners, GXVec3 () )
    obj._corners = corners

    -- Methods
    obj.Activate = Activate
    obj.GetIdealLocation = GetIdealLocation
    obj.GetRight = GetRight
    obj.GetTarget = GetTarget
    obj.HandleCamera = HandleCamera
    obj.HandleDirection = HandleDirection
    obj.HandleDistance = HandleDistance
    obj.HandleLookBack = HandleLookBack

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
