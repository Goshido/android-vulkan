require "av://engine/script_component.lua"
require "av://engine/ui_layer.lua"


-- Class declaration
local Level = {}

-- Engine events
local function OnActorConstructed ( self, actor )
    local uiLayer = self._uiLayer;

    uiLayer:Find ( "fock" )
    uiLayer:Hide ()
    uiLayer:Show ()
    uiLayer:IsVisible ()

    self._uiLayer = nil
    collectgarbage ()
end

-- Metamethods
local function Constructor ( self, handle, params )
    assert ( params and type ( params._uiAsset ) == "string",
        [['_uiAsset' is incorrect. Please check it in map assset.]]
    )

    local obj = ScriptComponent ( handle )

    -- Data
    obj._uiLayer = UILayer ( params._uiAsset )

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed

    return obj
end

setmetatable ( Level, { __call = Constructor } )

-- Module function: fabric callable for Level class.
return Level
