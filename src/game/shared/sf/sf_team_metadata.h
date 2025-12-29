//========= Copyright Valve Corporation, All rights reserved. ============//
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================


#ifdef _WIN32
#pragma once
#endif


#include "teamplayroundbased_gamerules.h"
#include "convar.h"
#include "gamevars_shared.h"
#include "GameEventListener.h"
#include "tf_gamestats_shared.h"
#include "tf_match_description.h"

//WE NEED TO LINK THIS TO AN ENTITY
//#ifdef GAME_DLL
class CSFTeamOverride : public CPointEntity
{
	DECLARE_CLASS( CSFTeamOverride, CPointEntity )
public:
	DECLARE_DATADESC();


	int m_iTeamNum;
	bool m_bEnabled;
	bool m_bClassColors;
	color32 m_colMaster;
	color32 m_colScout;
	color32 m_colSoldier;
	color32 m_colPyro;
	color32 m_colDemoman;
	color32 m_colHeavy;
	color32 m_colEngineer;
	color32 m_colMedic;
	color32 m_colSniper;
	color32 m_colSpy;
	color32 m_colScientist;
	color32 m_colRusher;
	string_t m_iszTeamName;

	virtual void Activate(void);
	virtual void InputDisable( inputdata_t& inputdata );
	virtual void InputEnable( inputdata_t& inputdata );
	virtual void InputMasterColor( inputdata_t& inputdata );
	virtual void InputScoutColor( inputdata_t& inputdata );
	virtual void InputSoldierColor( inputdata_t& inputdata );
	virtual void InputPyroColor( inputdata_t& inputdata );
	virtual void InputDemomanColor( inputdata_t& inputdata );
	virtual void InputHeavyColor( inputdata_t& inputdata );
	virtual void InputEngineerColor( inputdata_t& inputdata );
	virtual void InputMedicColor( inputdata_t& inputdata );
	virtual void InputSniperColor( inputdata_t& inputdata );
	virtual void InputSpyColor( inputdata_t& inputdata );
	virtual void InputScientistColor( inputdata_t& inputdata );
	virtual void InputRusherColor( inputdata_t& inputdata );
};
//#endif