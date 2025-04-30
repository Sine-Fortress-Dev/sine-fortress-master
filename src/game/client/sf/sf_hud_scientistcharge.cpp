//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "c_tf_player.h"
#include "iclientmode.h"
#include "ienginevgui.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ProgressBar.h>
#include <sf/tf_weapon_googun.h>
#include "tf_weaponbase.h"
#include "tf_gamerules.h"
#include "tf_logic_halloween_2014.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar hud_sf_scientistcharge_transition_time("hud_sf_scientistcharge_transition_time", "1.3", 0, "The time it takes for the scientist's charge bar to adjust to the new value", true, 0.0f,false, 0.0f);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHudScientistChargeMeter : public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE(CHudScientistChargeMeter, EditablePanel);

public:
	CHudScientistChargeMeter(const char* pElementName);

	virtual void	ApplySchemeSettings(IScheme* scheme);
	virtual bool	ShouldDraw(void);
	virtual void	OnTick(void);

private:
	vgui::ContinuousProgressBar* m_pChargeMeter;
	int m_iAmmoOld;
	int m_iAmmoCur;;
	float m_flLastAmmoChange;
};

DECLARE_HUDELEMENT(CHudScientistChargeMeter);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudScientistChargeMeter::CHudScientistChargeMeter(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudScientistCharge")
{
	Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_pChargeMeter = new ContinuousProgressBar(this, "ChargeMeter");
	m_iAmmoOld = 0;
	m_iAmmoCur = 0;
	m_flLastAmmoChange = 0;
	SetDialogVariable("charge", 200);

	SetHiddenBits(HIDEHUD_MISCSTATUS | HIDEHUD_PIPES_AND_CHARGE);

	vgui::ivgui()->AddTickSignal(GetVPanel());

	RegisterForRenderGroup("inspect_panel");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScientistChargeMeter::ApplySchemeSettings(IScheme* pScheme)
{
	// load control settings...
	LoadControlSettings("resource/UI/HudScientistCharge.res");

	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudScientistChargeMeter::ShouldDraw(void)
{
	C_TFPlayer* pPlayer = C_TFPlayer::GetLocalTFPlayer();

	if (!pPlayer || !pPlayer->IsAlive())
		return false;

	if (!pPlayer->IsPlayerClass(TF_CLASS_SCIENTIST))
		return false;

	if (pPlayer->m_Shared.InCond(TF_COND_HALLOWEEN_GHOST_MODE))
		return false;

	if (CTFMinigameLogic::GetMinigameLogic() && CTFMinigameLogic::GetMinigameLogic()->GetActiveMinigame())
		return false;

	if (TFGameRules() && TFGameRules()->ShowMatchSummary())
		return false;

	return CHudElement::ShouldDraw();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudScientistChargeMeter::OnTick(void)
{
	if (!m_pChargeMeter)
		return;

	C_TFPlayer* pPlayer = C_TFPlayer::GetLocalTFPlayer();

	if (!pPlayer)
		return;

	CTFWeaponBase* pWpn = pPlayer->GetActiveTFWeapon();

	if (!pWpn)
		return;

	if (pWpn->GetWeaponID() != TF_WEAPON_GOOGUN)
		pWpn = pPlayer->Weapon_OwnsThisID(TF_WEAPON_GOOGUN);

	if (!pWpn || (pWpn->GetWeaponID() != TF_WEAPON_GOOGUN))
		return;

	CTFGooGun* pGooGun = assert_cast<CTFGooGun*>(pWpn);

	float flChargeMaxTime = pGooGun->GetChargeMaxTime();

	if (flChargeMaxTime <= 0)
		return;

	if (m_iAmmoCur != pPlayer->GetAmmoCount(TF_AMMO_PRIMARY))
	{
		m_iAmmoOld = m_iAmmoCur;
		m_iAmmoCur = pPlayer->GetAmmoCount(TF_AMMO_PRIMARY);
		m_flLastAmmoChange = gpGlobals->curtime;
		SetDialogVariable("charge", m_iAmmoCur);
	}

	float flChargeBeginTime = pGooGun->GetChargeBeginTime();

	int iAmmoCur = pPlayer->GetAmmoCount(TF_AMMO_PRIMARY);
	int iAmmoMax = pPlayer->GetMaxAmmo(TF_AMMO_PRIMARY);

	float flCurAmmoPercentage = Clamp(float(iAmmoCur) / float(iAmmoMax), 0.0f, 1.0f);
	float flOldAmmoPercentage = Clamp(float(m_iAmmoOld) / float(iAmmoMax), 0.0f, 1.0f);

	float flChargeWeightedPercentage = 0.0f;

	if (flChargeBeginTime > 0)
	{
		float flTimeCharged = Max(0.0f, gpGlobals->curtime - flChargeBeginTime);
		float flPercentCharged = Min(1.0f, flTimeCharged / flChargeMaxTime);

		ConVarRef sf_googun_ammo_cost_toxic_max("sf_googun_ammo_cost_toxic");

		float flAmmoPerShotPercentage = Clamp(sf_googun_ammo_cost_toxic_max.GetFloat() / iAmmoMax, 0.0f, 1.0f);

		flChargeWeightedPercentage = flAmmoPerShotPercentage * flPercentCharged;
	}


	float flLerpPercentage = Min(1.0f, (gpGlobals->curtime - m_flLastAmmoChange) / hud_sf_scientistcharge_transition_time.GetFloat());

	m_pChargeMeter->SetPrevProgress(Lerp(flLerpPercentage, flOldAmmoPercentage, flCurAmmoPercentage));

	m_pChargeMeter->SetProgress(flCurAmmoPercentage - flChargeWeightedPercentage);

}