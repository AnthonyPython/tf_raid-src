//=============================================================================//
//
// Purpose: Intermediary class to deal with running parkour checks
// "What goes here and what goes in the weapon?"
// I don't know. Whatever works
// Date created: 08/02/2023
// Last modifed: 10/02/2023
//
//=============================================================================//

#include <cbase.h>
#include "hdtf_parkour_controller.h"
#include "hdtf_player_shared.h"
#include "in_buttons.h"

ConVar sk_plr_parkour_max_climb_kicks("sk_plr_parkour_max_climb_kicks", "2", FCVAR_REPLICATED);
ConVar sk_plr_parkour_climb_kick_force("sk_plr_parkour_climb_kick_force", "220", FCVAR_REPLICATED);
ConVar sk_plr_parkour_roll_threshold("sk_plr_parkour_roll_threshold", "500", FCVAR_REPLICATED);

ConVar sk_plr_parkour_pole_slide_acceleration("sk_plr_parkour_pole_slide_acceleration", "0.1", FCVAR_REPLICATED);
ConVar sk_plr_parkour_pole_slide_maxspeed("sk_plr_parkour_pole_slide_maxspeed", "5.0", FCVAR_REPLICATED);

ConVar hdtf_parkour_force_climb("hdtf_parkour_force_climb", "0", FCVAR_CHEAT, "Forces the player to climb the ledge even if something is blocking him");
ConVar hdtf_parkour_debug("hdtf_parkour_debug", "0", FCVAR_CHEAT, "Shows debug info for parkour");

//we have to make a sanity check here in the cvar for min and max , we know some one will do it, I know it, you know, every one knows it damn it!
ConVar sk_plr_parkour_push_blocked_baseforce("sk_plr_parkour_push_blocked_baseforce", "25", FCVAR_REPLICATED,
	"base foce to use to push a physics object out of the way.", 1, 1, 1, 300);
ConVar sk_plr_parkour_push_max_mass("sk_plr_parkour_push_max_mass", "300", FCVAR_REPLICATED);


BEGIN_NETWORK_TABLE_NOBASE(CParkourController, DT_ParkourController)
#ifdef CLIENT_DLL
RecvPropBool(RECVINFO(m_bCanClimb)),
RecvPropBool(RECVINFO(m_bFacingWall)),
RecvPropBool(RECVINFO(m_bSwitchedOnAction)),

RecvPropBool(RECVINFO(m_bHoldingOnLedge)),
RecvPropFloat(RECVINFO(m_flNextAction)),
RecvPropInt(RECVINFO(m_iClimbKicks)),
RecvPropFloat(RECVINFO(m_flNextWallClimb)),
RecvPropFloat(RECVINFO(m_flNextLedgeMove)),
RecvPropFloat(RECVINFO(m_flLedgeSurface)),
RecvPropFloat(RECVINFO(m_flLedgeDelta)),
RecvPropVector(RECVINFO(m_vLedgePlane)),
RecvPropVector(RECVINFO(m_vLedgeTarget)),
RecvPropInt(RECVINFO(m_eAnimState)),
RecvPropFloat(RECVINFO(m_flPoleJumpTime)),

RecvPropBool(RECVINFO(m_bRollReady)),

RecvPropVector(RECVINFO(m_vLedgeClimbTarget)),

RecvPropBool(RECVINFO(m_bHoldingOnPole)),
RecvPropVector(RECVINFO(m_vPoleTarget)),
RecvPropVector(RECVINFO(m_vPoleCenter)),
RecvPropFloat(RECVINFO(m_flPoleSlideSpeed)),

RecvPropBool(RECVINFO(m_bSwimming)),
RecvPropBool(RECVINFO(m_bIsUnderwater)),
RecvPropFloat(RECVINFO(m_flSwimTime)),
#else

SendPropBool(SENDINFO(m_bCanClimb)),
SendPropBool(SENDINFO(m_bFacingWall)),
SendPropBool(SENDINFO(m_bSwitchedOnAction)),

SendPropBool(SENDINFO(m_bHoldingOnLedge)),
SendPropFloat(SENDINFO(m_flNextAction)),
SendPropInt(SENDINFO(m_iClimbKicks)),
SendPropFloat(SENDINFO(m_flNextWallClimb)),
SendPropFloat(SENDINFO(m_flNextLedgeMove)),
SendPropFloat(SENDINFO(m_flLedgeSurface)),
SendPropFloat(SENDINFO(m_flLedgeDelta)),
SendPropVector(SENDINFO(m_vLedgePlane)),
SendPropVector(SENDINFO(m_vLedgeTarget)),
SendPropInt(SENDINFO(m_eAnimState)),
SendPropFloat(SENDINFO(m_flPoleJumpTime)),

