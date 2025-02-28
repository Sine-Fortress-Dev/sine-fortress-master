#include "cbase.h"
#include "tf_projectile_goo.h"
#include "tf_gamerules.h"
#include "effect_dispatch_data.h"
#include "tf_weapon_googun.h"
#include "tf_player_shared.h"
#include "baseobject_shared.h"

#include "tf_prop_goopuddle.h"

#ifdef GAME_DLL
#include "tf_fx.h"
#else
#include "particles_new.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_Goo, DT_TFProjectile_Goo )

BEGIN_NETWORK_TABLE( CTFProjectile_Goo, DT_TFProjectile_Goo )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bCritical ) ),
	RecvPropFloat(RECVINFO(m_fPuddleStartTime)),
	RecvPropBool(RECVINFO(m_bIsPuddle)),
	RecvPropInt(RECVINFO(m_nGooType)),
	RecvPropFloat(RECVINFO(m_flPropGooLifetime)),
#else
	SendPropBool(SENDINFO(m_bCritical)),
	SendPropFloat(SENDINFO(m_fPuddleStartTime)),
	SendPropBool(SENDINFO(m_bIsPuddle)),
	SendPropInt(SENDINFO(m_nGooType)),
	SendPropFloat(SENDINFO(m_flPropGooLifetime)),
#endif
END_NETWORK_TABLE()

#ifdef GAME_DLL
BEGIN_DATADESC( CTFProjectile_Goo )
END_DATADESC()
#endif

ConVar sf_goo_show_radius("sf_goo_show_radius", "0", FCVAR_REPLICATED | FCVAR_ARCHIVE /*| FCVAR_DEVELOPMENTONLY*/, "Render goo radius");

LINK_ENTITY_TO_CLASS( tf_projectile_goo, CTFProjectile_Goo );
PRECACHE_REGISTER( tf_projectile_goo );

CTFProjectile_Goo::CTFProjectile_Goo()
{
}

