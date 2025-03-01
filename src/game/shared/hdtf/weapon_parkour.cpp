#include <cbase.h>
#include "in_buttons.h"
#include "hdtf_player_shared.h"
#include "hdtf_gamerules.h"
#include "debugoverlay_shared.h"

#ifndef CLIENT_DLL
#include "npcevent.h"
#include "ai_basenpc.h"
#endif

#include "weapon_parkour.h"

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponParkour, DT_WeaponParkour)

BEGIN_NETWORK_TABLE(CWeaponParkour, DT_WeaponParkour)
END_NETWORK_TABLE()

#ifdef GAME_DLL
BEGIN_DATADESC(CWeaponParkour)
DEFINE_FUNCTION(DropAnimstateThink),
END_DATADESC()
#else
BEGIN_PREDICTION_DATA(CWeaponParkour)
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_parkour, CWeaponParkour);
PRECACHE_WEAPON_REGISTER(weapon_parkour);

#ifndef CLIENT_DLL

acttable_t CWeaponParkour::m_acttable[] =
{
	{ ACT_MELEE_ATTACK1, ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE, ACT_IDLE_ANGRY_MELEE, false },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_MELEE, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_MELEE, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_MELEE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_MELEE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_MELEE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_MELEE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_MELEE, false },
};

IMPLEMENT_ACTTABLE(CWeaponParkour);

#endif

CWeaponParkour::CWeaponParkour()
{
}

// -------------------------------------------------
// checks if we can holster this weapon
// -------------------------------------------------
bool CWeaponParkour::CanHolster() const
{
	CParkourController* pParkourController = &ToHDTFPlayer(GetOwner())->m_pParkourController;
	if (!pParkourController)
		return true;

	return (!pParkourController->m_bHoldingOnLedge && !pParkourController->m_bHoldingOnPole && pParkourController->m_eAnimState == PK_ANIM_NONE);
}

// -------------------------------------------------
// show player's crosshair on holster
// -------------------------------------------------
bool CWeaponParkour::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	CHDTF_Player* pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner == NULL)
		return false;

	if (pOwner)
	{
		pOwner->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
		CParkourController* pParkourController = &pOwner->m_pParkourController;
		pParkourController->m_eAnimState = PK_ANIM_NONE;
		pParkourController->m_bSwitchedOnAction = false;
	}

	return BaseClass::Holster(pSwitchingTo); 
}

// -------------------------------------------------
// hide player's crosshair on deploy
// -------------------------------------------------
bool CWeaponParkour::Deploy()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		pOwner->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;
	}

	const bool deployed = BaseClass::Deploy();

	if (deployed)
	{
		SendWeaponAnim(ACT_VM_IDLE);
		pOwner->SetNextAttack(gpGlobals->curtime);
	}

	return deployed;
}

// -------------------------------------------------
// triggers various parkour actions based on current conditions
// -------------------------------------------------
void CWeaponParkour::ItemPostFrame()
{
	CHDTF_Player* pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner == NULL)
		return;

	MaintainAnimations(pOwner);

	// if this fails you have some other problems...
	CParkourController* pParkourController = &pOwner->m_pParkourController;

	if (pParkourController->m_flNextAction >= gpGlobals->curtime)
		return;

	if (pParkourController->m_eAnimState & PK_ANIM_WAITING)
		return;

	if (pParkourController->m_bSwimming)
	{
		if (pParkourController->m_bIsUnderwater
			&& (pOwner->m_nButtons & IN_FORWARD) != 0
			&& !pParkourController->HasFlagSet(PK_ANIM_SWIMMING))
		{
			pParkourController->m_eAnimState = PK_ANIM_SWIMMING;
			pParkourController->m_flNextAction = gpGlobals->curtime + SequenceDuration(LookupSequence("swim")) + 0.15f;
		}

		// just in case
		pParkourController->m_bFacingWall = false;

		return;
	}

	if (pParkourController->m_bHoldingOnLedge)
	{
		ManageLedgeActions(pOwner);
		return;
	}

	if (pParkourController->m_bHoldingOnPole)
	{
		ManagePoleActions(pOwner);
		return;
	}

#ifndef CLIENT_DLL
	if (pParkourController->m_bSwitchedOnAction && !pParkourController->m_eAnimState && !pOwner->IsOnLadder())
	{
		pOwner->FastSwitchToParkour(true);
	}
#endif
}

