#include "cbase.h"
#include "in_buttons.h"
#include "weapon_basehdtfcombat.h"
#include "npcevent.h"
#include "hdtf_player_shared.h"

#ifndef CLIENT_DLL
#include "soundent.h"
#include "ai_basenpc.h"
#endif

float slide_delay;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	PISTOL_FASTEST_REFIRE_TIME 0.1f
#define	PISTOL_FASTEST_DRY_REFIRE_TIME 0.2f

#define	PISTOL_ACCURACY_SHOT_PENALTY_TIME 0.2f // Applied amount of time each shot adds to the time we must recover from
#define	PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME 1.5f // Maximum penalty to deal out

#ifdef CLIENT_DLL

#define CWeaponHL2Pistol C_WeaponHL2Pistol

#endif

class CWeaponHL2Pistol : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponHL2Pistol, CBaseHDTFCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE();

#endif

	CWeaponHL2Pistol();

	void ItemPostFrame();
	void ItemPreFrame();
	void ItemBusyFrame();
	void PrimaryAttack();
	void AddViewKick();
	void DryFire();

	void EnableIronsights();
	void DisableIronsights(bool ignoreAnimation = false);

	void UpdatePenaltyTime();

#ifndef CLIENT_DLL
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	int	CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	const WeaponProficiencyInfo_t *GetProficiencyValues();
#endif

	virtual bool Reload();

	virtual Activity GetPrimaryAttackActivity()
	{
		if (m_iClip1 == 1)
			return IsIronsightsEnabled() ? ACT_VM_PRIMARYATTACK_IRONSIGHTS_SPECIAL : ACT_VM_PRIMARYATTACK_SPECIAL;

		return IsIronsightsEnabled() ? ACT_VM_PRIMARYATTACK_IRONSIGHTS : ACT_VM_PRIMARYATTACK;
	}

	virtual const Vector &GetBulletSpread()
	{
		static Vector npcCone = VECTOR_CONE_5DEGREES;
		if (GetOwner() && GetOwner()->IsNPC())
			return npcCone;

		static Vector cone;

		float ramp = RemapValClamped(m_flAccuracyPenalty,
			0.0f,
			PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME,
			0.0f,
			1.0f);

		// We lerp from very accurate to inaccurate over time
		VectorLerp(VECTOR_CONE_1DEGREES, VECTOR_CONE_6DEGREES, ramp, cone);

		return cone;
	}

	virtual int	GetMinBurst()
	{
		return 1;
	}

	virtual int	GetMaxBurst()
	{
		return 3;
	}

	virtual float GetFireRate()
	{
		return 0.25f;
	}

private:
	CNetworkVar(float, m_flSoonestPrimaryAttack);
	CNetworkVar(float, m_flLastAttackTime);
	CNetworkVar(float, m_flAccuracyPenalty);
	CNetworkVar(int, m_nNumShotsFired);

private:
	CWeaponHL2Pistol(const CWeaponHL2Pistol &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponHL2Pistol, DT_WeaponHL2Pistol)

BEGIN_NETWORK_TABLE(CWeaponHL2Pistol, DT_WeaponHL2Pistol)

#ifdef CLIENT_DLL

RecvPropTime(RECVINFO(m_flSoonestPrimaryAttack)),
RecvPropTime(RECVINFO(m_flLastAttackTime)),
RecvPropFloat(RECVINFO(m_flAccuracyPenalty)),
RecvPropInt(RECVINFO(m_nNumShotsFired)),

#else

SendPropTime(SENDINFO(m_flSoonestPrimaryAttack)),
SendPropTime(SENDINFO(m_flLastAttackTime)),
SendPropFloat(SENDINFO(m_flAccuracyPenalty)),
SendPropInt(SENDINFO(m_nNumShotsFired)),

#endif

END_NETWORK_TABLE()

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA(CWeaponHL2Pistol)
DEFINE_PRED_FIELD(m_flSoonestPrimaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flLastAttackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flAccuracyPenalty, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_nNumShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()

#endif

LINK_ENTITY_TO_CLASS(weapon_hl2pistol, CWeaponHL2Pistol);
PRECACHE_WEAPON_REGISTER(weapon_hl2pistol);

#ifndef CLIENT_DLL

acttable_t CWeaponHL2Pistol::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PISTOL, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PISTOL, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PISTOL, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_PISTOL, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PISTOL, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_PISTOL, false },

	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	false },
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_WALK,						ACT_WALK_PISTOL,				false },
	{ ACT_RUN,						ACT_RUN_PISTOL,					false },
};

IMPLEMENT_ACTTABLE(CWeaponHL2Pistol);

#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponHL2Pistol::CWeaponHL2Pistol()
{
	m_flSoonestPrimaryAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0.0f;

	m_fMinRange1 = 24;
	m_fMaxRange1 = 1500;
	m_fMinRange2 = 24;
	m_fMaxRange2 = 200;

	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: Handle NPC fire events
//-----------------------------------------------------------------------------
#ifndef CLIENT_DLL
void CWeaponHL2Pistol::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_PISTOL_FIRE:
	{
		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);

		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

		WeaponSound(SINGLE);
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
		pOperator->DoMuzzleFlash();
		m_iClip1 = m_iClip1 - 1;

		break;
	}
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponHL2Pistol::DryFire()
{
	WeaponSound(EMPTY);
	SendWeaponAnim(IsIronsightsEnabled() ? ACT_VM_DRYFIRE_IRONSIGHTS : ACT_VM_DRYFIRE);

	m_flSoonestPrimaryAttack = gpGlobals->curtime + PISTOL_FASTEST_DRY_REFIRE_TIME;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponHL2Pistol::PrimaryAttack()
{
	


	if (gpGlobals->curtime - m_flLastAttackTime > 0.5f)
		m_nNumShotsFired = 0;
	else
		++m_nNumShotsFired;

	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + PISTOL_FASTEST_REFIRE_TIME;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	CHDTF_Player* pHDTFPlayer = ToHDTFPlayer(pOwner);
	if (pOwner != NULL)
		// Each time the player fires the pistol, reset the view punch. This prevents
		// the aim from 'drifting off' when the player fires very quickly. This may
		// not be the ideal way to achieve this, but it's cheap and it works, which is
		// great for a feature we're evaluating. (sjb)
			
		if (!pHDTFPlayer->IsSliding() && slide_delay < gpGlobals->curtime)
		{
			pOwner->ViewPunchReset();
		}

	BaseClass::PrimaryAttack();

	// Add an accuracy penalty which can move past our maximum penalty time if we're really spastic
	m_flAccuracyPenalty += PISTOL_ACCURACY_SHOT_PENALTY_TIME;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHL2Pistol::UpdatePenaltyTime()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	// Check our penalty time decay
	if ((pOwner->m_nButtons & IN_ATTACK) == 0 && m_flSoonestPrimaryAttack < gpGlobals->curtime)
	{
		m_flAccuracyPenalty -= gpGlobals->frametime;
		m_flAccuracyPenalty = clamp(m_flAccuracyPenalty, 0.0f, PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHL2Pistol::ItemPreFrame()
{
	UpdatePenaltyTime();

	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	CHDTF_Player* pHDTFPlayer = ToHDTFPlayer(pOwner);
	
	if (pHDTFPlayer && pOwner)
	{
		if (pHDTFPlayer->IsSliding())
		{
			slide_delay = gpGlobals->curtime + 2;
		}
	}

	BaseClass::ItemPreFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHL2Pistol::ItemBusyFrame()
{
	UpdatePenaltyTime();

	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as button is pressed
//-----------------------------------------------------------------------------
void CWeaponHL2Pistol::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	if (m_bInReload)
		return;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	//Allow a refire as fast as the player can click
	if ((pOwner->m_nButtons & IN_ATTACK) == 0 && m_flSoonestPrimaryAttack < gpGlobals->curtime)
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	else if ((pOwner->m_nButtons & IN_ATTACK) != 0 && m_flNextPrimaryAttack < gpGlobals->curtime && m_iClip1 <= 0)
		DryFire();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponHL2Pistol::Reload()
{
	bool fRet = BaseClass::Reload();
	if (fRet)
	{
		WeaponSound(RELOAD);
		m_flAccuracyPenalty = 0.0f;
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHL2Pistol::AddViewKick()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	QAngle viewPunch;
	viewPunch.x = SharedRandomFloat("pistolpax", 0.25f, 0.5f);
	viewPunch.y = SharedRandomFloat("pistolpay", -.6f, .6f);
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch(viewPunch);
}

void CWeaponHL2Pistol::EnableIronsights()
{
	BaseClass::EnableIronsights();
	m_flLastAttackTime = m_flNextPrimaryAttack;
	m_flSoonestPrimaryAttack = m_flNextPrimaryAttack;
}

void CWeaponHL2Pistol::DisableIronsights(bool ignoreAnimation)
{
	BaseClass::DisableIronsights(ignoreAnimation);
	m_flLastAttackTime = m_flNextPrimaryAttack;
	m_flSoonestPrimaryAttack = m_flNextPrimaryAttack;
}

#ifndef CLIENT_DLL

const WeaponProficiencyInfo_t *CWeaponHL2Pistol::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0f, 0.75f },
		{ 5.0f, 0.75f },
		{ 3.0f, 0.85f },
		{ 5.0f / 3.0f, 0.75f },
		{ 1.0f, 1.0f },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}

#endif
