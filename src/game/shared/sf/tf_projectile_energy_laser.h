#ifndef TF_PROJECTILE_ENERGY_LASER_H
#define TF_PROJECTILE_ENERGY_LASER_H
#ifdef _WIN32
#pragma once
#endif

#ifdef GAME_DLL
#include "tf_player.h"
#include "iscorer.h"
#endif
#include "tf_weaponbase_rocket.h"


#ifdef CLIENT_DLL
#define CTFProjectile_Laser C_TFProjectile_Laser
#endif

#define TF_LAZER_RADIUS 		 72.0f

class CTFProjectile_Laser : public CTFBaseRocket//, public IScorer
{
public:

	DECLARE_CLASS(CTFProjectile_Laser, CTFBaseRocket);
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

	CTFProjectile_Laser();
	~CTFProjectile_Laser();

	// Creation.
#ifdef GAME_DLL
	static CTFProjectile_Laser* Create(const Vector& vecOrigin, const QAngle& vecAngles, const float fSpeed, const float fGravity, CBaseEntity* pOwner = NULL, CBaseEntity* pScorer = NULL);
	virtual void	InitLaser(const Vector& vecOrigin, const QAngle& vecAngles, const float fSpeed, const float fGravity, CBaseEntity* pOwner = NULL, CBaseEntity* pScorer = NULL);
#endif
	
	
	virtual void	Spawn();
	virtual void	Precache();
	virtual int		GetWeaponID(void) const { return TF_WEAPON_RAYGUN_SCIENTIST; }

	// IScorer interface
	virtual CBasePlayer* GetScorer(void);
	virtual CBasePlayer* GetAssistant(void) { return NULL; }

#ifdef GAME_DLL
	virtual void	RocketTouch(CBaseEntity* pOther) OVERRIDE;
	virtual void	ResolveFlyCollisionCustom(trace_t& trace, Vector& vecVelocity) OVERRIDE;
	virtual void	Explode(trace_t* pTrace, CBaseEntity* pOther) OVERRIDE;
	
	virtual float	GetDamage();
	virtual int		GetDamageCustom() { return TF_DMG_CUSTOM_PLASMA; }
	virtual int		GetDamageType();
	virtual float	GetRadius();
	
	void			SetCritical(bool bCritical) { m_bCritical = bCritical; }
#else
	virtual void	OnDataChanged(DataUpdateType_t updateType) OVERRIDE;
#endif

	virtual float	GetInitialVelocity(void) { return 1200.f; }

	void			SetScorer(CBaseEntity* pScorer);

	virtual bool	CanHeadshot() { return false; }

	void			ImpactSound(const char* pszSoundName, bool bLoudForAttacker = false);
	virtual void	ImpactTeamPlayer(CTFPlayer* pOther) {}

	void			FadeOut(int iTime);
	void			RemoveThink();

	virtual bool	IsDeflectable() { return false; }

	const char* GetExplosionParticleName(void);

private:

	bool			ShouldPenetrate() const;
	const char* GetTrailParticleName() const;

#ifdef GAME_DLL
	float m_flLastHitTime;
	void PlayImpactEffects(const Vector& vecPos );
	CNetworkVar(bool, m_bCritical);

	CUtlMap<EHANDLE, float>	m_EntTouchTracker;
#else
	bool			m_bCritical;
#endif

	CBaseHandle		m_Scorer;

protected:
	float			m_flInitTime;
};

#endif	//TF_PROJECTILE_ENERGY_LASER_H