// -------------------------------------------------
// update our climb position after moving on the ledge
// -------------------------------------------------
void CWeaponParkour::RecomputeClimbPos(CHDTF_Player *pOwner)
{
	CParkourController* pParkourController = &pOwner->m_pParkourController;

	Vector vEyePos = pOwner->EyePosition();

	Vector trPos = pParkourController->m_vLedgeTarget + Vector(0, 0, 15) - pParkourController->m_vLedgePlane.m_Value * 24.f;
	trPos.z += vEyePos.z - pParkourController->m_vLedgeTarget.m_Value.z;

	trace_t tClimb;

	UTIL_TraceLine(
		trPos,
		trPos - Vector(0, 0, 15),
		MASK_PLAYERSOLID,
		pOwner,
		COLLISION_GROUP_NONE,
		&tClimb
		);

	pParkourController->m_vLedgeClimbTarget = tClimb.endpos;
}

// -------------------------------------------------
// check if we can move this way; adjust move target vector
// -------------------------------------------------
bool CWeaponParkour::LedgeMoveCheck(CHDTF_Player *pOwner, bool isLeft)
{
	CParkourController* pParkourController = &pOwner->m_pParkourController;
	pParkourController->m_flLedgeDelta = 1.f;

	const Vector vStartPos = pOwner->GetAbsOrigin();
	trace_t tr;

	UTIL_TraceHull(
		vStartPos, 
		pParkourController->m_vLedgeTarget,
		VEC_HULL_MIN,
		VEC_HULL_MAX,
		MASK_PLAYERSOLID, 
		pOwner, 
		COLLISION_GROUP_NONE, 
		&tr
		);

	if (tr.DidHit())
	{
		Vector vEndPos = tr.endpos;
		vEndPos.z = vStartPos.z;
		pParkourController->m_flLedgeDelta = (vEndPos - vStartPos).Length() / PARKOUR_LEDGE_MOVE_RANGE;

		Vector mvShort, mvLong;

		if (isLeft)
		{
			GetAttachment(LookupAttachment("move_left_short"), mvShort);
			GetAttachment(LookupAttachment("move_left_long"), mvLong);
		}
		else
		{
			GetAttachment(LookupAttachment("move_right_short"), mvShort);
			GetAttachment(LookupAttachment("move_right_long"), mvLong);
		}

		pParkourController->m_vLedgeTarget.GetForModify() = VectorLerp(mvShort, mvLong, pParkourController->m_flLedgeDelta);
		pParkourController->m_vLedgeTarget.GetForModify().z = pOwner->GetAbsOrigin().z;

		// recalculate new climb location
		RecomputeClimbPos(pOwner);

		return pParkourController->m_flLedgeDelta > 1;
	}

	// now check if we can move further and not stuck
	// our hands in the wall next to ledge
	const Vector eyePos = pOwner->EyePosition();
	Vector checkPos = eyePos - pParkourController->m_vLedgePlane.m_Value * 24.f;
	checkPos.z = pParkourController->m_flLedgeSurface + 1.f;

	Vector dir = vStartPos - pParkourController->m_vLedgeTarget;
	dir.z = 0;
	const Vector endPos = checkPos - dir - dir.Normalized() * VEC_HULL_MAX.x;

	UTIL_TraceLine(
		checkPos,
		endPos,
		MASK_PLAYERSOLID,
		pOwner,
		COLLISION_GROUP_NONE,
		&tr
		);

#if defined(GAME_DLL) && defined(DEBUG)
	NDebugOverlay::Line(tr.startpos, tr.endpos, 0, 0, 255, true, 3.5f);
#endif

	if (tr.DidHit())
		return false;

	// now we need to check if ledge is still valid
	// so we're not going to move 'on air'
	// we don't need to do that if our previous trace
	// hit something

	const Vector sub = Vector(0, 0, 5);

	UTIL_TraceLine(
		tr.endpos - sub,
		checkPos - sub,
		MASK_PLAYERSOLID,
		pOwner,
		COLLISION_GROUP_NONE,
		&tr
		);

#if defined(GAME_DLL) && defined(DEBUG)
	NDebugOverlay::Line(tr.startpos, tr.endpos, 0, 255, 0, true, 3.5f);
#endif

	// if we start solid this means that there's a
	// ledge to hold on to
	if (!tr.startsolid)
		return false;

	// recalculate new climb location
	RecomputeClimbPos(pOwner);

	return true;
}

