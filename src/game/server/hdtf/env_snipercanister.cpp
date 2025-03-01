#include "cbase.h"
#include "ai_hint.h"
#include "explode.h"
#include "beam_shared.h"
#include "SpriteTrail.h"
#include "ar2_explosion.h"
#include "SkyCamera.h"
#include "smoke_trail.h"
#include "ai_basenpc.h"
#include "ai_motor.h"
#include "te_effect_dispatch.h"
#include "env_snipercanister_shared.h"
#include "proto_sniper.h"
#include "env_snipercanister.h"
#include "npc_bullseye.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Models!
//-----------------------------------------------------------------------------
#define ENV_SNIPERCANISTER_MODEL				"models/hdtf/combine/sniper_canister.mdl"
#define ENV_SNIPERCANISTER_SKYBOX_MODEL			"models/hdtf/combine/sniper_canister_sky.mdl"
#define ENV_SNIPERCANISTER_INCOMING_SOUND_TIME	1.0f

ConVar sk_env_snipercanister_health("sk_env_snipercanister_health", "3"); // difficulty dependent!
ConVar sk_env_snipercanister_shake_amplitude("sk_env_snipercanister_shake_amplitude", "50");
ConVar sk_env_snipercanister_shake_radius("sk_env_snipercanister_shake_radius", "1024");
ConVar sk_env_snipercanister_shake_radius_vehicle("sk_env_snipercanister_shake_radius_vehicle", "2500");

#define ENV_SNIPERCANISTER_TRAIL_TIME			3.0f
#define ENV_SNIPERCANISTER_SNIPE_THINK_DELAY	0.015f
#define ENV_SNIPERCANISTER_CORE_HITBOX			1

//-----------------------------------------------------------------------------
// Spawn flags
//-----------------------------------------------------------------------------
enum
{
	SF_NO_IMPACT_SOUND = 0x1,
	SF_NO_LAUNCH_SOUND = 0x2,
	SF_START_IMPACTED = 0x1000,
	SF_LAND_AT_INITIAL_POSITION = 0x2000,
	SF_WAIT_FOR_INPUT_TO_OPEN = 0x4000,
	SF_PULSE_EFFECT = 0x8000,
	SF_NO_SMOKE = 0x10000,
	SF_NO_SHAKE = 0x20000,
	SF_REMOVE_ON_IMPACT = 0x40000,
	SF_NO_IMPACT_EFFECTS = 0x80000,
};

//-----------------------------------------------------------------------------
// Context think
//-----------------------------------------------------------------------------
static const char *s_pOpenThinkContext = "OpenThink";
static const char *s_pDeathThinkContext = "DeathThink";
static const char *s_pDecoyRemoveThinkContext = "DecoyRemove";

//-----------------------------------------------------------------------------
// Pose params
//-----------------------------------------------------------------------------
static const char *s_c1Yaw = "cannon_1_aim_yaw";
static const char *s_c2Yaw = "cannon_2_aim_yaw";
static const char *s_c3Yaw = "cannon_3_aim_yaw";

static const char *s_c1Pitch = "cannon_1_aim_pitch";
static const char *s_c2Pitch = "cannon_2_aim_pitch";
static const char *s_c3Pitch = "cannon_3_aim_pitch";

LINK_ENTITY_TO_CLASS(env_snipercanister, CEnvSniperCanister);

BEGIN_DATADESC(CEnvSniperCanister)

DEFINE_FIELD(m_bLanded, FIELD_BOOLEAN),
DEFINE_EMBEDDED(m_Shared),
DEFINE_FIELD(m_hTrail, FIELD_EHANDLE),
DEFINE_FIELD(m_hSmokeTrail, FIELD_EHANDLE),
DEFINE_KEYFIELD(m_flSmokeLifetime, FIELD_FLOAT, "SmokeLifetime"),
DEFINE_KEYFIELD(m_iszLaunchPositionName, FIELD_STRING, "LaunchPositionName"),
DEFINE_FIELD(m_vecImpactPosition, FIELD_POSITION_VECTOR),
DEFINE_FIELD(m_angImpactAngle, FIELD_VECTOR),
DEFINE_FIELD(m_bIncomingSoundStarted, FIELD_BOOLEAN),
DEFINE_FIELD(m_bHasDetonated, FIELD_BOOLEAN),
DEFINE_FIELD(m_bLaunched, FIELD_BOOLEAN),
DEFINE_FIELD(m_bOpened, FIELD_BOOLEAN),
DEFINE_KEYFIELD(m_flMinRefireTime, FIELD_FLOAT, "MinSkyboxRefireTime"),
DEFINE_KEYFIELD(m_flMaxRefireTime, FIELD_FLOAT, "MaxSkyboxRefireTime"),
DEFINE_KEYFIELD(m_nSkyboxCannisterCount, FIELD_INTEGER, "SkyboxCannisterCount"),
DEFINE_KEYFIELD(m_flDamageRadius, FIELD_FLOAT, "DamageRadius"),
DEFINE_KEYFIELD(m_flDamage, FIELD_FLOAT, "Damage"),

DEFINE_AUTO_ARRAY(pSnipers, FIELD_EHANDLE),

DEFINE_FIELD(m_iDamageLevel, FIELD_INTEGER),

// Function Pointers.
DEFINE_FUNCTION(CanisterSkyboxThink),
DEFINE_FUNCTION(CanisterWorldThink),
DEFINE_FUNCTION(WaitForOpenSequenceThink),
DEFINE_FUNCTION(CanisterSkyboxOnlyThink),
DEFINE_FUNCTION(CanisterSkyboxRestartThink),

