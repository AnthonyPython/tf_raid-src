//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basegrenade_shared.h"
#include "grenade_frag.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "soundent.h"
#include <particle_parse.h>

#if !defined( CLIENT_DLL )

#include "gamestats.h"

#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define FRAG_GRENADE_WARN_TIME 1.5f

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

ConVar sk_plr_dmg_m67("sk_plr_dmg_m67", "0");
ConVar sk_npc_dmg_m67("sk_npc_dmg_m67", "0");
ConVar sk_m67_radius("sk_m67_radius", "0");

#define GRENADE_MODEL "models/weapons/w_m67.mdl"

extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion

class CGrenadeM67 : public CBaseGrenade
{
	DECLARE_CLASS(CGrenadeM67, CBaseGrenade);

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif

public:
	void	Spawn(void);
	void	Precache(void);
	bool	CreateVPhysics(void);
	void	CreateEffects(void);
	void	SetTimer(float detonateDelay, float warnDelay);
	void	SetVelocity(const Vector &velocity, const AngularImpulse &angVelocity);
	int		OnTakeDamage(const CTakeDamageInfo &inputInfo);
	void	DelayThink();
	void	VPhysicsUpdate(IPhysicsObject *pPhysics);
	void	SetCombineSpawned(bool combineSpawned) { m_combineSpawned = combineSpawned; }
	virtual void		Explode(trace_t* pTrace, int bitsDamageType);
	bool	IsCombineSpawned(void) const { return m_combineSpawned; }
	void	SetPunted(bool punt) { m_punted = punt; }
	bool	WasPunted(void) const { return m_punted; }
	float   m_fl_damage;

	// this function only used in episodic.
#if defined(HL2_EPISODIC) && 0 // FIXME: HandleInteraction() is no longer called now that base grenade derives from CBaseAnimating
	bool	HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);
#endif 

	void	InputSetTimer(inputdata_t &inputdata);

protected:
	bool	m_inSolid;
	bool	m_combineSpawned;
	bool	m_punted;
	CHandle<CSpriteTrail>	m_pGlowTrail;
};

LINK_ENTITY_TO_CLASS(grenade_m67, CGrenadeM67);

BEGIN_DATADESC(CGrenadeM67)

// Fields
DEFINE_FIELD(m_pGlowTrail, FIELD_EHANDLE),
DEFINE_FIELD(m_inSolid, FIELD_BOOLEAN),
DEFINE_FIELD(m_combineSpawned, FIELD_BOOLEAN),
DEFINE_FIELD(m_punted, FIELD_BOOLEAN),

// Function Pointers
DEFINE_THINKFUNC(DelayThink),

// Inputs
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetTimer", InputSetTimer),

END_DATADESC()

void CGrenadeM67::Spawn(void)
{
	Precache();

	SetModel(GRENADE_MODEL);

	if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer())
	{
		m_fl_damage = sk_plr_dmg_m67.GetFloat();
		SetDamageRadius(sk_m67_radius.GetFloat());
		
	}
	else
	{
		m_fl_damage = sk_npc_dmg_m67.GetFloat();
		SetDamageRadius(sk_m67_radius.GetFloat());
		
	}

	m_takedamage = DAMAGE_YES;
	m_iHealth = 1;

	SetSize(-Vector(4, 4, 4), Vector(4, 4, 4));
	SetCollisionGroup(COLLISION_GROUP_WEAPON);
	CreateVPhysics();

	AddSolidFlags(FSOLID_NOT_STANDABLE);

	m_combineSpawned = false;
	m_punted = false;

	BaseClass::Spawn();
}

bool CGrenadeM67::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, 0, false);
	return true;
}

void CGrenadeM67::CreateEffects()
{

	m_pGlowTrail = CSpriteTrail::SpriteTrailCreate("sprites/bluelaser1.vmt", GetLocalOrigin(), false);

	if (m_pGlowTrail != NULL)
	{
		m_pGlowTrail->FollowEntity(this);
	
		int	nAttachment = LookupAttachment("smoke");

		m_pGlowTrail->SetAttachment(this, nAttachment);

#ifdef HDTF
			m_pGlowTrail->SetTransparency(kRenderTransAdd, 255, 87, 51, 200, kRenderFxPulseFastWide);
#else
		m_pGlowTrail->SetTransparency(kRenderTransAdd, 255, 0, 0, 255, kRenderFxNone);
#endif
		m_pGlowTrail->SetStartWidth(8.0f);
		m_pGlowTrail->SetEndWidth(1.0f);
		m_pGlowTrail->SetLifeTime(0.5f);
	}
}

// this will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked
class CTraceFilterCollisionGroupDelta : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE(CTraceFilterCollisionGroupDelta);

	CTraceFilterCollisionGroupDelta(const IHandleEntity *passentity, int collisionGroupAlreadyChecked, int newCollisionGroup)
		: m_pPassEnt(passentity), m_collisionGroupAlreadyChecked(collisionGroupAlreadyChecked), m_newCollisionGroup(newCollisionGroup)
	{
	}

	virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
	{
		if (!PassServerEntityFilter(pHandleEntity, m_pPassEnt))
			return false;
		CBaseEntity *pEntity = EntityFromEntityHandle(pHandleEntity);

		if (pEntity)
		{
			if (g_pGameRules->ShouldCollide(m_collisionGroupAlreadyChecked, pEntity->GetCollisionGroup()))
				return false;
			if (g_pGameRules->ShouldCollide(m_newCollisionGroup, pEntity->GetCollisionGroup()))
				return true;
		}

		return false;
	}

protected:
	const IHandleEntity *m_pPassEnt;
	int		m_collisionGroupAlreadyChecked;
	int		m_newCollisionGroup;
};

void CGrenadeM67::VPhysicsUpdate(IPhysicsObject *pPhysics)
{
	BaseClass::VPhysicsUpdate(pPhysics);
	Vector vel;
	AngularImpulse angVel;
	pPhysics->GetVelocity(&vel, &angVel);

	Vector start = GetAbsOrigin();
	// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
	CTraceFilterCollisionGroupDelta filter(this, GetCollisionGroup(), COLLISION_GROUP_NONE);
	trace_t tr;

	// UNDONE: Hull won't work with hitboxes - hits outer hull.  But the whole point of this test is to hit hitboxes.
#if 0
	UTIL_TraceHull(start, start + vel * gpGlobals->frametime, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), CONTENTS_HITBOX | CONTENTS_MONSTER | CONTENTS_SOLID, &filter, &tr);
#else
	UTIL_TraceLine(start, start + vel * gpGlobals->frametime, CONTENTS_HITBOX | CONTENTS_MONSTER | CONTENTS_SOLID, &filter, &tr);
#endif
	if (tr.startsolid)
	{
		if (!m_inSolid)
		{
			// UNDONE: Do a better contact solution that uses relative velocity?
			vel *= -GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
			pPhysics->SetVelocity(&vel, NULL);
		}
		m_inSolid = true;
		return;
	}
	m_inSolid = false;
	if (tr.DidHit())
	{
		Vector dir = vel;
		VectorNormalize(dir);
		// send a tiny amount of damage so the character will react to getting bonked
		CTakeDamageInfo info(this, GetThrower(), pPhysics->GetMass() * vel, GetAbsOrigin(), 0.1f, DMG_CRUSH);
		tr.m_pEnt->TakeDamage(info);

		// reflect velocity around normal
		vel = -2.0f * tr.plane.normal * DotProduct(vel, tr.plane.normal) + vel;

		// absorb 80% in impact
		vel *= GRENADE_COEFFICIENT_OF_RESTITUTION;
		angVel *= -0.5f;
		pPhysics->SetVelocity(&vel, &angVel);
	}
}


void CGrenadeM67::Precache(void)
{
	PrecacheModel(GRENADE_MODEL);

	BaseClass::Precache();
}

void CGrenadeM67::SetTimer(float detonateDelay, float warnDelay)
{
	m_flDetonateTime = gpGlobals->curtime + detonateDelay;
	m_flWarnAITime = gpGlobals->curtime + warnDelay;
	SetThink(&CGrenadeM67::DelayThink);
	SetNextThink(gpGlobals->curtime);

	CreateEffects();
}

