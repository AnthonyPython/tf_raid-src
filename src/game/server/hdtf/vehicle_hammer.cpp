#include "cbase.h"
#include "vehicle_hammer.h"
#include "particle_parse.h"
#include "particle_system.h"
#include "soundent.h"
#include "collisionutils.h"
#include "vphysics/friction.h"
#include "physics_npc_solver.h"
#include "npc_alyx_episodic.h"
#include "vphysicsupdateai.h"
#include "rumble_shared.h"
#include "haptics/haptic_utils.h"
#include "in_buttons.h"
#include "ammodef.h"

#define VEHICLE_ARMS_MODEL "models/vehicles/hdtf/hammer_01_arms.mdl"

#define JEEP_GUN_YAW		"vehicle_weapon_yaw"
#define JEEP_GUN_PITCH		"vehicle_weapon_pitch"

extern ConVar phys_upimpactforcescale;
ConVar g_hammerexitspeed("g_hammerexitspeed", "100", FCVAR_CHEAT);

LINK_ENTITY_TO_CLASS(prop_vehicle_hammer, CPropHammer);

BEGIN_DATADESC(CPropHammer)

DEFINE_FIELD(m_bEntranceLocked, FIELD_BOOLEAN),
DEFINE_FIELD(m_bExitLocked, FIELD_BOOLEAN),
DEFINE_ARRAY(m_hWheelDust, FIELD_EHANDLE, NUM_WHEEL_EFFECTS),
DEFINE_ARRAY(m_hWheelWater, FIELD_EHANDLE, NUM_WHEEL_EFFECTS),
DEFINE_FIELD(m_flNextWaterSound, FIELD_TIME),
DEFINE_FIELD(m_bHasGun, FIELD_BOOLEAN),

DEFINE_FIELD(m_hArms, FIELD_EHANDLE),

DEFINE_INPUTFUNC(FIELD_VOID, "LockEntrance", InputLockEntrance),
DEFINE_INPUTFUNC(FIELD_VOID, "UnlockEntrance", InputUnlockEntrance),
DEFINE_INPUTFUNC(FIELD_VOID, "LockExit", InputLockExit),
DEFINE_INPUTFUNC(FIELD_VOID, "UnlockExit", InputUnlockExit),
DEFINE_INPUTFUNC(FIELD_VOID, "DisablePhysGun", InputDisablePhysGun),
DEFINE_INPUTFUNC(FIELD_VOID, "EnablePhysGun", InputEnablePhysGun),
DEFINE_INPUTFUNC(FIELD_VOID, "DisableGun", InputDisableGun),
DEFINE_INPUTFUNC(FIELD_VOID, "EnableGun", InputEnableGun),

END_DATADESC();

IMPLEMENT_SERVERCLASS_ST(CPropHammer, DT_CPropHammer)
END_SEND_TABLE()

