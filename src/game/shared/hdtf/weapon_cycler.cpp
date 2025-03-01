#include "cbase.h"
#include "weapon_basehdtfcombat.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL

#define CWeaponCycler C_WeaponCycler

#endif

// FIXME: this doesn't work anymore, and hasn't for a while now.

class CWeaponCycler : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponCycler, CBaseHDTFCombatWeapon );
	DECLARE_NETWORKCLASS( );

#ifndef CLIENT_DLL

	DECLARE_DATADESC( );

#endif

	void Spawn( );

	void PrimaryAttack( );
	void SecondaryAttack( );
	bool Deploy( );
	bool Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	string_t m_iszModel;
	int m_iModel;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponCycler, DT_WeaponCycler )

BEGIN_NETWORK_TABLE( CWeaponCycler, DT_WeaponCycler )
END_NETWORK_TABLE( )

LINK_ENTITY_TO_CLASS( cycler_weapon, CWeaponCycler );
PRECACHE_WEAPON_REGISTER( cycler_weapon );

#ifndef CLIENT_DLL

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CWeaponCycler )

DEFINE_FIELD( m_iszModel, FIELD_STRING ),
DEFINE_FIELD( m_iModel, FIELD_INTEGER ),

END_DATADESC( )

#endif

void CWeaponCycler::Spawn( )
{
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_NONE );

	PrecacheModel( STRING( GetModelName( ) ) );
	SetModel( STRING( GetModelName( ) ) );
	m_iszModel = GetModelName( );
	m_iModel = GetModelIndex( );

#ifndef CLIENT_DLL

	UTIL_SetSize( this, Vector( -16, -16, 0 ), Vector( 16, 16, 16 ) );

#endif

	SetTouch( &CWeaponCycler::DefaultTouch );
}

bool CWeaponCycler::Deploy( )
{
	CBaseCombatCharacter *pOwner = GetOwner( );
	if( pOwner != NULL )
	{
		pOwner->m_flNextAttack = gpGlobals->curtime + 1.0;
		SendWeaponAnim( 0 );
		m_iClip1 = 0;
		m_iClip2 = 0;
		return true;
	}

	return false;
}

bool CWeaponCycler::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	CBaseCombatCharacter *pOwner = GetOwner( );
	if( pOwner != NULL )
		pOwner->m_flNextAttack = gpGlobals->curtime + 0.5;

	return true;
}

void CWeaponCycler::PrimaryAttack( )
{
	SendWeaponAnim( GetSequence( ) );
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
}

void CWeaponCycler::SecondaryAttack( )
{
	// BUG:  Why do we set this here and then set to zero right after?
	SetModelIndex( m_iModel );
	float flFrameRate = 0.0;

	SetModelIndex( 0 );

	int nSequence = ( GetSequence( ) + 1 ) % 8;
	if( flFrameRate == 0.0 )
		nSequence = 0;

	SetSequence( nSequence );
	SendWeaponAnim( nSequence );

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
}
