#ifndef BASECOMBATWEAPON_H
#define BASECOMBATWEAPON_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_basehdtfcombat.h"

#if defined( CLIENT_DLL )

#define CMachineGun C_MachineGun

#endif

//=========================================================
// Machine gun base class
//=========================================================
class CMachineGun : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CMachineGun, CBaseHDTFCombatWeapon );
	DECLARE_DATADESC( );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

	CMachineGun( );

	void PrimaryAttack( );

	// Default calls through to m_hOwner, but plasma weapons can override and shoot projectiles here.
	virtual void ItemPostFrame( );
	virtual void FireBullets( const FireBulletsInfo_t &info );
	virtual bool Deploy( );

	int WeaponSoundRealtime( WeaponSound_t shoot_type );

	// utility function
	static void DoMachineGunKick( CBasePlayer *pPlayer, float dampEasy, float maxVerticleKickAngle, float fireDurationTime, float slideLimitTime );

	virtual bool UsesSecondaryAmmo();

private:
	CMachineGun( const CMachineGun & );

protected:
	int	m_nShotsFired;	// Number of consecutive shots fired
	float m_flNextSoundTime;	// real-time clock of when to make next sound
};

#endif // BASECOMBATWEAPON_H
