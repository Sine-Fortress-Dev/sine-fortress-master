#ifndef TF_PROP_GOOPUDDLE
#define TF_PROP_GOOPUDDLE

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define CTFPropGooPuddle C_TFPropGooPuddle
#else
#include "iscorer.h"
#endif

#define MAX_PUDDLES 32
#define PUDDLE_TOXIC_MATERIAL "models/weapons/c_models/c_tox_gun/prop_goopuddle_toxic_custom.vmt"
#define PUDDLE_JUMP_MATERIAL "models/weapons/c_models/c_tox_gun/prop_goopuddle_jump_custom.vmt"
#define PUDDLE_MIN_SIZE 70.0f
#define PUDDLE_MAX_SIZE 90.0f
#define PUDDLE_MAX_HALF_WIDTH PUDDLE_MAX_SIZE / 2.0f
#define PUDDLE_MAX_HEIGHT 80.0f
#define PUDDLE_FADE_TIME 1.7f
#define PUDDLE_DAMAGE_BUILDINGS_INTERVAL 1.0f

#ifdef GAME_DLL
class CTFPropGooPuddle : public CBaseAnimating, public IScorer
#else
class CTFPropGooPuddle : public CBaseAnimating
#endif
{
	DECLARE_CLASS(CTFPropGooPuddle, CBaseAnimating);
	DECLARE_NETWORKCLASS();

#ifdef GAME_DLL
private:
	enum PropPuddleState
	{
		PROPPUDDLESTATE_STARTING = 0,
		PROPPUDDLESTATE_SPREADING,
		PROPPUDDLESTATE_ACTIVE,
		PROPPUDDLESTATE_DYING
	};

	struct puddleinfo_t
	{
		Vector pos;
		bool finishedSpreading;
		CountdownTimer lifetime;
	};
#else
	enum PuddleState
	{
		PUDDLESTATE_UNINITIALIZED = 0,
		PUDDLESTATE_STARTING,
		PUDDLESTATE_ACTIVE,
		PUDDLESTATE_FINISHING,
		PUDDLESTATE_DEAD,
	};

	struct c_puddleinfo_t
	{
		Vector pos;
		Vector normal;
		float size;
		float maxSize;
		PuddleState state;
		CountdownTimer lifetime;

		void SetState(PuddleState state)
		{
			this->state = state;
		}
	};
	//DECLARE_DATADESC();
#endif
public:
#ifdef GAME_DLL
	CTFPropGooPuddle();
	~CTFPropGooPuddle();

	virtual void		Spawn();
	virtual void		Precache();

	void				RecalculateBounds();
	void				AffectEntitiesInBounds();

	void SetRadius(float radius) { m_flRadius.Set(radius); }
	float GetRadius() { return m_flRadius.Get(); }

	void SetDamage(float damage) { m_flDamage = damage; }
	float GetDamage() { return m_flDamage; }

	void SetCritical(bool bCritical) { m_bCritical = bCritical; }
	bool GetCritical() { return m_bCritical; }

	// IScorer interface
	void			SetScorer(CBaseEntity* pScorer) { m_Scorer = pScorer; }
	virtual CBasePlayer* GetScorer(void) { return NULL; }
	virtual CBasePlayer* GetAssistant(void) { return dynamic_cast<CBasePlayer*>(m_Scorer.Get()); }

	// Check tf_sharddefs for goo types enum
	void SetGooType(int nGooType) { m_nGooType = nGooType; }
	int GetGooType(void) { return m_nGooType; }

	void SetLifetime( float flLifetime ) { m_flLifetime = flLifetime; }
	float GetLifetime( void ) { return m_flLifetime; }

	virtual void		SetState(PropPuddleState state);
	virtual bool		CreatePuddle(Vector parentPosition, int *traceCount = nullptr);
	virtual int			Spread();
	virtual Vector		GroundPuddle(Vector pos, int *tracecount = nullptr);
	virtual void		PuddleThink();

#else
	CTFPropGooPuddle();
	~CTFPropGooPuddle();

	virtual void		Spawn();
	virtual void		Precache();

	virtual void		OnDataChanged(DataUpdateType_t updateType);
	float				GetTimeSinceSpawned();

	void				InitializeNewPuddleData();
	void				InitializePuddleInfo(c_puddleinfo_t* info, Vector pos);
	virtual void		ClientThink();
	void				KillPuddle(c_puddleinfo_t* info);
	//void				UpdatePuddleCount();

	void				RecalculateBounds();
	virtual void		GetRenderBounds(Vector& mins, Vector& maxs);
	virtual void		GetRenderBoundsWorldspace(Vector& mins, Vector& maxs);
	virtual int			DrawModel(int flags);
	void				DrawPuddle(c_puddleinfo_t*info, color32 color);
#endif

private:
#ifdef GAME_DLL
	// Networked
	CNetworkArray(Vector, m_vecPuddlePos, MAX_PUDDLES);
	CNetworkVar(int, m_PuddleCount);
	CNetworkVar(float, m_flRadius);
	CNetworkVar(float, m_flLifetime);
	CNetworkVar(int, m_nGooType);
	CNetworkVar(bool, m_bCritical);

	// Not Networked
	PropPuddleState m_PropPuddleState;
	puddleinfo_t *m_puddles[MAX_PUDDLES];

	bool m_bBoundsNeedRecomputing;
	Vector m_minBounds;
	Vector m_maxBounds;

	CountdownTimer m_DamageBuildingsTimer;

	EHANDLE m_Scorer;

	float m_flDamage;

	float m_flStateTimestamp;
	int m_CurPuddleOffset;
	bool m_FinishedSpreading;
#else

	// Networked
	Vector m_vecPuddlePos[MAX_PUDDLES];
	int m_PuddleCount;
	float m_flRadius;
	float m_flLifetime;
	int m_nGooType;
	bool m_bCritical;

	//Not networked
	c_puddleinfo_t m_puddleArray[MAX_PUDDLES];

	CMaterialReference m_pMaterial;
	bool m_bBoundsNeedRecomputed;

	float m_flSpawnTimestamp;

	Vector m_minBounds;
	Vector m_maxBounds;

	//Used for sound events
	int m_iInitializedPuddles;
	int m_iActivePuddles;

	int m_nLoopSoundGuid;
	bool m_bStopPlayingLoopSound;
#endif
};
#endif // TF_PROP_GOOPUDDLE
