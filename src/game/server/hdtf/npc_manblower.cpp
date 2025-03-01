#include "cbase.h"
#include "npc_manhack.h"
#include "explode.h"
#include "ai_node.h"
#include "ai_route.h"
#include "ai_moveprobe.h"

#include "Sprite.h"
#include "SpriteTrail.h"

#include "te_effect_dispatch.h"
#include <props.h>

#define GLOW_SPRITE	"sprites/glow1.vmt"

// Sound stuff
#define MANHACK_PITCH_DIST1		512
#define MANHACK_MIN_PITCH1		(100)
#define MANHACK_MAX_PITCH1		(160)
#define MANHACK_WATER_PITCH1	(85)
#define MANHACK_VOLUME1			0.55

#define MANHACK_PITCH_DIST2		400
#define MANHACK_MIN_PITCH2		(85)
#define MANHACK_MAX_PITCH2		(190)
#define MANHACK_WATER_PITCH2	(90)
#define MANHACK_GLOW_SPRITE	"sprites/glow1.vmt"

ConVar	sk_manblower_health("sk_manblower_health", "0");
ConVar	sk_manblower_explosion_radius("sk_manblower_explosion_radius", "0");
ConVar	sk_manblower_explosion_damage("sk_manblower_explosion_damage", "0");

class CNPC_Manblower : public CNPC_Manhack
{
public:
	DECLARE_CLASS( CNPC_Manblower, CNPC_Manhack );
	DECLARE_SERVERCLASS( );
	DECLARE_DATADESC( );

	void Precache();
	void Spawn();

	void Explode();

	void Slice( CBaseEntity *pHitEntity, float flInterval, trace_t &tr );
	void StartEye();
	void SetEyeState(int state);
	void ShowHostile(bool hostile = true);

	bool CorpseGib(const CTakeDamageInfo &info);
	void Event_Killed(const CTakeDamageInfo &info);
	void DeathSound(const CTakeDamageInfo &info);

	void Bump(CBaseEntity *pHitEntity, float flInterval, trace_t &tr);
};

BEGIN_DATADESC( CNPC_Manblower )
END_DATADESC( )

IMPLEMENT_SERVERCLASS_ST( CNPC_Manblower, DT_NPC_Manblower )
END_SEND_TABLE( )

LINK_ENTITY_TO_CLASS( npc_manblower, CNPC_Manblower );

void CNPC_Manblower::Precache()
{
	

	// Allow multiple models, but default to manhack.mdl
	char* szModel = (char*)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		szModel = "models/hdtf/npcs/manblower.mdl";
		SetModelName(AllocPooledString(szModel));
		
	}
	else
	{
		SetModelName(AllocPooledString(szModel));
		
	}

	PrecacheModel(MANHACK_GLOW_SPRITE);

	PropBreakablePrecacheAll(MAKE_STRING("models/manhack.mdl"));
	PropBreakablePrecacheAll(GetModelName());

	PrecacheScriptSound("NPC_Manhack.Die");
	PrecacheScriptSound("NPC_Manhack.Bat");
	PrecacheScriptSound("NPC_Manhack.Grind");
	PrecacheScriptSound("NPC_Manhack.Slice");
	PrecacheScriptSound("NPC_Manhack.EngineNoise");
	PrecacheScriptSound("NPC_Manhack.Unpack");
	PrecacheScriptSound("NPC_Manhack.ChargeAnnounce");
	PrecacheScriptSound("NPC_Manhack.ChargeEnd");
	PrecacheScriptSound("NPC_Manhack.Stunned");

	// Sounds used on Client:
	PrecacheScriptSound("NPC_Manhack.EngineSound1");
	PrecacheScriptSound("NPC_Manhack.EngineSound2");
	PrecacheScriptSound("NPC_Manhack.BladeSound");

	PrecacheModel("models/hdtf/npcs/manblower.mdl");
	PrecacheModel(STRING(GetModelName()));
	PropBreakablePrecacheAll(GetModelName());


	PrecacheScriptSound("NPC_SScanner.FlyLoop");
	PrecacheScriptSound("NPC_Manhack.EngineSound2");
	PrecacheScriptSound("NPC_SScanner.Illuminate");
	PrecacheScriptSound("NPC_SScanner.Pain");

	CAI_BaseNPC::Precache();
}

void CNPC_Manblower::Spawn()
{
	BaseClass::Spawn();

	
	m_iHealth = sk_manblower_health.GetFloat();
	
}


void CNPC_Manblower::Slice( CBaseEntity *pHitEntity, float flInterval, trace_t &tr )
{
	Explode();
}

void CNPC_Manblower::Event_Killed(const CTakeDamageInfo &info)
{
	
	CTakeDamageInfo info2(info.GetInflictor(), info.GetAttacker(),info.GetWeapon(), info.GetDamageForce(), info.GetDamagePosition(), info.GetDamage(), info.GetDamageType() | DMG_DISSOLVE);
	
	BaseClass::Event_Killed(info2);

	Explode();
}

bool CNPC_Manblower::CorpseGib(const CTakeDamageInfo &info)
{
	RemoveDeferred();

	KillSprites(0.0f);

	// we've just dissolved! no gibs!

	return false;
}

void CNPC_Manblower::DeathSound(const CTakeDamageInfo &info)
{
	StopSound("NPC_Manhack.Stunned");
	CPASAttenuationFilter filter2(this, "Manblower.Die");
	EmitSound(filter2, entindex(), "Manblower.Die");
}

