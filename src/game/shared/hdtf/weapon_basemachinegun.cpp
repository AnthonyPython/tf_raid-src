#include "cbase.h"

#if defined( CLIENT_DLL )

#include "c_basehlplayer.h"
#define CMachineGun C_MachineGun

#else

#include "hl2_player.h"

#endif

#include "weapon_basemachinegun.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar sv_weapons_recoil( "sv_weapons_recoil", "1", FCVAR_REPLICATED, "0 = Off, 1 = On (default), enables weapon recoil effects" );

#define	KICK_MIN_X 0.2f // Degrees
#define	KICK_MIN_Y 0.2f // Degrees
#define	KICK_MIN_Z 0.1f // Degrees

IMPLEMENT_NETWORKCLASS_ALIASED( MachineGun, DT_MachineGun )

BEGIN_NETWORK_TABLE( CMachineGun, DT_MachineGun )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CMachineGun )
END_PREDICTION_DATA( )

//=========================================================
//	>> CMachineGun
//=========================================================
BEGIN_DATADESC( CMachineGun )
DEFINE_FIELD( m_nShotsFired, FIELD_INTEGER ),
DEFINE_FIELD( m_flNextSoundTime, FIELD_TIME ),
END_DATADESC( )

CMachineGun::CMachineGun( )
{ }

void CMachineGun::PrimaryAttack( )
{
	if( !CanPrimaryAttack( ) )
		return;

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	// Abort here to handle burst and auto fire modes
	if( ( UsesClipsForAmmo1( ) && m_iClip1 == 0 ) || ( !UsesClipsForAmmo1( ) && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) == 0 ) )
		return;

	++m_nShotsFired;

	pPlayer->DoMuzzleFlash( );
	m_fShotLast = gpGlobals->curtime;
	DoSmokeBarrel();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;
	float fireRate = GetFireRate( );

	while( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		// MUST call sound before removing a round from the clip of a CHLMachineGun
		WeaponSound( SINGLE, m_flNextPrimaryAttack );
		m_flNextPrimaryAttack += fireRate;
		++iBulletsToFire;
	}

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if( UsesClipsForAmmo1( ) )
	{
		if( iBulletsToFire > m_iClip1 )
			iBulletsToFire = m_iClip1;

		m_iClip1 -= iBulletsToFire;
	}

	// Fire the bullets
	FireBulletsInfo_t info;
	info.m_iShots = iBulletsToFire;
	info.m_vecSrc = pPlayer->Weapon_ShootPosition( );
	info.m_vecDirShooting = pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	info.m_vecSpread = GetBulletSpread( );
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;
	FireBullets( info );

	//Factor in the view kick
	AddViewKick( );

	if( m_iClip1 == 0 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );

	SendWeaponAnim( GetPrimaryAttackActivity( ) );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
}

void CMachineGun::FireBullets( const FireBulletsInfo_t &info )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer != NULL )
		pPlayer->FireBullets( info );
}

#ifdef CLIENT_DLL

static void UTIL_ClipPunchAngleOffset( QAngle &in, const QAngle &punch, const QAngle &clip )
{
	QAngle final = in + punch;

	//Clip each component
	for( int i = 0; i < 3; ++i )
	{
		if( final[i] > clip[i] )
			final[i] = clip[i];
		else if( final[i] < -clip[i] )
			final[i] = -clip[i];

		//Return the result
		in[i] = final[i] - punch[i];
	}
}

#endif

void CMachineGun::DoMachineGunKick( CBasePlayer *pPlayer, float dampEasy, float maxVerticleKickAngle, float fireDurationTime, float slideLimitTime )
{
	QAngle vecScratch;
	int iSeed = CBaseEntity::GetPredictionRandomSeed( ) & 255;

	//Find how far into our accuracy degradation we are
	float duration = fireDurationTime > slideLimitTime ? slideLimitTime : fireDurationTime;
	float kickPerc = duration / slideLimitTime;

	// do this to get a hard discontinuity, clear out anything under 10 degrees punch
	pPlayer->ViewPunchReset( 10.0f );

	//Apply this to the view angles as well
	vecScratch.x = -( KICK_MIN_X + maxVerticleKickAngle * kickPerc );
	vecScratch.y = -( KICK_MIN_Y + maxVerticleKickAngle * kickPerc ) / 3.0f;
	vecScratch.z = KICK_MIN_Z + maxVerticleKickAngle * kickPerc / 8.0f;

	RandomSeed( iSeed );

	//Wibble left and right
	if( RandomInt( -1, 1 ) >= 0 )
		vecScratch.y *= -1.0f;

	++iSeed;

	//Wobble up and down
	if( RandomInt( -1, 1 ) >= 0 )
		vecScratch.z *= -1.0f;

	//Clip this to our desired min/max
	UTIL_ClipPunchAngleOffset( vecScratch, pPlayer->m_Local.m_vecPunchAngle, QAngle( 24.0f, 3.0f, 1.0f ) );

	if( sv_weapons_recoil.GetInt( ) == 1 )
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon( );
		if( pWeapon == NULL )
			return;

		QAngle angle = pPlayer->GetPunchAngle( );
		angle.x -= random->RandomInt( 2.75f, 3.25f ) + angle.x / 4.0f;
		angle.y += random->RandomInt( -1.25f, 1.5f );
		pPlayer->SetPunchAngle( angle );
	}
	else
	{
		//Add it to the view punch
		// NOTE: 0.5 is just tuned to match the old effect before the punch became simulated
		pPlayer->ViewPunch( vecScratch * 0.5f );
	}
}

bool CMachineGun::Deploy( )
{
	m_nShotsFired = 0;

	return BaseClass::Deploy( );
}

//-----------------------------------------------------------------------------
// Purpose: Make enough sound events to fill the estimated think interval
// returns: number of shots needed
//-----------------------------------------------------------------------------
int CMachineGun::WeaponSoundRealtime( WeaponSound_t shoot_type )
{
	int numBullets = 0;

	// ran out of time, clamp to current
	if( m_flNextSoundTime < gpGlobals->curtime )
		m_flNextSoundTime = gpGlobals->curtime;

	// make enough sound events to fill up the next estimated think interval
	float dt = clamp( m_flAnimTime - m_flPrevAnimTime, 0, 0.2 );
	if( m_flNextSoundTime < gpGlobals->curtime + dt )
	{
		WeaponSound( SINGLE_NPC, m_flNextSoundTime );
		m_flNextSoundTime += GetFireRate( );
		++numBullets;
	}

	if( m_flNextSoundTime < gpGlobals->curtime + dt )
	{
		WeaponSound( SINGLE_NPC, m_flNextSoundTime );
		m_flNextSoundTime += GetFireRate( );
		++numBullets;
	}

	return numBullets;
}

void CMachineGun::ItemPostFrame( )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	// Debounce the recoiling counter
	if( ( pOwner->m_nButtons & IN_ATTACK ) == 0 )
		m_nShotsFired = 0;

	BaseClass::ItemPostFrame( );
}

bool CMachineGun::UsesSecondaryAmmo()
{
	// this is an ugly hack which allows me to hide existence of secondary attack
	// on weapon_smg1 from M3SA
	if (FClassnameIs(this, "weapon_smg1"))
	{
		ConVarRef hdtf_smg1_shoots_grenades("hdtf_smg1_shoots_grenades");
		return hdtf_smg1_shoots_grenades.GetBool();
	}

	return BaseClass::UsesSecondaryAmmo();
}
