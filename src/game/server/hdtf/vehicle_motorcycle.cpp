#include "cbase.h"
#include "vehicle_motorcycle.h"
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

extern ConVar phys_upimpactforcescale;
ConVar g_bikeexitspeed("g_bikeexitspeed", "100", FCVAR_CHEAT);

LINK_ENTITY_TO_CLASS(prop_vehicle_bike, CPropBike);

BEGIN_DATADESC(CPropBike)

DEFINE_FIELD(m_bEntranceLocked, FIELD_BOOLEAN),
DEFINE_FIELD(m_bExitLocked, FIELD_BOOLEAN),
DEFINE_ARRAY(m_hWheelDust, FIELD_EHANDLE, NUM_WHEEL_EFFECTS),
DEFINE_ARRAY(m_hWheelWater, FIELD_EHANDLE, NUM_WHEEL_EFFECTS),
DEFINE_FIELD(m_flNextWaterSound, FIELD_TIME),

DEFINE_INPUTFUNC(FIELD_VOID, "LockEntrance", InputLockEntrance),
DEFINE_INPUTFUNC(FIELD_VOID, "UnlockEntrance", InputUnlockEntrance),
DEFINE_INPUTFUNC(FIELD_VOID, "LockExit", InputLockExit),
DEFINE_INPUTFUNC(FIELD_VOID, "UnlockExit", InputUnlockExit),
DEFINE_INPUTFUNC(FIELD_VOID, "DisablePhysGun", InputDisablePhysGun),
DEFINE_INPUTFUNC(FIELD_VOID, "EnablePhysGun", InputEnablePhysGun),

END_DATADESC();

IMPLEMENT_SERVERCLASS_ST(CPropBike, DT_CPropBike)
END_SEND_TABLE()

CPropBike::CPropBike(void) :
	m_bEntranceLocked(false),
	m_bExitLocked(false)
{
	m_bHasGun = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropBike::UpdateOnRemove(void)
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
void CPropBike::Precache(void)
{
	PrecacheScriptSound("Physics.WaterSplash");

	PrecacheParticleSystem("WheelDust");
	PrecacheParticleSystem("WheelSplash");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropBike::Spawn(void)
{
	BaseClass::Spawn();

	SetBlocksLOS(false);

	CBasePlayer	*pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer != NULL)
	{
		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_VEHICLE_CROSSHAIR;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropBike::InputLockEntrance(inputdata_t &data)
{
	m_bEntranceLocked = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropBike::InputUnlockEntrance(inputdata_t &data)
{
	m_bEntranceLocked = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropBike::InputLockExit(inputdata_t &data)
{
	m_bExitLocked = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropBike::InputUnlockExit(inputdata_t &data)
{
	m_bExitLocked = false;
}

//-----------------------------------------------------------------------------
// Purpose: Override velocity if our passenger is transitioning or we're upside-down
//-----------------------------------------------------------------------------
Vector CPropBike::PhysGunLaunchVelocity(const Vector &forward, float flMass)
{
	Vector vecPuntDir = BaseClass::PhysGunLaunchVelocity(forward, flMass);
	vecPuntDir.z = 150.0f;
	vecPuntDir *= 600.0f;
	return vecPuntDir;
}

//-----------------------------------------------------------------------------
// Purpose: Rolls the vehicle when its trying to upright itself from a punt
//-----------------------------------------------------------------------------
AngularImpulse CPropBike::PhysGunLaunchAngularImpulse(void)
{
	if (IsOverturned())
		return AngularImpulse(0, 300, 0);

	// Don't spin randomly, always spin reliably
	return AngularImpulse(0, 0, 0);
}

//-----------------------------------------------------------------------------
// Purpose: Get the upright strength
//-----------------------------------------------------------------------------
float CPropBike::GetUprightStrength(void)
{
	return 5.0f;
}

//-----------------------------------------------------------------------------
// Purpose: If the player uses the jeep while at the back, he gets ammo from the crate instead
//-----------------------------------------------------------------------------
void CPropBike::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
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
void CPropBike::UpdateWheelDust(void)
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
void CPropBike::CreateAvoidanceZone(void)
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
void CPropBike::Think(void)
{
	BaseClass::Think();

	// See if the wheel dust should be on or off
	UpdateWheelDust();

	CreateAvoidanceZone();
}

// adds a collision solver for any small props that are stuck under the vehicle
static void SolveBlockingProps(CPropBike *pVehicleEntity, IPhysicsObject *pVehiclePhysics)
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

			CTakeDamageInfo dmgInfo(pVehicleEntity, pVehicleEntity, damageForce, contactList[i], 200.0f, DMG_CRUSH | DMG_VEHICLE);
			npcList[i]->TakeDamage(dmgInfo);
			pVehiclePhysics->ApplyForceOffset(vehicleForce, contactList[i]);
			PhysCollisionSound(pVehicleEntity, npcList[i]->VPhysicsGetObject(), CHAN_BODY, pVehiclePhysics->GetMaterialIndex(), npcList[i]->VPhysicsGetObject()->GetMaterialIndex(), gpGlobals->frametime, 200.0f);
		}
	}
}

void CPropBike::DriveVehicle(float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased)
{
	if (ucmd->forwardmove != 0.0f)
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetDriver());

		if (pPlayer && VPhysicsGetObject())
		{
			KillBlockingEnemyNPCs(pPlayer, this, VPhysicsGetObject());
			SolveBlockingProps(this, VPhysicsGetObject());
		}
	}

	BaseClass::BaseClass::DriveVehicle(flFrameTime, ucmd, iButtonsDown, iButtonsReleased);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropBike::HandleWater(void)
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
int	CPropBike::DrawDebugTextOverlays(void)
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
void CPropBike::InputDisablePhysGun(inputdata_t &data)
{
	AddEFlags(EFL_NO_PHYSCANNON_INTERACTION);
}
//-----------------------------------------------------------------------------
// Purpose: Return to normal
//-----------------------------------------------------------------------------
void CPropBike::InputEnablePhysGun(inputdata_t &data)
{
	RemoveEFlags(EFL_NO_PHYSCANNON_INTERACTION);
}

bool CPropBike::CanExitVehicle(CBaseEntity *pEntity)
{
	return (!m_bEnterAnimOn && !m_bExitAnimOn && !m_bExitLocked && !m_bLocked && (m_nSpeed <= g_bikeexitspeed.GetFloat()));
}