CTFProjectile_Goo::~CTFProjectile_Goo()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Goo::Precache(void)
{
	PrecacheModel(TF_WEAPON_GOO_ACID_MODEL);
	PrecacheModel(TF_WEAPON_GOO_JUMP_MODEL);

	PrecacheParticleSystem("goo_trail_red");
	PrecacheParticleSystem("goo_trail_blue");
	PrecacheParticleSystem("peejar_impact");
	PrecacheParticleSystem("effect_acid_parent");

	//PrecacheScriptSound("Jar.Explode");
	PrecacheScriptSound("Weapon_GooGun.ProjectileImpactWorldCrit");
	PrecacheScriptSound("Weapon_GooGun.ProjectileImpactWorld");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Goo::Spawn( void )
{	
	// Checks the goo type and sets the appropriate model

	/*switch (m_nGooType) 
	{
	case TF_GOO_TOXIC:
	SetModel(TF_WEAPON_GOO_ACID_MODEL);
	break;

	case TF_GOO_JUMP:
	default:
	SetModel(TF_WEAPON_GOO_JUMP_MODEL);
	break;
	}*/

	SetModel(TF_WEAPON_GOO_JUMP_MODEL);
	
#ifdef GAME_DLL
	SetDetonateTimerLength(TF_WEAPON_GOO_AIR_LIFETIME);
#endif

	m_bIsPuddle = false;

	BaseClass::Spawn();
	SetTouch( &CTFProjectile_Goo::GooTouch );

	// Pumpkin Bombs
	AddFlag( FL_GRENADE );

	AddSolidFlags( FSOLID_TRIGGER );

	SetGooLevel(1);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Goo::GooTouch(CBaseEntity* pOther)
{
	if (m_bIsPuddle)
		return;

	if (pOther == GetThrower())
	{
		// Make us solid if we're not already
		if (GetCollisionGroup() == COLLISION_GROUP_NONE)
		{
			SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
		}
		return;
	}

	// Verify a correct "other."
	if (!pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS))
		return;

	// Handle hitting skybox (disappear).
	trace_t pTrace;
	Vector velDir = GetAbsVelocity();
	VectorNormalize(velDir);
	Vector vecSpot = GetAbsOrigin() - velDir * 32;
	UTIL_TraceLine(vecSpot, vecSpot + velDir * 64, MASK_SOLID, this, COLLISION_GROUP_NONE, &pTrace);

	if (pTrace.fraction < 1.0 && pTrace.surface.flags & SURF_SKY)
	{
#ifdef GAME_DLL
		UTIL_Remove(this);
#endif
		return;
	}

	// Blow up if we hit a player if we are not a puddle
	if (pOther->IsPlayer())
	{
		Explode(&pTrace, GetDamageType());
		Expand();
	}
	// We should bounce off of certain surfaces (resupply cabinets, spawn doors, etc.)
	else
	{
		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Expand goo into a puddle
//-----------------------------------------------------------------------------
void CTFProjectile_Goo::Expand()
{
	//TODO: Setmodel to puddle needs to go somewhere in this function once we have a model

	IPhysicsObject* pPhysicsObject = VPhysicsGetObject();
	if (pPhysicsObject)
	{
		pPhysicsObject->EnableMotion(false);
	}

	Vector DownDirection = GetAbsOrigin() + Vector(0, 0, -1) * TF_WEAPON_GOO_MAXFALL;
	trace_t pTrace;
	Ray_t ray;
	ray.Init(GetAbsOrigin(), DownDirection);
	UTIL_TraceRay(ray, MASK_PLAYERSOLID_BRUSHONLY & ~CONTENTS_PLAYERCLIP, this, COLLISION_GROUP_PLAYER_MOVEMENT, &pTrace);

	//needs to be pulled up out of the ground a bit which is the +5 z vector
	Vector endpos = pTrace.endpos + Vector(0, 0, 5);

	if (pTrace.DidHitWorld())
	{
#ifdef GAME_DLL

		//Teleport goo to the ground as a puddle 
		//Why doesn't this work?
		//SetAbsOrigin(endpos);

		//Just set the puddle to the endpos instead of teleporting this projectile, stupid. (me to myself) -Vruk
		//Teleport(&endpos, &GetAbsAngles(), NULL);
		//UTIL_SetOrigin(this, endpos);

		CTFPropGooPuddle* pProp = (CTFPropGooPuddle*)CBaseEntity::Create("sf_prop_goopuddle", endpos, GetAbsAngles(), GetThrower());
		
		pProp->SetScorer(GetScorer());
		pProp->ChangeTeam(GetTeamNumber());
		pProp->SetGooType(GetGooType());
		pProp->SetCritical(m_bCritical);
		pProp->SetDamage(GetDamage());
		pProp->SetRadius(GetDamageRadius());
		pProp->SetLifetime(GetPropGooLifetime());

		RemoveThis();
#endif
	}
	else
	{
		Detonate();
#ifdef GAME_DLL
		RemoveThis();
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Goo::Explode(trace_t* pTrace, int bitsDamageType)
{
	// Invisible.
	//SetModelName( NULL_STRING );
	
	//Don't change collision rules during callbacks, stupid. (me to myself) -Vruk
	//AddSolidFlags(FSOLID_NOT_SOLID);
	m_takedamage = DAMAGE_YES;

	// Pull out of the wall a bit
	if (pTrace->fraction != 1.0)
	{
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 1.0f));
	}

	// Pull out a bit.
	if (pTrace->fraction != 1.0)
	{
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 1.0f));
	}
#ifdef GAME_DLL

	// Play explosion sound and effect.
	Vector vecOrigin = GetAbsOrigin();
	//EmitSound( "Jar.Explode" );
	CPVSFilter filter(vecOrigin);
	TE_TFExplosion(filter, 0.0f, vecOrigin, pTrace->plane.normal, GetWeaponID(), -1);

	// Damage.
	//Grenades must use GetThrower, not GetOwnerEntity
	CBaseEntity* pAttacker = GetThrower();

	/*float flRadius = GetDamageRadius();

	CTFRadiusDamageInfo radiusInfo;
	radiusInfo.info.Set( this, pAttacker, m_hLauncher, vec3_origin, vecOrigin, GetDamage(), GetDamageType() );
	radiusInfo.m_vecSrc = vecOrigin;
	radiusInfo.m_flRadius = flRadius;
	radiusInfo.m_flSelfDamageRadius = 121.0f; // Original rocket radius?
	*/

	float flRadius = GetDamageRadius();

	/*
	// Use the thrower's position as the reported position
	Vector vecReported = GetThrower() ? GetThrower()->GetAbsOrigin() : vec3_origin;

	CTFRadiusDamageInfo radiusInfo;
	radiusInfo.info.Set(this, GetThrower(), m_hLauncher, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, TF_DMG_CUSTOM_NONE, &vecReported);
	radiusInfo.m_vecSrc = vecOrigin;
	radiusInfo.m_flRadius = flRadius;
	radiusInfo.m_flSelfDamageRadius = 146.0f;

	TFGameRules()->RadiusDamage(radiusInfo);
	*/

	//Damage enemies in explosion
	if (m_nGooType == TF_GOO_TOXIC)
	{
		CBaseEntity* pListOfNearbyEntities[16];
		int iNumberOfNearbyEntities = UTIL_EntitiesInSphere(pListOfNearbyEntities, 16, vecOrigin, flRadius, FL_ONGROUND);

		for (int i = 0; i < iNumberOfNearbyEntities; i++)
		{
			CBaseEntity* pEntity = pListOfNearbyEntities[i];
			if (!pEntity)
				return;

			//Detect if wall is between player and goo
			trace_t	trace;
			Ray_t ray;
			Vector EntityVecOrigin = pEntity->GetAbsOrigin();
			ray.Init(vecOrigin, EntityVecOrigin);
			UTIL_TraceRay(ray, MASK_PLAYERSOLID_BRUSHONLY, this, COLLISION_GROUP_PLAYER_MOVEMENT, &trace);
			if (trace.DidHitWorld())
				continue;

			float distance = trace.startpos.DistTo(trace.endpos);

			if (pEntity->IsPlayer())
			{
				CTFPlayer* pPlayerEntity = ToTFPlayer(pEntity);
				if (!pPlayerEntity)
					continue;

				if (!pAttacker->InSameTeam(pPlayerEntity))
				{
					CTakeDamageInfo damageInfo;
					damageInfo.SetDamage(max((1 - distance / (flRadius * 1.5)) * m_flDamage, 0));
					damageInfo.SetAttacker(pAttacker);
					damageInfo.AddDamageType(bitsDamageType);
					pPlayerEntity->TakeDamage(damageInfo);
				}
			}

		}
	}
	// If we extinguish a friendly player reduce our recharge time by four seconds
	/*if ( TFGameRules()->RadiusJarEffect( radiusInfo, TF_COND_URINE ) && m_iDeflected == 0 && pWeapon )
	{
		pWeapon->ReduceEffectBarRegenTime( 4.0f );
	}*/

	// Debug!
	/*if (sf_goo_show_radius.GetBool())
	{
		DrawRadius(flRadius);
	}*/
#endif
}

void CTFProjectile_Goo::RemoveThis(void)
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
	SetThink(&CBaseGrenade::SUB_Remove);
	SetTouch(NULL);
#ifdef GAME_DLL
	
	if (PhysIsInCallback())
		PhysCallbackRemove(this->GetNetworkable());
	else
		UTIL_Remove(this);
#endif
}

