#include "cbase.h"
#include "weapon_basehdtfcombat.h"
#include "in_buttons.h"
#include "hdtf_player_shared.h"

#ifdef CLIENT_DLL

#define CWeaponGasMask C_WeaponGasMask

#endif

class CWeaponGasMask : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponGasMask, CBaseHDTFCombatWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	bool Deploy();
	bool Holster(CBaseCombatWeapon *pSwitchingTo);
	void ItemPostFrame( );
	void WeaponIdle();

	virtual bool CanHolster() const;

	void PrimaryAttack( );
	void SecondaryAttack( );
	void SetActive( bool state );
	void DoFadeOut();
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGasMask, DT_WeaponGasMask )

BEGIN_NETWORK_TABLE( CWeaponGasMask, DT_WeaponGasMask )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CWeaponGasMask )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_gasmask, CWeaponGasMask );
PRECACHE_WEAPON_REGISTER( weapon_gasmask );

#ifndef CLIENT_DLL

acttable_t CWeaponGasMask::m_acttable[] =
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

IMPLEMENT_ACTTABLE( CWeaponGasMask );

#endif

bool CWeaponGasMask::Deploy()
{
	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());
	const bool shouldDeploy = pPlayer && pPlayer->IsGasMaskActive();
	const bool deployed = DefaultDeploy((char *)GetViewModel(), (char *)GetWorldModel(),
		(shouldDeploy ? ACT_IDLE : ACT_VM_DRAW),
		(char *)GetAnimPrefix());

	if (shouldDeploy)
		m_flTimeWeaponIdle = m_flNextPrimaryAttack = gpGlobals->curtime;
	else
		m_flTimeWeaponIdle = m_flNextPrimaryAttack;

	return deployed;
}

bool CWeaponGasMask::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());
	if (pPlayer && pPlayer->IsGasMaskActive())
	{
		SetWeaponVisible(false);
		return true;
	}
	else
	{
		return BaseClass::Holster(pSwitchingTo);
	}
}

void CWeaponGasMask::ItemPostFrame( )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	if( ( pOwner->m_nButtons & IN_ATTACK ) != 0 && m_flNextPrimaryAttack <= gpGlobals->curtime )
		PrimaryAttack( );
	else if( ( pOwner->m_nButtons & IN_ATTACK2 ) != 0 && m_flNextSecondaryAttack <= gpGlobals->curtime )
		SecondaryAttack( );
	else
		WeaponIdle( );
}

void CWeaponGasMask::WeaponIdle()
{
	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());
	if (pPlayer && pPlayer->IsGasMaskActive())
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

bool CWeaponGasMask::CanHolster() const
{
	return m_flNextPrimaryAttack <= gpGlobals->curtime;
}

void CWeaponGasMask::SetActive( bool state )
{
	CHDTF_Player *pPlayer = ToHDTFPlayer( GetOwner( ) );
	if( pPlayer == NULL || state == pPlayer->IsGasMaskActive( ) || !pPlayer->CanSwitchGasMask( ) )
		return;

	WeaponSound( SINGLE );
	SendWeaponAnim( state ? ACT_VM_PRIMARYATTACK : ACT_VM_SECONDARYATTACK );
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	if( state )
		pPlayer->ApplyGasMask( );
	else
		pPlayer->RemoveGasMask( );
}

void CWeaponGasMask::PrimaryAttack( )
{
	SetActive( true );
}

void CWeaponGasMask::SecondaryAttack( )
{
	SetActive( false );
}

void CWeaponGasMask::DoFadeOut()
{
#ifndef CLIENT_DLL
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (!pOwner)
		return;

	color32 black = { 0, 0, 0, 255 };
	UTIL_ScreenFade(pOwner, black, 0.25f, 0.f, FFADE_IN);
#endif
}

