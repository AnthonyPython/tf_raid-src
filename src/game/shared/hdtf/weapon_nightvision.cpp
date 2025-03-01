#include "cbase.h"
#include "weapon_basehdtfcombat.h"
#include "in_buttons.h"
#include "hdtf_player_shared.h"
#include "npcevent.h"

#ifdef CLIENT_DLL

#define CWeaponNightVision C_WeaponNightVision

#endif

class CWeaponNightVision : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponNightVision, CBaseHDTFCombatWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	void ItemPostFrame( );

	virtual bool CanHolster() const;

	bool Deploy();
	bool Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

	void PrimaryAttack( );
	void SecondaryAttack( );
	void SetActive( bool state );

	void WeaponIdle();

	virtual bool AllowsAutoSwitchTo() { return false; }
	virtual bool AllowsAutoSwitchFrom() { return true; }

#ifndef CLIENT_DLL
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponNightVision, DT_WeaponNightVision )

BEGIN_NETWORK_TABLE( CWeaponNightVision, DT_WeaponNightVision )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CWeaponNightVision )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_nightvision, CWeaponNightVision );
PRECACHE_WEAPON_REGISTER( weapon_nightvision );

#ifndef CLIENT_DLL

acttable_t CWeaponNightVision::m_acttable[] =
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

IMPLEMENT_ACTTABLE( CWeaponNightVision );

#endif

void CWeaponNightVision::ItemPostFrame( )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	if ((pOwner->m_nButtons & IN_ATTACK) != 0 && m_flNextPrimaryAttack <= gpGlobals->curtime)
		PrimaryAttack( );
	else if ((pOwner->m_nButtons & IN_ATTACK2) != 0 && m_flNextPrimaryAttack <= gpGlobals->curtime)
		SecondaryAttack( );
	else
		WeaponIdle( );
}

bool CWeaponNightVision::CanHolster() const
{
	return m_flNextPrimaryAttack <= gpGlobals->curtime;
}

void CWeaponNightVision::SetActive( bool state )
{
	CHDTF_Player *pPlayer = ToHDTFPlayer( GetOwner( ) );
	if( pPlayer == NULL || state == pPlayer->IsNightVisionActive( ) || !pPlayer->CanSwitchNightVision( ) )
		return;

	WeaponSound( SINGLE );
	SendWeaponAnim( state ? ACT_VM_PRIMARYATTACK : ACT_VM_SECONDARYATTACK );
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

	if (state)
		m_flTimeWeaponIdle = FLT_MAX;
	else
		m_flTimeWeaponIdle = m_flNextPrimaryAttack;

	pPlayer->SetAnimation( PLAYER_ATTACK1 );
}

bool CWeaponNightVision::Deploy()
{
	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());
	const bool shouldDeploy = pPlayer && pPlayer->IsNightVisionActive();
	const bool deployed = DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(),
		(shouldDeploy ? ACT_IDLE : ACT_VM_DRAW),
		(char*)GetAnimPrefix());
	
	if (shouldDeploy)
		m_flTimeWeaponIdle = m_flNextPrimaryAttack = gpGlobals->curtime;
	else
		m_flTimeWeaponIdle = m_flNextPrimaryAttack;

	return deployed;
}

bool CWeaponNightVision::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());
	if (pPlayer && pPlayer->IsNightVisionActive())
	{
		SetWeaponVisible(false);
		return true;
	}
	else
	{
		return BaseClass::Holster(pSwitchingTo);
	}
}

void CWeaponNightVision::PrimaryAttack( )
{
	SetActive( true );
}

void CWeaponNightVision::SecondaryAttack( )
{
	SetActive( false );
}

void CWeaponNightVision::WeaponIdle()
{
	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());
	if (pPlayer && pPlayer->IsNightVisionActive())
	{
		if (HasWeaponIdleTimeElapsed())
		{
			SendWeaponAnim(ACT_IDLE);
			m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();
		}
		
		return;
	}

	BaseClass::WeaponIdle();
}

#ifndef CLIENT_DLL
void CWeaponNightVision::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	switch (pEvent->event)
	{
	case EVENT_WEAPON_MELEE_HIT:
		pPlayer->ApplyNightVision();
		break;

	case EVENT_WEAPON_MELEE_SWISH:
		pPlayer->RemoveNightVision();
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif
