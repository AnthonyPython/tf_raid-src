#ifndef WEAPON_PARKOUR
#define WEAPON_PARKOUR

#ifdef _WIN32
#pragma once
#endif

#ifndef CLIENT_DLL
#include "ndebugoverlay.h"
#include "func_climbable_pole.h"
#endif

#include "weapon_basehdtfcombat.h"
#include "hdtf_parkour_controller.h"

#define PARKOUR_DROPANIM_CONTEXT "Parkour_DropAnimState"

#ifdef CLIENT_DLL
#define CWeaponParkour C_WeaponParkour
#endif

class CHDTF_Player;

class CWeaponParkour : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponParkour, CBaseHDTFCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL

	DECLARE_DATADESC();
	DECLARE_ACTTABLE();

#endif

	CWeaponParkour();

	void ItemPostFrame();

	bool VisibleInWeaponSelection() { return true; }
	bool ShouldDrawCrosshair() { return false; }
	bool HasIronsights() { return false; }

	virtual bool CanHolster() const;
	virtual bool Holster(CBaseCombatWeapon* pSwitchingTo = NULL);
	virtual bool Deploy();

	void ManageLedgeActions(CHDTF_Player* pOwner);
	bool LedgeMoveCheck(CHDTF_Player* pOwner, bool isLeft = false);
	void RecomputeClimbPos(CHDTF_Player* pOwner);

	void ManagePoleActions(CHDTF_Player* pOwner);
	bool PoleMoveCheck(CHDTF_Player* pOwner);

	// animations
	void MaintainAnimations(CHDTF_Player* pOwner);
	void DropAnimstateThink();
	void ManagePostAnimActions();

	bool AllowCustomizedMovement();

#ifdef CLIENT_DLL
	bool ShouldDrawPickup() { return false; }
	void CreateMove(float flInputSampleTime, CUserCmd* pCmd, const QAngle& vecOldViewAngles);
#endif

private:
	CWeaponParkour(const CWeaponParkour&);
};
#endif