#ifdef GAME_DLL
CTFProjectile_Goo *CTFProjectile_Goo::Create( CBaseEntity *pWeapon, const Vector &vecOrigin, const QAngle &vecAngles, const Vector &vecVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pScorer, const AngularImpulse &angVelocity, const CTFWeaponInfo &weaponInfo )
{
	CTFProjectile_Goo *pGoo = static_cast<CTFProjectile_Goo *>( CBaseEntity::CreateNoSpawn( "tf_projectile_goo", vecOrigin, vecAngles, pOwner ) );

	if ( pGoo )
	{
		// Set scorer.
		pGoo->SetScorer( pScorer );

		// Set firing weapon.
		pGoo->SetLauncher( pWeapon );

		pGoo->ChangeTeam(pOwner->GetTeamNumber());

		DispatchSpawn( pGoo );

		pGoo->InitGrenade( vecVelocity, angVelocity, pOwner, weaponInfo );

		pGoo->ApplyLocalAngularVelocityImpulse( angVelocity );

		pGoo->m_flDamage.Set(25);
	}

	return pGoo;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Goo::Detonate()
{
	trace_t		tr;
	Vector		vecSpot;// trace starts here!

	vecSpot = GetAbsOrigin() + Vector(0, 0, 8);
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	Explode(&tr, GetDamageType());
}

//-----------------------------------------------------------------------------
// Purpose: Goo splatter think function to do AOE damage over time
//-----------------------------------------------------------------------------
void CTFProjectile_Goo::PuddleThink()
{
	if (m_flLifeTime < gpGlobals->curtime)
	{
		RemoveThis();
	}

	//SetThink( &CTFProjectile_Goo::PuddleThink );
	SetNextThink(gpGlobals->curtime);

}

void CTFProjectile_Goo::VPhysicsUpdate(IPhysicsObject* pPhysics)
{
	if (pPhysics->IsDragEnabled())
		pPhysics->EnableDrag(false);

	BaseClass::VPhysicsUpdate(pPhysics);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Goo::VPhysicsCollision(int index, gamevcollisionevent_t* pEvent)
{
	BaseClass::VPhysicsCollision(index, pEvent);

	int otherIndex = ~index;
	CBaseEntity* pHitEntity = pEvent->pEntities[otherIndex];

	if (!pHitEntity)
	{
		return;
	}

	if (m_bCritical)
		EmitSound("Weapon_GooGun.ProjectileImpactWorldCrit");
	else
		EmitSound("Weapon_GooGun.ProjectileImpactWorld");

	Vector startPos = GetAbsOrigin();
	Vector endPos;
	pEvent->pInternalData->GetContactPoint(endPos);

	Vector endDir = endPos - startPos;
	endPos += endDir * 5.0f;

	trace_t trace;
	UTIL_TraceLine(startPos, endPos, MASK_SOLID, this, COLLISION_GROUP_PROJECTILE, &trace);

	//Msg("x: %d, y: %d, z: %d", trace.plane.normal.x, trace.plane.normal.y, trace.plane.normal.z);

	if (VectorsAreEqual(trace.plane.normal, Vector(0, 0, 1), 0.99f))
	{
		// Expand into a puddle
		if (m_nGooType == TF_GOO_TOXIC)
			Detonate();
		Expand();
	}

	//QAngle vecAngNorm;
	//QAngle vecAngUp;
	//VectorAngles(Vector(0, 0, 1), vecAngUp);
	//VectorAngles(trace.plane.normal, vecAngNorm);




	// TODO: This needs to be redone properly
	/*if ( pHitEntity->GetMoveType() == MOVETYPE_NONE )
	{
		// Blow up
		SetThink( &CTFProjectile_Jar::Detonate );
		SetNextThink( gpGlobals->curtime );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Goo::SetScorer(CBaseEntity* pScorer)
{
	m_Scorer = pScorer;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBasePlayer* CTFProjectile_Goo::GetAssistant(void)
{
	return dynamic_cast<CBasePlayer*>(m_Scorer.Get());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Goo::Deflected(CBaseEntity* pDeflectedBy, Vector& vecDir)
{
	//We don't want the goo to be deflected when it's a puddle
	if (m_bIsPuddle)
	{
		return;
	}

	BaseClass::Deflected(pDeflectedBy, vecDir);

	SetScorer(pDeflectedBy);
}
#endif


#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFProjectile_Goo::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	/*if ( updateType == DATA_UPDATE_CREATED )
	{
		m_flCreationTime = gpGlobals->curtime;
		CreateTrails();
	}

	if ( m_iOldTeamNum && m_iOldTeamNum != m_iTeamNum )
	{
		ParticleProp()->StopEmission();
		CreateTrails();
	}

	if (!m_bWasPuddle && m_bIsPuddle)
	{
		ParticleProp()->StopEmission();
		CreatePuddleEffects();
	}*/

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TFProjectile_Goo::CreateTrails( void )
{
	if ( IsDormant() )
		return;

	const char* pszEffectName;
	if (GetTeamNumber() == TF_TEAM_RED)
		pszEffectName = "goo_trail_red";
	else
		pszEffectName = "goo_trail_blue";

	ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
}

void C_TFProjectile_Goo::CreatePuddleEffects ( void )
{
	C_TFPlayer* LocalRenderPlayer = C_TFPlayer::GetLocalTFPlayer();

	if (!LocalRenderPlayer)
		return;

	const char* pszEffectName;
	if (GetTeamNumber() == TF_TEAM_RED)
		pszEffectName = "effect_acid_parent_red";
	else
		pszEffectName = "effect_acid_parent_blue";

	switch (GetGooType())
	{
		case TF_GOO_TOXIC:
			if (LocalRenderPlayer->GetTeamNumber() == this->GetTeamNumber())
				ParticleProp()->Create("effect_acid_parent", PATTACH_ABSORIGIN_FOLLOW);
			else
				ParticleProp()->Create(pszEffectName, PATTACH_ABSORIGIN_FOLLOW);
			break;

		case TF_GOO_JUMP:
			ParticleProp()->Create(pszEffectName, PATTACH_ABSORIGIN_FOLLOW);
			break;
	}
	
}

//-----------------------------------------------------------------------------
// Purpose: Don't draw if we haven't yet gone past our original spawn point
//-----------------------------------------------------------------------------
int CTFProjectile_Goo::DrawModel( int flags )
{
	if ( gpGlobals->curtime - m_flCreationTime < 0.1f )
		return 0;

	return BaseClass::DrawModel( flags );
}
#endif