// -------------------------------------------------
// handle actions like ledge climb, drop and move
// -------------------------------------------------
void CWeaponParkour::ManageLedgeActions(CHDTF_Player *pOwner)
{
	CParkourController* pParkourController = &pOwner->m_pParkourController;
#ifndef CLIENT_DLL
	if (pOwner->IsOnLadder() || ToHDTFPlayer(pOwner)->GetLadderMove()->m_bForceLadderMove)
	{
		pParkourController->m_bHoldingOnLedge = false;
		pParkourController->m_flNextAction = gpGlobals->curtime + 0.5f;
		return;
	}
#endif

	// climb up
	if ((pOwner->m_nButtons & IN_JUMP) != 0)
	{
		trace_t tr;
		
		bool tr_isPropOrWeapon = false;

		UTIL_TraceHull(
			pParkourController->m_vLedgeClimbTarget,
			pParkourController->m_vLedgeClimbTarget,
			VEC_DUCK_HULL_MIN,
			VEC_DUCK_HULL_MAX,
			MASK_PLAYERSOLID,
			pOwner,
			COLLISION_GROUP_NONE,
			&tr
			);

		if (tr.m_pEnt != NULL && (tr.m_pEnt->ClassMatches("prop_physics*") || tr.m_pEnt->ClassMatches("weapon_*")))
		{
			DevMsg("detected, %s", tr.m_pEnt->GetClassname());
			//push us the hell away!
			IPhysicsObject* pPhys = tr.m_pEnt->VPhysicsGetObject();
			if (pPhys && tr.m_pEnt->GetMoveType() == MOVETYPE_VPHYSICS && pPhys->GetMass() < sk_plr_parkour_push_max_mass.GetFloat())
			{
				Vector vecVelocity = (Vector(0, 0, sk_plr_parkour_push_blocked_baseforce.GetFloat()) + tr.m_pEnt->GetAbsOrigin());
				VectorNormalize(vecVelocity);
				vecVelocity *= 5 * pPhys->GetMass();
				pPhys->AddVelocity(&vecVelocity, NULL);
			}
			tr_isPropOrWeapon = true;
		}

		Color hullColor;

		if (tr.DidHit())
		{
			hullColor.SetColor(255, 0, 0, 64);
		}
		else {
			hullColor.SetColor(0, 255, 0, 64);
		}

		if(hdtf_parkour_debug.GetBool())
			NDebugOverlay::Box(pParkourController->m_vLedgeClimbTarget, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, hullColor.r(), hullColor.g(), hullColor.b(), hullColor.a(), 5.f);

		// something's blocking our way up
		if (tr.DidHit() && !hdtf_parkour_force_climb.GetBool() && !tr_isPropOrWeapon)
			return;

		pParkourController->m_bHoldingOnLedge = false;

		float delay = SequenceDuration(LookupSequence("ledgeclimb"));
		pParkourController->m_flNextAction = gpGlobals->curtime + delay;

#ifndef CLIENT_DLL
		if (!pOwner->GetToggledDuckState())
			pOwner->ToggleDuck();
#endif

		pParkourController->m_eAnimState = PK_ANIM_CLIMBLEDGE | PK_ANIM_CAMCONTROL | PK_ANIM_RESET;

		return;
	}

#ifndef CLIENT_DLL
	// drop down
	// RAY: just need to press back now
	if (pOwner->m_flForwardMove < -0.5f || (pOwner->m_nButtons & IN_DUCK) != 0 || !pParkourController->m_bCanClimb)
	{
		pParkourController->m_bHoldingOnLedge = false;
		pOwner->SetMoveType(MOVETYPE_WALK);
		pParkourController->m_flNextAction = gpGlobals->curtime + 0.5f;
		return;
	}

	trace_t tr;
	UTIL_TraceLine(pOwner->GetAbsOrigin(), pOwner->GetAbsOrigin() - Vector(0, 0, 5), MASK_PLAYERSOLID, pOwner, COLLISION_GROUP_NONE, &tr);
	if (tr.DidHit())
	{
		// wow wow, we've got a floor right under our legs?!
		// drop off immediatly!
		pParkourController->m_bHoldingOnLedge = false;
		pOwner->SetMoveType(MOVETYPE_WALK);
		pParkourController->m_flNextAction = gpGlobals->curtime + 0.5f;
		return;
	}

	UTIL_TraceLine(pOwner->EyePosition(), pOwner->EyePosition() + Vector(0, 0, 15), MASK_PLAYERSOLID, pOwner, COLLISION_GROUP_NONE, &tr);
	if (tr.DidHit() && tr.m_pEnt)
	{
		// whoops. something just crushed us. rip.
		// (note: right now it can literaly be anything, even a soda can...)
		CTakeDamageInfo dmginf;
		dmginf.SetDamage(9999);
		dmginf.SetDamageType(DMG_CRUSH);
		dmginf.SetDamageForce(Vector(0, 0, -90000));
		pOwner->TakeDamage(dmginf);
		return;
	}

	if (pParkourController->m_flNextLedgeMove < gpGlobals->curtime)
	{
		const bool bWantsToMoveLeft = pOwner->m_flSideMove < -0.5f || (pOwner->m_nButtons & IN_MOVELEFT) != 0;
		const bool bWantsToMoveRight = pOwner->m_flSideMove > 0.5f || (pOwner->m_nButtons & IN_MOVERIGHT) != 0;
		
		// just why people keep doing that
		if (bWantsToMoveLeft && bWantsToMoveRight)
		{
			return;
		}

		// move left
		if (bWantsToMoveLeft)
		{
			QAngle ang;
			VectorAngles(pParkourController->m_vLedgePlane, ang);
			ang.y += 180;
			ang.x = 0.f;
			SetAbsAngles(ang);

			GetAttachment(LookupAttachment("move_left_long"), pParkourController->m_vLedgeTarget.GetForModify());
			pParkourController->m_vLedgeTarget.GetForModify().z = pOwner->GetAbsOrigin().z;

			if (LedgeMoveCheck(pOwner, true))
			{
				pParkourController->m_eAnimState = PK_ANIM_LEDGEMOVELEFT;
				pParkourController->m_flNextLedgeMove = gpGlobals->curtime + SequenceDuration(LookupSequence("ledgemove_left"));
				pParkourController->m_flNextAction = pParkourController->m_flNextLedgeMove + 0.05f;
			}
		}

		// move right
		if (bWantsToMoveRight)
		{
			QAngle ang;
			VectorAngles(pParkourController->m_vLedgePlane, ang);
			ang.y += 180;
			ang.x = 0.f;
			SetAbsAngles(ang);

			GetAttachment(LookupAttachment("move_right_long"), pParkourController->m_vLedgeTarget.GetForModify());
			pParkourController->m_vLedgeTarget.GetForModify().z = pOwner->GetAbsOrigin().z;

			if (LedgeMoveCheck(pOwner))
			{
				pParkourController->m_eAnimState = PK_ANIM_LEDGEMOVERIGHT;
				pParkourController->m_flNextLedgeMove = gpGlobals->curtime + SequenceDuration(LookupSequence("ledgemove_right"));
				pParkourController->m_flNextAction = pParkourController->m_flNextLedgeMove + 0.05f;
			}
		}
	}
#endif
}

// -------------------------------------------------
// handle actions like pole move, drop and jump
// -------------------------------------------------
void CWeaponParkour::ManagePoleActions(CHDTF_Player *pOwner)
{
#ifndef CLIENT_DLL
	CParkourController* pParkourController = &ToHDTFPlayer(GetOwner())->m_pParkourController;
	if (!pParkourController)
		return;

	if (pOwner->IsOnLadder() || ToHDTFPlayer(pOwner)->GetLadderMove()->m_bForceLadderMove)
	{
		pParkourController->m_bHoldingOnPole = false;
		pParkourController->m_hPole = NULL;
		pParkourController->m_flNextAction = gpGlobals->curtime + 0.5f;
		return;
	}

	float m_flMaxMoveSpeed = sk_plr_parkour_pole_slide_maxspeed.GetFloat();
	float m_flSlideAccel = sk_plr_parkour_pole_slide_acceleration.GetFloat();

	// drop off
	if ((pOwner->m_nButtons & IN_DUCK) != 0 || !pParkourController->m_bCanClimb)
	{
		Vector mountPos = pOwner->GetAbsOrigin();

		pParkourController->m_bHoldingOnPole = false;
		pOwner->SetMoveType(MOVETYPE_WALK);
		pParkourController->m_flNextAction = gpGlobals->curtime + 0.5f;
		pParkourController->m_hPole = NULL;
		pOwner->Teleport(&mountPos, &pOwner->EyeAngles(), &pOwner->GetAbsVelocity());

		return;
	}

	// move up
	if ((((pOwner->m_nButtons & IN_FORWARD) != 0) || (pOwner->m_flForwardMove > 0.5f)) && ((pOwner->m_nButtons & IN_JUMP) == 0))
	{
		QAngle ang = GetAbsAngles();
		ang.x = 0.f;
		SetAbsAngles(ang);

		pParkourController->m_vPoleTarget = pOwner->GetAbsOrigin();
		pParkourController->m_vPoleTarget += Vector(0, 0, 23.0);

		//NDebugOverlay::Cross3D(target, 24.f, 255, 0, 0, false, 10.f);

		if (PoleMoveCheck(pOwner))
		{
			pParkourController->m_eAnimState = PK_ANIM_POLEMOVE;
			pParkourController->m_flNextAction = gpGlobals->curtime + SequenceDuration(LookupSequence("pole_moveup")) + 0.025f;
		}

		return;
	}

	// move down
	if (((pOwner->m_nButtons & IN_BACK) != 0) || (pOwner->m_flForwardMove < -0.5f))
	{
		pParkourController->m_vPoleTarget = pOwner->GetAbsOrigin() + Vector(0, 0, -pParkourController->m_flPoleSlideSpeed);
		if (PoleMoveCheck(pOwner))
		{
			pParkourController->m_flPoleSlideSpeed = min(pParkourController->m_flPoleSlideSpeed + m_flSlideAccel, m_flMaxMoveSpeed);
			pOwner->Teleport(&pParkourController->m_vPoleTarget.Get(), &pOwner->EyeAngles(), &pOwner->GetAbsVelocity());
		}
	}
	else
	{
		pParkourController->m_flPoleSlideSpeed = 0.f;
	}

	// jump in the direction
	if ((pOwner->m_nButtons & IN_JUMP) != 0)
	{
		QAngle aAngles = pOwner->EyeAngles();
		Vector vDir;
		aAngles.x = 0;
		AngleVectors(aAngles, &vDir);
		float dot = vDir.Dot(pParkourController->m_vLedgePlane);

		if (dot > -0.70f)
		{
			Vector tossVelocity = vDir * 375 + Vector(0, 0, 200);
			Vector mountPos = pOwner->GetAbsOrigin();

			pParkourController->m_bHoldingOnPole = false;
			pOwner->SetMoveType(MOVETYPE_WALK);
			pParkourController->m_flNextAction = gpGlobals->curtime + 0.5f;
			pParkourController->m_hPole = NULL;
			pOwner->Teleport(&mountPos, &pOwner->EyeAngles(), &tossVelocity);
			pOwner->ViewPunch(QAngle(-15, 0, 0));
			pParkourController->m_flPoleJumpTime = gpGlobals->curtime + 0.25f;
		}
	}
#endif
}

// -------------------------------------------------
// check if we can move this way being on the pole
// -------------------------------------------------
bool CWeaponParkour::PoleMoveCheck(CHDTF_Player *pOwner)
{
#ifndef CLIENT_DLL
	CParkourController* pParkourController = &pOwner->m_pParkourController;

	Vector vBottomLimit, vTopLimit;
	CClimbablePole *poleEntity = static_cast<CClimbablePole *>(pParkourController->m_hPole.Get());
	poleEntity->GetLimits(vTopLimit, vBottomLimit);

	vTopLimit.z -= VEC_HULL_MAX.z + 28.f;

	trace_t tHull;

	bool tSpace_isPropOrWeapon = false;

	CTraceFilterSkipTwoEntities traceFilter(pOwner, poleEntity, COLLISION_GROUP_NONE);

	UTIL_TraceHull(
		pOwner->GetAbsOrigin(),
		pParkourController->m_vPoleTarget,
		VEC_HULL_MIN,
		VEC_HULL_MAX,
		MASK_PLAYERSOLID,
		&traceFilter,
		&tHull
		);

	if (hdtf_parkour_debug.GetBool())
	{
		if (tHull.DidHit())
			NDebugOverlay::Box(tHull.endpos, VEC_HULL_MIN, VEC_HULL_MAX, 255, 0, 0, 255, 10.f);
		else
			NDebugOverlay::Box(tHull.endpos, VEC_HULL_MIN, VEC_HULL_MAX, 0, 255, 0, 255, 2.5f);
	}

	// hit the ground? drop off
	if (tHull.DidHit() 
		&& pParkourController->m_vPoleTarget.Get().z <= pOwner->GetAbsOrigin().z)
	{
		Vector mountPos = pOwner->GetAbsOrigin();
		pOwner->Teleport(&mountPos, &pOwner->EyeAngles(), &pOwner->GetAbsVelocity());

		pParkourController->m_bHoldingOnPole = false;
		pOwner->SetMoveType(MOVETYPE_WALK);
		pParkourController->m_flNextAction = gpGlobals->curtime + 0.5f;
		pParkourController->m_hPole = NULL;

		return false;
	}

	if (pParkourController->m_vPoleTarget.Get().z > vTopLimit.z)
	{
		Vector vPoleTop = vTopLimit + Vector(0, 0, 31.f + VEC_HULL_MAX.z);

		// reached top of the pole
		// at this point we need to check if there's
		// free space atop of the pole
		trace_t tSpace;

		UTIL_TraceHull(
			vPoleTop,
			vPoleTop,
			VEC_HULL_MIN,
			VEC_HULL_MAX,
			MASK_PLAYERSOLID,
			pOwner,
			COLLISION_GROUP_NONE,
			&tSpace
			);

		if (tSpace.m_pEnt != NULL && (tSpace.m_pEnt->ClassMatches("prop_physics*") || tSpace.m_pEnt->ClassMatches("weapon_*")))
		{
			DevMsg("detected, %s", tSpace.m_pEnt->GetClassname());

			//push us the hell away!
			IPhysicsObject* pPhys = tSpace.m_pEnt->VPhysicsGetObject();
			if (pPhys && tSpace.m_pEnt->GetMoveType() == MOVETYPE_VPHYSICS && pPhys->GetMass() < sk_plr_parkour_push_max_mass.GetFloat())
			{
				Vector vecVelocity = (Vector(0,0, sk_plr_parkour_push_blocked_baseforce.GetFloat()) + tSpace.m_pEnt->GetAbsOrigin());
				VectorNormalize(vecVelocity);
				vecVelocity *= 5 * pPhys->GetMass();
				pPhys->AddVelocity(&vecVelocity, NULL);
			}
			tSpace_isPropOrWeapon = true;
		}

		// not enough space
		// no actions required
		if (tSpace.DidHit() && !tSpace_isPropOrWeapon)
		{
			return false;
		}

		pParkourController->m_vLedgeClimbTarget = vPoleTop;
		pParkourController->m_flLedgeSurface = vPoleTop.z - 28.f;

		pParkourController->m_eAnimState = PK_ANIM_POLECLIMBUP | PK_ANIM_CAMCONTROL | PK_ANIM_RESET;

		return false;
	}

	return (pParkourController->m_vPoleTarget.Get().z > vBottomLimit.z) && (pParkourController->m_vPoleTarget.Get().z < vTopLimit.z) && !tHull.DidHit();
#else
	return false;
#endif
}

