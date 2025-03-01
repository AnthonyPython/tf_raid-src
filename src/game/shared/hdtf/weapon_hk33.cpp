#include "cbase.h"
#include "weapon_basemachinegun.h"

#ifdef CLIENT_DLL

#include "c_te_effect_dispatch.h"

#else

#include "te_effect_dispatch.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL

#define CWeaponHK33 C_WeaponHK33

#endif

#define	EASY_DAMPEN 0.5f
#define	MAX_VERTICAL_KICK 8.0f //Degrees
#define	SLIDE_LIMIT 5.0f //Seconds

class CWeaponHK33 : public CMachineGun
{
public:
	DECLARE_CLASS( CWeaponHK33, CMachineGun );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	CWeaponHK33( );

	const char *GetTracerType( )
	{
		return "AR2Tracer";
	}

	int GetMinBurst( )
	{
		return 2;
	}

	int GetMaxBurst( )
	{
		return 5;
	}

	float GetFireRate( )
	{
		return 0.1f;
	}

	const Vector &GetBulletSpread( )
	{
		static Vector cone = VECTOR_CONE_3DEGREES;
		return cone;
	}

	Activity GetPrimaryAttackActivity( );

	void DoImpactEffect( trace_t &tr, int nDamageType );

	void AddViewKick( );

	const WeaponProficiencyInfo_t *GetProficiencyValues( );

private:
	CWeaponHK33( const CWeaponHK33 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponHK33, DT_WeaponHK33 )

BEGIN_NETWORK_TABLE( CWeaponHK33, DT_WeaponHK33 )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CWeaponHK33 )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_hk33, CWeaponHK33 );
PRECACHE_WEAPON_REGISTER( weapon_hk33 );


#ifndef CLIENT_DLL

acttable_t CWeaponHK33::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_AR2, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_AR2, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_AR2, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_AR2, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_AR2, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_AR2, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_AR2, false },
};

IMPLEMENT_ACTTABLE( CWeaponHK33 );

#endif

CWeaponHK33::CWeaponHK33( )
{
	m_fMinRange1 = 65.0f;
	m_fMaxRange1 = 2048.0f;

	m_fMinRange2 = 256.0f;
	m_fMaxRange2 = 1024.0f;

	m_nShotsFired = 0;
}

Activity CWeaponHK33::GetPrimaryAttackActivity( )
{
	if( m_nShotsFired < 2 )
		return ACT_VM_PRIMARYATTACK;

	if( m_nShotsFired < 3 )
		return ACT_VM_RECOIL1;

	if( m_nShotsFired < 4 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

void CWeaponHK33::DoImpactEffect( trace_t &tr, int nDamageType )
{
	CEffectData data;

	data.m_vOrigin = tr.endpos + ( tr.plane.normal * 1.0f );
	data.m_vNormal = tr.plane.normal;

	DispatchEffect( "AR2Impact", data );

	BaseClass::DoImpactEffect( tr, nDamageType );
}

void CWeaponHK33::AddViewKick( )
{
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}

const WeaponProficiencyInfo_t *CWeaponHK33::GetProficiencyValues( )
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0, 0.75 },
		{ 5.00, 0.75 },
		{ 3.0, 0.85 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE( proficiencyTable ) == WEAPON_PROFICIENCY_PERFECT + 1 );

	return proficiencyTable;
}