void CGrenadeM67::DelayThink()
{
	if (gpGlobals->curtime > m_flDetonateTime)
	{
		Detonate();
		return;
	}

	if (!m_bHasWarnedAI && gpGlobals->curtime >= m_flWarnAITime)
	{
#if !defined( CLIENT_DLL )
		CSoundEnt::InsertSound(SOUND_DANGER, GetAbsOrigin(), 400, 1.5, this);
#endif
		m_bHasWarnedAI = true;
	}

	SetNextThink(gpGlobals->curtime + 0.1);
}

void CGrenadeM67::SetVelocity(const Vector &velocity, const AngularImpulse &angVelocity)
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if (pPhysicsObject)
	{
		pPhysicsObject->AddVelocity(&velocity, &angVelocity);
	}
}

int CGrenadeM67::OnTakeDamage(const CTakeDamageInfo &inputInfo)
{
	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	VPhysicsTakeDamage(inputInfo);

	// Grenades only suffer blast damage and burn damage.
	if (!(inputInfo.GetDamageType() & (DMG_BLAST | DMG_BURN)))
		return 0;

	return BaseClass::OnTakeDamage(inputInfo);
}

#if defined(HL2_EPISODIC) && 0 // FIXME: HandleInteraction() is no longer called now that base grenade derives from CBaseAnimating
extern int	g_interactionBarnacleVictimGrab; ///< usually declared in ai_interactions.h but no reason to haul all of that in here.
extern int g_interactionBarnacleVictimBite;
extern int g_interactionBarnacleVictimReleased;
bool CGrenadeM67::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	// allow fragnades to be grabbed by barnacles. 
	if (interactionType == g_interactionBarnacleVictimGrab)
	{
		// give the grenade another five seconds seconds so the player can have the satisfaction of blowing up the barnacle with it
		float timer = m_flDetonateTime - gpGlobals->curtime + 5.0f;
		SetTimer(timer, timer - FRAG_GRENADE_WARN_TIME);

		return true;
	}
	else if (interactionType == g_interactionBarnacleVictimBite)
	{
		// detonate the grenade immediately 
		SetTimer(0, 0);
		return true;
	}
	else if (interactionType == g_interactionBarnacleVictimReleased)
	{
		// take the five seconds back off the timer.
		float timer = MAX(m_flDetonateTime - gpGlobals->curtime - 5.0f, 0.0f);
		SetTimer(timer, timer - FRAG_GRENADE_WARN_TIME);
		return true;
	}
	else
	{
		return BaseClass::HandleInteraction(interactionType, data, sourceEnt);
	}
}
#endif

void CGrenadeM67::InputSetTimer(inputdata_t &inputdata)
{
	SetTimer(inputdata.value.Float(), inputdata.value.Float() - FRAG_GRENADE_WARN_TIME);
}

CBaseGrenade *GrenadeM67_Create(const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned)
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeM67 *pGrenade = (CGrenadeM67 *)CBaseEntity::Create("grenade_m67", position, angles, pOwner);

	pGrenade->SetTimer(timer, timer - FRAG_GRENADE_WARN_TIME);
	pGrenade->SetVelocity(velocity, angVelocity);
	pGrenade->SetThrower(ToBaseCombatCharacter(pOwner));
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->SetCombineSpawned(combineSpawned);
	
	int IRingBodyGroup = pGrenade->FindBodygroupByName("ring");

	pGrenade->SetBodygroup(IRingBodyGroup, 1);


	return pGrenade;
}

void CGrenadeM67::Explode(trace_t* pTrace, int bitsDamageType)
{
#if !defined( CLIENT_DLL )

	SetModelName(NULL_STRING);//invisible
	AddSolidFlags(FSOLID_NOT_SOLID);

	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if (pTrace->fraction != 1.0)
	{
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 0.6));
	}

	Vector vecAbsOrigin = GetAbsOrigin();
	int contents = UTIL_PointContents(vecAbsOrigin);

#if defined( TF_DLL )
	// Since this code only runs on the server, make sure it shows the tempents it creates.
	// This solves a problem with remote detonating the pipebombs (client wasn't seeing the explosion effect)
	CDisablePredictionFiltering disabler;
#endif

	if (pTrace->fraction != 1.0)
	{
		Vector vecNormal = pTrace->plane.normal;
		surfacedata_t* pdata = physprops->GetSurfaceData(pTrace->surface.surfaceProps);
		CPASFilter filter(vecAbsOrigin);

		te->Explosion(filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin,
			!(contents & MASK_WATER) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			sk_m67_radius.GetFloat() * .03,
			25,
#ifdef HDTF
			TE_EXPLFLAG_NOFIREBALL | TE_EXPLFLAG_NOFIREBALLSMOKE | TE_EXPLFLAG_NOPARTICLES,
#else
			TE_EXPLFLAG_NONE,
#endif
			sk_m67_radius.GetFloat(),
			m_fl_damage,
			&vecNormal,
			(char)pdata->game.material);

#ifdef HDTF
		if (GetShouldUseAlienEffect())
			DispatchParticleEffect("hdtf_alien_grenade", GetAbsOrigin(), GetAbsAngles(), NULL);
		else
			DispatchParticleEffect("hdtf_grenade", GetAbsOrigin(), GetAbsAngles(), NULL);
#endif
	}
	else
	{
		CPASFilter filter(vecAbsOrigin);
		te->Explosion(filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin,
			!(contents & MASK_WATER) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			sk_m67_radius.GetFloat() * .03,
			25,
#ifdef HDTF
			TE_EXPLFLAG_NOFIREBALL | TE_EXPLFLAG_NOFIREBALLSMOKE | TE_EXPLFLAG_NOPARTICLES,
#else
			TE_EXPLFLAG_NONE,
#endif
			sk_m67_radius.GetFloat(),
			m_fl_damage);

#ifdef HDTF
		if (GetShouldUseAlienEffect())
			DispatchParticleEffect("hdtf_alien_grenade_air", GetAbsOrigin(), GetAbsAngles(), NULL);
		else
			DispatchParticleEffect("hdtf_grenade_air", GetAbsOrigin(), GetAbsAngles(), NULL);
#endif
	}

#if !defined( CLIENT_DLL )
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0);
#endif

	// Use the thrower's position as the reported position
	Vector vecReported = m_hThrower ? m_hThrower->GetAbsOrigin() : vec3_origin;

	CTakeDamageInfo info(this, m_hThrower, GetBlastForce(), GetAbsOrigin(), m_fl_damage, bitsDamageType, 0, &vecReported);

	RadiusDamage(info, GetAbsOrigin(), sk_m67_radius.GetFloat(), CLASS_NONE, NULL);

	UTIL_DecalTrace(pTrace, "Scorch");

#ifdef HDTF
	if (GetShouldUseAlienEffect())
		EmitSound("BaseGrenade.Explode");
	else
		EmitSound("BaseGrenade.Explode");
#endif

	SetThink(&CBaseGrenade::SUB_Remove);
	SetTouch(NULL);
	SetSolid(SOLID_NONE);

	AddEffects(EF_NODRAW);
	SetAbsVelocity(vec3_origin);

#if HL2_EPISODIC
	// Because the grenade is zipped out of the world instantly, the EXPLOSION sound that it makes for
	// the AI is also immediately destroyed. For this reason, we now make the grenade entity inert and
	// throw it away in 1/10th of a second instead of right away. Removing the grenade instantly causes
	// intermittent bugs with env_microphones who are listening for explosions. They will 'randomly' not
	// hear explosion sounds when the grenade is removed and the SoundEnt thinks (and removes the sound)
	// before the env_microphone thinks and hears the sound.
	SetNextThink(gpGlobals->curtime + 0.1);
#else
	SetNextThink(gpGlobals->curtime);
#endif//HL2_EPISODIC

#if defined( HL2_DLL )
	CBasePlayer* pPlayer = ToBasePlayer(m_hThrower.Get());
	if (pPlayer)
	{
		gamestats->Event_WeaponHit(pPlayer, true, "weapon_frag", info);
	}
#endif

#endif
}