// -------------------------------------------------
// whether or not we allow customized movement actions (like proning)
// -------------------------------------------------
bool CWeaponParkour::AllowCustomizedMovement()
{
	CHDTF_Player* pOwner = ToHDTFPlayer(GetOwner());
	if (!pOwner)
		return false;

	CParkourController* pParkourController = &pOwner->m_pParkourController;
	return !pParkourController->m_bHoldingOnLedge && !pParkourController->m_bHoldingOnPole;
}

// -------------------------------------------------
// makes sure that animation corresponds to our current state
// -------------------------------------------------
void CWeaponParkour::MaintainAnimations(CHDTF_Player* pOwner)
{
	CParkourController* pParkourController = &pOwner->m_pParkourController;

	if (pParkourController->m_eAnimState != PK_ANIM_NONE)
	{
		// are we just waiting for animation to complete?
		if ((pParkourController->m_eAnimState & PK_ANIM_WAITING) != 0)
			return;
		
		int clearFlag = pParkourController->m_eAnimState;
		clearFlag &= ~PK_ANIM_CAMCONTROL;
		clearFlag &= ~PK_ANIM_RESET;

		switch (clearFlag)
		{
		case PK_ANIM_CLIMBLEDGE:
		case PK_ANIM_LOWOBSTACLE:
			SendWeaponAnim(ACT_VM_PARKOUR_LEDGE_CLIMB);
			break;

		case PK_ANIM_LEDGEGRAB:
			SendWeaponAnim(ACT_VM_PARKOUR_LEDGE_GRAB);
			break;

		case PK_ANIM_LEDGEHARD:
			SendWeaponAnim(ACT_VM_PARKOUR_LEDGE_GRAB2);
			break;

		case PK_ANIM_LEDGETRAUMATIC:
			SendWeaponAnim(ACT_VM_PARKOUR_LEDGE_GRAB3);
			break;

		case PK_ANIM_LEDGEMOVELEFT:
			SendWeaponAnim(ACT_VM_PARKOUR_LEDGE_MOVE_LEFT);
			break;

		case PK_ANIM_LEDGEMOVERIGHT:
			SendWeaponAnim(ACT_VM_PARKOUR_LEDGE_MOVE_RIGHT);
			break;

		case PK_ANIM_ROLLING:
			SendWeaponAnim(ACT_VM_PARKOUR_ROLL);
			break;

		case PK_ANIM_SWIMMING:
			SendWeaponAnim(ACT_VM_PARKOUR_SWIM);
			break;

		case PK_ANIM_POLECATCH:
			SendWeaponAnim(ACT_VM_PARKOUR_POLE_CATCH);
			break;

		case PK_ANIM_POLECLIMBUP:
			SendWeaponAnim(ACT_VM_PARKOUR_POLE_CLIMBUP);
			break;

		case PK_ANIM_POLEMOVE:
			SendWeaponAnim(ACT_VM_PARKOUR_POLE_MOVE);
			break;

		default:
			break;
		}

		CBaseViewModel *vm = pOwner->GetViewModel();
		vm->SetPoseParameter("ledgemove_delta", pParkourController->m_flLedgeDelta);

		float duration = SequenceDuration();

		SetContextThink(&CWeaponParkour::DropAnimstateThink,
			gpGlobals->curtime + duration,
			PARKOUR_DROPANIM_CONTEXT);

		pParkourController->m_eAnimState = pParkourController->m_eAnimState | PK_ANIM_WAITING;

		return;
	}

	Activity ideal = ACT_VM_IDLE;
	Activity current = GetActivity();

	// are we holding on ledge?
	if (pParkourController->m_bHoldingOnLedge)
		ideal = ACT_VM_PARKOUR_LEDGE_IDLE;

	// on pole?
	else if (pParkourController->m_bHoldingOnPole)
		ideal = ACT_VM_PARKOUR_POLE_IDLE;

	// climbing up?
	else if (pParkourController->m_flNextWallClimb >= gpGlobals->curtime - 0.15f)
		ideal = ACT_VM_PARKOUR_WALL_CLIMB;

	else if (pParkourController->m_bFacingWall && pOwner->GetGroundEntity() == NULL && pOwner->GetAbsVelocity().z < 0)
		ideal = ACT_VM_PARKOUR_WALL_SLIP;

	else if (pParkourController->m_bFacingWall)
		ideal = ACT_VM_PARKOUR_HANDS_RISE;

	else if (!pParkourController->m_bFacingWall && current == ACT_VM_PARKOUR_HANDS_RISE)
		ideal = ACT_VM_PARKOUR_HANDS_LOWER;

	else if (current == ACT_VM_PARKOUR_HANDS_LOWER && !HasWeaponIdleTimeElapsed())
		ideal = ACT_VM_PARKOUR_HANDS_LOWER;

	else if (pOwner->IsSprinting() && pOwner->GetGroundEntity())
		ideal = ACT_VM_SPRINT_IDLE;

	else if (current == ACT_VM_SPRINT_IDLE 
		|| current == ACT_VM_SPRINT_ENTER 
		|| (current == ACT_VM_SPRINT_LEAVE && GetCycle() != 1.f))
		ideal = ACT_VM_SPRINT_LEAVE;

	if (current != ideal)
	{
		SendWeaponAnim(ideal);
		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();
	}
}

