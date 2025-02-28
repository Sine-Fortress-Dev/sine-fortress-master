//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_GOOGUN_H
#define TF_WEAPON_GOOGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"
#include "tf_projectile_goo.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFGooGun C_TFGooGun
#endif

//=============================================================================
//
// TF Weapon GooGun gun.
//
class CTFGooGun : public CTFWeaponBaseGun, public ITFChargeUpWeapon
{
public:

	DECLARE_CLASS( CTFGooGun, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

// Server specific.
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CTFGooGun() {}
	~CTFGooGun() {}

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_GOOGUN; }
	virtual void 	PrimaryAttack( void );
	virtual void 	SecondaryAttack(void);
			//void	SwitchGoo(void);
	virtual void	WeaponIdle(void);
	virtual void	CTFGooGun::ItemBusyFrame(void);
	virtual bool	CTFGooGun::Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool	Deploy(void);
			bool	AddGoo(CTFProjectile_Goo *pGoo);
	virtual float	GetProjectileSpeed(void);
	virtual bool	Reload(void);
	virtual void 	FireGoo( int GooType );
	virtual int		GetGooType();

public:

	// ITFChargeUpWeapon
	virtual bool		CanCharge(void) { return true; }
	virtual float		GetChargeBeginTime( void ) 	{ return m_flChargeBeginTime; }
	virtual float		GetChargeMaxTime( void );
	virtual float		GetCurrentCharge(void);
	virtual const char 	*GetChargeSound( void ) 	{ return NULL; }

	typedef CHandle<CTFProjectile_Goo>	GooProjectilesHandle;
	CUtlVector<GooProjectilesHandle>		m_ToxicGooProjectiles;
	CUtlVector<GooProjectilesHandle>		m_MovementGooProjectiles;

/* 
//Figure this out sometime if needed
#ifdef GAME_DLL

private:
	// This is here so we can network the pipebomb count for prediction purposes
	CNetworkVar( int,				m_iGooBlobCount );	
#endif

#ifdef CLIENT_DLL
	int				m_iGooBlobCount;
#endif

	// List of active goo globs
	typedef CHandle<CBaseEntity>	GooGlobHandle;
	CUtlVector<GooGlobHandle>		m_GooGlobs;
*/

private:

	//false = slot 1, true = slot 2
	//bool m_GooSlotTwo = false;

	float	m_flChargeBeginTime;

	CTFGooGun( const CTFGooGun & ) {}
};

#endif // TF_WEAPON_GOOGUN_H
