#ifndef WEAPON_BASEMELEE_H
#define WEAPON_BASEMELEE_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_basehdtfcombat.h"

#if defined( CLIENT_DLL )

#define CBaseMeleeWeapon C_BaseMeleeWeapon

#endif

class CBaseMeleeWeapon : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CBaseMeleeWeapon, CBaseHDTFCombatWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

	CBaseMeleeWeapon( );

	//Functions to select animation sequences 
	virtual Activity GetPrimaryAttackActivity( );
	virtual Activity GetPrimaryMissActivity( );

	virtual Activity GetSecondaryAttackActivity( );
	virtual Activity GetSecondaryMissActivity( );

	virtual	float GetFireRate( );
	virtual float GetRange( );
	virtual	float GetDamageForActivity( Activity hitActivity );
	virtual int GetDamageType( bool isSecondary );

	virtual	void Spawn( );

	//Attack functions
	virtual	void PrimaryAttack( );
	virtual	void SecondaryAttack( );

	virtual void ItemPostFrame( );

private:
	CBaseMeleeWeapon( const CBaseMeleeWeapon & );

protected:
	virtual	void	ImpactEffect( trace_t &trace, bool bIsSecondary );
	bool			ImpactWater( const Vector &start, const Vector &end );
	void			Swing( bool bIsSecondary, bool bSendAnim );
	void			Hit( trace_t &traceHit, Activity nHitActivity, bool bIsSecondary );
	Activity		ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner );
};

#endif // WEAPON_BASEBLUDGEON_H
