#include "cbase.h"
#include "tf_projectile_energy_laser.h"

#ifdef GAME_DLL
#include "soundent.h"
#include "tf_fx.h"
#include "props.h"
#include "te_effect_dispatch.h"
#include "tf_player.h"
#include "te_effect_dispatch.h"
#endif

#include "baseobject_shared.h"
#include "SpriteTrail.h"
#include "IEffects.h"
#include "collisionutils.h"
#include "bone_setup.h"
#include "decals.h"
#include "tf_gamerules.h"
#include "tf_weapon_rocketlauncher.h"

#define ENERGY_BALL_MODEL					"models/weapons/w_models/w_drg_ball.mdl"


//const char* g_pszBisonImpactWorldSound("Weapon_Bison.ProjectileImpactFlesh");
//const char* g_pszBisonTrailParticle("drg_bison_projectile");
//const char* g_pszBisonTrailParticleCrit("drg_bison_projectile_crit");

//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(tf_projectile_energy_laser, CTFProjectile_Laser);
PRECACHE_WEAPON_REGISTER(tf_projectile_energy_laser);

IMPLEMENT_NETWORKCLASS_ALIASED(TFProjectile_Laser, DT_TFProjectile_Laser)

BEGIN_NETWORK_TABLE(CTFProjectile_Laser, DT_TFProjectile_Laser)
#ifdef GAME_DLL
	SendPropBool(SENDINFO(m_bCritical)),
#else
	RecvPropBool(RECVINFO(m_bCritical)),
#endif
END_NETWORK_TABLE()

BEGIN_DATADESC(CTFProjectile_Laser)
END_DATADESC()

#ifdef GAME_DLL
ConVar sf_raygun_sci_tick_time("sf_raygun_sci_tick_time", "0.05", FCVAR_CHEAT);

static bool EntityLessInfo(const EHANDLE& key1, const EHANDLE& key2)
{
	return key1.ToInt() < key2.ToInt();
}
#endif

CTFProjectile_Laser::CTFProjectile_Laser()
{
#ifdef GAME_DLL
	m_flLastHitTime = 0.f;
	m_EntTouchTracker.SetLessFunc(EntityLessInfo);
#endif
}

CTFProjectile_Laser::~CTFProjectile_Laser()
{
#ifdef GAME_DLL
	m_EntTouchTracker.Purge();
#endif
}

