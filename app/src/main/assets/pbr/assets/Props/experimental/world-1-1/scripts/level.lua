require "av://engine/scene.lua"
require "av://engine/script_component.lua"
require "av://engine/sound_emitter_global_component.lua"
require "av://engine/ui_layer.lua"


-- Class declaration
local Level = {}

-- Methods
local function QuitGame ( self )
    self._music:Stop ()
    g_scene:Quit ()
end

-- Engine events
local function OnInputIdle ( self, inputEvent )
    -- NOTHING
end

-- Engine event handlers
local function OnInputActive ( self, inputEvent )
    if inputEvent._type == eEventType.KeyUp and inputEvent._key == eKey.Home then
        self:QuitGame ()
    end
end

local function OnUpdateIdle ( self, deltaTime )
    -- NOTHING
end

local function OnUpdateActive ( self, deltaTime )
    self._music:Play ()
    self.OnUpdate = OnUpdateIdle
end

local function OnActorConstructed ( self, actor )
    self.OnInput = OnInputActive
    self.OnUpdate = OnUpdateActive

    local music = SoundEmitterGlobalComponent ( "Music", eSoundChannel.Music )
    music:SetSoundAsset ( "sounds/Credits.ogg", true )
    music:SetVolume ( 0.25 )
    actor:AppendComponent ( music )
    self._music = music
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Data
    obj._uiLayer = UILayer ( params._uiAsset )

    -- Methods
    obj.QuitGame = QuitGame

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInputIdle
    obj.OnUpdate = OnUpdateIdle

    return obj
end

setmetatable ( Level, { __call = Constructor } )

-- Module function: fabric callable for Level class.
return Level