SendPropBool(SENDINFO(m_bRollReady)),

SendPropVector(SENDINFO(m_vLedgeClimbTarget)),

SendPropBool(SENDINFO(m_bHoldingOnPole)),
SendPropVector(SENDINFO(m_vPoleTarget)),
SendPropVector(SENDINFO(m_vPoleCenter)),
SendPropFloat(SENDINFO(m_flPoleSlideSpeed)),

SendPropBool(SENDINFO(m_bSwimming)),
SendPropBool(SENDINFO(m_bIsUnderwater)),
SendPropFloat(SENDINFO(m_flSwimTime)),
#endif

END_NETWORK_TABLE()

#ifndef CLIENT_DLL

BEGIN_DATADESC_NO_BASE(CParkourController)
DEFINE_FIELD(m_bCanClimb, FIELD_BOOLEAN),
DEFINE_FIELD(m_bFacingWall, FIELD_BOOLEAN),
DEFINE_FIELD(m_bSwitchedOnAction, FIELD_BOOLEAN),
DEFINE_FIELD(m_bHoldingOnLedge, FIELD_BOOLEAN),
DEFINE_FIELD(m_flNextAction, FIELD_TIME),
DEFINE_FIELD(m_iClimbKicks, FIELD_INTEGER),
DEFINE_FIELD(m_flNextWallClimb, FIELD_TIME),
DEFINE_FIELD(m_flNextLedgeMove, FIELD_TIME),
DEFINE_FIELD(m_flLedgeSurface, FIELD_FLOAT),
DEFINE_FIELD(m_flLedgeDelta, FIELD_FLOAT),
DEFINE_FIELD(m_vLedgePlane, FIELD_VECTOR),
DEFINE_FIELD(m_vLedgeTarget, FIELD_VECTOR),
DEFINE_FIELD(m_eAnimState, FIELD_INTEGER),
DEFINE_FIELD(m_bRollReady, FIELD_BOOLEAN),
DEFINE_FIELD(m_vLedgeClimbTarget, FIELD_VECTOR),
DEFINE_FIELD(m_bHoldingOnPole, FIELD_BOOLEAN),
DEFINE_FIELD(m_vPoleTarget, FIELD_VECTOR),
DEFINE_FIELD(m_vPoleCenter, FIELD_VECTOR),
DEFINE_FIELD(m_flPoleSlideSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_bSwimming, FIELD_BOOLEAN),
DEFINE_FIELD(m_bIsUnderwater, FIELD_BOOLEAN),
DEFINE_FIELD(m_flSwimTime, FIELD_FLOAT),
DEFINE_FIELD(m_flPoleJumpTime, FIELD_TIME),
DEFINE_FIELD(m_hPole, FIELD_EHANDLE),
DEFINE_FIELD(m_flGroundTime, FIELD_TIME),

//DEFINE_FUNCTION(DropAnimstateThink),
END_DATADESC()

#else

BEGIN_PREDICTION_DATA_NO_BASE(CParkourController)
DEFINE_PRED_FIELD(m_bCanClimb, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bFacingWall, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bSwitchedOnAction, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),

DEFINE_PRED_FIELD(m_bHoldingOnLedge, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flNextAction, FIELD_TIME, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_iClimbKicks, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flNextWallClimb, FIELD_TIME, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flNextLedgeMove, FIELD_TIME, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flLedgeSurface, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flLedgeDelta, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_vLedgePlane, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_vLedgeTarget, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_eAnimState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flPoleJumpTime, FIELD_TIME, FTYPEDESC_INSENDTABLE),

DEFINE_PRED_FIELD(m_bRollReady, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),

DEFINE_PRED_FIELD(m_vLedgeClimbTarget, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),

DEFINE_PRED_FIELD(m_bHoldingOnPole, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_vPoleTarget, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_vPoleCenter, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flPoleSlideSpeed, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),