// -------------------------------------------------
// drops animstate
// -------------------------------------------------
void CWeaponParkour::DropAnimstateThink()
{
	CHDTF_Player* pOwner = ToHDTFPlayer(GetOwner());

	if (!pOwner)
		return;

	CParkourController* pParkourController = &pOwner->m_pParkourController;
	if (!pParkourController)
		return;

	if (pParkourController->m_eAnimState & PK_ANIM_RESET)
	{
		pParkourController->m_eAnimState &= ~PK_ANIM_RESET;
		SendWeaponAnim(ACT_VM_IDLE);
	}

	ManagePostAnimActions();

	pParkourController->m_eAnimState = PK_ANIM_NONE;
}

// -------------------------------------------------
// do actions, based on played animation
// -------------------------------------------------
void CWeaponParkour::ManagePostAnimActions()
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());

	if (!pOwner)
		return;

	CParkourController* pParkourController = &pOwner->m_pParkourController;
	if (!pParkourController)
		return;


	int clearFlag = pParkourController->m_eAnimState;
	clearFlag &= ~PK_ANIM_CAMCONTROL;
	clearFlag &= ~PK_ANIM_WAITING;

#ifndef CLIENT_DLL
	if (clearFlag == PK_ANIM_CLIMBLEDGE
		|| clearFlag == PK_ANIM_LOWOBSTACLE)
	{
		pParkourController->m_bFacingWall = false;

		pOwner->Teleport(&pParkourController->m_vLedgeClimbTarget.Get(), &pOwner->EyeAngles(), &pOwner->GetAbsVelocity());
		pOwner->SetMoveType(MOVETYPE_WALK);

		if (pOwner->GetToggledDuckState())
			pOwner->ToggleDuck();
	}
	else if(clearFlag == PK_ANIM_ROLLING)
	{
		if (pOwner->GetToggledDuckState())
			pOwner->ToggleDuck();
	}
	else if (clearFlag == PK_ANIM_POLECLIMBUP)
	{
		pParkourController->m_bHoldingOnPole = false;
		pParkourController->m_flNextAction = gpGlobals->curtime + 0.15f;
		pParkourController->m_hPole = NULL;

		pOwner->Teleport(&pParkourController->m_vLedgeClimbTarget.Get(), &pOwner->EyeAngles(), &pOwner->GetAbsVelocity());
		pOwner->SetMoveType(MOVETYPE_WALK);
		SendWeaponAnim(ACT_VM_IDLE);
	}
	else if (clearFlag == PK_ANIM_LEDGEMOVELEFT || clearFlag == PK_ANIM_LEDGEMOVERIGHT)
	{
		pOwner->Teleport(&pParkourController->m_vLedgeTarget.Get(), &pOwner->EyeAngles(), &pOwner->GetAbsVelocity());
		SendWeaponAnim(ACT_VM_PARKOUR_LEDGE_IDLE);
	}
	else if (clearFlag == PK_ANIM_POLEMOVE)
	{
		pOwner->Teleport(&pParkourController->m_vPoleTarget.Get(), &pOwner->EyeAngles(), &pOwner->GetAbsVelocity());
		SendWeaponAnim(ACT_VM_PARKOUR_POLE_IDLE);
	}