// Inputs
DEFINE_INPUTFUNC(FIELD_VOID, "FireCanister", InputFireCanister),
DEFINE_INPUTFUNC(FIELD_VOID, "OpenCanister", InputOpenCanister),
DEFINE_INPUTFUNC(FIELD_VOID, "StopSmoke", InputStopSmoke),

// Outputs
DEFINE_OUTPUT(m_OnLaunched, "OnLaunched"),
DEFINE_OUTPUT(m_OnImpacted, "OnImpacted"),
DEFINE_OUTPUT(m_OnOpened, "OnOpened"),
DEFINE_OUTPUT(m_OnDestroyed, "OnKilled"),

END_DATADESC()


EXTERN_SEND_TABLE(DT_EnvSniperCanisterShared);

IMPLEMENT_SERVERCLASS_ST(CEnvSniperCanister, DT_EnvSniperCanister)
SendPropDataTable(SENDINFO_DT(m_Shared), &REFERENCE_SEND_TABLE(DT_EnvSniperCanisterShared)),
SendPropBool(SENDINFO(m_bLanded)),
SendPropBool(SENDINFO(m_bActive)),
END_SEND_TABLE()


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CEnvSniperCanister::CEnvSniperCanister()
{
	m_flMinRefireTime = -1.0f;
	m_flMaxRefireTime = -1.0f;

	m_iDamageLevel = 0;

	for (int i = 0; i < 3; i++)
	{
		flPitchTargets[i] = 0.f;
		flYawTargets[i] = 0.f;
	}
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CEnvSniperCanister::~CEnvSniperCanister()
{
}

//-----------------------------------------------------------------------------
// Precache!
//-----------------------------------------------------------------------------
void CEnvSniperCanister::Precache(void)
{
	BaseClass::Precache();
	PrecacheModel(ENV_SNIPERCANISTER_MODEL);
	PrecacheModel(ENV_SNIPERCANISTER_SKYBOX_MODEL);
	PrecacheModel("sprites/smoke.vmt");

	s_nExplosionTexture = PrecacheModel("sprites/lgtning.vmt");

	PrecacheScriptSound("HeadcrabCanister.LaunchSound");
	PrecacheScriptSound("HeadcrabCanister.Explosion");
	PrecacheScriptSound("HeadcrabCanister.IncomingSound");
	PrecacheScriptSound("HeadcrabCanister.SkyboxExplosion");
	PrecacheScriptSound("NPC_SniperCanister.Open");

	PrecacheScriptSound("NPC_SniperCanister.Pain");
	PrecacheScriptSound("NPC_SniperCanister.BulletDisentegrate");
	PrecacheScriptSound("NPC_SniperCanister.ShieldGone");
	PrecacheScriptSound("NPC_SniperCanister.Idle");

	PrecacheParticleSystem("hd_combine_ball");
	PrecacheParticleSystem("snipercanister_shield_compromised");
	PrecacheParticleSystem("snipercanister_shield_impact");
}


//-----------------------------------------------------------------------------
// Spawn!
//-----------------------------------------------------------------------------
void CEnvSniperCanister::Spawn(void)
{
	Precache();
	BaseClass::Spawn();

	m_bActive = false;

	// Do we have a position to launch from?
	if (m_iszLaunchPositionName != NULL_STRING)
	{
		// It doesn't have any real presence at first.
		SetSolid(SOLID_NONE);

		m_vecImpactPosition = GetAbsOrigin();
		m_angImpactAngle = GetAbsAngles();
		m_bIncomingSoundStarted = false;
		m_bLanded = false;
		m_bHasDetonated = false;
		m_bOpened = false;
	}
	else if (!HasSpawnFlags(SF_START_IMPACTED))
	{
		// It doesn't have any real presence at first.
		SetSolid(SOLID_NONE);

		if (!HasSpawnFlags(SF_LAND_AT_INITIAL_POSITION))
		{
			Vector vecForward;
			GetVectors(&vecForward, NULL, NULL);
			vecForward *= -1.0f;

			trace_t trace;
			UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vecForward * 10000, MASK_NPCWORLDSTATIC,
				this, COLLISION_GROUP_NONE, &trace);

			m_vecImpactPosition = trace.endpos;
			VectorAngles(trace.plane.normal, m_angImpactAngle);
		}
		else
		{
			m_vecImpactPosition = GetAbsOrigin();
			m_angImpactAngle = GetAbsAngles();
		}

		m_bIncomingSoundStarted = false;
		m_bLanded = false;
		m_bHasDetonated = false;
		m_bOpened = false;
	}
	else
	{
		m_bHasDetonated = true;
		m_bIncomingSoundStarted = true;
		m_bOpened = false;
		m_vecImpactPosition = GetAbsOrigin();
		m_angImpactAngle = GetAbsAngles();
		Landed();
	}
}


//-----------------------------------------------------------------------------
// On remove!
//-----------------------------------------------------------------------------
void CEnvSniperCanister::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();
	if (m_hTrail)
	{
		UTIL_Remove(m_hTrail);
		m_hTrail = NULL;
	}
	if (m_hSmokeTrail)
	{
		UTIL_Remove(m_hSmokeTrail);
		m_hSmokeTrail = NULL;
	}
}


//-----------------------------------------------------------------------------
// Set up the world model
//-----------------------------------------------------------------------------
void CEnvSniperCanister::SetupWorldModel()
{
	SetModel(ENV_SNIPERCANISTER_MODEL);
	SetSolid(SOLID_BBOX);

	float flRadius = CollisionProp()->BoundingRadius();
	Vector vecMins(-flRadius, -flRadius, -flRadius);
	Vector vecMaxs(flRadius, flRadius, flRadius);
	SetSize(vecMins, vecMaxs);
}


//-----------------------------------------------------------------------------
// Figure out where we enter the world
//-----------------------------------------------------------------------------
void CEnvSniperCanister::ComputeWorldEntryPoint(Vector *pStartPosition, QAngle *pStartAngles, Vector *pStartDirection)
{
	SetupWorldModel();

	Vector vecForward;
	GetVectors(&vecForward, NULL, NULL);

	// Raycast up to the place where we should start from (start raycast slightly off the ground,
	// since it'll be buried in the ground oftentimes)
	trace_t tr;
	CTraceFilterWorldOnly filter;
	UTIL_TraceLine(GetAbsOrigin() + vecForward * 100, GetAbsOrigin() + vecForward * 10000,
		CONTENTS_SOLID, &filter, &tr);

	*pStartPosition = tr.endpos;
	*pStartAngles = GetAbsAngles();
	VectorMultiply(vecForward, -1.0f, *pStartDirection);
}


//-----------------------------------------------------------------------------
// Place the canister in the world
//-----------------------------------------------------------------------------
CSkyCamera *CEnvSniperCanister::PlaceCanisterInWorld()
{
	CSkyCamera *pCamera = NULL;

	// Are we launching from a point? If so, use that point.
	if (m_iszLaunchPositionName != NULL_STRING)
	{
		// Get the launch position entity
		CBaseEntity *pLaunchPos = gEntList.FindEntityByName(NULL, m_iszLaunchPositionName);
		if (!pLaunchPos)
		{
			Warning("%s (%s) could not find an entity matching LaunchPositionName of '%s'\n", GetEntityName().ToCStr(), GetDebugName(), STRING(m_iszLaunchPositionName));
			SUB_Remove();
		}
		else
		{
			SetupWorldModel();

			Vector vecForward, vecImpactDirection;
			GetVectors(&vecForward, NULL, NULL);
			VectorMultiply(vecForward, -1.0f, vecImpactDirection);

			m_Shared.InitInWorld(gpGlobals->curtime, pLaunchPos->GetAbsOrigin(), GetAbsAngles(),
				vecImpactDirection, m_vecImpactPosition, true);
			SetThink(&CEnvSniperCanister::CanisterWorldThink);
			SetNextThink(gpGlobals->curtime);
		}
	}
	else if (DetectInSkybox())
	{
		pCamera = GetEntitySkybox();

		SetModel(ENV_SNIPERCANISTER_SKYBOX_MODEL);
		SetSolid(SOLID_NONE);

		Vector vecForward;
		GetVectors(&vecForward, NULL, NULL);
		vecForward *= -1.0f;

		m_Shared.InitInSkybox(gpGlobals->curtime, m_vecImpactPosition, GetAbsAngles(), vecForward,
			m_vecImpactPosition, pCamera->m_skyboxData.origin, pCamera->m_skyboxData.scale);
		AddEFlags(EFL_IN_SKYBOX);
		SetThink(&CEnvSniperCanister::CanisterSkyboxOnlyThink);
		SetNextThink(gpGlobals->curtime + m_Shared.GetEnterWorldTime() + TICK_INTERVAL);
	}
	else
	{
		Vector vecStartPosition, vecDirection;
		QAngle vecStartAngles;
		ComputeWorldEntryPoint(&vecStartPosition, &vecStartAngles, &vecDirection);

		// Figure out which skybox to place the entity in.
		pCamera = GetCurrentSkyCamera();
		if (pCamera)
		{
			m_Shared.InitInSkybox(gpGlobals->curtime, vecStartPosition, vecStartAngles, vecDirection,
				m_vecImpactPosition, pCamera->m_skyboxData.origin, pCamera->m_skyboxData.scale);

			if (m_Shared.IsInSkybox())
			{
				SetModel(ENV_SNIPERCANISTER_SKYBOX_MODEL);
				SetSolid(SOLID_NONE);
				AddEFlags(EFL_IN_SKYBOX);
				SetThink(&CEnvSniperCanister::CanisterSkyboxThink);
				SetNextThink(gpGlobals->curtime + m_Shared.GetEnterWorldTime());
			}
			else
			{
				SetThink(&CEnvSniperCanister::CanisterWorldThink);
				SetNextThink(gpGlobals->curtime);
			}
		}
		else
		{
			m_Shared.InitInWorld(gpGlobals->curtime, vecStartPosition, vecStartAngles,
				vecDirection, m_vecImpactPosition);
			SetThink(&CEnvSniperCanister::CanisterWorldThink);
			SetNextThink(gpGlobals->curtime);
		}
	}

	Vector vecEndPosition;
	QAngle vecEndAngles;
	m_Shared.GetPositionAtTime(gpGlobals->curtime, vecEndPosition, vecEndAngles);
	SetAbsOrigin(vecEndPosition);
	SetAbsAngles(vecEndAngles);

	return pCamera;
}


