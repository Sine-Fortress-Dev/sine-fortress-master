//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_googun.h"
#include "tf_fx_shared.h"
#include "in_buttons.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "ihudlcd.h"
// Server specific.
#else
#include "tf_player.h"
#endif

//=============================================================================
//
// Weapon GooGun Gun tables.
//


IMPLEMENT_NETWORKCLASS_ALIASED( TFGooGun, DT_WeaponGooGun )

BEGIN_NETWORK_TABLE( CTFGooGun, DT_WeaponGooGun )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFGooGun )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_googun, CTFGooGun );
PRECACHE_WEAPON_REGISTER( tf_weapon_googun );

// Server specific.
#ifndef CLIENT_DLL
BEGIN_DATADESC(CTFGooGun)
END_DATADESC()
#endif

#define TF_WEAPON_GOOGLOB_COUNT 4

ConVar sf_googun_ammo_cost_toxic_max("sf_googun_ammo_cost_toxic", "40", FCVAR_REPLICATED, "The maximum ammo the googuns use when fully charged");
ConVar sf_googun_ammo_cost_movement("sf_googun_ammo_cost_movement", "65", FCVAR_REPLICATED, "The amount of ammo a googun will use to fire a movement goo");
ConVar sf_googun_goo_max_toxic("sf_googun_goo_max_toxic", "3", FCVAR_REPLICATED, "The number of toxic goos that a scientist can have at one time");
ConVar sf_googun_goo_max_movement("sf_googun_goo_max_movement", "1", FCVAR_REPLICATED, "The number of movement goos that a scientist can have at one time");
ConVar sf_googun_required_charge_percent("sf_googun_required_charge_percent", "0.0", FCVAR_REPLICATED, "The required percent charge of a googun before a goo projectile can be fired", true, 0.0, true, 1.0);
ConVar sf_googun_max_charge_time("sf_googun_max_charge_time", "3", FCVAR_REPLICATED, "The time it takes to fully charge the googun", true, 0, false, 0);
ConVar sf_googun_goo_lifetime_max("sf_googun_goo_lifetime_max", "12.0", FCVAR_REPLICATED, "The length of time in seconds that a full charged goo will stay on the ground before disappearing");
ConVar sf_googun_goo_lifetime_min("sf_googun_goo_lifetime_min", "2", FCVAR_REPLICATED, "The length of time in seconds that a minimally charged goo will stay on the ground before disappearing");


ConVar sf_goo_min_vel("sf_goo_min_vel", "650", FCVAR_REPLICATED, "The velocity of a goo projectile when fired at low charge percent");
ConVar sf_goo_max_vel("sf_goo_max_vel", "1200", FCVAR_REPLICATED, "The velocity of a goo projectile when fired at high charge percent");


//=============================================================================
//
// Weapon SyringeGun functions.
//

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGooGun::PrimaryAttack( void )
{

	CTFPlayer* pTFPlayerOwner = GetTFPlayerOwner();
	if (!pTFPlayerOwner)
		return;

	//Check for ammunition
	int ammoCount = pTFPlayerOwner->GetAmmoCount(m_iPrimaryAmmoType);
	if (ammoCount == 0 || sf_googun_required_charge_percent.GetFloat() * sf_googun_ammo_cost_toxic_max.GetInt() > ammoCount)
		return;

	// Are we capable of firing again?
	if ( m_flNextPrimaryAttack > gpGlobals->curtime )
		return;

	if (!CanAttack())
	{
		m_flChargeBeginTime = 0;
		return;
	}


	if (m_flChargeBeginTime <= 0)
	{
		// Set the weapon mode.
		m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;

		// save that we had the attack button down
		m_flChargeBeginTime = gpGlobals->curtime;

		SendWeaponAnim(ACT_VM_PULLBACK);
	}
	else
	{
		float chargedPercent = GetCurrentCharge();
		if (chargedPercent * sf_googun_ammo_cost_toxic_max.GetInt() >= ammoCount || chargedPercent == 1.0f)
		{
			FireGoo(TF_GOO_TOXIC);
			//SwitchBodyGroups(); // Do we need this? from old code -Vruk
		}
	}
}