DEFINE_PRED_FIELD(m_bSwimming, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bIsUnderwater, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flSwimTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()

#endif


CParkourController::CParkourController()
{
	m_bCanClimb = true;
	m_bFacingWall = false;

	m_bHoldingOnLedge = false;
	m_flNextAction = 0;
	m_iClimbKicks = 0;
	m_flNextWallClimb = 0;
	m_flNextLedgeMove = 0;
	m_flLedgeSurface = 0;

	m_bRollReady = false;

	m_bSwimming = false;
	m_flSwimTime = 0;

	m_flPoleJumpTime = 0;

	m_eAnimState = PK_ANIM_NONE;

	m_bSwitchedOnAction = false;

#ifdef CLIENT_DLL
	m_vSavedViewOrigin = Vector();

	m_flAngleInterpHardness = 1.0f;
	m_flOriginInterpHardness = 1.0f;

	m_flViewLerp = 0.f;

	m_bResetInterp = false;
	m_bSyncVMWithCam = false;
	m_flLastViewCalcTime = 0.f;
#endif
}

CParkourController::~CParkourController()
{

}

void CParkourController::PreFrame()
{
	CHDTF_Player* pOwner = ToHDTFPlayer(m_hPlayer);
	if (pOwner == NULL)
		return;

#ifdef GAME_DLL
	if (pOwner->GetGroundEntity() != NULL)
	{
		m_flGroundTime = gpGlobals->curtime;
	}
#endif
}

void CParkourController::PostFrame()
{
#ifndef CLIENT_DLL
	CHDTF_Player* pOwner = ToHDTFPlayer(m_hPlayer);
	if (pOwner == NULL)
		return;

	// you need the parkour weapon stupid!
	if (!pOwner->Weapon_OwnsThisType("weapon_parkour"))
	{
		return;
	}

	// if we don't have an active weapon the default is parkour
	if (!pOwner->GetActiveWeapon())
	{
		pOwner->FastSwitchToParkour(true);
	}

	// Something has gone horrifically wrong or we've deliberately removed parkour
	if (!pOwner->GetActiveWeapon())
	{
		return;
	}

	// if we are doing something switch to parkour
	if (pOwner->IsOnLadder() || m_eAnimState & ~(PK_ANIM_NONE | PK_ANIM_RESET | PK_ANIM_SWIMMING))
	{
		if (!pOwner->GetActiveWeapon()->ClassMatches("weapon_parkour"))
		{
 			pOwner->FastSwitchToParkour(true);
			m_bSwitchedOnAction = true;
		}
	}

	// no actions allowed yet
	if (m_flNextAction >= gpGlobals->curtime)
		return;

	// we should not allow any actions in scenes
	if (HasFlagSet(PK_ANIM_CAMCONTROL))
		return;

	m_bSwimming = pOwner->GetWaterLevel() >= 2;
	m_bIsUnderwater = pOwner->GetWaterLevel() == 3;

	Vector vCenter, vEyePos, vLookDir;
	QAngle aEyeAngle;

	// wokring positions
	vEyePos = pOwner->EyePosition();
	aEyeAngle = pOwner->EyeAngles();
	vCenter = pOwner->GetAbsOrigin() + (vEyePos - pOwner->GetAbsOrigin()) * 0.9f;

	// constrain pitch
	if (pOwner->GetGroundEntity() != NULL)
		aEyeAngle.x = max(aEyeAngle.x, 0);
	else
		aEyeAngle.x = 0;

	AngleVectors(aEyeAngle, &vLookDir);

	if (m_bHoldingOnLedge)
	{
		return;
	}

	if (m_bHoldingOnPole)
	{
		return;
	}

	// if our movement is already being handled abort 
	if (pOwner->IsOnLadder() || pOwner->GetLadderMove()->m_bForceLadderMove || pOwner->IsInAVehicle())
	{
		m_bFacingWall = false;
		return;
	}

	m_bRollReady = (pOwner->m_nButtons & IN_DUCK) != 0;

	// can't do parkour while crouching or proning
	if (pOwner->m_nButtons & IN_DUCK || pOwner->IsProning())
	{
		m_bFacingWall = false;
		return;
	}

	// MAIN TRACE: ARE WE FACING A WALL?
	trace_t tr_wall;

	UTIL_TraceLine(
		vCenter, vCenter + vLookDir * 22, MASK_SOLID_BRUSHONLY,
		pOwner, COLLISION_GROUP_NONE, &tr_wall
	);

	// can climb the sky!
	if (tr_wall.surface.flags == SURF_SKY)
	{
		m_bFacingWall = false;
		return;
	}

	m_bFacingWall = FacingWall(pOwner, tr_wall);

	// only do this if we're in air
	if (
		pOwner->GetGroundEntity() != NULL
		|| pOwner->GetWaterLevel() != 0
		|| pOwner->GetMoveType() != MOVETYPE_WALK)
	{
		m_iClimbKicks = 0;
		return;
	}

	// if we didn't hit anything stop here
	if (!tr_wall.DidHit())
	{
		return;
	}

	// we hit the pole entity
	if (tr_wall.m_pEnt->ClassMatches("func_climbable_pole"))
	{
		HandlePoleLook(pOwner, tr_wall.m_pEnt, vCenter);
		return;
	}

	// We're facing something else

	// climb up
	if ((pOwner->m_nButtons & IN_JUMP) != 0
		&& m_flNextWallClimb <= gpGlobals->curtime
		&& m_iClimbKicks < sk_plr_parkour_max_climb_kicks.GetInt()
		&& pOwner->GetAbsVelocity().Length() < 400)
	{
		m_flNextWallClimb = gpGlobals->curtime + 0.25f;

		// NOTE(wheatley): if we've jumped from the ground don't perform the wall climb.
		// This fixes the tripple-jump trick. RIP 2015-2022.
		if (m_flGroundTime < gpGlobals->curtime)
		{
			pOwner->ViewPunch(QAngle(-3.5f, 0, 0));
			pOwner->SetAbsVelocity(Vector(0, 0, sk_plr_parkour_climb_kick_force.GetFloat()));
			m_iClimbKicks++;
		}
	}

	// Not allowed to climb
	if (!m_bCanClimb)
		return;

	// check if we can grab the ledge
	trace_t tr_ledge;
	UTIL_TraceLine(
		tr_wall.endpos + Vector(0, 0, PARKOUR_LEDGE_HEIGHT),
		tr_wall.endpos + Vector(0, 0, PARKOUR_LEDGE_HEIGHT) - tr_wall.plane.normal * 4,
		MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr_ledge);

	if (hdtf_parkour_debug.GetBool())
		NDebugOverlay::Line(tr_ledge.startpos, tr_ledge.endpos, 255, 0, 0, false, 10.f);

	// Something in the way?
	if (tr_ledge.DidHit() || (pOwner->m_nButtons & IN_DUCK))
		return;


	trace_t tr_ledgeAdj, tr_ledgeFloor, tr_postAttachFloor;
	UTIL_TraceLine(
		tr_ledge.endpos,
		tr_ledge.endpos - Vector(0, 0, PARKOUR_LEDGE_HEIGHT * 2.f),
		MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr_ledgeAdj);

	if (hdtf_parkour_debug.GetBool())
		NDebugOverlay::Line(tr_ledgeAdj.startpos, tr_ledgeAdj.endpos, 0, 255, 0, false, 10.f);

	if (tr_ledgeAdj.plane.normal.z < 0.75f)
		return;

	UTIL_TraceHull(
		pOwner->GetAbsOrigin(),
		pOwner->GetAbsOrigin() - Vector(0, 0, 12),
		VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID,
		pOwner, COLLISION_GROUP_NONE, &tr_ledgeFloor);

	if (!LedgeCheck(pOwner, tr_ledgeAdj.endpos, tr_wall.plane.normal))
		return;

	m_flLedgeSurface = tr_ledgeAdj.endpos.z;

	Vector hangedEyePos = tr_wall.endpos + tr_wall.plane.normal * VEC_HULL_MAX.x;
	hangedEyePos.z = m_flLedgeSurface - 10.f;

	Vector hangedPlayerPos = hangedEyePos;
	hangedPlayerPos.z -= pOwner->GetViewOffset().z;

	// checks if we WILL become to low to hang after being repositioned
	UTIL_TraceHull(
		pOwner->GetAbsOrigin(),
		hangedPlayerPos - Vector(0, 0, 12),
		VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID,
		pOwner, COLLISION_GROUP_NONE, &tr_postAttachFloor
	);

	// if we're too close to ground just
	// climb up without hanging on ledge
	float flVel = pOwner->GetAbsVelocity().z;
	if (tr_ledgeFloor.DidHit() || tr_postAttachFloor.DidHit())
	{
		if (flVel < 0.f)
			return;

		m_vLedgeClimbTarget = hangedEyePos - tr_wall.plane.normal * 24.f;
		m_vLedgeClimbTarget.GetForModify().z = m_flLedgeSurface;

		trace_t tr;

		UTIL_TraceHull(
			m_vLedgeClimbTarget, m_vLedgeClimbTarget,
			VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, MASK_PLAYERSOLID,
			pOwner, COLLISION_GROUP_NONE, &tr
		);

		// something's blocking our way up
		if (tr.DidHit())
			return;

		pOwner->SetMoveType(MOVETYPE_NONE);
		pOwner->SetAbsVelocity(Vector(0, 0, 0));
		pOwner->Teleport(&hangedPlayerPos, &pOwner->EyeAngles(), &pOwner->GetAbsVelocity());

		m_flNextAction = gpGlobals->curtime + 0.5f;

		if (!pOwner->GetToggledDuckState())
			pOwner->ToggleDuck();

		m_eAnimState = PK_ANIM_LOWOBSTACLE | PK_ANIM_CAMCONTROL | PK_ANIM_RESET;

		return;
	}

	trace_t tPlayerStuck;
	UTIL_TraceHull(
		hangedPlayerPos, hangedPlayerPos,
		VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID,
		pOwner, COLLISION_GROUP_NONE, &tPlayerStuck
	);

	if (hdtf_parkour_debug.GetBool())
	{
		if (tPlayerStuck.DidHit())
			NDebugOverlay::Box(hangedPlayerPos, VEC_HULL_MIN, VEC_HULL_MAX, 255, 0, 0, 255, 10.f);
		else
			NDebugOverlay::Box(hangedPlayerPos, VEC_HULL_MIN, VEC_HULL_MAX, 0, 255, 0, 255, 10.f);
	}

	// checks if player will become stuck in world geometry by hanging on that ledge
	if (tPlayerStuck.DidHit())
		return;

	// Set anim state and maybe do damage
	if (!OnLedgeImpact(pOwner, flVel))
		return; // you died.


	//? what this do?
	Vector mountPos = tr_wall.endpos + tr_wall.plane.normal * VEC_HULL_MAX.x;
	mountPos.z = m_flLedgeSurface - pOwner->GetViewOffset().z - 10.f;

	pOwner->SetMoveType(MOVETYPE_NONE);
	pOwner->SetAbsVelocity(Vector(0, 0, 0));
	pOwner->Teleport(&mountPos, &pOwner->EyeAngles(), &pOwner->GetAbsVelocity());

	m_vLedgeClimbTarget = tr_ledgeAdj.endpos;
	m_vLedgeTarget = mountPos;
	m_bHoldingOnLedge = true;

	m_flNextAction = m_flNextLedgeMove = gpGlobals->curtime + 0.1f;

	// forcefully close the inventory if it was open
	engine->ClientCommand(pOwner->edict(), "-inventory");

#endif
}

#ifdef GAME_DLL
bool CParkourController::FacingWall(CBasePlayer* pOwner, trace_t& tr)
{
	if (tr.DidHit())
	{
		m_vLedgePlane = tr.plane.normal;

		// tell the client side that we're facing the wall
		if (!tr.startsolid && !tr.m_pEnt->ClassMatches("func_climbable_pole"))
		{
			return true;
		}
	}

	return false;
}

void CParkourController::HandlePoleLook(CBasePlayer* pPlayer, CBaseEntity* pEnt, Vector& center)
{
	m_hPole = static_cast<CClimbablePole*>(pEnt);
	if (m_hPole)
	{
		CClimbablePole* pPole = static_cast<CClimbablePole*>(m_hPole.Get());
		Vector vBottomLimit, vTopLimit;
		pPole->GetLimits(vTopLimit, vBottomLimit);
		vTopLimit.z -= VEC_HULL_MAX.z + 28.f;

		// NOTE(wheatley): do not allow catching the pole if it will result in player
		// being above or below climbable range
		Vector mountPos = pPlayer->GetAbsOrigin();
		if (mountPos.z > vTopLimit.z || mountPos.z < vBottomLimit.z)
			return;

		m_bHoldingOnPole = true;

		m_vPoleCenter = m_hPole->WorldSpaceCenter();
		Vector vPole = m_vPoleCenter;
		vPole.z = center.z;

		m_vLedgePlane = (center - vPole).Normalized();

		pPlayer->ViewPunch(QAngle(20, 0, 0));
		pPlayer->SetMoveType(MOVETYPE_NONE);
		pPlayer->SetAbsVelocity(Vector(0, 0, 0));
		pPlayer->Teleport(&mountPos, &pPlayer->EyeAngles(), &pPlayer->GetAbsVelocity());

		m_eAnimState = PK_ANIM_POLECATCH | PK_ANIM_CAMCONTROL;

		// forcefully close the inventory if it was open
		engine->ClientCommand(pPlayer->edict(), "-inventory");
	}
}

// Returns false if we hit the ledge and died
bool CParkourController::OnLedgeImpact(CBasePlayer* pOwner, float velocity)
{
	// traumatic impact
	if (velocity < -450.f)
	{
		CTakeDamageInfo info;
		info.SetDamage(pow(abs(pOwner->GetAbsVelocity().z) / 120.f, 1.5f));
		info.SetDamageType(DMG_FALL);
		info.SetInflictor(NULL);
		info.SetAttacker(pOwner);
		info.SetDamagePosition(pOwner->GetAbsOrigin());
		info.SetDamageForce(pOwner->GetAbsVelocity());

		pOwner->TakeDamage(info);

		if (pOwner->GetHealth() <= 0)
			return false;

		m_eAnimState = PK_ANIM_LEDGETRAUMATIC | PK_ANIM_CAMCONTROL;
	}
	// heavy impact animation
	else if (velocity < -200.f)
	{
		m_eAnimState = PK_ANIM_LEDGEHARD | PK_ANIM_CAMCONTROL;
	}
	// climb up impact
	else
	{
		m_eAnimState = PK_ANIM_LEDGEGRAB | PK_ANIM_CAMCONTROL;
	}

	return true;
}
#endif

// -------------------------------------------------
// checks if we can roll and, if so, triggers it
// -------------------------------------------------
bool CParkourController::TriggerRoll()
{
	CBasePlayer* pOwner = ToBasePlayer(m_hPlayer);
	if (pOwner == NULL)
		return false;

	float vel = abs(pOwner->GetAbsVelocity().z);

	bool bCanRoll = m_bRollReady && (vel >= sk_plr_parkour_roll_threshold.GetFloat());

	if (bCanRoll)
	{
		m_eAnimState = PK_ANIM_ROLLING | PK_ANIM_CAMCONTROL;

#ifndef CLIENT_DLL
		if (!pOwner->GetToggledDuckState())
			pOwner->ToggleDuck();
#endif
	}

	return bCanRoll;
}

// -------------------------------------------------
// checks if we can climb this ledge
// -------------------------------------------------
bool CParkourController::LedgeCheck(CBasePlayer* pOwner, Vector& ledgeFloor, Vector& wallNormal)
{
	QAngle m_aAngle(0, 90, 0);
	Vector m_vRight;

	VectorRotate(wallNormal, m_aAngle, m_vRight);

	trace_t tr_r, tr_l;

	UTIL_TraceLine(
		ledgeFloor + Vector(0, 0, 2),
		ledgeFloor + Vector(0, 0, 2) + m_vRight * 20.f,
		MASK_PLAYERSOLID,
		pOwner,
		COLLISION_GROUP_NONE,
		&tr_r
	);

	UTIL_TraceLine(
		ledgeFloor + Vector(0, 0, 2),
		ledgeFloor + Vector(0, 0, 2) - m_vRight * 20.f,
		MASK_PLAYERSOLID,
		pOwner,
		COLLISION_GROUP_NONE,
		&tr_l
	);

	trace_t tr_dr, tr_dl;

	UTIL_TraceLine(
		ledgeFloor - Vector(0, 0, 2) + m_vRight * 20.f,
		ledgeFloor - Vector(0, 0, 2),
		MASK_PLAYERSOLID,
		pOwner,
		COLLISION_GROUP_NONE,
		&tr_dr
	);

	UTIL_TraceLine(
		ledgeFloor - Vector(0, 0, 2) - m_vRight * 20.f,
		ledgeFloor - Vector(0, 0, 2),
		MASK_PLAYERSOLID,
		pOwner,
		COLLISION_GROUP_NONE,
		&tr_dl
	);

#if defined(GAME_DLL) && defined(DEBUG)
	NDebugOverlay::Line(tr_dr.startpos, tr_dr.endpos, 0, 0, 255, false, 10.f);
	NDebugOverlay::Line(tr_dl.startpos, tr_dl.endpos, 255, 0, 0, false, 10.f);
#endif

	return !(tr_r.DidHit() || tr_l.DidHit()) && (tr_dr.startsolid && tr_dl.startsolid);
}

#ifdef CLIENT_DLL
// -------------------------------------------------
// first-person hands animations
// -------------------------------------------------
void CParkourController::CalcViewModel(const Vector& origin, const QAngle& angles, Vector& camPos, QAngle& camAngle)
{
	CBasePlayer* pOwner = ToBasePlayer(m_hPlayer);

	if (pOwner == NULL)
		return;

	CBaseViewModel* vm = pOwner->GetViewModel();

	if (vm == NULL)
		return;

	if (HasFlagSet(PK_ANIM_POLECLIMBUP) || m_flPoleJumpTime > gpGlobals->curtime)
		m_bSyncVMWithCam = true;

	if (m_bHoldingOnLedge
		|| HasFlagSet(PK_ANIM_LOWOBSTACLE)
		|| HasFlagSet(PK_ANIM_CLIMBLEDGE)
		|| HasFlagSet(PK_ANIM_LEDGEGRAB)
		|| HasFlagSet(PK_ANIM_LEDGEHARD)
		|| HasFlagSet(PK_ANIM_LEDGETRAUMATIC))
	{
		if (HasFlagSet(PK_ANIM_LEDGEGRAB) && vm->GetSequenceActivity(vm->GetSequence()) != ACT_VM_PARKOUR_LEDGE_GRAB)
		{
			QAngle ang;
			VectorAngles(m_vLedgePlane, ang);
			ang += QAngle(0, 180, 0);

			float diff = 1.f - (min(0, ang.x) / 90.f);

			m_vVMOrigin = origin + Vector(0, 0, 5 - 10 * diff);
			m_aVMAngles = ang;
			m_aVMAngles.x = max(angles.x / 1.75f, 0);

			m_flAngleInterpHardness = 0.5f;
		}
		else
		{
			if (HasFlagSet(PK_ANIM_LEDGEMOVERIGHT)
				|| HasFlagSet(PK_ANIM_LEDGEMOVELEFT))
				m_flAngleInterpHardness = 1.f;
			else
				m_flAngleInterpHardness = 0.5f;

			QAngle ang;
			VectorAngles(m_vLedgePlane, ang);

			ang += QAngle(0, 180, 0);

			ang.x = 0.f;

			m_vVMOrigin = origin - m_vLedgePlane * 3.5f;
			m_vVMOrigin.z = m_flLedgeSurface;

			m_aVMAngles = ang;
		}
	}
	else if (m_bHoldingOnPole)
	{
		QAngle ang;
		VectorAngles(m_vLedgePlane, ang);

		ang += QAngle(0, 180, 0);

		m_vVMOrigin = m_vPoleCenter + m_vLedgePlane * 25.f;
		m_vVMOrigin.z = origin.z;
		m_aVMAngles = ang;

		if (HasFlagSet(PK_ANIM_POLECLIMBUP))
		{
			if (m_flAngleInterpHardness == 0.6f)
				m_flAngleInterpHardness = 0.f;

			m_flAngleInterpHardness = Approach(0.5f, m_flAngleInterpHardness, gpGlobals->frametime * 4.15f);
			m_flOriginInterpHardness = 0.05f;

			float flLedgeSurface = m_flLedgeSurface.Get();
			m_vVMOrigin.z = Lerp(m_flAngleInterpHardness / 0.5f, m_vVMOrigin.z, flLedgeSurface);
		}
		else
		{
			m_flOriginInterpHardness = 1.0f;
			m_flAngleInterpHardness = 0.6f;
		}
	}
	else if (m_flNextWallClimb >= gpGlobals->curtime - 0.15f)
	{
		QAngle ang;
		VectorAngles(m_vLedgePlane, ang);
		ang += QAngle(0, 180, 0);

		float diff = 1.f - (min(0, ang.x) / 90.f);

		m_vVMOrigin = origin + Vector(0, 0, 5 - 10 * diff);
		m_aVMAngles = ang;
		m_aVMAngles.x = max(angles.x / 1.75f, 0);

		m_flAngleInterpHardness = 0.5f;
	}
	else if (m_bFacingWall)
	{
		VectorAngles(m_vLedgePlane, m_aVMAngles);
		m_aVMAngles.y += 180.f;
		m_aVMAngles.x = max(angles.x, 0);

		m_vVMOrigin = origin;

		m_flAngleInterpHardness = 0.20f;
	}
	else if (HasFlagSet(PK_ANIM_ROLLING))
	{
		m_vVMOrigin = origin;
		m_aVMAngles = m_aVMAngles;

		m_flAngleInterpHardness = 0.25f;
	}
	else if (m_bSwimming && !m_bIsUnderwater)
	{
		m_vVMOrigin = origin;
		m_aVMAngles = angles;
	}
	else if (!HasFlagSet(PK_ANIM_CAMCONTROL) && !m_bSyncVMWithCam)
	{
		m_flAngleInterpHardness = Approach(1.f, m_flAngleInterpHardness, gpGlobals->frametime * 3.f);
		m_flOriginInterpHardness = Approach(1.f, m_flOriginInterpHardness, gpGlobals->frametime * 3.f);

		m_vVMOrigin = origin;
		m_aVMAngles = angles;
	}

	if (m_bSyncVMWithCam && !HasFlagSet(PK_ANIM_WAITING))
	{
		InterpolateVector(m_flViewLerp, m_vSavedViewOrigin, m_vVMOrigin, m_vVMActualOrigin);
		InterpolateAngles(m_aVMActualAngles, m_aVMAngles, m_aVMActualAngles, m_flAngleInterpHardness);

		if (m_flViewLerp == 0.f)
			m_bSyncVMWithCam = false;
	}
	else
	{
		InterpolateVector(m_flOriginInterpHardness, m_vVMActualOrigin, m_vVMOrigin, m_vVMActualOrigin);
		InterpolateAngles(m_aVMActualAngles, m_aVMAngles, m_aVMActualAngles, m_flAngleInterpHardness);
	}

	vm->SetAbsOrigin(m_vVMActualOrigin);
	vm->SetAbsAngles(m_aVMActualAngles);

	int att = vm->LookupAttachment("camera");

	if (att == -1)
	{
		camPos = origin;
		camAngle = angles;

		Warning("Missing 'camera' attachment on parkour hands model!\n");
	}
	else
	{
		vm->GetAttachment(att, camPos, camAngle);
	}
}

// -------------------------------------------------
// camera origin & angles override
// -------------------------------------------------
void CParkourController::CalcView(Vector& origin, QAngle& angles, float& fov)
{
	C_HDTF_Player* pOwner = ToHDTFPlayer(m_hPlayer);

	// HACK(wheatley): there's a rare case when player drops from a ledge, then immediately
	// switch to other weapon (before m_flViewLerp had time to drop to zero) and then
	// switching back to parkour sometime later the camera will snap away for couple frames.
	if ((gpGlobals->curtime - m_flLastViewCalcTime) > (1.f / 6.f))
	{
		m_flViewLerp = 0.f;
		m_flAngleInterpHardness = 1.0f;
		m_flOriginInterpHardness = 1.0f;
	}

	m_flLastViewCalcTime = gpGlobals->curtime;

	if (!HasFlagSet(PK_ANIM_CAMCONTROL))
	{
		m_vViewBlend = origin;
		m_aViewBlend = angles;
	}

	Vector camPos;
	QAngle camAng;
	CalcViewModel(origin, angles, camPos, camAng);

	if (HasFlagSet(PK_ANIM_CAMCONTROL))
	{
		if (m_bResetInterp)
		{
			m_flViewLerp = 0.f;
			m_bResetInterp = false;
		}

		InterpolateVector(m_flViewLerp, m_vViewBlend, camPos, origin);
		InterpolateAngles(m_aViewBlend, camAng, angles, m_flViewLerp);

		m_aViewAngles = angles;
		m_vSavedViewOrigin = origin;

		m_flViewLerp = Approach(1.f, m_flViewLerp, gpGlobals->frametime * 5.5f);
	}
	else if (m_bHoldingOnLedge
		|| HasFlagSet(PK_ANIM_LEDGEMOVERIGHT)
		|| HasFlagSet(PK_ANIM_LEDGEMOVELEFT))
	{
		origin = camPos;
		m_vSavedViewOrigin = origin;

		matrix3x4_t mat;
		AngleMatrix(m_aVMAngles, mat);
		QAngle ang = TransformAnglesToLocalSpace(camAng, mat);
		angles += ang;

		m_flViewLerp = 1.f;

		m_bResetInterp = true;

		m_vViewBlend = origin;
		m_aViewBlend = angles;
	}
	else if (m_bHoldingOnPole)
	{
		origin = camPos;
		m_vSavedViewOrigin = origin;

		matrix3x4_t mat;
		AngleMatrix(m_aVMAngles, mat);
		QAngle ang = TransformAnglesToLocalSpace(camAng, mat);
		// NOTE(wheatley): what was I thinking then developing it in the first place?
		//  If someone misses that awful-vomit-inducing camera jittering 
		//  when climbing poles - uncomment this.
		// angles += ang;

		m_flViewLerp = 1.f;

		m_bResetInterp = true;
	}
	else if (m_flViewLerp > 0.f)
	{
		m_bResetInterp = false;

		m_flViewLerp = Approach(
			0.f,
			m_flViewLerp,
			gpGlobals->frametime * 6.f);

		InterpolateVector(clamp(1.f - m_flViewLerp, 0, 1), m_vSavedViewOrigin, origin, origin);
	}

	if (IsControllingView() && pOwner && pOwner->IsNightVisionActive())
	{
		Quaternion quat;
		AngleQuaternion(angles, quat);
		pOwner->CreateNightVisionLight(&origin, &quat);
	}
}
#endif