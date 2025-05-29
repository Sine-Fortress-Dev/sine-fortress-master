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
#include "tf_prop_goopuddle.h"

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
	// to allow only this class to add itself to our goo list
	friend class CTFProjectile_Goo;

public:

	DECLARE_CLASS( CTFGooGun, CTFWeaponBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

// Server specific.
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CTFGooGun();
	~CTFGooGun() {}

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_GOOGUN; }
	virtual void 	PrimaryAttack( void );
	virtual void 	SecondaryAttack(void);
			//void	SwitchGoo(void);
	virtual void	WeaponIdle(void);
	virtual void	ItemBusyFrame(void);
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool	Deploy(void);
	virtual float	GetProjectileSpeed(void);
	virtual bool	Reload(void);
	virtual void 	FireGoo( int GooType );
	virtual int		GetGooType();

protected:
#ifdef GAME_DLL
	bool			AddGoo(CTFPropGooPuddle* pGoo);
#endif

public:

	// ITFChargeUpWeapon
	virtual bool		CanCharge(void) { return true; }
	virtual float		GetChargeBeginTime( void ) 	{ return m_flChargeBeginTime; }
	virtual float		GetChargeMaxTime( void );
	virtual float		GetCurrentCharge(void);
	virtual const char 	*GetChargeSound( void ) 	{ return NULL; }

#ifdef GAME_DLL
	typedef CHandle<CTFPropGooPuddle>	GooPuddlesHandle;
	CUtlVector<GooPuddlesHandle>		m_MovementGooPuddles;
#endif

private:

	//false = slot 1, true = slot 2
	//bool m_GooSlotTwo = false;

	float	m_flChargeBeginTime;

	CTFGooGun( const CTFGooGun & ) {}
};

#endif // TF_WEAPON_GOOGUN_H