void CTFGooGun::SecondaryAttack( void )
{
	// Check for ammunition.
	CBaseCombatCharacter* pOwner = GetOwner();
	if (!pOwner)
		return;
	//TODO: Have a sound play when not enough ammo available
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) < sf_googun_ammo_cost_movement.GetInt())
		return;

	// Are we capable of firing again?
	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return;

	// Set the weapon mode.
	m_iWeaponMode = TF_WEAPON_SECONDARY_MODE;

	FireGoo(GetGooType());

	/*
	else
	{
		float flTotalChargeTime = gpGlobals->curtime - m_flChargeBeginTime;
		m_bSecondaryCharge = true;

		if (flTotalChargeTime >= GetChargeMaxTime())
		{
			FireGoo(GetGooType());
			SwitchBodyGroups();
		}
	}
	*/

	/*
	//Original code for switching the goo type
	if ( m_flNextSecondaryAttack > gpGlobals->curtime )
		return;

#ifdef GAME_DLL
	SwitchGoo();
#endif
	EmitSound("Weapon_StickyBombLauncher.ModeSwitch");

	m_flNextSecondaryAttack = gpGlobals->curtime + m_pWeaponInfo->GetWeaponData(m_iWeaponMode).m_flTimeFireDelay;
	*/
} 
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGooGun::ItemBusyFrame(void)
{
	/*
#ifdef GAME_DLL
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner && pOwner->m_nButtons & IN_ATTACK2)
	{
		// We need to do this to catch the case of player trying to detonate
		// pipebombs while in the middle of reloading.
		SecondaryAttack();
	}
#endif
	*/
	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGooGun::WeaponIdle(void)
{
	if (GetCurrentCharge() < sf_googun_required_charge_percent.GetFloat())
	{
		m_flChargeBeginTime = 0;
		return;
	}
	if(m_flChargeBeginTime > 0 /*&& m_iClip1 > 0*/)
	{
		FireGoo(TF_GOO_TOXIC);
	}
	else
	{
		BaseClass::WeaponIdle();
	}
}

int CTFGooGun::GetGooType()
{
	int iMode = 0;
	CALL_ATTRIB_HOOK_INT(iMode, set_weapon_mode);

	iMode += 1; //This adjusts the difference between the enum for googun types and goo types because there's one extra goo type that isn't included in googun types

	if (iMode >= 0 && iMode < TF_GOO_COUNT)
		return iMode;

	AssertMsg(0, "Invalid googun type!\n");
	return TF_GOO_TOXIC + 1;
}

bool CTFGooGun::Deploy(void)
{
	m_flChargeBeginTime = 0;

	return BaseClass::Deploy();
}

bool CTFGooGun::AddGoo(CTFProjectile_Goo *pGoo)
{
	GooProjectilesHandle hHandle;
	hHandle = pGoo;

	switch (pGoo->GetGooType())
	{
		case TF_GOO_JUMP:
			m_MovementGooProjectiles.AddToTail(hHandle);

			// If we've gone over the max goo count, remove the oldest
			if (m_MovementGooProjectiles.Count() > sf_googun_goo_max_movement.GetInt())
			{
				CTFProjectile_Goo* pTemp = m_MovementGooProjectiles[0];
				if (pTemp)
				{
					pTemp->RemoveThis();
				}

				m_MovementGooProjectiles.Remove(0);
			}
			break;
		case TF_GOO_TOXIC:
			m_ToxicGooProjectiles.AddToTail(hHandle);

			// If we've gone over the max goo count, remove the oldest
			if (m_ToxicGooProjectiles.Count() > sf_googun_goo_max_toxic.GetInt())
			{
				CTFProjectile_Goo* pTemp = m_ToxicGooProjectiles[0];
				if (pTemp)
				{
					pTemp->RemoveThis();
				}

				m_ToxicGooProjectiles.Remove(0);
			}
			break;
		default:
			Warning("Attempted to add goo that is not of known index: %d", pGoo->GetGooType());
			return false;
			break;
	}
	return true;
}

float CTFGooGun::GetProjectileSpeed(void)
{
	float value = GetCurrentCharge();

	if (m_iWeaponMode == TF_WEAPON_SECONDARY_MODE)
		value = 0.66;

	float flVelocity = RemapValClamped(value, 0.0f, 1.0f, sf_goo_min_vel.GetFloat(), sf_goo_max_vel.GetFloat());

	CALL_ATTRIB_HOOK_FLOAT( flVelocity, mult_projectile_speed );

	return flVelocity;
}

float CTFGooGun::GetChargeMaxTime(void)
{
	float flMaxChargeTime = sf_googun_max_charge_time.GetFloat();
	
	float flChargeRateBonus = 0;
	CALL_ATTRIB_HOOK_FLOAT(flChargeRateBonus, googun_charge_rate_bonus);

	return flMaxChargeTime - flChargeRateBonus;
}

float CTFGooGun::GetCurrentCharge()
{
	if (GetChargeBeginTime() <= 0.0f)
		return 0.0f;

	float flTimeCharging = gpGlobals->curtime - GetChargeBeginTime();
	float flTotalChargeTime = Clamp<float>(flTimeCharging, 0.0f, GetChargeMaxTime());
	return (flTotalChargeTime / GetChargeMaxTime());
}

bool CTFGooGun::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	m_flChargeBeginTime = 0;

	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFGooGun::FireGoo( int GooType )
{
	if (GooType >= TF_GOO_COUNT || GooType < 0)
		return;

	// Get the player owning the weapon.
	CTFPlayer *pPlayer = ToTFPlayer( GetPlayerOwner() );
	if ( !pPlayer )
		return;

	CalcIsAttackCritical();

	float flPercentageCharged;

	if (m_iWeaponMode == TF_WEAPON_PRIMARY_MODE)
	{
		SendWeaponAnim(ACT_VM_PRIMARYATTACK);
		pPlayer->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);

		flPercentageCharged = GetCurrentCharge();
	}
	else
	{
		//Do anims
		SendWeaponAnim(ACT_VM_SECONDARYATTACK);
		pPlayer->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_SECONDARY);

		flPercentageCharged = 0.66;
	}

	pPlayer->SetAnimation(PLAYER_ATTACK1);
	