#ifdef GAME_DLL

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFProjectile_Laser* CTFProjectile_Laser::Create(const Vector& vecOrigin, const QAngle& vecAngles, const float fSpeed, const float fGravity, CBaseEntity* pOwner, CBaseEntity* pScorer)
{
	CTFProjectile_Laser* pBall = NULL;
	pBall = static_cast<CTFProjectile_Laser*>(CBaseEntity::Create("tf_projectile_energy_laser", vecOrigin, vecAngles, pOwner));
	if (pBall)
	{
		pBall->InitLaser(vecOrigin, vecAngles, fSpeed, fGravity, pOwner, pScorer);
	}
	return pBall;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Laser::InitLaser(const Vector& vecOrigin, const QAngle& vecAngles, const float fSpeed, const float fGravity, CBaseEntity* pOwner, CBaseEntity* pScorer)
{
	// Initialize the owner.
	SetOwnerEntity(pOwner);

	// Set team.
	ChangeTeam(pOwner->GetTeamNumber());

	// Spawn.
	Spawn();

	SetGravity(fGravity);

#ifdef GAME_DLL
	SetCritical(true);
#endif

	// Setup the initial velocity.
	Vector vecForward, vecRight, vecUp;
	AngleVectors(vecAngles, &vecForward, &vecRight, &vecUp);

	Vector vecVelocity = vecForward * GetInitialVelocity();

	SetAbsVelocity(vecVelocity);
#ifdef GAME_DLL
	SetupInitialTransmittedGrenadeVelocity(vecVelocity);
#endif

	// Setup the initial angles.
	QAngle angles;
	VectorAngles(vecVelocity, angles);
	SetAbsAngles(angles);

	// Save the scoring player.
	SetScorer(pScorer);
#ifdef GAME_DLL
	if (pScorer)
	{
		SetTruceValidForEnt(pScorer->IsTruceValidForEnt());
	}
#endif

	m_flInitTime = gpGlobals->curtime;
}

#endif
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Laser::Spawn()
{
	BaseClass::Spawn();

	SetModel(ENERGY_BALL_MODEL);
	SetSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
#ifdef GAME_DLL
	SetTouch(&CTFProjectile_Laser::RocketTouch);
#endif
	RemoveFlag(FL_GRENADE);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Laser::Precache()
{
	//PrecacheParticleSystem("drg_cow_rockettrail_normal");
	//PrecacheParticleSystem("drg_cow_rockettrail_normal_blue");

	PrecacheParticleSystem("derstrahl_trail_crit_red");
	PrecacheParticleSystem("derstrahl_trail_red");
	PrecacheParticleSystem("derstrahl_trail_crit_blue");
	PrecacheParticleSystem("derstrahl_trail_blue");

	PrecacheModel(ENERGY_BALL_MODEL);

	PrecacheScriptSound("Weapon_CowMangler.Explode");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Laser::SetScorer(CBaseEntity* pScorer)
{
	m_Scorer = pScorer;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBasePlayer* CTFProjectile_Laser::GetScorer(void)
{
	return dynamic_cast<CBasePlayer*>(m_Scorer.Get());
}

#ifdef GAME_DLL
void CTFProjectile_Laser::RocketTouch(CBaseEntity* pOther)
{
	// Copied from tf_projectile_energy_ring :sob: -Vruk

	// Verify a correct "other."
	Assert(pOther);
	if (!pOther ||
		!pOther->IsSolid() ||
		pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) ||
		(pOther->GetCollisionGroup() == TFCOLLISION_GROUP_RESPAWNROOMS) ||
		pOther->IsFuncLOD())
	{
		return;
	}

	CBaseEntity* pOwner = GetOwnerEntity();
	// Don't shoot ourselves
	if (pOwner == pOther)
		return;

	// Handle hitting skybox (disappear).
	const trace_t* pTrace = &CBaseEntity::GetTouchTrace();
	if (pTrace->surface.flags & SURF_SKY)
	{
		UTIL_Remove(this);
		return;
	}

	// pass through ladders
	if (pTrace->surface.flags & CONTENTS_LADDER)
		return;

	if (!ShouldTouchNonWorldSolid(pOther, pTrace))
		return;
	
	touchlink_t* link;
	touchlink_t* root = (touchlink_t*)GetDataObject(TOUCHLINK);
	bool bTouching = false;
	if (root)
	{
		for (link = root->nextLink; link != root; link = link->nextLink)
		{
			// Already touching this entity, don't fire again
			if (link->entityTouched == pOther)
			{
				bTouching = true;

				float flLastHit = m_EntTouchTracker.FindElement(pOther, -1.0f);
				if (flLastHit == -1.0f)
				{
					// We haven't hit this entity before, add an entry right now cause
					// we are gonna hit it
					m_EntTouchTracker.Insert(pOther, gpGlobals->curtime);
				}
				else
				{
					// We've hit this entity before
					// if we haven't waited long enough, don't hit it again

					if (gpGlobals->curtime < flLastHit + sf_raygun_sci_tick_time.GetFloat())
						return;
				}
				break;
			}
		}
		// We didn't find the entity in our touch list, see if it is in our tracker
		if (!bTouching)
		{
			float flLastHit = m_EntTouchTracker.FindElement(pOther, -1.0f);
			if (flLastHit == -1.0f)
			{
				// We haven't hit this entity before, add an entry for right now
				m_EntTouchTracker.Insert(pOther, gpGlobals->curtime);
			}
			else
			{
				// We've hit this entity before, but we stopped touching it at some point
				// if we haven't waited long enough, don't hit it again
				if (flLastHit + sf_raygun_sci_tick_time.GetFloat() < gpGlobals->curtime)
					return;
			}

		}
	}

	// The stuff we collide with
	bool bCombatEntity = pOther->IsPlayer() ||
		pOther->IsBaseObject() ||
		pOther->IsCombatCharacter() ||
		pOther->IsCombatItem() ||
		pOther->IsProjectileCollisionTarget();

	if (bCombatEntity)
	{
		// Bison projectiles shouldn't collide with friendly things
		if (ShouldPenetrate() && (pOther->InSameTeam(this) || (gpGlobals->curtime - m_flLastHitTime) < sf_raygun_sci_tick_time.GetFloat()))
			return;

		m_flLastHitTime = gpGlobals->curtime;

		const int nDamage = GetDamage();

		CTakeDamageInfo info(this, pOwner, GetLauncher(), nDamage, GetDamageType(), TF_DMG_CUSTOM_PLASMA);
		info.SetReportedPosition(pOwner->GetAbsOrigin());
		info.SetDamagePosition(pTrace->endpos);

		if (info.GetDamageType() & DMG_CRITICAL)
		{
			info.SetCritType(CTakeDamageInfo::CRIT_FULL);
		}

		trace_t traceAttack;
		UTIL_TraceLine(WorldSpaceCenter(), pOther->WorldSpaceCenter(), MASK_SOLID | CONTENTS_HITBOX, this, COLLISION_GROUP_NONE, &traceAttack);

		pOther->DispatchTraceAttack(info, GetAbsVelocity(), &traceAttack);

		ApplyMultiDamage();

		// Get a position on whatever we hit
		Vector vecDelta = pOther->GetAbsOrigin() - GetAbsOrigin();
		Vector vecNormalVel = GetAbsVelocity().Normalized();
		Vector vecNewPos = (DotProduct(vecDelta, vecNormalVel) * vecNormalVel) + GetAbsOrigin();

		PlayImpactEffects(vecNewPos);

		if (ShouldPenetrate())
			return;

		UTIL_Remove(this);
		return;
	}

	if (pOther->IsWorld())
	{
		SetAbsVelocity(vec3_origin);
		AddSolidFlags(FSOLID_NOT_SOLID);
	}

	PlayImpactEffects(pTrace->endpos);

	trace_t trace;
	memcpy(&trace, pTrace, sizeof(trace_t));
	Explode(&trace, pOther);

	// Remove by default.  Fixes this entity living forever on things like doors.
	UTIL_Remove(this);
}

void CTFProjectile_Laser::ResolveFlyCollisionCustom(trace_t& trace, Vector& vecVelocity)
{
	// Also copied from tf_projectile_energy_ring -Vruk
	PlayImpactEffects(trace.endpos);

	// Remove by default.  Fixes this entity living forever on things like doors.
	UTIL_Remove(this);
}

void CTFProjectile_Laser::PlayImpactEffects(const Vector& vecPos)
{
	/*CTFWeaponBaseGun* pTFGun = dynamic_cast<CTFWeaponBaseGun*>(GetLauncher());
	if (pTFGun)
	{
		//DispatchParticleEffect(g_pszEnergyProjectileImpactParticle, vecPos, GetAbsAngles(), pTFGun->GetParticleColor(1), pTFGun->GetParticleColor(2), true, NULL, 0);

		//EmitSound(pszSoundString);
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Laser::Explode(trace_t* pTrace, CBaseEntity* pOther)
{
	if (ShouldNotDetonate())
	{
		Destroy(true);
		return;
	}

	// Save this entity as enemy, they will take 100% damage.
	m_hEnemy = pOther;

	// Invisible.
	SetModelName(NULL_STRING);
	AddSolidFlags(FSOLID_NOT_SOLID);
	m_takedamage = DAMAGE_NO;

	// Pull out a bit.
	if (pTrace->fraction != 1.0)
	{
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 1.0f));
	}

	// Particle (oriented)
	Vector vecOrigin = GetAbsOrigin();
	CPVSFilter filter(vecOrigin);
	QAngle angExplosion(0.f, 0.f, 0.f);
	VectorAngles(pTrace->plane.normal, angExplosion);
	TE_TFParticleEffect(filter, 0.f, GetExplosionParticleName(), vecOrigin, pTrace->plane.normal, angExplosion, NULL);

	// Sound
	ImpactSound("Weapon_CowMangler.Explode");
	CSoundEnt::InsertSound(SOUND_COMBAT, vecOrigin, 1024, 3.0);

	// Damage.
	CBaseEntity* pAttacker = GetOwnerEntity();
	IScorer* pScorerInterface = dynamic_cast<IScorer*>(pAttacker);
	if (pScorerInterface)
	{
		pAttacker = pScorerInterface->GetScorer();
	}

	float flRadius = GetRadius();

	if (pAttacker) // No attacker, deal no damage. Otherwise we could potentially kill teammates.
	{
		CTFPlayer* pTarget = ToTFPlayer(GetEnemy());
		if (pTarget)
		{
			// Rocket Specialist
			CheckForStunOnImpact(pTarget);

			if (pTarget->GetTeamNumber() != pAttacker->GetTeamNumber())
			{
				RecordEnemyPlayerHit(pTarget, true);
			}
		}

		CTakeDamageInfo info(this, pAttacker, GetOriginalLauncher(), vec3_origin, vecOrigin, GetDamage(), GetDamageType(), GetDamageCustom());
		CTFRadiusDamageInfo radiusinfo(&info, vecOrigin, flRadius, NULL, TF_ROCKET_RADIUS_FOR_RJS);
		TFGameRules()->RadiusDamage(radiusinfo);
	}

	// Don't decal players with scorch.
	if (!pOther->IsPlayer())
	{
		UTIL_DecalTrace(pTrace, "Scorch");
	}

	// Remove the rocket.
	UTIL_Remove(this);

	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFProjectile_Laser::GetDamage()
{
	float flDmgMult = 1.0f;
	if(m_bCritical)
		flDmgMult = RemapValClamped(gpGlobals->curtime - m_flInitTime, 0.45f, 0.8f, 1.0f, 0.50f);
	return m_flDamage * flDmgMult;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFProjectile_Laser::GetDamageType()
{
	int iDamageType = DMG_BLAST | DMG_PREVENT_PHYSICS_FORCE;
	if (m_bCritical)
	{
		iDamageType |= DMG_CRITICAL;
	}

	return iDamageType;
}

float CTFProjectile_Laser::GetRadius()
{
	float flRadius = TF_LAZER_RADIUS;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(GetOriginalLauncher(), flRadius, mult_explosion_radius);
	return flRadius;
}

#else

void CTFProjectile_Laser::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED)
	{
		CNewParticleEffect* pEffect = ParticleProp()->Create(GetTrailParticleName(), PATTACH_ABSORIGIN_FOLLOW);
	}
}

#endif

//-----------------------------------------------------------------------------
// Purpose: Plays an impact sound. Louder for the attacker.
//-----------------------------------------------------------------------------
void CTFProjectile_Laser::ImpactSound(const char* pszSoundName, bool bLoudForAttacker)
{
	CTFPlayer* pAttacker = ToTFPlayer(GetScorer());
	if (!pAttacker)
		return;

	if (bLoudForAttacker)
	{
		float soundlen = 0;
		EmitSound_t params;
		params.m_flSoundTime = 0;
		params.m_pSoundName = pszSoundName;
		params.m_pflSoundDuration = &soundlen;
		CPASFilter filter(GetAbsOrigin());
		filter.RemoveRecipient(ToTFPlayer(pAttacker));
		EmitSound(filter, entindex(), params);

		CSingleUserRecipientFilter attackerFilter(ToTFPlayer(pAttacker));
		EmitSound(attackerFilter, pAttacker->entindex(), params);
	}
	else
	{
		EmitSound(pszSoundName);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Laser::FadeOut(int iTime)
{
	SetMoveType(MOVETYPE_NONE);
	SetAbsVelocity(vec3_origin);
	AddSolidFlags(FSOLID_NOT_SOLID);
	AddEffects(EF_NODRAW);

	// Start remove timer.
	SetContextThink(&CTFProjectile_Laser::RemoveThink, gpGlobals->curtime + iTime, "ENERGY_LASER_REMOVE_THINK");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Laser::RemoveThink(void)
{
#ifdef GAME_DLL
	UTIL_Remove(this);
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char* CTFProjectile_Laser::GetExplosionParticleName(void)
{
	return "ExplosionCore_wall";
}

bool CTFProjectile_Laser::ShouldPenetrate() const
{
	//int iPenetrate = 0;
	//CALL_ATTRIB_HOOK_INT_ON_OTHER(GetOwnerEntity(), iPenetrate, energy_weapon_penetration);

	//return iPenetrate != 0;
	return true;
}

const char* CTFProjectile_Laser::GetTrailParticleName() const
{
	if ((GetTeamNumber() == TF_TEAM_RED))
	{
		return m_bCritical ? "derstrahl_trail_crit_red" : "derstrahl_trail_red";
	}
	else
	{
		return m_bCritical ? "derstrahl_trail_crit_blue" : "derstrahl_trail_blue";
	}
}