void CNPC_Manblower::Explode()
{
	float radius = sk_manblower_explosion_radius.GetFloat(), dmg = sk_manblower_explosion_damage.GetFloat();
	Vector pos = GetAbsOrigin();

	DispatchParticleEffect("hdtf_alien_grenade_air", GetAbsOrigin(), GetAbsAngles(), NULL);

	CTakeDamageInfo info(this, this, vec3_origin, pos, dmg, DMG_BLAST, 0, &pos);
	RadiusDamage(info, pos, radius, CLASS_NONE, NULL);

	EmitSound("Manblower.Explode");

#if !defined( CLIENT_DLL )

	CSoundEnt::InsertSound(SOUND_COMBAT, pos, 1024, 3.0f);

#endif
}

void CNPC_Manblower::StartEye()
{
	//Create our Eye sprite
	if (m_pEyeGlow == NULL)
	{
		m_pEyeGlow = CSprite::SpriteCreate(GLOW_SPRITE, GetLocalOrigin(), false);
		m_pEyeGlow->SetAttachment(this, LookupAttachment("Eye"));

		m_pEyeGlow->SetTransparency(kRenderTransAdd, 255, 0, 0, 128, kRenderFxNoDissipation);
		m_pEyeGlow->SetColor(255, 0, 0);

		m_pEyeGlow->SetBrightness(164, 0.1f);
		m_pEyeGlow->SetScale(0.25f, 0.1f);
		m_pEyeGlow->SetAsTemporary();
	}

	//Create our light sprite
	if (m_pLightGlow == NULL)
	{
		m_pLightGlow = CSprite::SpriteCreate(GLOW_SPRITE, GetLocalOrigin(), false);
		m_pLightGlow->SetAttachment(this, LookupAttachment("Light"));

		m_pLightGlow->SetTransparency(kRenderTransAdd, 0, 255, 216, 128, kRenderFxNoDissipation);
		m_pLightGlow->SetColor(0, 255, 216);

		m_pLightGlow->SetBrightness(164, 0.1f);
		m_pLightGlow->SetScale(0.25f, 0.1f);
		m_pLightGlow->SetAsTemporary();
	}
}

void CNPC_Manblower::SetEyeState(int state)
{
	StartEye();

	// manblower never change it's light colors
}

void CNPC_Manblower::ShowHostile(bool hostile)
{
	if (m_bShowingHostile == hostile)
		return;

	m_bShowingHostile = hostile;

	if (hostile)
	{
		EmitSound("NPC_SScanner.Illuminate");
	}
	else
	{
		EmitSound("NPC_SScanner.Pain");
	}
}

void CNPC_Manblower::Bump(CBaseEntity *pHitEntity, float flInterval, trace_t &tr)
{
	if (!VPhysicsGetObject())
		return;

	// Surpressing this behavior
	if (m_flBumpSuppressTime > gpGlobals->curtime)
		return;

	if (pHitEntity->GetMoveType() == MOVETYPE_VPHYSICS && pHitEntity->Classify() != CLASS_MANHACK)
	{
		HitPhysicsObject(pHitEntity);
	}

	// We've hit something so deflect our velocity based on the surface
	// norm of what we've hit
	if (flInterval > 0)
	{
		float moveLen = ((GetCurrentVelocity() * flInterval) * (1.0 - tr.fraction)).Length();

		Vector moveVec = moveLen*tr.plane.normal / flInterval;

		// If I'm totally dead, don't bounce me up
		if (m_iHealth <= 0 && moveVec.z > 0)
		{
			moveVec.z = 0;
		}

		// If I'm right over the ground don't push down
		if (moveVec.z < 0)
		{
			float floorZ = GetFloorZ(GetAbsOrigin());
			if (abs(GetAbsOrigin().z - floorZ) < 36)
			{
				moveVec.z = 0;
			}
		}

		if (!(m_spawnflags	& SF_NPC_GAG))
		{
			EmitSound("Manblower.Impact");
		}

		// See if we will not have a valid surface
		if (tr.allsolid || tr.startsolid)
		{
			// Build a fake reflection back along our current velocity because we can't know how to reflect off
			// a non-existant surface! -- jdw

			Vector vecRandomDir = RandomVector(-1.0f, 1.0f);
			SetCurrentVelocity(vecRandomDir * 50.0f);
			m_flBumpSuppressTime = gpGlobals->curtime + 0.5f;
		}
		else
		{
			// This is a valid hit and we can deflect properly

			VectorNormalize(moveVec);
			float hitAngle = -DotProduct(tr.plane.normal, -moveVec);

			Vector vReflection = 2.0 * tr.plane.normal * hitAngle + -moveVec;

			float flSpeed = GetCurrentVelocity().Length();
			SetCurrentVelocity(GetCurrentVelocity() + vReflection * flSpeed * 0.5f);
		}
	}

	// -------------------------------------------------------------
	// If I'm on a path check LOS to my next node, and fail on path
	// if I don't have LOS.  Note this is the only place I do this,
	// so the manhack has to collide before failing on a path
	// -------------------------------------------------------------
	if (GetNavigator()->IsGoalActive() && !(GetNavigator()->GetPath()->CurWaypointFlags() & bits_WP_TO_PATHCORNER))
	{
		AIMoveTrace_t moveTrace;
		GetMoveProbe()->MoveLimit(NAV_GROUND, GetAbsOrigin(), GetNavigator()->GetCurWaypointPos(),
			MoveCollisionMask(), GetEnemy(), &moveTrace);

		if (IsMoveBlocked(moveTrace) &&
			!moveTrace.pObstruction->ClassMatches(GetClassname()))
		{
			TaskFail(FAIL_NO_ROUTE);
			GetNavigator()->ClearGoal();
			return;
		}
	}
}
