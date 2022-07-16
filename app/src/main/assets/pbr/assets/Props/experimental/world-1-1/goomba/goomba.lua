require "av://engine/script_component.lua"


-- Constants
local SPEED = 44.44448

-- Class declaration
local Goomba = {}

-- Methods
local function GetOrigin ( self, actor, component )
    local t = GXMat4 ()
    actor:FindComponent ( component ):GetTransform ( t )

    local result = GXVec3 ()
    t:GetW ( result )
    return result
end

local function GetCenter ( self, actor, min, max )
    local minOrigin = self:GetOrigin ( actor, min )

    local diff = GXVec3 ()
    diff:Subtract ( self:GetOrigin ( actor, max ), minOrigin )

    local result = GXVec3 ()
    result:SumScaled ( minOrigin, 0.5, diff )
    return result
end

local function GetOffset ( self, parentOrigin, actor, min, max )
    local result = GXVec3 ()
    result:Subtract ( self:GetCenter ( actor, min, max ), parentOrigin )
    return result
end

local function GetSensorParentTransform ( self, sensor, toParentTransform )
    local m = GXMat4 ()
    sensor:GetLocal ( m )

    local result = GXMat4 ()
    result:Multiply ( m, toParentTransform )
    return result
end

local function MoveTo ( self, origin )
    local m = self._localMatrix
    m:SetW ( origin )
    self._mesh:SetLocal ( m )

    local attachment = GXMat4 ()
    attachment:Multiply ( self._sensorTopParent, m )
    self._sensorTop:SetLocal ( attachment )

    attachment:Multiply ( self._sensorLeftParent, m )
    self._sensorLeft:SetLocal ( attachment )

    attachment:Multiply ( self._sensorRightParent, m )
    self._sensorRight:SetLocal ( attachment )
end

local function UpdateRoute ( self )
    local route = self._route
    local routeIdx = self._routeIdx
    local routeNext = self._routeNext
    local from = route[ routeIdx ]

    routeNext = routeIdx + routeNext > ( routeNext > 0 and #route or 0 ) and -1 or 1
    local routeIdx = routeIdx + routeNext
    local to = route[ routeIdx ]

    local v = self._velocity
    v:Subtract ( to, from )
    v:Normalize ()
    v:MultiplyScalar ( v, SPEED )

    self._to = to
    self._routeIdx = routeIdx
    self._routeNext = routeNext
end

-- Engine event handlers
local function OnActorConstructed ( self, actor )
    local origin = self:GetCenter ( actor, "BodyMin", "BodyMax" )

    self._localMatrix = GXMat4 ()
    self._localMatrix:Identity ()
    self._localMatrix:SetW ( origin )

    self._mesh = actor:FindComponent ( "Mesh" )
    local p = GXMat4 ()
    self._mesh:GetLocal ( p )
    local toParentTransform = GXMat4 ()
    toParentTransform:Inverse ( p )

    self._sensorTop = actor:FindComponent ( "SensorTop" )
    self._sensorTopOffset = self:GetOffset ( origin, actor, "SensorTopMin", "SensorTopMax" )
    self._sensorTopParent = self:GetSensorParentTransform ( self._sensorTop, toParentTransform )

    self._sensorLeft = actor:FindComponent ( "SensorLeft" )
    self._sensorLeftOffset = self:GetOffset ( origin, actor, "SensorLeftMin", "SensorLeftMax" )
    self._sensorLeftParent = self:GetSensorParentTransform ( self._sensorLeft, toParentTransform )

    self._sensorRight = actor:FindComponent ( "SensorRight" )
    self._sensorRightOffset = self:GetOffset ( origin, actor, "SensorRightMin", "SensorRightMax" )
    self._sensorRightParent = self:GetSensorParentTransform ( self._sensorRight, toParentTransform )

    local route = {}
    table.insert ( route, self:GetOrigin ( actor, "Path001" ) )
    table.insert ( route, self:GetOrigin ( actor, "Path002" ) )
    self._route = route
    self._routeIdx = 1
    self._routeNext = 1
    self._velocity = GXVec3 ()

    self:UpdateRoute ()
end

local function OnPrePhysics ( self, deltaTime )
    local tmp = GXVec3 ()
    self._localMatrix:GetW ( tmp )

    local v = GXVec3 ()
    local velocity = self._velocity
    v:MultiplyScalar ( velocity, deltaTime )
    tmp:Sum ( tmp, v )

    self:MoveTo ( tmp )
    tmp:Subtract ( self._to, tmp )

    if tmp:DotProduct ( velocity ) < 0 then
        self:UpdateRoute ()
    end
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Methods
    obj.GetCenter = GetCenter
    obj.GetOffset = GetOffset
    obj.GetOrigin = GetOrigin
    obj.GetSensorParentTransform = GetSensorParentTransform
    obj.MoveTo = MoveTo
    obj.UpdateRoute = UpdateRoute

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnPrePhysics = OnPrePhysics
    return obj
end

setmetatable ( Goomba, { __call = Constructor } )

-- Module function: fabric callable for Goomba class
return Goomba
