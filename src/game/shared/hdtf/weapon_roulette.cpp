#include "cbase.h"
#include "weapon_basehdtfcombat.h"
#include "npcevent.h"

#ifdef CLIENT_DLL

#define CWeaponRoulette C_WeaponRoulette

#endif

class CWeaponRoulette : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponRoulette, CBaseHDTFCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE();

#endif

	CWeaponRoulette();

	void PrimaryAttack();
	bool Reload();
	void FinishReload();

	void ItemPostFrame();
	void ItemBusyFrame();

#ifndef CLIENT_DLL
	void Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);
	void FillClip();
#endif

private:
	CWeaponRoulette(const CWeaponRoulette&);

	CNetworkVar(float, m_flReloadSuspendPause);

	CNetworkVar(int, m_iRandomChanceMax);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponRoulette, DT_WeaponRoulette)

BEGIN_NETWORK_TABLE(CWeaponRoulette, DT_WeaponRoulette)
#ifdef CLIENT_DLL
RecvPropTime(RECVINFO(m_flReloadSuspendPause))
#else
SendPropTime(SENDINFO(m_flReloadSuspendPause))
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponRoulette)

END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_roulette, CWeaponRoulette);
PRECACHE_WEAPON_REGISTER(weapon_roulette);

#ifndef CLIENT_DLL

acttable_t CWeaponRoulette::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PISTOL, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PISTOL, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PISTOL, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_PISTOL, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PISTOL, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_PISTOL, false },
};

IMPLEMENT_ACTTABLE(CWeaponRoulette);

#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponRoulette::CWeaponRoulette()
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = false;

	m_flReloadSuspendPause = -1.f;
	m_iRandomChanceMax = 6;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponRoulette::PrimaryAttack()
{
	// Only the player fires this way so we can cast
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	

	WeaponSound(SINGLE);
	pPlayer->DoMuzzleFlash();

	m_fShotLast = gpGlobals->curtime;
	DoSmokeBarrel();

	SendWeaponAnim(GetPrimaryAttackActivity());
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.75;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.75;

	--m_iClip1;

	//Vector vecSrc = pPlayer->Weapon_ShootPosition();
	//Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	//
	//FireBulletsInfo_t info(1, vecSrc, vecAiming, vec3_origin, MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	//info.m_pAttacker = pPlayer;
	//
	//// Fire the bullets, and force the first shot to be perfectly accuracy
	//pPlayer->FireBullets(info);

	//Disorient the player
	QAngle angles = pPlayer->GetLocalAngles();
	angles.x += random->RandomInt(-1, 1);
	angles.y += random->RandomInt(-1, 1);
	angles.z = 0;

#ifndef CLIENT_DLL

	pPlayer->SnapEyeAngles(angles);

#endif

	pPlayer->ViewPunch(QAngle(-8, random->RandomFloat(-2, 2), 0));

	if (m_iClip1 == 0 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#ifdef GAME_DLL
	if (RandomInt(1, m_iRandomChanceMax) == 1)
	{

		CTakeDamageInfo info(pPlayer, pPlayer, 100, DMG_BULLET);
		
		pPlayer->TakeDamage(info);

	}
	else if (m_iRandomChanceMax != 1)
	{
		--m_iRandomChanceMax;
	}


	QAngle gunAngles;
	VectorAngles(pPlayer->BodyDirection2D(), gunAngles);

	Vector vecForward;
	AngleVectors(gunAngles, &vecForward, NULL, NULL);

	float flDiameter = sqrt(CollisionProp()->OBBSize().x * CollisionProp()->OBBSize().x +
		CollisionProp()->OBBSize().y * CollisionProp()->OBBSize().y);
	pPlayer->DropWeaponForWeaponStrip(this, vecForward, gunAngles, flDiameter);
	pPlayer->SwitchToNextBestWeapon(this);
#endif // GAME_DLL
}

bool CWeaponRoulette::Reload()
{
	CBaseCombatCharacter* pOwner = GetOwner();
	if (!pOwner)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	if (IsIronsightsEnabled())
	{
		DisableIronsights();
		m_bDelayedReload = true;
		return false;
	}

	if (pOwner->IsPlayer())
	{
		((CBasePlayer*)pOwner)->SetAnimation(PLAYER_RELOAD);
	}

	SendWeaponAnim(ACT_VM_RELOAD);

	float flSequenceEndTime = gpGlobals->curtime + SequenceDuration();
	pOwner->SetNextAttack(flSequenceEndTime);
	m_flNextPrimaryAttack = flSequenceEndTime;

	m_bInReload = true;

	return true;
}

void CWeaponRoulette::FinishReload()
{
	// overrided to prevent default behavior
}

#ifndef CLIENT_DLL
void CWeaponRoulette::FillClip()
{
	if (!m_bInReload)
		return;

	CBaseCombatCharacter* pOwner = GetOwner();
	if (pOwner == NULL)
		return;

	WeaponSound(RELOAD);

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0)
	{
		m_iClip1++;
		pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);

		SetBodygroup(1 + m_iClip1, 0);

		// we run out of ammo - suspend reload
		if (m_iClip1 != GetMaxClip1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) == 0)
		{
			// give it some time to load the round
			m_flReloadSuspendPause = gpGlobals->curtime + 0.1f;
			m_bInReload = false;
		}
	}
}

void CWeaponRoulette::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	CBaseCombatCharacter* pOwner = GetOwner();
	if (pOwner == NULL)
		return;

	switch (pEvent->event)
	{
		// this event called twice:
		// first time it resets the clip
		// second time it hide all of the bullets
	case EVENT_WEAPON_RELOAD:
		if (m_iClip1 != 0)
		{
			const int actuallyStored = pOwner->GiveAmmo(m_iClip1, m_iPrimaryAmmoType, true);
			const int leftUnused = m_iClip1 - actuallyStored;
			m_iClip1 = 0;

			if (leftUnused > 0)
			{
				// HACK: this allows us to store excess ammo and load it in later
				pOwner->SetAmmoCount(pOwner->GetAmmoCount(m_iPrimaryAmmoType) + leftUnused, m_iPrimaryAmmoType);
			}
		}
		else
			for (int i = 0; i < 6; i++)
				SetBodygroup(2 + i, 0);

		break;

		// fill clip with a single bullet
	case EVENT_WEAPON_RELOAD_FILL_CLIP:
		FillClip();
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif

void CWeaponRoulette::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	if (m_bInReload && m_flNextPrimaryAttack < gpGlobals->curtime)
		m_bInReload = false;
}

void CWeaponRoulette::ItemBusyFrame()
{
	if (m_flReloadSuspendPause > 0.f && m_flReloadSuspendPause < gpGlobals->curtime)
	{
		// when we set another animation cylinder will rotate
		// to another angle so we need to invert all body groups
		// before playing the animation

		for (int i = 2; i < 7; i++)
			SetBodygroup(i, (GetBodygroup(i) == 0) ? 1 : 0);

		SendWeaponAnim(ACT_VM_RELOAD_EMPTY);

		float flSequenceEndTime = gpGlobals->curtime + SequenceDuration();
		m_flNextPrimaryAttack = flSequenceEndTime;

		CBaseCombatCharacter* pOwner = GetOwner();
		if (pOwner)
			pOwner->SetNextAttack(flSequenceEndTime);

		m_flReloadSuspendPause = -1.f;
	}

	BaseClass::ItemBusyFrame();
}
