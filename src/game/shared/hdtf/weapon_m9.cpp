#include "cbase.h"
#include "in_buttons.h"
#include "weapon_basehdtfcombat.h"
#include "hdtf_player_shared.h"

#define	PISTOL_FASTEST_REFIRE_TIME 0.1f
#define	PISTOL_FASTEST_DRY_REFIRE_TIME 0.2f

#define	PISTOL_ACCURACY_SHOT_PENALTY_TIME 0.2f // Applied amount of time each shot adds to the time we must recover from
#define	PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME 1.5f // Maximum penalty to deal out

#ifdef CLIENT_DLL

#define CWeaponM9 C_WeaponM9

#endif

class CWeaponM9 : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponM9, CBaseHDTFCombatWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	CWeaponM9( );

	void ItemPostFrame( );
	void ItemPreFrame( );
	void ItemBusyFrame( );
	void PrimaryAttack( );
	void AddViewKick( );
	void DryFire( );

	void EnableIronsights();
	void DisableIronsights(bool ignoreAnimation = false);

	void UpdatePenaltyTime( );

	virtual bool Reload( );

	virtual Activity GetPrimaryAttackActivity()
	{
		if(m_iClip1 == 1)
			return IsIronsightsEnabled() ? ACT_VM_PRIMARYATTACK_IRONSIGHTS_SPECIAL : ACT_VM_PRIMARYATTACK_SPECIAL;

		return IsIronsightsEnabled() ? ACT_VM_PRIMARYATTACK_IRONSIGHTS : ACT_VM_PRIMARYATTACK;
	}

	virtual const Vector &GetBulletSpread( )
	{
		static Vector cone;

		float ramp = RemapValClamped( m_flAccuracyPenalty,
			0.0f,
			PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME,
			0.0f,
			1.0f );

		// We lerp from very accurate to inaccurate over time
		VectorLerp( VECTOR_CONE_1DEGREES, VECTOR_CONE_6DEGREES, ramp, cone );

		return cone;
	}

	virtual int	GetMinBurst( )
	{
		return 1;
	}

	virtual int	GetMaxBurst( )
	{
		return 3;
	}

	virtual float GetFireRate( )
	{
		return 0.25f;
	}

private:
	CNetworkVar( float, m_flSoonestPrimaryAttack );
	CNetworkVar( float, m_flLastAttackTime );
	CNetworkVar( float, m_flAccuracyPenalty );
	CNetworkVar( int, m_nNumShotsFired );
	CNetworkVar( bool, m_bLastShotFired );

private:
	CWeaponM9( const CWeaponM9 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM9, DT_WeaponM9 )

BEGIN_NETWORK_TABLE( CWeaponM9, DT_WeaponM9 )

#ifdef CLIENT_DLL

RecvPropTime( RECVINFO( m_flSoonestPrimaryAttack ) ),
RecvPropTime( RECVINFO( m_flLastAttackTime ) ),
RecvPropFloat( RECVINFO( m_flAccuracyPenalty ) ),
RecvPropInt( RECVINFO( m_nNumShotsFired ) ),

#else

SendPropTime( SENDINFO( m_flSoonestPrimaryAttack ) ),
SendPropTime( SENDINFO( m_flLastAttackTime ) ),
SendPropFloat( SENDINFO( m_flAccuracyPenalty ) ),
SendPropInt( SENDINFO( m_nNumShotsFired ) ),

#endif

END_NETWORK_TABLE( )

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponM9 )
DEFINE_PRED_FIELD( m_flSoonestPrimaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_flLastAttackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_flAccuracyPenalty, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_nNumShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA( )

#endif

LINK_ENTITY_TO_CLASS( weapon_m9, CWeaponM9 );
PRECACHE_WEAPON_REGISTER( weapon_m9 );

#ifndef CLIENT_DLL

acttable_t CWeaponM9::m_acttable[] =
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

IMPLEMENT_ACTTABLE( CWeaponM9 );

#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponM9::CWeaponM9( )
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
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponM9::DryFire( )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( IsIronsightsEnabled() ? ACT_VM_DRYFIRE_IRONSIGHTS : ACT_VM_DRYFIRE );

	m_flSoonestPrimaryAttack = gpGlobals->curtime + PISTOL_FASTEST_DRY_REFIRE_TIME;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponM9::PrimaryAttack( )
{
	if( gpGlobals->curtime - m_flLastAttackTime > 0.5f )
		m_nNumShotsFired = 0;
	else
		++m_nNumShotsFired;

	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + PISTOL_FASTEST_REFIRE_TIME;

	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner != NULL && !pOwner->IsSliding())
		// Each time the player fires the pistol, reset the view punch. This prevents
		// the aim from 'drifting off' when the player fires very quickly. This may
		// not be the ideal way to achieve this, but it's cheap and it works, which is
		// great for a feature we're evaluating. (sjb)
		// Player sliding feature rely on view punch, resetting it will cause view to
		// snap when player sliding, so we're not going to do that. -Wheatley
		pOwner->ViewPunchReset();

	BaseClass::PrimaryAttack( );

	// Add an accuracy penalty which can move past our maximum penalty time if we're really spastic
	m_flAccuracyPenalty += PISTOL_ACCURACY_SHOT_PENALTY_TIME;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM9::UpdatePenaltyTime( )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	// Check our penalty time decay
	if( ( pOwner->m_nButtons & IN_ATTACK ) == 0 && m_flSoonestPrimaryAttack < gpGlobals->curtime )
	{
		m_flAccuracyPenalty -= gpGlobals->frametime;
		m_flAccuracyPenalty = clamp( m_flAccuracyPenalty, 0.0f, PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM9::ItemPreFrame( )
{
	UpdatePenaltyTime( );

	BaseClass::ItemPreFrame( );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM9::ItemBusyFrame( )
{
	UpdatePenaltyTime( );

	BaseClass::ItemBusyFrame( );
}

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as button is pressed
//-----------------------------------------------------------------------------
void CWeaponM9::ItemPostFrame( )
{
	BaseClass::ItemPostFrame( );

	if( m_bInReload )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	//Allow a refire as fast as the player can click
	if( ( pOwner->m_nButtons & IN_ATTACK ) == 0 && m_flSoonestPrimaryAttack < gpGlobals->curtime )
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	else if( ( pOwner->m_nButtons & IN_ATTACK ) != 0 && m_flNextPrimaryAttack < gpGlobals->curtime && m_iClip1 <= 0 )
		DryFire( );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponM9::Reload( )
{
	bool fRet = BaseClass::Reload();
	if ( fRet )
	{
		m_flLastAttackTime = m_flNextPrimaryAttack;
		m_flSoonestPrimaryAttack = m_flNextPrimaryAttack;
		m_flAccuracyPenalty = 0.0f;
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponM9::AddViewKick( )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	QAngle viewPunch;
	viewPunch.x = SharedRandomFloat( "pistolpax", 0.25f, 0.5f );
	viewPunch.y = SharedRandomFloat( "pistolpay", -.6f, .6f );
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch( viewPunch );
}

void CWeaponM9::EnableIronsights()
{
	BaseClass::EnableIronsights();
	m_flLastAttackTime = m_flNextPrimaryAttack;
	m_flSoonestPrimaryAttack = m_flNextPrimaryAttack;
}

void CWeaponM9::DisableIronsights(bool ignoreAnimation)
{
	BaseClass::DisableIronsights(ignoreAnimation);
	m_flLastAttackTime = m_flNextPrimaryAttack;
	m_flSoonestPrimaryAttack = m_flNextPrimaryAttack;
}
