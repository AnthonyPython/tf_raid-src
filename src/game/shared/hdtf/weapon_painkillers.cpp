#include "cbase.h"
#include "weapon_basehdtfcombat.h"
#include "in_buttons.h"
#include "hdtf_player_shared.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"

#ifdef CLIENT_DLL

#define CWeaponPainkillers C_WeaponPainkillers

#else

#include "props.h"
ConVar	sk_painkillers("sk_painkillers", "100");

#endif

#define PILLS_MODEL_SIZE 6

class CWeaponPainkillers : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponPainkillers, CBaseHDTFCombatWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_DATADESC();
	DECLARE_ACTTABLE( );

	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	void TakeHealth();
	void DropPills();

#endif

	CWeaponPainkillers();

	bool Deploy();

	void ItemPostFrame( );

	virtual bool CanHolster() const;

	void PrimaryAttack( );
	void SecondaryAttack( );

	bool Reload();

	void CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc);

private:
	bool m_bRedraw;
	bool m_bIsTakingPills;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPainkillers, DT_WeaponPainkillers )

BEGIN_NETWORK_TABLE( CWeaponPainkillers, DT_WeaponPainkillers )
END_NETWORK_TABLE( )

#ifndef CLIENT_DLL
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CWeaponPainkillers)
DEFINE_FIELD(m_bRedraw, FIELD_BOOLEAN),
DEFINE_FIELD(m_bIsTakingPills, FIELD_BOOLEAN),
END_DATADESC()
#endif

BEGIN_PREDICTION_DATA( CWeaponPainkillers )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_painkillers, CWeaponPainkillers );
PRECACHE_WEAPON_REGISTER( weapon_painkillers );

#ifndef CLIENT_DLL

acttable_t CWeaponPainkillers::m_acttable[] =
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

IMPLEMENT_ACTTABLE( CWeaponPainkillers );

#endif

CWeaponPainkillers::CWeaponPainkillers()
{
	PrecacheScriptSound("Weapon_PainKillers.Use");
}

bool CWeaponPainkillers::Deploy()
{
	m_bRedraw = false;
	m_bIsTakingPills = false;
	return BaseClass::Deploy();
}

void CWeaponPainkillers::ItemPostFrame( )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	if (m_bRedraw && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		m_bIsTakingPills = false;

		if (!HasAnyAmmo())
		{
			pOwner->SwitchToNextBestWeapon(this);
			return;
		}

		SendWeaponAnim(ACT_VM_DRAW);
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		m_bRedraw = false;
	}

	if( ( pOwner->m_nButtons & IN_ATTACK ) != 0 && m_flNextPrimaryAttack <= gpGlobals->curtime )
		PrimaryAttack( );
	else
		WeaponIdle( );
}

bool CWeaponPainkillers::CanHolster() const
{
	return !m_bIsTakingPills;
}

void CWeaponPainkillers::PrimaryAttack( )
{
	CHDTF_Player *pPlayer = ToHDTFPlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	if (!HasAnyAmmo())
		return;

	if (m_bRedraw)
		return;
	// This prevents the player from taking the painkillers when already at max health
	if (pPlayer->GetHealth() == pPlayer->GetMaxHealth())
		return;

	pPlayer->SetAnimation(PLAYER_ATTACK1);
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	m_bRedraw = true;
	m_bIsTakingPills = true;

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponPainkillers::SecondaryAttack( )
{ }

bool CWeaponPainkillers::Reload()
{
	if (!HasPrimaryAmmo())
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (pPlayer != NULL)
			pPlayer->SwitchToNextBestWeapon(this);

		return false;
	}

	return BaseClass::Reload();
}

#ifndef CLIENT_DLL
void CWeaponPainkillers::TakeHealth()
{
	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
	pPlayer->TakeHealth(sk_painkillers.GetFloat(), DMG_GENERIC);

	CPASAttenuationFilter filter(pPlayer, "WeaponPainKillers.Use");
	EmitSound(filter, pPlayer->entindex(), "WeaponPainKillers.Use");
}

void CWeaponPainkillers::CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc)
{
	trace_t tr;
	UTIL_TraceHull(vecEye, vecSrc,
		-Vector(PILLS_MODEL_SIZE, PILLS_MODEL_SIZE, PILLS_MODEL_SIZE),
		Vector(PILLS_MODEL_SIZE, PILLS_MODEL_SIZE, PILLS_MODEL_SIZE),
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr);
	if (tr.DidHit())
		vecSrc = tr.endpos;
}

void CWeaponPainkillers::DropPills()
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner == NULL)
		return;

	QAngle ang = pOwner->EyeAngles();
	Vector pos = pOwner->Weapon_ShootPosition(),
		forward,
		right,
		up,
		vel;

	AngleVectors(ang, &forward, &right, &up);
	vel = forward * 15.f + right * 120.f;

	Vector spawnPos = pos + right * 4.0f - up * 3.5f;

	CPhysicsProp *pProp = dynamic_cast<CPhysicsProp *>(CreateEntityByName("prop_physics"));
	if (pProp)
	{
		UTIL_SetOrigin(pProp, spawnPos);
		pProp->SetAbsAngles(ang);

		pProp->KeyValue("model", GetWorldModel());

		pProp->Precache();
		DispatchSpawn(pProp);
		pProp->Activate();
		pProp->SetCollisionGroup(COLLISION_GROUP_WEAPON);
		pProp->VPhysicsGetObject()->ApplyForceCenter(vel);

		pProp->SetBodygroup(pProp->FindBodygroupByName("cap"), 1);
	}
}

void CWeaponPainkillers::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_RELOAD:
		TakeHealth();
		break;

	case EVENT_WEAPON_THROW:
		DropPills();
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif
