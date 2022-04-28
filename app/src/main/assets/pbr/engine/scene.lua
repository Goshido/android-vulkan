require "av://engine/actor.lua"
require "av://engine/script_component.lua"


local Scene = {}

-- methods
local function AppendActor ( self, actor )
    local name = actor:GetName ()
    local actors = self._actors
    local list = actors[ name ]

    if not list then
        list = {}
        actors[ name ] = list
    end

    table.insert ( list, actor )

    local postPhysicsScripts = self._postPhysicsScripts
    local prePhysicsScripts = self._prePhysicsScripts
    local updateScripts = self._updateScripts

    for groupKey, group in pairs ( actor._components ) do
        for k, v in pairs ( group ) do
            if v._type == eObjectType.ScriptComponent then
                local script = v._script

                if type ( script.OnPostPhysics ) == "function" then
                    table.insert ( postPhysicsScripts, script )
                end

                if type ( script.OnPrePhysics ) == "function" then
                    table.insert ( prePhysicsScripts, script )
                end

                if type ( script.OnUpdate ) == "function" then
                    table.insert ( updateScripts, script )
                end
            end
        end
    end
end

local function FindActor ( self, name )
    local query = self._actors[ name ]

    -- Lua style ternary operator
    return query and query[ 1 ] or nil
end

local function FindActors ( self, name )
    return self._actors[ name ]
end

local function OnPostPhysics ( self, deltaTime )
    for k, v in pairs ( self._postPhysicsScripts ) do
        v:OnPostPhysics ( deltaTime )
    end
end

local function OnPrePhysics ( self, deltaTime )
    for k, v in pairs ( self._prePhysicsScripts ) do
        v:OnPrePhysics ( deltaTime )
    end
end

local function OnUpdate ( self, deltaTime )
    for k, v in pairs ( self._updateScripts ) do
        v:OnUpdate ( deltaTime )
    end
end

-- metamethods
local function Constructor ( self, handle )
    local obj = Object ( eObjectType.Scene )

    -- data
    obj._actors = {}
    obj._handle = handle
    obj._postPhysicsScripts = {}
    obj._prePhysicsScripts = {}
    obj._updateScripts = {}

    -- methods
    obj.AppendActor = AppendActor
    obj.FindActor = FindActor
    obj.FindActors = FindActors
    obj.OnPostPhysics = OnPostPhysics
    obj.OnPrePhysics = OnPrePhysics
    obj.OnUpdate = OnUpdate

    return obj
end

setmetatable ( Scene, { __call = Constructor } )

-- module contract
function CreateScene ( handle )
    g_scene = Scene ( handle )
    return g_scene
end

return nil
