require "av://engine/object.lua"


Actor = {}

-- Methods
-- This function is exported to C++ side.
function AppendComponent ( self, component )
    local name = component:GetName ()
    local components = self._components
    local list = components[ name ]

    if not list then
        list = {}
        components[ name ] = list
    end

    table.insert ( list, component )
end

local function CommitComponents ( self )
    for groupKey, group in pairs ( self._components ) do
        for k, v in pairs ( group ) do
            if v._type == eObjectType.ScriptComponent and type ( v.OnActorConstructed ) == "function" then
                v:OnActorConstructed ( self )
            end
        end
    end
end

local function FindComponent ( self, name )
    local query = self._components[ name ]

    -- Lua style ternary operator
    return query and query[ 1 ] or nil
end

local function FindComponents ( self, name )
    return self._components[ name ]
end

local function GetName ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.Actor,
        [[Actor:GetName - Calling not via ":" syntax.]]
    )

    return av_ActorGetName ( self._handle )
end

-- Helper
-- This function is exported to C++ side.
function MakeActor ( handle )
    local obj = Object ( eObjectType.Actor )

    -- Data
    obj._components = {}
    obj._handle = handle

    -- Methods
    obj.AppendComponent = AppendComponent
    obj.CommitComponents = CommitComponents
    obj.FindComponent = FindComponent
    obj.FindComponents = FindComponents
    obj.GetName = GetName

    return obj
end

-- Metamethods
local function Constructor ( self, name )
    return MakeActor ( av_ActorCreate ( name ) )
end

setmetatable ( Actor, { __call = Constructor } )

-- Module contract
return nil