//-----------------------------------------------------------------------------
// Fires the canister!
//-----------------------------------------------------------------------------
void CEnvSniperCanister::InputFireCanister(inputdata_t &inputdata)
{
	if (m_bLaunched)
		return;

	m_bLaunched = true;

	if (HasSpawnFlags(SF_START_IMPACTED))
	{
		return;
	}

	// Play a firing sound
	CPASAttenuationFilter filter(this, ATTN_NONE);

	if (!HasSpawnFlags(SF_NO_LAUNCH_SOUND))
	{
		EmitSound(filter, entindex(), "HeadcrabCanister.LaunchSound");
	}

	// Place the canister
	CSkyCamera *pCamera = PlaceCanisterInWorld();

	// Hook up a smoke trail
	m_hTrail = CSpriteTrail::SpriteTrailCreate("sprites/smoke.vmt", GetAbsOrigin(), true);
	m_hTrail->SetTransparency(kRenderTransAdd, 224, 224, 255, 255, kRenderFxNone);
	m_hTrail->SetAttachment(this, 0);
	m_hTrail->SetStartWidth(32.0);
	m_hTrail->SetEndWidth(200.0);
	m_hTrail->SetStartWidthVariance(15.0f);
	m_hTrail->SetTextureResolution(0.002);
	m_hTrail->SetLifeTime(ENV_SNIPERCANISTER_TRAIL_TIME);
	m_hTrail->SetMinFadeLength(1000.0f);

	if (pCamera && m_Shared.IsInSkybox())
	{
		m_hTrail->SetSkybox(pCamera->m_skyboxData.origin, pCamera->m_skyboxData.scale);
	}

	// Fire that output!
	m_OnLaunched.Set(this, this, this);
}


