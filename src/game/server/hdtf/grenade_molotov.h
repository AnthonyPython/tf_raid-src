#ifndef	GRENADE_MOLOTOV_H
#define	GRENADE_MOLOTOV_H

#include "basegrenade_shared.h"
#include "smoke_trail.h"

class CGrenade_Molotov : public CBaseGrenade
{
public:
	DECLARE_CLASS( CGrenade_Molotov, CBaseGrenade );
	DECLARE_DATADESC( );

	~CGrenade_Molotov( );

	void Spawn( );
	void Precache( );
	void Detonate( );
	void MolotovTouch( CBaseEntity *pOther );
	void MolotovThink( );
	void CreateFlyingChunk( const Vector &vecChunkPos, const QAngle &vecChunkAngles, const char *pszChunkName, bool bSmall );
	void SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );

	CBasePlayer *m_pDamageParent;

protected:
	SmokeTrail *m_pFireTrail;
};

#endif	//GRENADE_MOLOTOV_H