CPropHammer::CPropHammer(void) :
	m_bEntranceLocked(false),
	m_bExitLocked(false)
{
	m_bHasGun = true;
	m_bUnableToFire = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropHammer::UpdateOnRemove(void)
{
	BaseClass::UpdateOnRemove();

	// Kill our wheel dust
	for (int i = 0; i < NUM_WHEEL_EFFECTS; i++)
	{
		if (m_hWheelDust[i] != NULL)
		{
			UTIL_Remove(m_hWheelDust[i]);
		}

		if (m_hWheelWater[i] != NULL)
		{
			UTIL_Remove(m_hWheelWater[i]);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropHammer::Precache(void)
{
	PrecacheScriptSound("Physics.WaterSplash");
	PrecacheScriptSound("PropHammer.FireCannon");

	PrecacheParticleSystem("WheelDust");
	PrecacheParticleSystem("WheelSplash");

	PrecacheModel(VEHICLE_ARMS_MODEL);
	PrecacheModel("models/vehicles/hdtf/hammer_01.mdl");
	PrecacheModel("models/vehicles/hdtf/hammer_02.mdl");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropHammer::Spawn(void)
{
	BaseClass::Spawn();

	SetBlocksLOS(false);

	m_nBulletType = GetAmmoDef()->Index("50cal");

	CAmmoDef *pAmmoDef = GetAmmoDef();
	m_nAmmoType = pAmmoDef->Index("50cal");

	CBasePlayer	*pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer != NULL)
	{
		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_VEHICLE_CROSSHAIR;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropHammer::InputLockEntrance(inputdata_t &data)
{
	m_bEntranceLocked = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropHammer::InputUnlockEntrance(inputdata_t &data)
{
	m_bEntranceLocked = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropHammer::InputLockExit(inputdata_t &data)
{
	m_bExitLocked = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropHammer::InputUnlockExit(inputdata_t &data)
{
	m_bExitLocked = false;
}

//-----------------------------------------------------------------------------
// Purpose: Override velocity if our passenger is transitioning or we're upside-down
//-----------------------------------------------------------------------------
Vector CPropHammer::PhysGunLaunchVelocity(const Vector &forward, float flMass)
{
	Vector vecPuntDir = BaseClass::PhysGunLaunchVelocity(forward, flMass);
	vecPuntDir.z = 150.0f;
	vecPuntDir *= 600.0f;
	return vecPuntDir;
}

//-----------------------------------------------------------------------------
// Purpose: Rolls the vehicle when its trying to upright itself from a punt
//-----------------------------------------------------------------------------
AngularImpulse CPropHammer::PhysGunLaunchAngularImpulse(void)
{
	if (IsOverturned())
		return AngularImpulse(0, 300, 0);

	// Don't spin randomly, always spin reliably
	return AngularImpulse(0, 0, 0);
}

//-----------------------------------------------------------------------------
// Purpose: Get the upright strength
//-----------------------------------------------------------------------------
float CPropHammer::GetUprightStrength(void)
{
	return 5.0f;
}

//-----------------------------------------------------------------------------
// Purpose: If the player uses the jeep while at the back, he gets ammo from the crate instead
//-----------------------------------------------------------------------------
void CPropHammer::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// do nothing if entrace is locked
	if (m_bEntranceLocked)
		return;

	// Fall back and get in the vehicle instead, skip giving ammo
	BaseClass::BaseClass::Use(pActivator, pCaller, useType, value);
}

#define	MIN_WHEEL_DUST_SPEED	5

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropHammer::UpdateWheelDust(void)
{
	// See if this wheel should emit dust
	const vehicleparams_t *vehicleData = m_pServerVehicle->GetVehicleParams();
	const vehicle_operatingparams_t *carState = m_pServerVehicle->GetVehicleOperatingParams();
	bool bAllowDust = vehicleData->steering.dustCloud;

	// Car must be active
	bool bCarOn = m_VehiclePhysics.IsOn();

	// Must be moving quickly enough or skidding along the ground
	bool bCreateDust = (bCarOn &&
		bAllowDust &&
		(m_VehiclePhysics.GetSpeed() >= MIN_WHEEL_DUST_SPEED || carState->skidSpeed > DEFAULT_SKID_THRESHOLD));

	// Update our wheel dust
	Vector	vecPos;
	for (int i = 0; i < NUM_WHEEL_EFFECTS; i++)
	{
		m_pServerVehicle->GetWheelContactPoint(i, vecPos);

		// Make sure the effect is created
		if (m_hWheelDust[i] == NULL)
		{
			// Create the dust effect in place
			m_hWheelDust[i] = (CParticleSystem *)CreateEntityByName("info_particle_system");
			if (m_hWheelDust[i] == NULL)
				continue;

			// Setup our basic parameters
			m_hWheelDust[i]->KeyValue("start_active", "0");
			m_hWheelDust[i]->KeyValue("effect_name", "WheelDust");
			m_hWheelDust[i]->SetParent(this);
			m_hWheelDust[i]->SetLocalOrigin(vec3_origin);
			DispatchSpawn(m_hWheelDust[i]);
			if (gpGlobals->curtime > 0.5f)
				m_hWheelDust[i]->Activate();
		}

		// Make sure the effect is created
		if (m_hWheelWater[i] == NULL)
		{
			// Create the dust effect in place
			m_hWheelWater[i] = (CParticleSystem *)CreateEntityByName("info_particle_system");
			if (m_hWheelWater[i] == NULL)
				continue;

			// Setup our basic parameters
			m_hWheelWater[i]->KeyValue("start_active", "0");
			m_hWheelWater[i]->KeyValue("effect_name", "WheelSplash");
			m_hWheelWater[i]->SetParent(this);
			m_hWheelWater[i]->SetLocalOrigin(vec3_origin);
			DispatchSpawn(m_hWheelWater[i]);
			if (gpGlobals->curtime > 0.5f)
				m_hWheelWater[i]->Activate();
		}

		// Turn the dust on or off
		if (bCreateDust)
		{
			// Angle the dust out away from the wheels
			Vector vecForward, vecRight, vecUp;
			GetVectors(&vecForward, &vecRight, &vecUp);

			const vehicle_controlparams_t *vehicleControls = m_pServerVehicle->GetVehicleControlParams();
			float flWheelDir = (i & 1) ? 1.0f : -1.0f;
			QAngle vecAngles;
			vecForward += vecRight * flWheelDir;
			vecForward += vecRight * (vehicleControls->steering*0.5f) * flWheelDir;
			vecForward += vecUp;
			VectorAngles(vecForward, vecAngles);

			// NDebugOverlay::Axis( vecPos, vecAngles, 8.0f, true, 0.1f );

			if (m_WaterData.m_bWheelInWater[i])
			{
				m_hWheelDust[i]->StopParticleSystem();

				// Set us up in the right position
				m_hWheelWater[i]->StartParticleSystem();
				m_hWheelWater[i]->SetAbsAngles(vecAngles);
				m_hWheelWater[i]->SetAbsOrigin(vecPos + Vector(0, 0, 8));

				if (m_flNextWaterSound < gpGlobals->curtime)
				{
					m_flNextWaterSound = gpGlobals->curtime + random->RandomFloat(0.25f, 1.0f);
					EmitSound("Physics.WaterSplash");
				}
			}
			else
			{
				m_hWheelWater[i]->StopParticleSystem();

				// Set us up in the right position
				m_hWheelDust[i]->StartParticleSystem();
				m_hWheelDust[i]->SetAbsAngles(vecAngles);
				m_hWheelDust[i]->SetAbsOrigin(vecPos + Vector(0, 0, 8));
			}
		}
		else
		{
			// Stop emitting
			m_hWheelDust[i]->StopParticleSystem();
			m_hWheelWater[i]->StopParticleSystem();
		}
	}
}

#define VEHICLE_AVOID_BROADCAST_RATE	0.5f

//-----------------------------------------------------------------------------
// Purpose: This function isn't really what we want
//-----------------------------------------------------------------------------
void CPropHammer::CreateAvoidanceZone(void)
{
	if (m_flNextAvoidBroadcastTime > gpGlobals->curtime)
		return;

	// Only do this when we're stopped
	if (m_VehiclePhysics.GetSpeed() > 5.0f)
		return;

	float flHullRadius = CollisionProp()->BoundingRadius2D();

	Vector	vecPos;
	CollisionProp()->NormalizedToWorldSpace(Vector(0.5f, 0.33f, 0.25f), &vecPos);
	CSoundEnt::InsertSound(SOUND_MOVE_AWAY, vecPos, (flHullRadius*0.4f), VEHICLE_AVOID_BROADCAST_RATE, this);
	// NDebugOverlay::Sphere( vecPos, vec3_angle, flHullRadius*0.4f, 255, 0, 0, 0, true, VEHICLE_AVOID_BROADCAST_RATE );

	CollisionProp()->NormalizedToWorldSpace(Vector(0.5f, 0.66f, 0.25f), &vecPos);
	CSoundEnt::InsertSound(SOUND_MOVE_AWAY, vecPos, (flHullRadius*0.4f), VEHICLE_AVOID_BROADCAST_RATE, this);
	// NDebugOverlay::Sphere( vecPos, vec3_angle, flHullRadius*0.4f, 255, 0, 0, 0, true, VEHICLE_AVOID_BROADCAST_RATE );

	// Don't broadcast again until these are done
	m_flNextAvoidBroadcastTime = gpGlobals->curtime + VEHICLE_AVOID_BROADCAST_RATE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropHammer::Think(void)
{
	BaseClass::Think();

	// See if the wheel dust should be on or off
	UpdateWheelDust();

	CreateAvoidanceZone();
}

// adds a collision solver for any small props that are stuck under the vehicle
static void SolveBlockingProps(CPropHammer *pVehicleEntity, IPhysicsObject *pVehiclePhysics)
{
	CUtlVector<CBaseEntity *> solveList;
	float vehicleMass = pVehiclePhysics->GetMass();
	Vector vehicleUp;
	pVehicleEntity->GetVectors(NULL, NULL, &vehicleUp);
	IPhysicsFrictionSnapshot *pSnapshot = pVehiclePhysics->CreateFrictionSnapshot();
	while (pSnapshot->IsValid())
	{
		IPhysicsObject *pOther = pSnapshot->GetObject(1);
		float otherMass = pOther->GetMass();
		CBaseEntity *pOtherEntity = static_cast<CBaseEntity *>(pOther->GetGameData());
		Assert(pOtherEntity);
		if (pOtherEntity && pOtherEntity->GetMoveType() == MOVETYPE_VPHYSICS && pOther->IsMoveable() && (otherMass*4.0f) < vehicleMass)
		{
			Vector normal;
			pSnapshot->GetSurfaceNormal(normal);
			// this points down in the car's reference frame, then it's probably trapped under the car
			if (DotProduct(normal, vehicleUp) < -0.9f)
			{
				Vector point, pointLocal;
				pSnapshot->GetContactPoint(point);
				VectorITransform(point, pVehicleEntity->EntityToWorldTransform(), pointLocal);
				Vector bottomPoint = physcollision->CollideGetExtent(pVehiclePhysics->GetCollide(), vec3_origin, vec3_angle, Vector(0, 0, -1));
				// make sure it's under the bottom of the car
				float bottomPlane = DotProduct(bottomPoint, vehicleUp) + 8;	// 8 inches above bottom
				if (DotProduct(pointLocal, vehicleUp) <= bottomPlane)
				{
					//Msg("Solved %s\n", pOtherEntity->GetClassname());
					if (solveList.Find(pOtherEntity) < 0)
					{
						solveList.AddToTail(pOtherEntity);
					}
				}
			}
		}
		pSnapshot->NextFrictionData();
	}
	pVehiclePhysics->DestroyFrictionSnapshot(pSnapshot);
	if (solveList.Count())
	{
		for (int i = 0; i < solveList.Count(); i++)
		{
			EntityPhysics_CreateSolver(pVehicleEntity, solveList[i], true, 4.0f);
		}
		pVehiclePhysics->RecheckContactPoints();
	}
}

static void SimpleCollisionResponse(Vector velocityIn, const Vector &normal, float coefficientOfRestitution, Vector *pVelocityOut)
{
	Vector Vn = DotProduct(velocityIn, normal) * normal;
	Vector Vt = velocityIn - Vn;
	*pVelocityOut = Vt - coefficientOfRestitution * Vn;
}

static void KillBlockingEnemyNPCs(CBasePlayer *pPlayer, CBaseEntity *pVehicleEntity, IPhysicsObject *pVehiclePhysics)
{
	Vector velocity;
	pVehiclePhysics->GetVelocity(&velocity, NULL);
	float vehicleMass = pVehiclePhysics->GetMass();

	// loop through the contacts and look for enemy NPCs that we're pushing on
	CUtlVector<CAI_BaseNPC *> npcList;
	CUtlVector<Vector> forceList;
	CUtlVector<Vector> contactList;
	IPhysicsFrictionSnapshot *pSnapshot = pVehiclePhysics->CreateFrictionSnapshot();
	while (pSnapshot->IsValid())
	{
		IPhysicsObject *pOther = pSnapshot->GetObject(1);
		float otherMass = pOther->GetMass();
		CBaseEntity *pOtherEntity = static_cast<CBaseEntity *>(pOther->GetGameData());
		CAI_BaseNPC *pNPC = pOtherEntity ? pOtherEntity->MyNPCPointer() : NULL;
		// Is this an enemy NPC with a small enough mass?
		if (pNPC && pPlayer->IRelationType(pNPC) != D_LI && ((otherMass*2.0f) < vehicleMass))
		{
			// accumulate the stress force for this NPC in the lsit
			float force = pSnapshot->GetNormalForce();
			Vector normal;
			pSnapshot->GetSurfaceNormal(normal);
			normal *= force;
			int index = npcList.Find(pNPC);
			if (index < 0)
			{
				vphysicsupdateai_t *pUpdate = NULL;
				if (pNPC->VPhysicsGetObject() && pNPC->VPhysicsGetObject()->GetShadowController() && pNPC->GetMoveType() == MOVETYPE_STEP)
				{
					if (pNPC->HasDataObjectType(VPHYSICSUPDATEAI))
					{
						pUpdate = static_cast<vphysicsupdateai_t *>(pNPC->GetDataObject(VPHYSICSUPDATEAI));
						// kill this guy if I've been pushing him for more than half a second and I'm 
						// still pushing in his direction
						if ((gpGlobals->curtime - pUpdate->startUpdateTime) > 0.5f && DotProduct(velocity, normal) > 0)
						{
							index = npcList.AddToTail(pNPC);
							forceList.AddToTail(normal);
							Vector pos;
							pSnapshot->GetContactPoint(pos);
							contactList.AddToTail(pos);
						}
					}
					else
					{
						pUpdate = static_cast<vphysicsupdateai_t *>(pNPC->CreateDataObject(VPHYSICSUPDATEAI));
						pUpdate->startUpdateTime = gpGlobals->curtime;
					}
					// update based on vphysics for the next second
					// this allows the car to push the NPC
					pUpdate->stopUpdateTime = gpGlobals->curtime + 1.0f;
					float maxAngular;
					pNPC->VPhysicsGetObject()->GetShadowController()->GetMaxSpeed(&pUpdate->savedShadowControllerMaxSpeed, &maxAngular);
					pNPC->VPhysicsGetObject()->GetShadowController()->MaxSpeed(1.0f, maxAngular);
				}
			}
			else
			{
				forceList[index] += normal;
			}
		}
		pSnapshot->NextFrictionData();
	}
	pVehiclePhysics->DestroyFrictionSnapshot(pSnapshot);
	// now iterate the list and check each cumulative force against the threshold
	if (npcList.Count())
	{
		for (int i = npcList.Count(); --i >= 0; )
		{
			Vector damageForce;
			npcList[i]->VPhysicsGetObject()->GetVelocity(&damageForce, NULL);
			Vector vel;
			pVehiclePhysics->GetVelocityAtPoint(contactList[i], &vel);
			damageForce -= vel;
			Vector normal = forceList[i];
			VectorNormalize(normal);
			SimpleCollisionResponse(damageForce, normal, 1.0, &damageForce);
			damageForce += (normal * 300.0f);
			damageForce *= npcList[i]->VPhysicsGetObject()->GetMass();
			float len = damageForce.Length();
			damageForce.z += len*phys_upimpactforcescale.GetFloat();
			Vector vehicleForce = -damageForce;

			CTakeDamageInfo dmgInfo(pVehicleEntity, pVehicleEntity, damageForce, contactList[i], 350.0f, DMG_CRUSH | DMG_VEHICLE);
			pVehiclePhysics->ApplyForceOffset(vehicleForce, contactList[i]);
			PhysCollisionSound(pVehicleEntity, npcList[i]->VPhysicsGetObject(), CHAN_BODY, pVehiclePhysics->GetMaterialIndex(), npcList[i]->VPhysicsGetObject()->GetMaterialIndex(), gpGlobals->frametime, 200.0f);
			npcList[i]->TakeDamage(dmgInfo);
		}
	}
}

void CPropHammer::DriveVehicle(float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased)
{
	int iButtons = ucmd->buttons;

	if (ucmd->forwardmove != 0.0f)
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetDriver());

		if (pPlayer && VPhysicsGetObject())
		{
			KillBlockingEnemyNPCs(pPlayer, this, VPhysicsGetObject());
			SolveBlockingProps(this, VPhysicsGetObject());
		}
	}

	if (m_bHasGun)
	{
		if (IsOverturned() == false)
		{
			if (iButtons & IN_ATTACK)
			{
				FireCannon();
			}
		}
	}

	BaseClass::BaseClass::DriveVehicle(flFrameTime, ucmd, iButtonsDown, iButtonsReleased);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropHammer::HandleWater(void)
{
	// Only check the wheels and engine in water if we have a driver (player).
	if (!GetDriver())
		return;

	// Update our internal state
	CheckWater();

	// Save of data from last think.
	for (int iWheel = 0; iWheel < JEEP_WHEEL_COUNT; ++iWheel)
	{
		m_WaterData.m_bWheelWasInWater[iWheel] = m_WaterData.m_bWheelInWater[iWheel];
	}
}

//-----------------------------------------------------------------------------
// Purpose: Report our lock state
//-----------------------------------------------------------------------------
int	CPropHammer::DrawDebugTextOverlays(void)
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		EntityText(text_offset, CFmtStr("Entrance: %s", m_bEntranceLocked ? "Locked" : "Unlocked"), 0);
		text_offset++;

		EntityText(text_offset, CFmtStr("Exit: %s", m_bExitLocked ? "Locked" : "Unlocked"), 0);
		text_offset++;
	}

	return text_offset;
}

//-----------------------------------------------------------------------------
// Purpose: Stop players punting the car around.
//-----------------------------------------------------------------------------
void CPropHammer::InputDisablePhysGun(inputdata_t &data)
{
	AddEFlags(EFL_NO_PHYSCANNON_INTERACTION);
}
//-----------------------------------------------------------------------------
// Purpose: Return to normal
//-----------------------------------------------------------------------------
void CPropHammer::InputEnablePhysGun(inputdata_t &data)
{
	RemoveEFlags(EFL_NO_PHYSCANNON_INTERACTION);
}

void CPropHammer::EnterVehicle(CBaseCombatCharacter *pPassenger)
{
	BaseClass::EnterVehicle(pPassenger);

	if (m_hArms)
	{
		UTIL_Remove(m_hArms);
	}

	Vector armsOrigin;
	QAngle armsAngles;

	const int arms = LookupAttachment("arms");

	GetAttachment(arms, armsOrigin, armsAngles);

	CBaseAnimating *pArms = dynamic_cast<CBaseAnimating *>(CreateEntityByName("prop_dynamic"));
	UTIL_SetOrigin(pArms, armsOrigin);
	pArms->SetAbsAngles(armsAngles);
	pArms->SetModel(VEHICLE_ARMS_MODEL);
	pArms->Precache();
	pArms->SetParent(this, arms);
	DispatchSpawn(pArms);

	m_hArms = pArms;
}

void CPropHammer::ExitVehicle(int nRole)
{
	BaseClass::ExitVehicle(nRole);

	if (m_hArms)
	{
		UTIL_Remove(m_hArms);
		m_hArms = NULL;
	}
}

void CPropHammer::DoImpactEffect(trace_t &tr, int nDamageType)
{
	if ((tr.surface.flags & SURF_SKY) == false)
	{
		UTIL_ImpactTrace(&tr, m_nBulletType);
	}
}

Vector CPropHammer::BodyTarget(const Vector &posSrc, bool bNoisy)
{
	Vector	shotPos;
	matrix3x4_t	matrix;

	GetAttachment(LookupAttachment("bodytarget"), matrix);
	MatrixGetColumn(matrix, 3, shotPos);

	if (bNoisy)
	{
		shotPos[0] += random->RandomFloat(-8.0f, 8.0f);
		shotPos[1] += random->RandomFloat(-8.0f, 8.0f);
		shotPos[2] += random->RandomFloat(-8.0f, 8.0f);
	}

	return shotPos;
}

void CPropHammer::AimGunAt(Vector *endPos, float flInterval)
{
	Vector	aimPos = *endPos;

	if (IsOverturned() || m_bEngineLocked || m_bHasGun == false)
	{
		SetPoseParameter(JEEP_GUN_YAW, 0);
		SetPoseParameter(JEEP_GUN_PITCH, 0);
		return;

		Vector	v_forward, v_up;
		AngleVectors(GetLocalAngles(), NULL, &v_forward, &v_up);
		aimPos = WorldSpaceCenter() + (v_forward * -32.0f) - Vector(0, 0, 128.0f);
	}

	matrix3x4_t gunMatrix;
	GetAttachment(LookupAttachment("gun_ref"), gunMatrix);

	Vector localEnemyPosition;
	VectorITransform(aimPos, gunMatrix, localEnemyPosition);

	QAngle localEnemyAngles;
	VectorAngles(localEnemyPosition, localEnemyAngles);

	localEnemyAngles.x = UTIL_AngleDiff(localEnemyAngles.x, 0);

	float targetYaw = localEnemyAngles.y;
	float targetPitch = clamp(localEnemyAngles.x, -65.f, 25.f);
	float diff = UTIL_AngleDiff(targetYaw, m_aimYaw);

	m_aimYaw = UTIL_ApproachAngle(targetYaw, m_aimYaw, abs(diff) * 0.45);
	m_aimPitch = UTIL_Approach(targetPitch, m_aimPitch, 5.f);

	SetPoseParameter(JEEP_GUN_YAW, -m_aimYaw);
	SetPoseParameter(JEEP_GUN_PITCH, -m_aimPitch);

	InvalidateBoneCache();

	Vector	vecMuzzle, vecMuzzleDir;
	QAngle	vecMuzzleAng;

	GetAttachment("Muzzle", vecMuzzle, vecMuzzleAng);
	AngleVectors(vecMuzzleAng, &vecMuzzleDir);

	//trace_t	tr;
	//UTIL_TraceLine(vecMuzzle, vecMuzzle + (vecMuzzleDir * MAX_TRACE_LENGTH), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	// see if we hit something, if so, adjust endPos to hit location
	//if (tr.fraction < 1.0)
	//{
	//	m_vecGunCrosshair = tr.endpos;
	//}

	m_vecGunCrosshair = *endPos;
}

ConVar hap_hammer_cannon_mag("hap_hammer_cannon_mag", "10", 0);

void CPropHammer::FireCannon(void)
{
	if (m_flCannonTime > gpGlobals->curtime)
		return;

	if (m_bUnableToFire)
		return;

	m_flCannonTime = gpGlobals->curtime + 0.125f;
	m_bCannonCharging = false;

	Vector aimDir;
	GetCannonAim(aimDir);

#if defined( WIN32 ) && !defined( _X360 ) 
	HapticPunch(m_hPlayer, 0, 0, hap_hammer_cannon_mag.GetFloat());
#endif
	FireBulletsInfo_t info(1, m_vecGunOrigin, aimDir, VECTOR_CONE_1DEGREES, MAX_TRACE_LENGTH, m_nAmmoType);

	info.m_nFlags = FIRE_BULLETS_ALLOW_WATER_SURFACE_IMPACTS;
	info.m_pAttacker = m_hPlayer;

	FireBullets(info);

	if (m_hPlayer)
	{
		m_hPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
		m_hPlayer->RumbleEffect(RUMBLE_PISTOL, 0, RUMBLE_FLAG_RESTART);
	}

	CPASAttenuationFilter sndFilter(this, "PropHammer.FireCannon");
	EmitSound(sndFilter, entindex(), "PropHammer.FireCannon");

	ResetSequence(LookupSequence("gun_fire"));
}

void CPropHammer::GetCannonAim(Vector &resultDir)
{
	//Vector	muzzleOrigin;
	//QAngle	muzzleAngles;

	//GetAttachment(LookupAttachment("Muzzle"), muzzleOrigin, muzzleAngles);

	//AngleVectors(muzzleAngles, resultDir);
	resultDir = (m_vecGunCrosshair.Get() - m_vecGunOrigin).Normalized();
}

bool CPropHammer::CanExitVehicle(CBaseEntity *pEntity)
{
	return (!m_bEnterAnimOn && !m_bExitAnimOn && !m_bExitLocked && !m_bLocked && (m_nSpeed <= g_hammerexitspeed.GetFloat()));
}

void CPropHammer::InputDisableGun(inputdata_t &data)
{
	SetModel("models/vehicles/hdtf/hammer_02.mdl");
	m_bHasGun = false;
}

void CPropHammer::InputEnableGun(inputdata_t &data)
{
	SetModel("models/vehicles/hdtf/hammer_01.mdl");
	m_bHasGun = true;
}
