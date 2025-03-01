#include "cbase.h"
#include "hdtf_parkour_controller.h"
#include "hdtf_player_shared.h"
#include "weapon_physcannon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar hdtf_pronespeed( "hdtf_pronespeed", "50", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar hdtf_walkspeed( "hdtf_walkspeed", "82", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar hdtf_normspeed( "hdtf_normspeed", "190", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar hdtf_sprintspeed( "hdtf_sprintspeed", "320", FCVAR_CHEAT | FCVAR_REPLICATED );

Vector CHDTF_Player::EyePosition( )
{
	Vector eyePosition;
	VectorRotate( m_vLocalViewOffset, EyeAngles( ), eyePosition );
	return BaseClass::EyePosition( ) + eyePosition;
}

const QAngle &CHDTF_Player::LocalEyeAngles( )
{
	static QAngle sum;
	return ( sum = BaseClass::LocalEyeAngles( ) + m_vLocalViewAngles );
}

const Vector &CHDTF_Player::GetLocalEyeOffset( ) const
{
	return m_vLocalViewOffset;
}

const QAngle &CHDTF_Player::GetLocalEyeAngles( ) const
{
	return m_vLocalViewAngles;
}

void CHDTF_Player::SetLocalEyeOffset( const Vector &viewOffset )
{
	m_vLocalViewOffset = viewOffset;
}

void CHDTF_Player::SetLocalEyeAngles( const QAngle &viewAngles )
{
	m_vLocalViewAngles = viewAngles;
}

void CHDTF_Player::ApplyGasMask( )
{
	m_bGasMaskActive = true;
}

void CHDTF_Player::RemoveGasMask( )
{
	m_bGasMaskActive = false;
}

bool CHDTF_Player::IsGasMaskActive( ) const
{
	return m_bGasMaskActive;
}

bool CHDTF_Player::CanSwitchGasMask( ) const
{
	return !IsNightVisionActive( );
}

void CHDTF_Player::ApplyNightVision( )
{
	m_bNightVisionActive = true;
}

void CHDTF_Player::RemoveNightVision( )
{
	m_bNightVisionActive = false;
}

bool CHDTF_Player::IsNightVisionActive( ) const
{
	return m_bNightVisionActive;
}

bool CHDTF_Player::CanSwitchNightVision( ) const
{
	return !IsGasMaskActive( );
}

bool CHDTF_Player::IsBinocularsActive() const
{
	return m_bIsBinocularsActive;
}

void CHDTF_Player::Weapon_SetLast(CBaseCombatWeapon *pWeapon)
{
	// we can't set our last weapon to parkour!
	if (pWeapon && FClassnameIs(pWeapon, "weapon_parkour"))
		return;

	BaseClass::Weapon_SetLast(pWeapon);
}

bool CHDTF_Player::Weapon_ShouldSetLast( CBaseCombatWeapon *pOldWeapon, CBaseCombatWeapon *pNewWeapon )
{
	Assert( pOldWeapon != NULL && pNewWeapon != NULL );
	return pOldWeapon->GetSlot( ) != pNewWeapon->GetSlot( );
}

CBaseCombatWeapon *CHDTF_Player::GetWeaponByName( const char *name ) const
{
	for( int k = 0; k < WeaponCount( ); ++k )
	{
		CBaseCombatWeapon *weapon = GetWeapon( k );
		if( weapon != NULL && V_strcmp( name, weapon->GetName( ) ) == 0 )
			return weapon;
	}

	return NULL;
}

bool CHDTF_Player::CanSprint( ) const
{
	return GetAbsVelocity( ).Length2DSqr( ) > 80.0f * 80.0f &&
		!IsSprinting( ) &&									// Not if we're already sprinting
		!IsWalking( ) &&									// Nor if we're walking
		( !m_Local.m_bDucked || m_Local.m_bDucking ) &&		// Nor if we're ducking
		!IsProne( ) &&										// Nor if we're prone
		GetWaterLevel( ) != 3;								// Certainly not underwater
}

bool CHDTF_Player::CanChangePosition( ) const
{
	return !IsGoingProne( ) && !IsGettingUpFromProne( );
}

bool CHDTF_Player::CanProne( ) const
{
	return CanChangePosition( ) && !IsProne( );
}

bool CHDTF_Player::CanUnProne( ) const
{
	return IsProne( );
}

bool CHDTF_Player::IsProne( ) const
{
	return m_bProne;
}

bool CHDTF_Player::IsProning( ) const
{
	return IsProne( ) || IsGoingProne( ) || IsGettingUpFromProne( );
}

bool CHDTF_Player::IsGoingProne( ) const
{
	return m_flGoProneTime > 0.0f;
}

bool CHDTF_Player::IsGettingUpFromProne( ) const
{
	return m_flUnProneTime > 0.0f;
}

void CHDTF_Player::StartGoingProne( )
{
	// make the prone sound
	CPASFilter filter( GetAbsOrigin( ) );
	filter.UsePredictionRules( );
	EmitSound( filter, entindex( ), "Player.GoProne" );

	// slow to prone speed
	m_flGoProneTime = gpGlobals->curtime + TIME_TO_PRONE;

	m_flUnProneTime = 0.0f; //reset
}

void CHDTF_Player::StandUpFromProne( )
{
	// make the prone sound
	CPASFilter filter( GetAbsOrigin( ) );
	filter.UsePredictionRules( );
	EmitSound( filter, entindex( ), "Player.UnProne" );

	// speed up to target speed
	m_flUnProneTime = gpGlobals->curtime + TIME_TO_PRONE;

	m_flGoProneTime = 0.0f; //reset

	m_bProne = false;
}

float CHDTF_Player::GetLeaningPercentage( ) const
{
	return Clamp( ( gpGlobals->curtime - m_fLeaningStart ) / TIME_TO_LEAN, 0.0f, 1.0f );
}

Vector CHDTF_Player::GetLeaningVector( bool right, float perc ) const
{
	return Vector( 0.0f, ( right ? -32.0f : 32.0f ) * sin( LEANING_RADIANS * perc ), -32.0f * ( 1.0f - cos( LEANING_RADIANS * perc ) ) );
}

bool CHDTF_Player::CanLean( bool right ) const
{
	Vector origin = GetAbsOrigin( ) + GetViewOffset( );
	origin.z -= 32.0f * ( 1.0f - cos( LEANING_RADIANS ) );

	Vector r;
	AngleVectors( GetAbsAngles( ), NULL, &r, NULL );

	trace_t	tr;
	UTIL_TraceHull( origin, origin + r * ( right ? 32.0f : -32.0f ), Vector( -8, -8, -16 ), Vector( 8, 8, 16 ), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	return !tr.DidHit( );
}

bool CHDTF_Player::CanSlide( ) const
{
	return CanChangePosition( ) && !IsSliding( ) && !IsProne( ) && GetAbsVelocity( ).Length2DSqr( ) > 120.0f * 120.0f;
}

bool CHDTF_Player::IsSliding( ) const
{
	return m_bSliding;
}

int CHDTF_Player::GetArmsType() const
{
	return m_iArmsType;
}

int CHDTF_Player::GetWeaponCount()
{
	int count = 0;

	for (int i = 0; i < WeaponCount(); i++)
	{
		CBaseCombatWeapon *weapon = GetWeapon(i);
		if (weapon && !FClassnameIs(weapon, "weapon_parkour"))
			count++;
	}

	return count;
}

bool CHDTF_Player::IsInventoryEnabled()
{
	return m_bIsInventoryEnabled;
}
