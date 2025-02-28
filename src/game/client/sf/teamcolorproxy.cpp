#include "cbase.h"
#include "proxyentity.h"
#include "iclientrenderable.h"
#include "toolframework_client.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "c_team.h"
#include "tf_gamerules.h"
#include "c_tf_player.h"
#include "functionproxy.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class IMaterialVar;

ConVar sf_menu_rgb_color("sf_menu_rgb_color", "76 122 75", FCVAR_CLIENTDLL, "Sets the rgb colour for the menu RGB system");

//Okay, so basically vgui elements are not passed to proxy OnBind functions
//so I'm forced to use a boolean to indicate that we are rendering a UI element
//that uses the enemy's custom team color for the TeamColor proxy -Vruk
namespace VruksStupidUIHack
{
	static bool state;
	bool GetState() { return state; }
	void SetState(bool newState) { state = newState; }
};

//-----------------------------------------------------------------------------
// Purpose: Used for C_BaseEntity to change textures and colours based on their team
//-----------------------------------------------------------------------------
class CEntityTeamColorProxy : public CResultProxy
{
public:
	CEntityTeamColorProxy(void);
	virtual				~CEntityTeamColorProxy(void);
	virtual void		Release(void);
	virtual bool		Init(IMaterial* pMaterial, KeyValues* pKeyValues);
	virtual void		OnBind(void* pC_BaseEntity);

protected:
	virtual int			GetDisplayTeamNum(void* pC_BaseEntity);

};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEntityTeamColorProxy::CEntityTeamColorProxy(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEntityTeamColorProxy::~CEntityTeamColorProxy(void)
{
}

void CEntityTeamColorProxy::Release(void)
{
	delete this;
}

//-----------------------------------------------------------------------------
// Purpose: Get pointer to the color value
// Input  : *pMaterial - 
//-----------------------------------------------------------------------------
bool CEntityTeamColorProxy::Init(IMaterial* pMaterial, KeyValues* pKeyValues)
{
	Assert(pMaterial);

	return CResultProxy::Init(pMaterial, pKeyValues);
}

int CEntityTeamColorProxy::GetDisplayTeamNum(void* pC_BaseEntity)
{
	int iTeamNum = -1;

	C_BaseEntity* pEnt = BindArgToEntity(pC_BaseEntity);

	if (pEnt)
	{
		C_TFPlayer* pPlayer = ToTFPlayer(pEnt);
		C_TFPlayer* pOwner = NULL;
		C_TFWeaponBase* pWeapon = dynamic_cast<C_TFWeaponBase*>(pEnt);
		if (pWeapon)
		{
			pOwner = pWeapon->GetTFPlayerOwner();
			if (!pOwner)
				pOwner = (C_TFPlayer*)pWeapon->GetOwnerEntity();
			if (!pOwner)
			{
				Warning("Team color proxy error: No weapon owner for weapon %s\n", pWeapon->GetName());
				return iTeamNum;
			}
			pPlayer = pOwner;
		}

		//For some reason C_TFWeaponBase weapons aren't returning true for being base combat weapons?? -Vruk
		/*if (pEnt->IsBaseCombatWeapon()) {

			C_TFWeaponBase* pWeapon = dynamic_cast<C_TFWeaponBase*>(pEnt);
			pOwner = (C_TFPlayer*)pWeapon->GetOwner();

		}*/

		if (pPlayer && (pPlayer->m_Shared.InCond(TF_COND_DISGUISED) && pPlayer->GetTeamNumber() != GetLocalPlayerTeam())) {
			iTeamNum = pPlayer->m_Shared.GetDisguiseTeam();
		}
		else {
			iTeamNum = pEnt->GetTeamNumber();
		}
	}
	else // Usually a vgui element?
	{
		iTeamNum = GetLocalPlayerTeam();
		if (VruksStupidUIHack::GetState())
		{
			//Swap the team number
			iTeamNum = (iTeamNum == TF_TEAM_BLUE ? TF_TEAM_RED : TF_TEAM_BLUE);
		}
		
	}

	return iTeamNum;
}

void CEntityTeamColorProxy::OnBind(void* pC_BaseEntity)
{
	int iTeamNum = GetDisplayTeamNum(pC_BaseEntity);

	if (iTeamNum == -1)
	{
		Warning("OnBind Failed from a wrong team number %i\n", iTeamNum);
		return;
	}

	Vector color;

	switch (iTeamNum)
	{
	case TF_TEAM_RED:
			if (!TFGameRules()) break;
		color = TFGameRules()->GetRedTeamColor();
		break;

	case TF_TEAM_BLUE:
			if (!TFGameRules()) break;
		color = TFGameRules()->GetBlueTeamColor();
		break;

	case TEAM_UNASSIGNED:
		default:
		int r, g, b;
		r = g = b = 0;

		int scanned;
		scanned = sscanf(sf_menu_rgb_color.GetString(), "%i %i %i", &r, &g, &b);

		if (scanned != 3)
		{
				color = Vector(0, 0, 0);
			break;
		}

			color = Vector(r, g, b);
		break;
	}

	m_pResult->SetVecValue(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);

	if (ToolsEnabled())
	{
		ToolFramework_RecordMaterialParams(GetMaterial());
	}
}

EXPOSE_INTERFACE(CEntityTeamColorProxy, IMaterialProxy, "TeamColor" IMATERIAL_PROXY_INTERFACE_VERSION);