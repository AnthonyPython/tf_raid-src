#include "cbase.h"
#include "weapon_basehdtfcombat.h"
#include "in_buttons.h"
#include "hdtf_player_shared.h"
#include "npcevent.h"

#ifdef CLIENT_DLL

#define CWeaponBinoculars C_WeaponBinoculars

#endif

class CWeaponBinoculars : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponBinoculars, CBaseHDTFCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();

#endif

	void ItemPostFrame();

	virtual bool CanHolster() const;
	virtual bool CanPrimaryAttack() { return false; }
	virtual bool HasIronsights() const { return false; }

	void ZoomIn();
	void ZoomOut();
	void DoFadeOut();

private:
	CNetworkVar(bool, m_bMovingToActive);
	CNetworkVar(bool, m_bIsActive);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponBinoculars, DT_WeaponBinoculars)

BEGIN_NETWORK_TABLE(CWeaponBinoculars, DT_WeaponBinoculars)
#ifdef CLIENT_DLL
	RecvPropBool(RECVINFO(m_bMovingToActive)),
	RecvPropBool(RECVINFO(m_bIsActive))
#else
	SendPropBool(SENDINFO(m_bMovingToActive)),
	SendPropBool(SENDINFO(m_bIsActive))
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponBinoculars)
	DEFINE_PRED_FIELD(m_bMovingToActive, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_bIsActive, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE)
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_binoculars, CWeaponBinoculars);
PRECACHE_WEAPON_REGISTER(weapon_binoculars);

#ifndef CLIENT_DLL

BEGIN_DATADESC(CWeaponBinoculars)
	DEFINE_FIELD(m_bMovingToActive, FIELD_BOOLEAN),
	DEFINE_FIELD(m_bIsActive, FIELD_BOOLEAN),
END_DATADESC()

acttable_t CWeaponBinoculars::m_acttable[] =
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

IMPLEMENT_ACTTABLE(CWeaponBinoculars);

#endif

void CWeaponBinoculars::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner == NULL)
		return;

	bool primaryButton = (pOwner->m_nButtons & IN_ATTACK) != 0;

	if (primaryButton && !m_bMovingToActive && !m_bIsActive && !m_bLowered)
		ZoomIn();
	else if (!primaryButton && (m_bMovingToActive || m_bIsActive))
		ZoomOut();
	else
		WeaponIdle();

	if (m_bMovingToActive && IsViewModelSequenceFinished())
	{
		m_bMovingToActive = false;
		m_bIsActive = true;
		pOwner->SetFOV(this, 15, 0.f);
		pOwner->SetBinocularsActive(true);
		SetWeaponVisible(false);

		DoFadeOut();
	}
}

bool CWeaponBinoculars::CanHolster() const
{
	return !m_bIsActive;
}

void CWeaponBinoculars::ZoomIn()
{
	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());

	if (m_bMovingToActive || m_bIsActive || (pPlayer && pPlayer->IsNightVisionActive()))
		return;

	m_bMovingToActive = true;
	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
}

void CWeaponBinoculars::ZoomOut()
{
	m_bMovingToActive = false;
	m_bIsActive = false;
	SetWeaponVisible(true);

	SendWeaponAnim(ACT_VM_SECONDARYATTACK);

	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner == NULL)
		return;

	DoFadeOut();
	pOwner->SetFOV(this, 0, 0.f);
	pOwner->SetBinocularsActive(false);
}

void CWeaponBinoculars::DoFadeOut()
{
#ifndef CLIENT_DLL
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (!pOwner)
		return;

	color32 black = { 0, 0, 0, 255 };
	UTIL_ScreenFade(pOwner, black, 0.25f, 0.f, FFADE_IN);
#endif
}
