//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that, on initialisation, adds its pointer to the TFGameRules
// entity, from which its stored values for team colour can be accessed.
// 
//=============================================================================
#include "cbase.h"
#include "sf_team_metadata.h"
#include "KeyValues.h"
#include "tf_gamerules.h"
//REVISIT: Delete all unused .h files


#include "variant_t.h"
#include "tier3/tier3.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(sf_teamoverride, CSFTeamOverride);

#ifdef GAME_DLL
BEGIN_DATADESC(CSFTeamOverride)


DEFINE_KEYFIELD(m_iTeamNum, FIELD_INTEGER, "team_number"),
DEFINE_KEYFIELD(m_bEnabled, FIELD_BOOLEAN, "is_enabled"),
DEFINE_KEYFIELD(m_colMaster, FIELD_COLOR32, "master_color"),
DEFINE_KEYFIELD(m_bClassColors, FIELD_BOOLEAN, "use_class_colors"),
DEFINE_KEYFIELD(m_colScout, FIELD_COLOR32, "scout_color"),
DEFINE_KEYFIELD(m_colSoldier, FIELD_COLOR32, "soldier_color"),
DEFINE_KEYFIELD(m_colPyro, FIELD_COLOR32, "pyro_color"),
DEFINE_KEYFIELD(m_colDemoman, FIELD_COLOR32, "demoman_color"),
DEFINE_KEYFIELD(m_colHeavy, FIELD_COLOR32, "heavy_color"),
DEFINE_KEYFIELD(m_colEngineer, FIELD_COLOR32, "engineer_color"),
DEFINE_KEYFIELD(m_colMedic, FIELD_COLOR32, "medic_color"),
DEFINE_KEYFIELD(m_colSniper, FIELD_COLOR32, "sniper_color"),
DEFINE_KEYFIELD(m_colSpy, FIELD_COLOR32, "spy_color"),
DEFINE_KEYFIELD(m_colScientist, FIELD_COLOR32, "scientist_color"),
DEFINE_KEYFIELD(m_colRusher, FIELD_COLOR32, "rusher_color"),
DEFINE_KEYFIELD(m_iszTeamName, FIELD_STRING, "team_name"), //REVISIT: I'm putting this here in case it might be useful, but remove it if it doesn't end up used

// Inputs.
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetMasterColor", InputMasterColor),
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetScoutColor", InputScoutColor),
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetSoldierColor", InputSoldierColor),
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetPyroColor", InputPyroColor),
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetDemomanColor", InputDemomanColor),
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetHeavyColor", InputHeavyColor),
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetEngineerCollor", InputEngineerColor),
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetMedicColor", InputMedicColor),
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetSniperColor", InputSniperColor),
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetSpyColor", InputSpyColor),
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetScientistColor", InputScientistColor),
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetRusherColor", InputRusherColor),



END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Adds its own pointer to the TFGameRules team override table
//-----------------------------------------------------------------------------
void CSFTeamOverride::Activate() //skeleton of what we need
{
	TFGameRules()->AddTeamOverrideEnt(this, m_iTeamNum);
	Msg("SFTeamOverride spawned, added self to TFGamerules %i",m_iTeamNum);
}



//-----------------------------------------------------------------------------
// Purpose: inputs for changing values mid-game 
//-----------------------------------------------------------------------------
void CSFTeamOverride::InputDisable( inputdata_t& inputdata )
{
	m_bEnabled = false;
}
void CSFTeamOverride::InputEnable( inputdata_t& inputdata )
{
	m_bEnabled = true;
}
void CSFTeamOverride::InputMasterColor( inputdata_t& inputdata ) 
{
	m_colMaster = inputdata.value.Color32();
}
void CSFTeamOverride::InputScoutColor(inputdata_t& inputdata)
{
	m_colScout = inputdata.value.Color32();
}
void CSFTeamOverride::InputSoldierColor(inputdata_t& inputdata)
{
	m_colSoldier = inputdata.value.Color32();
}
void CSFTeamOverride::InputPyroColor(inputdata_t& inputdata)
{
	m_colPyro= inputdata.value.Color32();
}
void CSFTeamOverride::InputDemomanColor(inputdata_t& inputdata)
{
	m_colDemoman = inputdata.value.Color32();
}
void CSFTeamOverride::InputHeavyColor(inputdata_t& inputdata)
{
	m_colHeavy = inputdata.value.Color32();
}
void CSFTeamOverride::InputEngineerColor(inputdata_t& inputdata)
{
	m_colEngineer = inputdata.value.Color32();
}
void CSFTeamOverride::InputMedicColor(inputdata_t& inputdata)
{
	m_colMedic = inputdata.value.Color32();
}
void CSFTeamOverride::InputSniperColor(inputdata_t& inputdata)
{
	m_colSniper = inputdata.value.Color32();
}
void CSFTeamOverride::InputSpyColor(inputdata_t& inputdata)
{
	m_colSpy = inputdata.value.Color32();
}
void CSFTeamOverride::InputScientistColor(inputdata_t& inputdata)
{
	m_colScientist = inputdata.value.Color32();
}
void CSFTeamOverride::InputRusherColor(inputdata_t& inputdata)
{
	m_colRusher = inputdata.value.Color32();
}
#endif