#endif
}

#ifdef CLIENT_DLL
// -------------------------------------------------
// create move
// this used to actualy modify player's angles
// to force him look in certain direction after
// first-person scene ends OR keep eye angles
// in appropriate range
// -------------------------------------------------
void CWeaponParkour::CreateMove(float flInputSampleTime, CUserCmd *pCmd, const QAngle &vecOldViewAngles)
{
	CHDTF_Player* player = ToHDTFPlayer(GetOwner());
	CParkourController* pParkourController = &player->m_pParkourController;
	if (!pParkourController)
		return;

	if (pParkourController->HasFlagSet(PK_ANIM_CAMCONTROL))
	{
		pParkourController->m_aViewAngles.z = 0; // no roll
		pCmd->viewangles = pParkourController->m_aViewAngles;
		return;
	}

	if (pParkourController->m_bHoldingOnLedge || pParkourController->m_bHoldingOnPole)
	{
		QAngle &angles = pCmd->viewangles;

		Vector nLedgePlane = pParkourController->m_vLedgePlane;
		nLedgePlane = -nLedgePlane;

		QAngle ang;
		VectorAngles(nLedgePlane, ang);

		QAngle diff = angles - ang;
		diff.y = AngleNormalize(diff.y);

		if (diff.y > PARKOUR_LEDGE_MAX_YAW_DELTA)
			angles.y = ang.y + PARKOUR_LEDGE_MAX_YAW_DELTA;
		else if (diff.y < -PARKOUR_LEDGE_MAX_YAW_DELTA)
			angles.y = ang.y - PARKOUR_LEDGE_MAX_YAW_DELTA;

		if (angles.x > PARKOUR_LEDGE_MAX_PITCH_DELTA)
			angles.x = PARKOUR_LEDGE_MAX_PITCH_DELTA;
	}
}
#endif
