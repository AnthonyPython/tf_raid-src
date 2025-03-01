#include "cbase.h"
#include "weapon_basemelee.h"

#ifndef CLIENT_DLL

#include "hdtf_player.h"
#include "ilagcompensationmanager.h"
#include "npcevent.h"

#else

#define CWeaponSpooky C_WeaponSpooky

#endif

ConVar sk_plr_dmg_spooky_claws_primary("sk_plr_dmg_spooky_claws_primary", "15", FCVAR_REPLICATED);
ConVar sk_plr_dmg_spooky_claws_secondary("sk_plr_dmg_spooky_claws_secondary", "30", FCVAR_REPLICATED);
ConVar sk_npc_dmg_spooky_claws_primary("sk_npc_dmg_spooky_claws_primary", "15", FCVAR_REPLICATED);
ConVar sk_npc_dmg_spooky_claws_secondary("sk_npc_dmg_spooky_claws_secondary", "30", FCVAR_REPLICATED);

class CWeaponSpooky : public CBaseMeleeWeapon
{
public:
	DECLARE_CLASS(CWeaponSpooky, CBaseMeleeWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();

#endif

	CWeaponSpooky();

	Activity GetPrimaryAttackActivity()
	{
		return ACT_VM_HITCENTER;
	}

	Activity GetSecondaryAttackActivity()
	{
		return ACT_VM_SECONDARYATTACK;
	}

	Activity GetPrimaryMissActivity()
	{
		return ACT_VM_MISSCENTER;
	}

	Activity GetSecondaryMissActivity()
	{
		return ACT_VM_SECONDARYATTACK;
	}

	float GetRange()
	{
		return 75.0f;
	}

	float GetFireRate()
	{
		return 0.5f;
	}

	void ImpactSound(bool bIsWorld)
	{ }

	float GetDamageForActivity(Activity hitActivity);

	int GetDamageType(bool bIsSecondary)
	{
		return DMG_CLUB;
	}

	void AddViewKick();

	void PrimaryAttack();
	void SecondaryAttack();

#ifndef CLIENT_DLL
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
#endif

private:
	int m_nDamageType;
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponSpooky, DT_WeaponSpooky)

BEGIN_NETWORK_TABLE(CWeaponSpooky, DT_WeaponSpooky)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponSpooky)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_spooky_claws, CWeaponSpooky);
PRECACHE_WEAPON_REGISTER(weapon_spooky_claws);

#ifndef CLIENT_DLL

BEGIN_DATADESC(CWeaponSpooky)
END_DATADESC()

acttable_t CWeaponSpooky::m_acttable[] =
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

IMPLEMENT_ACTTABLE(CWeaponSpooky);

#endif

CWeaponSpooky::CWeaponSpooky()
{
	m_nDamageType = DMG_SLASH;
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponSpooky::GetDamageForActivity(Activity hitActivity)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (pOwner != NULL && pOwner->IsPlayer())
		return (hitActivity == GetPrimaryAttackActivity() ? sk_plr_dmg_spooky_claws_primary : sk_plr_dmg_spooky_claws_secondary).GetFloat();

	return (hitActivity == GetPrimaryAttackActivity() ? sk_npc_dmg_spooky_claws_primary : sk_npc_dmg_spooky_claws_secondary).GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponSpooky::AddViewKick()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	QAngle punchAng;
	punchAng.x = SharedRandomFloat("machetepax", 2.0f, 4.0f);
	punchAng.y = SharedRandomFloat("machetepay", -2.0f, -1.0f);
	punchAng.z = 0.0f;

	pPlayer->ViewPunch(punchAng);
}

void CWeaponSpooky::PrimaryAttack()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	pOwner->SetAnimation(PLAYER_ATTACK1);

	Swing(false, true);

	m_flNextSecondaryAttack = m_flNextPrimaryAttack;

	m_flNextPrimaryAttack -= gpGlobals->curtime;
	m_flNextPrimaryAttack = min(m_flNextPrimaryAttack, 0.33f);
	m_flNextPrimaryAttack = gpGlobals->curtime + m_flNextPrimaryAttack;
}

void CWeaponSpooky::SecondaryAttack()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	pOwner->SetAnimation(PLAYER_ATTACK1);

	SendWeaponAnim(GetSecondaryAttackActivity());
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Handle animation events
//-----------------------------------------------------------------------------
void CWeaponSpooky::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_MELEE_SWISH:
		Swing(true, false);
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif
