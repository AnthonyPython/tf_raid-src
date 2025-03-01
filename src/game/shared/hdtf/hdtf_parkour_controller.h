#ifndef HDTF_PARKOUR_CONTROLLER
#define HDTF_PARKOUR_CONTROLLER

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define CParkourController C_ParkourController
#endif

// parkour animation states
#define PK_ANIM_NONE			0
#define PK_ANIM_WAITING			1<<0
#define PK_ANIM_CAMCONTROL		1<<1
#define PK_ANIM_RESET			1<<2
#define PK_ANIM_LOWOBSTACLE		1<<3
#define PK_ANIM_CLIMBLEDGE		1<<4
#define PK_ANIM_LEDGEGRAB		1<<5
#define PK_ANIM_LEDGEHARD		1<<6
#define PK_ANIM_LEDGETRAUMATIC	1<<7
#define PK_ANIM_LEDGEMOVELEFT	1<<8
#define PK_ANIM_LEDGEMOVERIGHT	1<<9
#define PK_ANIM_ROLLING			1<<10
#define PK_ANIM_SWIMMING		1<<11
#define PK_ANIM_POLECATCH		1<<12
#define PK_ANIM_POLECLIMBUP		1<<13
#define PK_ANIM_POLEMOVE		1<<14

#include "baseplayer_shared.h"
#include "weapon_parkour.h"

#ifndef CLIENT_DLL
#include "ndebugoverlay.h"
#include "func_climbable_pole.h"
#endif

extern ConVar sk_plr_parkour_max_climb_kicks;
extern ConVar sk_plr_parkour_climb_kick_force;
extern ConVar sk_plr_parkour_roll_threshold;
extern ConVar sk_plr_parkour_pole_slide_acceleration;
extern ConVar sk_plr_parkour_pole_slide_maxspeed;
extern ConVar hdtf_parkour_force_climb;
extern ConVar hdtf_parkour_debug;
extern ConVar sk_plr_parkour_push_blocked_baseforce;
extern ConVar sk_plr_parkour_push_max_mass;

// this should be the same as root
// movement delta in the animation
// (defines how far player moves)
#define PARKOUR_LEDGE_MOVE_RANGE		17.28f

#define PARKOUR_LEDGE_MOVE_DELAY		0.67f
#define PARKOUR_LEDGE_MAX_YAW_DELTA		75.f
#define PARKOUR_LEDGE_MAX_PITCH_DELTA	14.5f
#define PARKOUR_LEDGE_HEIGHT			20.f
#define PARKOUR_ROLL_THRESHOLD			490.f
#define PARKOUR_POLEROTATION_DELTA		5.f

class CWeaponParkour;

class CParkourController
{
public:
	DECLARE_CLASS_NOBASE(CParkourController);
	DECLARE_EMBEDDED_NETWORKVAR();
	DECLARE_PREDICTABLE();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CParkourController();
	~CParkourController();

	void PreFrame();
	void PostFrame();
	bool TriggerRoll();

	bool HasFlagSet(int iFlag) { return (m_eAnimState & iFlag) != 0; };
	inline bool LedgeCheck(CBasePlayer* pOwner, Vector& ledgeFloor, Vector& wallNormal);

	bool IgnoreDepthHack() {
		return m_bHoldingOnPole;
	}
	void DisallowClimb() {
		m_bCanClimb = false;
	}
	void AllowClimb() {
		m_bCanClimb = true;
	}

#ifdef GAME_DLL
	inline void HandlePoleLook(CBasePlayer* pOwner, CBaseEntity* pEnt, Vector& center);
	inline bool FacingWall(CBasePlayer* pOwner, trace_t& tr);
	inline bool OnLedgeImpact(CBasePlayer* pOwner, float velocity);
#else
	void CalcView(Vector& origin, QAngle& angles, float& fov);
	void CalcViewModel(const Vector& origin, const QAngle& angles, Vector& camPos, QAngle& camAngle);
	bool IsControllingView() { return HasFlagSet(PK_ANIM_CAMCONTROL); }
#endif

	CBasePlayer *m_hPlayer;

	CNetworkVar(float, m_flNextAction);
	CNetworkVar(bool, m_bFacingWall);
	CNetworkVar(bool, m_bSwitchedOnAction);

	// climbing
	CNetworkVar(bool, m_bCanClimb);
	CNetworkVar(int, m_iClimbKicks);
	CNetworkVar(float, m_flNextWallClimb);
	CNetworkVector(m_vLedgeClimbTarget);

	// ledge movement
	CNetworkVar(bool, m_bHoldingOnLedge);
	CNetworkVector(m_vLedgePlane);
	CNetworkVector(m_vLedgeTarget);
	CNetworkVar(float, m_flNextLedgeMove);
	CNetworkVar(float, m_flLedgeSurface);
	CNetworkVar(float, m_flLedgeDelta);

	CNetworkVar(int, m_eAnimState);

	// pole movement
	CNetworkVar(bool, m_bHoldingOnPole);
	CNetworkVector(m_vPoleTarget);
	CNetworkVector(m_vPoleCenter);
	CNetworkVar(float, m_flPoleSlideSpeed);
	CNetworkVar(float, m_flPoleJumpTime);

	// roll
	CNetworkVar(bool, m_bRollReady);

	// swimming
	CNetworkVar(bool, m_bSwimming);
	CNetworkVar(bool, m_bIsUnderwater);
	CNetworkVar(float, m_flSwimTime);

#ifdef GAME_DLL
	EHANDLE m_hPole;
	float m_flGroundTime;
#else
	bool m_bResetInterp;
	bool m_bSyncVMWithCam;

	float m_flAngleInterpHardness;
	float m_flOriginInterpHardness;

	Vector m_vVMOrigin;
	Vector m_vVMActualOrigin;
	Vector m_vSavedViewOrigin;

	QAngle m_aVMAngles;
	QAngle m_aVMActualAngles;
	QAngle m_aViewAngles;

	Vector m_vViewBlend;
	QAngle m_aViewBlend;

	float m_flViewLerp;
	float m_flLastViewCalcTime;
#endif
};
#endif