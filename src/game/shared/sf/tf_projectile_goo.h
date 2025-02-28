//=============================================================================//
//
// Purpose: Goo Projectile
//
//=============================================================================//

#ifndef TF_PROJECTILE_GOO_H
#define TF_PROJECTILE_GOO_H

#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_grenadeproj.h"
#ifdef GAME_DLL
#include "iscorer.h"
#endif

#ifdef CLIENT_DLL
#define CTFProjectile_Goo C_TFProjectile_Goo
#endif

#define TF_WEAPON_GOO_ACID_MODEL 	"models/props_2fort/milkjug001.mdl"
#define TF_WEAPON_GOO_JUMP_MODEL 	"models/weapons/w_models/w_gooball/w_gooball.mdl"
#define TF_WEAPON_GOO_RADIUS_MAX 	150
#define TF_WEAPON_GOO_RADIUS_MIN	50
#define TF_WEAPON_GOO_MAXFALL 	120
#define TF_WEAPON_GOO_DAMAGE_INTERVAL 0.5f
#define TF_WEAPON_GOO_AIR_LIFETIME 7.0f

#ifdef GAME_DLL
class CTFProjectile_Goo : public CTFWeaponBaseGrenadeProj, public IScorer
#else
class C_TFProjectile_Goo : public C_TFWeaponBaseGrenadeProj
#endif
{
public:
	DECLARE_CLASS(CTFProjectile_Goo, CTFWeaponBaseGrenadeProj);
	DECLARE_NETWORKCLASS();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CTFProjectile_Goo();
	~CTFProjectile_Goo();

	virtual int	GetWeaponID(void) const 			{ return TF_WEAPON_GOOGUN; }

	// Goes from 1-4 for acid damage
	void SetGooLevel( int nGooLevel ) { m_nGooLevel = nGooLevel;}
	int GetGooLevel ( void ) { return m_nGooLevel; }

	// Check tf_sharddefs for goo types enum
	void SetGooType ( int nGooType ) { m_nGooType = nGooType; }
	int GetGooType ( void ) { return m_nGooType; }

	void SetPropGooLifetime( float flPropGooLifetime ) { m_flPropGooLifetime = flPropGooLifetime; }
	float GetPropGooLifetime( void ) { return m_flPropGooLifetime; }


	virtual void	Precache(void);
	virtual void	Spawn(void);

	void			SetCritical(bool bCritical) { m_bCritical = bCritical; }

	void			GooTouch(CBaseEntity *pOther);
	virtual void	Expand(void);
	virtual void	Explode(trace_t* pTrace, int bitsDamageType);

	void			RemoveThis(void);

	virtual bool	IsDeflectable() { return !m_bIsPuddle; }

#ifdef GAME_DLL //GAME_DLL

	static CTFProjectile_Goo* Create(CBaseEntity* pWeapon, const Vector& vecOrigin, const QAngle& vecAngles, const Vector& vecVelocity, CBaseCombatCharacter* pOwner, CBaseEntity* pScorer, const AngularImpulse& angVelocity, const CTFWeaponInfo& weaponInfo);

	virtual void	Detonate(void);
	virtual void	VPhysicsUpdate(IPhysicsObject* pPhysics);
	virtual void	VPhysicsCollision(int index, gamevcollisionevent_t* pEvent);

	void			PuddleThink(void);

	// IScorer interface
	void			SetScorer(CBaseEntity* pScorer);
	virtual CBasePlayer* GetScorer(void) { return NULL; }
	virtual CBasePlayer* GetAssistant(void);

	virtual void	Deflected(CBaseEntity* pDeflectedBy, Vector& vecDir);
#else

	virtual void	OnDataChanged(DataUpdateType_t updateType);
	virtual void	CreateTrails(void);
	virtual void	CreatePuddleEffects(void);
	virtual int		DrawModel(int flags);

#endif

private:
	int					m_nGooLevel;

	// Check tf_sharddefs for goo types enum
#ifdef GAME_DLL
	EHANDLE m_Scorer;
	CNetworkVar( bool, m_bCritical );
	CNetworkVar(float, m_fPuddleStartTime);
	CNetworkVar(bool, m_bIsPuddle);
	CNetworkVar(int, m_nGooType);
	CNetworkVar(float, m_flPropGooLifetime);


	float				m_flAcidBurnTime;
	float				m_flLifeTime;
#else
public:
	bool				m_bCritical;
	float				m_fPuddleStartTime;
	bool				m_bIsPuddle;
	bool				m_bWasPuddle;
	int					m_nGooType;
	float				m_flPropGooLifetime;
#endif
	
	float		m_flCreationTime;
};
#endif // TF_PROJECTILE_GOO_H