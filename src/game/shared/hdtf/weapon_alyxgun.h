#ifndef WEAPON_ALYXGUN_H
#define WEAPON_ALYXGUN_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_basemachinegun.h"

#ifdef CLIENT_DLL

#define CWeaponAlyxGun C_WeaponAlyxGun

#endif

class CWeaponAlyxGun : public CMachineGun
{
public:
	DECLARE_CLASS( CWeaponAlyxGun, CMachineGun );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );
	DECLARE_DATADESC( );

#endif

	CWeaponAlyxGun( );

	void Precache( );

	virtual int GetMinBurst( )
	{
		return 4;
	}

	virtual int GetMaxBurst( )
	{
		return 7;
	}

	virtual float GetMinRestTime( );
	virtual float GetMaxRestTime( );

	virtual void Equip( CBaseCombatCharacter *pOwner );

	float GetFireRate( )
	{
		return 0.1f;
	}

#ifndef CLIENT_DLL

	int CapabilitiesGet( )
	{
		return bits_CAP_WEAPON_RANGE_ATTACK1;
	}

	int WeaponRangeAttack1Condition( float flDot, float flDist );
	int WeaponRangeAttack2Condition( float flDot, float flDist );

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );

	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

#endif

	virtual const Vector &GetBulletSpread( );

	virtual void SetPickupTouch( )
	{
		// Alyx gun cannot be picked up
		SetTouch( NULL );
	}

	float m_flTooCloseTimer;
};

#endif // WEAPON_ALYXGUN_H
