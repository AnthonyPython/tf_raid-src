#include "cbase.h"
#include "in_buttons.h"
#include "weapon_basehdtfcombat.h"
#include "hdtf_player_shared.h"

#define	PISTOL_FASTEST_REFIRE_TIME 1.5f
#define	PISTOL_FASTEST_DRY_REFIRE_TIME 1.5f

#define	PISTOL_ACCURACY_SHOT_PENALTY_TIME 0.2f // Applied amount of time each shot adds to the time we must recover from
#define	PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME 1.5f // Maximum penalty to deal out

#ifdef CLIENT_DLL

#define CWeaponFuddGun C_WeaponFuddGun

#endif

//just assume anything that is broken in here is my doing. -m3sa

//float m_flDamage;
ConVar sk_plr_dmg_fuddgun("sk_plr_dmg_fuddgun", "500", FCVAR_REPLICATED);
ConVar sk_npc_dmg_fuddgun("sk_npc_dmg_fuddgun", "500", FCVAR_REPLICATED);

class CWeaponFuddGun : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponFuddGun, CBaseHDTFCombatWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	CWeaponFuddGun( );

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
		return 1.5;
	}

	virtual int	GetMaxBurst( )
	{
		return 1.5;
	}

	virtual float GetFireRate( )
	{
		return 1.5f;
	}

private:
	CNetworkVar( float, m_flSoonestPrimaryAttack );
	CNetworkVar( float, m_flLastAttackTime );
	CNetworkVar( float, m_flAccuracyPenalty );
	CNetworkVar( int, m_nNumShotsFired );

private:
	CWeaponFuddGun( const CWeaponFuddGun & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponFuddGun, DT_WeaponFuddGun )

BEGIN_NETWORK_TABLE( CWeaponFuddGun, DT_WeaponFuddGun )

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

BEGIN_PREDICTION_DATA( CWeaponFuddGun )
DEFINE_PRED_FIELD( m_flSoonestPrimaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_flLastAttackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_flAccuracyPenalty, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_nNumShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA( )

#endif

LINK_ENTITY_TO_CLASS( Weapon_FuddGun, CWeaponFuddGun );
PRECACHE_WEAPON_REGISTER( Weapon_FuddGun );

#ifndef CLIENT_DLL

acttable_t CWeaponFuddGun::m_acttable[] =
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

IMPLEMENT_ACTTABLE( CWeaponFuddGun );

#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponFuddGun::CWeaponFuddGun( )
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
void CWeaponFuddGun::DryFire( )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );

	m_flSoonestPrimaryAttack = gpGlobals->curtime + PISTOL_FASTEST_DRY_REFIRE_TIME;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponFuddGun::PrimaryAttack( )
{
	if( gpGlobals->curtime - m_flLastAttackTime > 10.5f )
		m_nNumShotsFired = 0;
	else
		++m_nNumShotsFired;

	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + PISTOL_FASTEST_REFIRE_TIME;
	//m_flDamage = sk_plr_dmg_fuddgun.GetFloat();

	CHDTF_Player *pOwner = ToHDTFPlayer( GetOwner( ) );
	if( pOwner != NULL && !pOwner->IsSliding() )

		pOwner->ViewPunchReset( );


	BaseClass::PrimaryAttack( );

	m_flAccuracyPenalty += PISTOL_ACCURACY_SHOT_PENALTY_TIME;
}

//void CWeaponFuddGun::PrimaryAttack()	//from awp
//{
//	if (!CanPrimaryAttack())
//		return;
//
//	CHDTF_Player *pOwner = ToHDTFPlayer( GetOwner( ) );
//	if( pOwner != NULL && !pOwner->IsSliding() )
//		pOwner->ViewPunchReset( );
//
//	// Only the player fires this way so we can cast
//	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
//	if (pPlayer == NULL)
//		return;
//
//	WeaponSound(SINGLE);
//
//	pPlayer->DoMuzzleFlash();
//
//	SendWeaponAnim(GetPrimaryAttackActivity());
//
//	// Don't fire again until fire animation has completed
//	//m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
//	//m_iClip1 -= 1;
//
//	pPlayer->SetAnimation(PLAYER_ATTACK1);
//
//	m_flLastAttackTime = gpGlobals->curtime;
//	m_flSoonestPrimaryAttack = gpGlobals->curtime + PISTOL_FASTEST_REFIRE_TIME;
//
//	Vector vecSrc = pPlayer->Weapon_ShootPosition();
//	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);
//
//	FireBulletsInfo_t info(1, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
//	info.m_pAttacker = pPlayer;
//	info.m_flDamage = sk_plr_dmg_fuddgun.GetFloat(); // override damage
//
//	pPlayer->FireBullets(info);
//
//	m_flAccuracyPenalty += PISTOL_ACCURACY_SHOT_PENALTY_TIME;
//
//	//QAngle punch;
//	//punch.Init(SharedRandomFloat("doublebarrelpax", -2, -1), SharedRandomFloat("doublebarrelpay", -2, 2), 0);
//	//pPlayer->ViewPunch(punch);
//
//	//if (m_iClip1 == 0 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
//	//	// HEV suit - indicate out of ammo condition
//	//	pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
//}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFuddGun::UpdatePenaltyTime( )
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
void CWeaponFuddGun::ItemPreFrame( )
{
	UpdatePenaltyTime( );

	BaseClass::ItemPreFrame( );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponFuddGun::ItemBusyFrame( )
{
	UpdatePenaltyTime( );

	BaseClass::ItemBusyFrame( );
}

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as button is pressed
//-----------------------------------------------------------------------------
void CWeaponFuddGun::ItemPostFrame( )
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
bool CWeaponFuddGun::Reload( )
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
void CWeaponFuddGun::AddViewKick( )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	QAngle viewPunch;
	viewPunch.x = SharedRandomFloat( "pistolpax", -25.5f, -30.5f );
	viewPunch.y = SharedRandomFloat( "pistolpay", .0f, -25.5f );
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch( viewPunch );
}

void CWeaponFuddGun::EnableIronsights()
{
	BaseClass::EnableIronsights();
	m_flLastAttackTime = m_flNextPrimaryAttack;
	m_flSoonestPrimaryAttack = m_flNextPrimaryAttack;
}

void CWeaponFuddGun::DisableIronsights(bool ignoreAnimation)
{
	BaseClass::DisableIronsights(ignoreAnimation);
	m_flLastAttackTime = m_flNextPrimaryAttack;
	m_flSoonestPrimaryAttack = m_flNextPrimaryAttack;
}