//-----------------------------------------------------------------------------
// Opens the canister!
//-----------------------------------------------------------------------------
void CEnvSniperCanister::InputOpenCanister(inputdata_t &inputdata)
{
	if (m_bLanded && !m_bOpened && HasSpawnFlags(SF_WAIT_FOR_INPUT_TO_OPEN))
	{
		OpenCanister();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CEnvSniperCanister::InputStopSmoke(inputdata_t &inputdata)
{
	if (m_hSmokeTrail != NULL)
	{
		UTIL_Remove(m_hSmokeTrail);
		m_hSmokeTrail = NULL;
	}
}

//=============================================================================
//
// Enumerator for swept bbox collision.
//
class CCollideList : public IEntityEnumerator
{
public:
	CCollideList(Ray_t *pRay, CBaseEntity* pIgnoreEntity, int nContentsMask) :
		m_Entities(0, 32), m_pIgnoreEntity(pIgnoreEntity),
		m_nContentsMask(nContentsMask), m_pRay(pRay) {}

	virtual bool EnumEntity(IHandleEntity *pHandleEntity)
	{
		// Don't bother with the ignore entity.
		if (pHandleEntity == m_pIgnoreEntity)
			return true;

		Assert(pHandleEntity);

		trace_t tr;
		enginetrace->ClipRayToEntity(*m_pRay, m_nContentsMask, pHandleEntity, &tr);
		if ((tr.fraction < 1.0f) || (tr.startsolid) || (tr.allsolid))
		{
			CBaseEntity *pEntity = gEntList.GetBaseEntity(pHandleEntity->GetRefEHandle());
			m_Entities.AddToTail(pEntity);
		}

		return true;
	}

	CUtlVector<CBaseEntity*>	m_Entities;

private:
	CBaseEntity		*m_pIgnoreEntity;
	int				m_nContentsMask;
	Ray_t			*m_pRay;
};


//-----------------------------------------------------------------------------
// Test for impact!
//-----------------------------------------------------------------------------
void CEnvSniperCanister::TestForCollisionsAgainstEntities(const Vector &vecEndPosition)
{
	// Debugging!!
	//	NDebugOverlay::Box( GetAbsOrigin(), m_vecMin * 0.5f, m_vecMax * 0.5f, 255, 255, 0, 0, 5 );
	//	NDebugOverlay::Box( vecEndPosition, m_vecMin, m_vecMax, 255, 0, 0, 0, 5 );

	float flRadius = CollisionProp()->BoundingRadius();
	Vector vecMins(-flRadius, -flRadius, -flRadius);
	Vector vecMaxs(flRadius, flRadius, flRadius);

	Ray_t ray;
	ray.Init(GetAbsOrigin(), vecEndPosition, vecMins, vecMaxs);

	CCollideList collideList(&ray, this, MASK_SOLID);
	enginetrace->EnumerateEntities(ray, false, &collideList);

	float flDamage = m_flDamage;

	// Now get each entity and react accordinly!
	for (int iEntity = collideList.m_Entities.Count(); --iEntity >= 0; )
	{
		CBaseEntity *pEntity = collideList.m_Entities[iEntity];
		Vector vecForceDir = m_Shared.m_vecDirection;

		// Check for a physics object and apply force!
		IPhysicsObject *pPhysObject = pEntity->VPhysicsGetObject();
		if (pPhysObject)
		{
			float flMass = PhysGetEntityMass(pEntity);
			vecForceDir *= flMass * 750;
			pPhysObject->ApplyForceCenter(vecForceDir);
		}

		if (pEntity->m_takedamage && (m_flDamage != 0.0f))
		{
			CTakeDamageInfo info(this, this, flDamage, DMG_BLAST);
			CalculateExplosiveDamageForce(&info, vecForceDir, pEntity->GetAbsOrigin());
			pEntity->TakeDamage(info);
		}
	}
}


//-----------------------------------------------------------------------------
// Test for impact!
//-----------------------------------------------------------------------------
#define INNER_RADIUS_FRACTION 0.25f

void CEnvSniperCanister::TestForCollisionsAgainstWorld(const Vector &vecEndPosition)
{
	// Splash damage!
	// Iterate on all entities in the vicinity.
	float flDamageRadius = m_flDamageRadius;
	float flDamage = m_flDamage;

	CBaseEntity *pEntity;
	for (CEntitySphereQuery sphere(vecEndPosition, flDamageRadius); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
	{
		if (pEntity == this)
			continue;

		if (!pEntity->IsSolid())
			continue;

		// Get distance to object and use it as a scale value.
		Vector vecSegment;
		VectorSubtract(pEntity->GetAbsOrigin(), vecEndPosition, vecSegment);
		float flDistance = VectorNormalize(vecSegment);

		float flFactor = 1.0f / (flDamageRadius * (INNER_RADIUS_FRACTION - 1));
		flFactor *= flFactor;
		float flScale = flDistance - flDamageRadius;
		flScale *= flScale * flFactor;
		if (flScale > 1.0f)
		{
			flScale = 1.0f;
		}

		// Check for a physics object and apply force!
		Vector vecForceDir = vecSegment;
		IPhysicsObject *pPhysObject = pEntity->VPhysicsGetObject();
		if (pPhysObject)
		{
			// Send it flying!!!
			float flMass = PhysGetEntityMass(pEntity);
			vecForceDir *= flMass * 750 * flScale;
			pPhysObject->ApplyForceCenter(vecForceDir);
		}

		if (pEntity->m_takedamage && (m_flDamage != 0.0f))
		{
			CTakeDamageInfo info(this, this, flDamage * flScale, DMG_BLAST);
			CalculateExplosiveDamageForce(&info, vecSegment, pEntity->GetAbsOrigin());
			pEntity->TakeDamage(info);
		}

		if (pEntity->IsPlayer() && !(static_cast<CBasePlayer*>(pEntity)->IsInAVehicle()))
		{
			if (vecSegment.z < 0.1f)
			{
				vecSegment.z = 0.1f;
				VectorNormalize(vecSegment);
			}
			float flAmount = SimpleSplineRemapVal(flScale, 0.0f, 1.0f, 250.0f, 1000.0f);
			pEntity->ApplyAbsVelocityImpulse(vecSegment * flAmount);
		}
	}
}

//-----------------------------------------------------------------------------
// Canister finished opening
//-----------------------------------------------------------------------------
void CEnvSniperCanister::CanisterFinishedOpening(void)
{
	ResetSequence(LookupSequence("idle"));
	m_OnOpened.FireOutput(this, this, 0);
	m_bOpened = true;
	m_bActive = true;
	SetContextThink(NULL, gpGlobals->curtime, s_pOpenThinkContext);

	CreateSnipers();
	SetThink(&CEnvSniperCanister::CanisterPoseUpdateThink);
	SetNextThink(gpGlobals->curtime + 0.03);
}


//-----------------------------------------------------------------------------
// Finish the opening sequence
//-----------------------------------------------------------------------------
void CEnvSniperCanister::WaitForOpenSequenceThink()
{
	StudioFrameAdvance();
	if ((GetSequence() == LookupSequence("deploy")) && IsSequenceFinished())
	{
		CanisterFinishedOpening();
	}
	else
	{
		SetContextThink(&CEnvSniperCanister::WaitForOpenSequenceThink, gpGlobals->curtime + 0.01f, s_pOpenThinkContext);
	}
}


//-----------------------------------------------------------------------------
// Open the canister!
//-----------------------------------------------------------------------------
void CEnvSniperCanister::OpenCanister(void)
{
	if (m_bOpened)
		return;

	int nOpenSequence = LookupSequence("deploy");
	if (nOpenSequence != ACT_INVALID)
	{
		EmitSound("NPC_SniperCanister.Open");

		ResetSequence(nOpenSequence);
		SetContextThink(&CEnvSniperCanister::WaitForOpenSequenceThink, gpGlobals->curtime + 0.01f, s_pOpenThinkContext);
	}
	else
	{
		CanisterFinishedOpening();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvSniperCanister::SetLanded(void)
{
	SetAbsOrigin(m_vecImpactPosition);
	SetAbsAngles(m_angImpactAngle);
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_VPHYSICS);
	VPhysicsInitStatic();

	IncrementInterpolationFrame();
	m_bLanded = true;
}

//-----------------------------------------------------------------------------
// Landed!
//-----------------------------------------------------------------------------
void CEnvSniperCanister::Landed(void)
{
	// Lock us now that we've stopped
	SetLanded();

	// Hook the follow trail to the lead of the canister (which should be buried)
	// to hide problems with the edge of the follow trail
	if (m_hTrail)
	{
		m_hTrail->SetAttachment(this, LookupAttachment("trail"));
	}

	// Start smoke, unless we don't want it
	if (!HasSpawnFlags(SF_NO_SMOKE))
	{
		// Create the smoke trail
		m_hSmokeTrail = SmokeTrail::CreateSmokeTrail();
		m_hSmokeTrail->FollowEntity(this, "smoke");

		m_hSmokeTrail->m_SpawnRate = 8;
		m_hSmokeTrail->m_ParticleLifetime = 2.0f;

		m_hSmokeTrail->m_StartColor.Init(0.7f, 0.7f, 0.7f);
		m_hSmokeTrail->m_EndColor.Init(0.6, 0.6, 0.6);

		m_hSmokeTrail->m_StartSize = 32;
		m_hSmokeTrail->m_EndSize = 64;
		m_hSmokeTrail->m_SpawnRadius = 8;
		m_hSmokeTrail->m_MinSpeed = 0;
		m_hSmokeTrail->m_MaxSpeed = 8;
		m_hSmokeTrail->m_MinDirectedSpeed = 32;
		m_hSmokeTrail->m_MaxDirectedSpeed = 64;
		m_hSmokeTrail->m_Opacity = 0.35f;

		m_hSmokeTrail->SetLifetime(m_flSmokeLifetime);
	}

	SetThink(NULL);

	int core = LookupAttachment("core");
	if(core)
		DispatchParticleEffect("hd_combine_ball", PATTACH_POINT_FOLLOW, this, core, true);

	if (!HasSpawnFlags(SF_WAIT_FOR_INPUT_TO_OPEN))
	{
		if (HasSpawnFlags(SF_START_IMPACTED))
		{
			CanisterFinishedOpening();
		}
		else
		{
			OpenCanister();
		}
	}
}


//-----------------------------------------------------------------------------
// Creates the explosion effect
//-----------------------------------------------------------------------------
void CEnvSniperCanister::Detonate()
{
	// Send the impact output
	m_OnImpacted.FireOutput(this, this, 0);

	if (!HasSpawnFlags(SF_NO_IMPACT_SOUND))
	{
		StopSound("HeadcrabCanister.IncomingSound");
		EmitSound("HeadcrabCanister.Explosion");
	}

	// If we're supposed to be removed, do that now
	if (HasSpawnFlags(SF_REMOVE_ON_IMPACT))
	{
		SetAbsOrigin(m_vecImpactPosition);
		SetMoveType(MOVETYPE_NONE);
		IncrementInterpolationFrame();
		m_bLanded = true;

		// Become invisible so our trail can finish up
		AddEffects(EF_NODRAW);
		SetSolidFlags(FSOLID_NOT_SOLID);

		SetThink(&CEnvSniperCanister::SUB_Remove);
		SetNextThink(gpGlobals->curtime + ENV_SNIPERCANISTER_TRAIL_TIME);

		return;
	}

	// Test for damaging things
	TestForCollisionsAgainstWorld(m_vecImpactPosition);

	// Shake the screen unless flagged otherwise
	if (!HasSpawnFlags(SF_NO_SHAKE))
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);

		// If the player is on foot, then do a more limited shake
		float shakeRadius = (pPlayer && pPlayer->IsInAVehicle()) ? sk_env_snipercanister_shake_radius_vehicle.GetFloat() : sk_env_snipercanister_shake_radius.GetFloat();

		UTIL_ScreenShake(m_vecImpactPosition, sk_env_snipercanister_shake_amplitude.GetFloat(), 150.0, 1.0, shakeRadius, SHAKE_START);
	}

	// Do explosion effects
	if (!HasSpawnFlags(SF_NO_IMPACT_EFFECTS))
	{
		// Normal explosion
		ExplosionCreate(m_vecImpactPosition, GetAbsAngles(), this, 50.0f, 500.0f,
			SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODAMAGE | SF_ENVEXPLOSION_NOSOUND, 1300.0f);

		// Dust explosion
		AR2Explosion *pExplosion = AR2Explosion::CreateAR2Explosion(m_vecImpactPosition);

		if (pExplosion)
		{
			pExplosion->SetLifetime(10);
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: This think function simulates (moves/collides) the canister while in
//          the world.
//-----------------------------------------------------------------------------
void CEnvSniperCanister::CanisterWorldThink(void)
{
	// Get the current time.
	float flTime = gpGlobals->curtime;

	Vector vecStartPosition = GetAbsOrigin();

	// Update canister position for swept collision test.
	Vector vecEndPosition;
	QAngle vecEndAngles;
	m_Shared.GetPositionAtTime(flTime, vecEndPosition, vecEndAngles);

	if (!m_bIncomingSoundStarted && !HasSpawnFlags(SF_NO_IMPACT_SOUND))
	{
		float flDistSq = ENV_SNIPERCANISTER_INCOMING_SOUND_TIME * m_Shared.m_flFlightSpeed;
		flDistSq *= flDistSq;
		if (vecEndPosition.DistToSqr(m_vecImpactPosition) <= flDistSq)
		{
			// Figure out if we're close enough to play the incoming sound
			EmitSound("HeadcrabCanister.IncomingSound");
			m_bIncomingSoundStarted = true;
		}
	}

	TestForCollisionsAgainstEntities(vecEndPosition);
	if (m_Shared.DidImpact(flTime))
	{
		if (!m_bHasDetonated)
		{
			Detonate();
			m_bHasDetonated = true;
		}

		if (!HasSpawnFlags(SF_REMOVE_ON_IMPACT))
		{
			Landed();
		}

		return;
	}

	// Always move full movement.
	SetAbsOrigin(vecEndPosition);

	// Touch triggers along the way
	PhysicsTouchTriggers(&vecStartPosition);

	SetNextThink(gpGlobals->curtime + 0.2f);
	SetAbsAngles(vecEndAngles);

	if (!m_bHasDetonated)
	{
		if (vecEndPosition.DistToSqr(m_vecImpactPosition) < BoundingRadius() * BoundingRadius())
		{
			Detonate();
			m_bHasDetonated = true;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: This think function should be called at the time when the canister 
//          will be leaving the skybox and entering the world.
//-----------------------------------------------------------------------------
void CEnvSniperCanister::CanisterSkyboxThink(void)
{
	// Use different position computation
	m_Shared.ConvertFromSkyboxToWorld();

	Vector vecEndPosition;
	QAngle vecEndAngles;
	m_Shared.GetPositionAtTime(gpGlobals->curtime, vecEndPosition, vecEndAngles);
	UTIL_SetOrigin(this, vecEndPosition);
	SetAbsAngles(vecEndAngles);
	RemoveEFlags(EFL_IN_SKYBOX);

	// Switch to the actual-scale model
	SetupWorldModel();

	// Futz with the smoke trail to get it working across the boundary
	m_hTrail->SetSkybox(vec3_origin, 1.0f);

	// Now we start looking for collisions
	SetThink(&CEnvSniperCanister::CanisterWorldThink);
	SetNextThink(gpGlobals->curtime + 0.01f);
}


//-----------------------------------------------------------------------------
// Purpose: This stops its motion in the skybox
//-----------------------------------------------------------------------------
void CEnvSniperCanister::CanisterSkyboxOnlyThink(void)
{
	Vector vecEndPosition;
	QAngle vecEndAngles;
	m_Shared.GetPositionAtTime(gpGlobals->curtime, vecEndPosition, vecEndAngles);
	UTIL_SetOrigin(this, vecEndPosition);
	SetAbsAngles(vecEndAngles);

	if (!HasSpawnFlags(SF_NO_IMPACT_SOUND))
	{
		CPASAttenuationFilter filter(this, ATTN_NONE);
		EmitSound(filter, entindex(), "HeadcrabCanister.SkyboxExplosion");
	}

	if (m_nSkyboxCannisterCount != 0)
	{
		if (--m_nSkyboxCannisterCount <= 0)
		{
			SetThink(NULL);
			return;
		}
	}

	float flRefireTime = random->RandomFloat(m_flMinRefireTime, m_flMaxRefireTime) + ENV_SNIPERCANISTER_TRAIL_TIME;
	SetThink(&CEnvSniperCanister::CanisterSkyboxRestartThink);
	SetNextThink(gpGlobals->curtime + flRefireTime);
}


//-----------------------------------------------------------------------------
// This will re-fire the canister
//-----------------------------------------------------------------------------
void CEnvSniperCanister::CanisterSkyboxRestartThink(void)
{
	if (m_hTrail)
	{
		UTIL_Remove(m_hTrail);
		m_hTrail = NULL;
	}

	m_bLaunched = false;

	inputdata_t data;
	InputFireCanister(data);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pInfo - 
//			bAlways - 
//-----------------------------------------------------------------------------
void CEnvSniperCanister::SetTransmit(CCheckTransmitInfo *pInfo, bool bAlways)
{
	// Are we already marked for transmission?
	if (pInfo->m_pTransmitEdict->Get(entindex()))
		return;

	BaseClass::SetTransmit(pInfo, bAlways);

	// Make our smoke trail always come with us
	if (m_hSmokeTrail)
	{
		m_hSmokeTrail->SetTransmit(pInfo, bAlways);
	}
}

void CEnvSniperCanister::TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator)
{
	if (!m_bActive)
		return;

	if (ptr->hitbox == ENV_SNIPERCANISTER_CORE_HITBOX)
	{
		if (info.GetDamageType() & DMG_BULLET)
		{
			m_iDamageLevel++;
		}
	}

	int hitsLeft = sk_env_snipercanister_health.GetInt() - m_iDamageLevel;

	if (hitsLeft <= 0)
	{
		SetBodygroup(FindBodygroupByName("inner_shield"), 1);
		SetBodygroup(FindBodygroupByName("mesh_rotor1"), 1);
	}

	if (hitsLeft <= 1)
		SetBodygroup(FindBodygroupByName("mesh_rotor2"), 1);

	if (hitsLeft <= 2)
		SetBodygroup(FindBodygroupByName("mesh_rotor3"), 1);

	if (hitsLeft < 0)
	{
		CommitDeath();
	}
	else if(info.GetAttacker()->IsPlayer())
	{
		AlertSnipers(info.GetAttacker());
		EmitSound("NPC_SniperCanister.Pain");

		if (hitsLeft != 0)
			EmitSound("NPC_SniperCanister.BulletDisentegrate");
		else
			EmitSound("NPC_SniperCanister.ShieldGone");

		EmitSound("NPC_SniperCanister.Pain");

		int core = LookupAttachment("core");
		if (core)
			DispatchParticleEffect("snipercanister_shield_impact", PATTACH_POINT_FOLLOW, this, core, false);
	}
}

void CEnvSniperCanister::CommitDeath()
{
	m_bActive = false;
	SetNextThink(TICK_NEVER_THINK);
	SetThink(NULL);

	EmitSound("NPC_CombineBall.Explosion");

	StopParticleEffects(this);

	int nDeathSequence = LookupSequence("deactivate");
	if (nDeathSequence != ACT_INVALID)
	{
		ResetSequence(nDeathSequence);
		SetContextThink(&CEnvSniperCanister::WaitForDeathSequenceThink, gpGlobals->curtime + 0.01f, s_pDeathThinkContext);
	}

	Vector vecEffect;
	QAngle angEffect;

	int core = LookupAttachment("core");

	if (core)
	{
		GetAttachment(core, vecEffect, angEffect);
		DispatchParticleEffect("snipercanister_shield_compromised", PATTACH_POINT_FOLLOW, this, core, true);
	}
	else
	{
		vecEffect = GetAbsOrigin();
		angEffect = GetAbsAngles();
	}

	CBroadcastRecipientFilter filter;

	te->BeamRingPoint(filter, 0, vecEffect,	//origin
		12,	//start radius
		1024,		//end radius
		s_nExplosionTexture, //texture
		0,			//halo index
		0,			//start frame
		2,			//framerate
		0.2f,		//life
		64,			//width
		0,			//spread
		0,			//amplitude
		255,	//r
		255,	//g
		225,	//b
		32,		//a
		0,		//speed
		FBEAM_FADEOUT
	);

	//Shockring
	te->BeamRingPoint(filter, 0, vecEffect,	//origin
		12,	//start radius
		1024,		//end radius
		s_nExplosionTexture, //texture
		0,			//halo index
		0,			//start frame
		2,			//framerate
		0.5f,		//life
		64,			//width
		0,			//spread
		0,			//amplitude
		255,	//r
		255,	//g
		225,	//b
		64,		//a
		0,		//speed
		FBEAM_FADEOUT
	);

	// at last, remove our snipers
	for (int i = 0; i < 3; i++)
		if (pSnipers[i])
			pSnipers[i]->Remove();

	m_OnDestroyed.FireOutput(this, this, 0);
}

void CEnvSniperCanister::CreateSnipers()
{
	QAngle sniperAngle = GetAbsAngles();
	sniperAngle.x = 0;
	sniperAngle.z = 0;

	Vector vecSpawn;
	if (int core = LookupAttachment("core"))
	{
		GetAttachment(core, vecSpawn);
		vecSpawn.z -= 0.f;
	}
	else
		vecSpawn = GetAbsOrigin();

	for (int i = 0; i < 3; i++)
	{
		pSnipers[i] = (CProtoSniper*)CBaseEntity::CreateNoSpawn("npc_sniper", vecSpawn, sniperAngle, this);
		sniperAngle.y += 120.f;

		pSnipers[i]->AddSpawnFlags(1 << 16); // SF_SNIPER_HIDDEN

		pSnipers[i]->KeyValue("iscanister", "1");

		if(i == 0)
			pSnipers[i]->KeyValue("gunindex", "0");
		else if(i == 1)
			pSnipers[i]->KeyValue("gunindex", "1");
		else if(i == 2)
			pSnipers[i]->KeyValue("gunindex", "2");

		//if (HasSpawnFlags(SF_PULSE_EFFECT))
			pSnipers[i]->KeyValue("pulseeffect", "1");

		pSnipers[i]->Spawn();
		pSnipers[i]->SetSolid(SOLID_NONE);
		pSnipers[i]->SetDistLook(12000.f);
	}
}

void CEnvSniperCanister::AlertSnipers(CBaseEntity *attacker)
{
	CBasePlayer *pPlayer = ToBasePlayer(attacker);
	if (!pPlayer)
		return;

	if (pDecoy)
	{
		pDecoy->Remove();
		pDecoy = NULL;
	}

	pDecoy = static_cast<CNPC_Bullseye*>(CreateEntityByName("npc_bullseye"));

	if (pDecoy)
	{
		Vector vecCore;
		if (int core = LookupAttachment("core"))
			GetAttachment(core, vecCore);
		else
			vecCore = GetAbsOrigin();

		Vector spawnOrigin = pPlayer->EyePosition();
		Vector direction = (spawnOrigin - pSnipers[0]->GetAbsOrigin());
		const float dist = direction.Length();
		
		VectorNormalize(direction);

		trace_t tr;
		AI_TraceLine(vecCore + direction * dist, vecCore + direction * (dist + 1024.f), MASK_VISIBLE_AND_NPCS, pPlayer, COLLISION_GROUP_NONE, &tr);
		
		spawnOrigin = tr.endpos;

		pDecoy->SetAbsOrigin(spawnOrigin);
		pDecoy->SetAbsAngles(QAngle(0, 0, 0));
		pDecoy->KeyValue("solid", "6");
		pDecoy->Spawn();
		pDecoy->SetCollisionGroup(COLLISION_GROUP_PASSABLE_DOOR);

		pDecoy->SetHealth(1);

		for (int i = 0; i < 3; i++)
		{
			pSnipers[i]->AddEntityRelationship(pDecoy, D_HT, 99);
			if(pSnipers[i]->GetEnemy() != pPlayer || !pSnipers[i]->FVisible(pSnipers[i]->GetEnemy()))
				pSnipers[i]->SetEnemy(pDecoy);

			pSnipers[i]->SetRapidAttack();
		}

		SetContextThink(&CEnvSniperCanister::RemoveAlertEntites, gpGlobals->curtime + 2.f, s_pDecoyRemoveThinkContext);
	}
}

void CEnvSniperCanister::RemoveAlertEntites()
{
	if (pDecoy)
	{
		pDecoy->Remove();
		pDecoy = NULL;
	}

	for (int i = 0; i < 3; i++)
	{
		pSnipers[i]->SetRapidAttack(false);
	}
}

void CEnvSniperCanister::SetPitchYawTarget(int cannon, float pitch, float yaw)
{
	if (cannon >= 0 && cannon < 3)
	{
		flPitchTargets[cannon] = pitch;
		flYawTargets[cannon] = yaw;
	}
}

void CEnvSniperCanister::CanisterPoseUpdateThink()
{
	// way better than it used to be.
	StudioFrameAdvance();

	SetPoseParameter(s_c1Pitch, flPitchTargets[0]);
	SetPoseParameter(s_c1Yaw, flYawTargets[0]);

	SetPoseParameter(s_c2Pitch, flPitchTargets[1]);
	SetPoseParameter(s_c2Yaw, flYawTargets[1]);

	SetPoseParameter(s_c3Pitch, flPitchTargets[2]);
	SetPoseParameter(s_c3Yaw, flYawTargets[2]);

	SetNextThink(gpGlobals->curtime + 0.03);
}

void CEnvSniperCanister::WaitForDeathSequenceThink()
{
	StudioFrameAdvance();
	if ((GetSequence() == LookupSequence("deactivate")) && IsSequenceFinished())
	{
		// do nothing
	}
	else
	{
		SetContextThink(&CEnvSniperCanister::WaitForDeathSequenceThink, gpGlobals->curtime + 0.01f, s_pDeathThinkContext);
	}
}
