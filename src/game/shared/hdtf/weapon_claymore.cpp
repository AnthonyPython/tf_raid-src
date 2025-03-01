#include "cbase.h"
#include "weapon_basehdtfcombat.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "hdtf_player_shared.h"

#ifndef CLIENT_DLL
#include "grenade_claymore.h"
#endif

#ifdef CLIENT_DLL
#define CWeaponClaymore C_WeaponClaymore
#endif

class CWeaponClaymore : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponClaymore, CBaseHDTFCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();
#endif

	CWeaponClaymore();

	virtual void Precache();

	virtual bool CanHolster() const { return !m_bPlacing; }
	bool ShouldDrawCrosshair() { return false; }
	bool HasIronsights() { return false; }

	void ItemPostFrame();

	bool Deploy();
	bool Holster(CBaseCombatWeapon *pSwitchingTo);

	void PrimaryAttack();
	bool Reload();

	void SpawnClaymore();

private:
	CWeaponClaymore(const CWeaponClaymore &);

	CNetworkVar(bool, m_bRedraw);
	CNetworkVar(bool, m_bPlacing);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponClaymore, DT_WeaponClaymore)

BEGIN_NETWORK_TABLE(CWeaponClaymore, DT_WeaponClaymore)

#ifdef CLIENT_DLL
	RecvPropBool(RECVINFO(m_bRedraw)),
	RecvPropBool(RECVINFO(m_bPlacing)),
#else
	SendPropBool(SENDINFO(m_bRedraw)),
	SendPropBool(SENDINFO(m_bPlacing)),
#endif

END_NETWORK_TABLE()

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA(CWeaponClaymore)
	DEFINE_PRED_FIELD(m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_bPlacing, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()

#else

BEGIN_DATADESC(CWeaponClaymore)
DEFINE_FIELD(m_bRedraw, FIELD_BOOLEAN),
DEFINE_FIELD(m_bPlacing, FIELD_BOOLEAN),
END_DATADESC()

#endif

LINK_ENTITY_TO_CLASS(weapon_claymore, CWeaponClaymore);
PRECACHE_WEAPON_REGISTER(weapon_claymore);

#ifndef CLIENT_DLL

acttable_t CWeaponClaymore::m_acttable[] =
{
	{ ACT_VM_IDLE_LOWERED, ACT_VM_IDLE_LOWERED, false },
};

IMPLEMENT_ACTTABLE(CWeaponClaymore);

#endif

CWeaponClaymore::CWeaponClaymore()
{
	m_bRedraw = false;
	m_bPlacing = false;
}

void CWeaponClaymore::Precache()
{
	BaseClass::Precache();
	PrecacheModel("models/weapons/w_claymore.mdl");
	PrecacheParticleSystem("hdtf_claymore");
}

bool CWeaponClaymore::Deploy()
{
	m_bPlacing = false;

	if (HasPrimaryAmmo())
		return BaseClass::Deploy();
	else
		m_flTimeWeaponIdle = FLT_MAX;

	return true;
}

bool CWeaponClaymore::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	m_bPlacing = false;

	return BaseClass::Holster(pSwitchingTo);
}

bool CWeaponClaymore::Reload()
{
	if (!HasPrimaryAmmo())
		return false;

	if ((m_bRedraw) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		SendWeaponAnim(ACT_VM_DRAW);

		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

		m_bRedraw = false;
	}

	return true;
}

void CWeaponClaymore::PrimaryAttack()
{
	if (m_bRedraw)
		return;

	CHDTF_Player *pOwner = dynamic_cast<CHDTF_Player *>(GetOwner());

	if (!pOwner)
		return;

	if (pOwner->GetGroundEntity() == NULL)
		return;

	if (!HasPrimaryAmmo())
		return;

#ifndef CLIENT_DLL
	if (!pOwner->GetToggledDuckState() && !pOwner->IsProne())
		pOwner->ToggleDuck();
#endif

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

	m_bPlacing = true;
}

void CWeaponClaymore::SpawnClaymore()
{
#ifndef CLIENT_DLL
	CHDTF_Player *pOwner = dynamic_cast<CHDTF_Player *>(GetOwner());

	if (pOwner)
	{
		if (pOwner->GetToggledDuckState() && !pOwner->IsProne())
			pOwner->ToggleDuck();

		if (pOwner->GetGroundEntity() == NULL)
			return;

		Vector spawnPos = pOwner->GetAbsOrigin();
		QAngle spawnAngles = pOwner->EyeAngles();
		spawnAngles.x = 0;

		spawnPos += pOwner->EyeDirection2D() * (pOwner->IsProne() ? 18.f : 6.f);
		spawnPos.z -= 6.5f;

		trace_t tr;

		UTIL_TraceLine(pOwner->WorldSpaceCenter(), 
			spawnPos, 
			MASK_SOLID, 
			pOwner, 
			COLLISION_GROUP_NONE, 
			&tr);

		ClaymoreGrenadeCreate(tr.endpos, spawnAngles, pOwner, tr.m_pEnt);

		pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);

		if (!HasPrimaryAmmo())
		{
			pOwner->SwitchToNextBestWeapon(this);
		}
	}
#endif

	m_bPlacing = false;
	m_bRedraw = true;

	m_flTimeWeaponIdle = FLT_MAX;
}

void CWeaponClaymore::ItemPostFrame()
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());

	if (pOwner)
	{
		if ((pOwner->m_nButtons & IN_ATTACK) != 0
			&& m_flNextPrimaryAttack < gpGlobals->curtime)
		{
			PrimaryAttack();
		}
	}

	BaseClass::ItemPostFrame();

	if (m_bPlacing)
	{
		// we lost the ground during placement: abort!
		if (pOwner->GetGroundEntity() == NULL)
		{
			m_bPlacing = false;

#ifndef CLIENT_DLL
			if (pOwner->GetToggledDuckState() && !pOwner->IsProne())
				pOwner->ToggleDuck();
#endif

			SendWeaponAnim(ACT_VM_IDLE);
			m_flNextPrimaryAttack = m_flTimeWeaponIdle = gpGlobals->curtime;

			return;
		}

		if (IsViewModelSequenceFinished())
		{
			SpawnClaymore();
		}
	}

	if (m_bRedraw)
	{
		if (IsViewModelSequenceFinished())
		{
			Reload();
		}
	}
}