#ifndef GAME_DLL
	FireProjectile(pPlayer);
#else
	CTFProjectile_Goo* pProjectile = static_cast<CTFProjectile_Goo*>(FireProjectile(pPlayer));

	if(!pProjectile)
		return;

	float newModelScale = RemapValClamped(flPercentageCharged, 0.0f, 1.0f, 0.5f, 1.0f);
	pProjectile->SetModelScale(newModelScale);
	pProjectile->SetDamageRadius(((TF_WEAPON_GOO_RADIUS_MAX - TF_WEAPON_GOO_RADIUS_MIN) * flPercentageCharged) + TF_WEAPON_GOO_RADIUS_MIN);
	pProjectile->SetGooType(GooType);
	pProjectile->SetPropGooLifetime(RemapValClamped(flPercentageCharged, 0.0f, 1.0f, sf_googun_goo_lifetime_min.GetFloat(), sf_googun_goo_lifetime_max.GetFloat()));

	float flChargedDamageValue = flPercentageCharged * GetProjectileDamage();
	pProjectile->SetDamage(flChargedDamageValue);

	//CALL_ATTRIB_HOOK_INT(nMaxGoo, add_max_gooprojectiles);

	//If the goo has an invalid type, delete it and stop the function
	if (!AddGoo(pProjectile))
	{
		pProjectile->RemoveThis();
		m_flChargeBeginTime = 0;
		return;
	}

	pPlayer->SpeakWeaponFire();
	//CTF_GameStats.Event_PlayerFiredWeapon( pPlayer, IsCurrentAttackACrit() );
#endif

	//Remove ammo based on charge time
	//SetPrimaryAmmoCount(GetPrimaryAmmoCount() - (int(flPercentageCharged * sf_googun_ammo_cost_max.GetInt())));

	int iAmmoToRemove = 0;
	switch (GooType)
	{
		case TF_GOO_TOXIC:
			// Charged ammo to remove
			iAmmoToRemove = Ceil2Int(flPercentageCharged * (sf_googun_ammo_cost_toxic_max.GetInt() - 1));
			break;
		case TF_GOO_JUMP:
			// Remove a flat 75
			iAmmoToRemove = sf_googun_ammo_cost_movement.GetInt();
			break;
	}

	pPlayer->RemoveAmmo(iAmmoToRemove, m_iPrimaryAmmoType);
	

	// Set next attack times.
	float flDelay = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay;
	CALL_ATTRIB_HOOK_FLOAT( flDelay, mult_postfiredelay );
	m_flNextPrimaryAttack = gpGlobals->curtime + flDelay;

	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );

	//m_flNextPrimaryAttack = gpGlobals->curtime + flDelay;

	// Check the reload mode and behave appropriately.
	if ( m_bReloadsSingly )
	{
		m_iReloadMode.Set( TF_RELOAD_START );
	}

	m_flChargeBeginTime = 0;
}

bool CTFGooGun::Reload(void)
{
	if (m_flChargeBeginTime > 0)
		return false;

	return BaseClass::Reload();
}