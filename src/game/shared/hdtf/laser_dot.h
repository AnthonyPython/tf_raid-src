#ifndef LASER_DOT_H
#define LASER_DOT_H

#ifdef _WIN32
#pragma once
#endif

#include "weapon_basehdtfcombat.h"
#include "Sprite.h"

#ifdef CLIENT_DLL

#define CLaserDot C_LaserDot

#endif

//-----------------------------------------------------------------------------
// Laser Dot
//-----------------------------------------------------------------------------
class CLaserDot : public CSprite
{
public:
	DECLARE_CLASS( CLaserDot, CSprite );
	DECLARE_NETWORKCLASS( );

#ifndef CLIENT_DLL

	DECLARE_DATADESC( );

#endif

	CLaserDot( );
	~CLaserDot( );

#ifndef CLIENT_DLL
	static CLaserDot *Create( const Vector &origin, CBaseEntity *pOwner = NULL, bool bVisibleDot = true );
#endif

	void SetTargetEntity( CBaseEntity *pTarget )
	{
		m_hTargetEnt = pTarget;
	}

	CBaseEntity *GetTargetEntity( )
	{
		return m_hTargetEnt;
	}

	void SetLaserPosition( const Vector &origin, const Vector &normal );
	Vector GetChasePosition( );

	void TurnOn( );
	void TurnOff( );
	bool IsOn( ) const
	{
		return m_bIsOn;
	}

	void LaserThink( );

	int ObjectCaps( )
	{
		return ( BaseClass::ObjectCaps( ) & ~FCAP_ACROSS_TRANSITION ) | FCAP_DONT_SAVE;
	}

#ifndef CLIENT_DLL

	virtual void Precache( );

#endif

	CLaserDot *m_pNext;

private:
	Vector m_vecSurfaceNormal;
	EHANDLE m_hTargetEnt;
	bool m_bVisibleLaserDot;
	CNetworkVar( bool, m_bIsOn );
	CNetworkHandle( CBaseHDTFCombatWeapon, m_hWeapon );

#ifdef CLIENT_DLL

	CMaterialReference m_hBeamMaterial; // Used for the laser beam
	Beam_t *m_pBeam; // Laser beam temp entity

#endif

};

//-----------------------------------------------------------------------------
// Laser dot control
//-----------------------------------------------------------------------------

CLaserDot *GetLaserDotList( );
#ifndef CLIENT_DLL
CBaseEntity *CreateLaserDot( const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot );
#endif
void SetLaserDotTarget( CBaseEntity *pLaserDot, CBaseEntity *pTarget );
void EnableLaserDot( CBaseEntity *pLaserDot, bool bEnable );

#endif // LASER_DOT_H
