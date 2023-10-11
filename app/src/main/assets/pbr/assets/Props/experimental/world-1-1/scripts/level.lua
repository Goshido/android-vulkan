require "av://engine/scene.lua"
require "av://engine/script_component.lua"
require "av://engine/skeletal_mesh_component.lua"
require "av://engine/sound_emitter_global_component.lua"
require "av://engine/ui_layer.lua"


-- Class declaration
local Level = {}

-- Methods
local function AddScore ( self, amount )
    self._score = self._score + amount
    self._uiScore:SetText ( string.format ( "%06d", self._score ) )
end

local function QuitGame ( self )
    self._music:Stop ()
    g_scene:Quit ()
end

local function UpdatePosition ( self )
    local location = GXVec3 ()
    self._mario:GetLocation ( location )

    self._uiX:SetText ( string.format ( "%.2f", location:GetX () ) )
    self._uiY:SetText ( string.format ( "%.2f", location:GetY () ) )
    self._uiZ:SetText ( string.format ( "%.2f", location:GetZ () ) )
end

local function UpdateTimer ( self, deltaTime )
    self._timerNow = self._timerNow - deltaTime
    local now = math.ceil ( self._timerNow )

    if now == self._timerLast then
        return
    end

    self._timerNow = now
    self._timerLast = now
    self._uiTimer:SetText ( string.format ( "%d", now ) )

    if now == 378 then
        self:QuitGame ()
    end
end

-- Engine event handlers
local function OnInputIdle ( self, inputEvent )
    -- NOTHING
end

local function OnInputActive ( self, inputEvent )
    if inputEvent._type == eEventType.KeyUp and inputEvent._key == eKey.Home then
        self:QuitGame ()
    end
end

local function OnUpdateMain ( self, deltaTime )
    self:UpdateTimer ( deltaTime )
    self:UpdatePosition ()
end

local function OnUpdateFirst ( self, deltaTime )
    self._music:Play ()
    self._mario = g_scene:FindActor ( "Mario" ):FindComponent ( "Collider" )

    self.OnUpdate = OnUpdateMain
    OnUpdateMain ( self, deltaTime )
end

local function OnActorConstructed ( self, actor )
    self.OnInput = OnInputActive

    local music = SoundEmitterGlobalComponent ( "Music", eSoundChannel.Music )
    music:SetSoundAsset ( "sounds/Credits.ogg", true )
    music:SetVolume ( 0.25 )
    actor:AppendComponent ( music )
    self._music = music

    local uiLayer = self._uiLayer
    self._uiScore = uiLayer:Find ( "score" ):GetTextElement ()

    local uiTimer = uiLayer:Find ( "timer" ):GetTextElement ()
    uiTimer:SetText ( string.format ( "%d", self._timerNow ) )
    self._uiTimer = uiTimer

    local uiDebug = self._uiDebug
    self._uiX = uiDebug:Find ( "x-value" ):GetTextElement ()
    self._uiY = uiDebug:Find ( "y-value" ):GetTextElement ()
    self._uiZ = uiDebug:Find ( "z-value" ):GetTextElement ()

    local test = Actor ( "TEST" )

    local mesh = SkeletalMeshComponent ( "Mesh",
        "pbr/assets/Props/experimental/exporter/leg.mesh2",
        "pbr/assets/Props/experimental/exporter/leg.skin",
        "pbr/assets/Props/experimental/exporter/leg.skeleton",
        "pbr/assets/Props/experimental/exporter/material.mtl"
    )

    local v = GXVec3 ()
    v:Init ( -24.32, 65.19, 286.18 )

    local m = GXMat4 ()
    m:Scale ( 2.0, 2.0, 2.0 )
    m:SetW ( v )
    mesh:SetLocal ( m )

    test:AppendComponent ( mesh )
    g_scene:AppendActor ( test )

    self._test = test
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Data
    obj._timerLast = 383.999
    obj._timerNow = 383.0
    obj._score = 4200

    obj._uiLayer = UILayer ( params._uiAsset )
    obj._uiDebug = UILayer ( "pbr/assets/Props/experimental/world-1-1/ui/debug.html" )

    -- Methods
    obj.AddScore = AddScore
    obj.QuitGame = QuitGame
    obj.UpdatePosition = UpdatePosition
    obj.UpdateTimer = UpdateTimer

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInputIdle
    obj.OnUpdate = OnUpdateFirst

    return obj
end

setmetatable ( Level, { __call = Constructor } )

-- Module function: fabric callable for Level class.
return Level
