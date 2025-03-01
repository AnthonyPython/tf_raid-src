#include "cbase.h"
#include "laser_dot.h"
#include "debugoverlay_shared.h"

#ifdef CLIENT_DLL

#include "c_baseplayer.h"
#include "model_types.h"
#include "beamdraw.h"
#include "iviewrender_beams.h"
#include "fx_line.h"
#include "view.h"

#else

#include "basecombatcharacter.h"
#include "movie_explosion.h"
#include "soundent.h"
#include "player.h"
#include "rope.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "explode.h"
#include "util.h"
#include "in_buttons.h"
#include "shake.h"
#include "te_effect_dispatch.h"
#include "triggers.h"
#include "smoke_trail.h"
#include "collisionutils.h"
#include "hl2_shareddefs.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	LASER_SPRITE "sprites/redglow1.vmt"

IMPLEMENT_NETWORKCLASS_ALIASED( LaserDot, DT_LaserDot )

BEGIN_NETWORK_TABLE( CLaserDot, DT_LaserDot )


#ifndef CLIENT_DLL

SendPropBool( SENDINFO( m_bIsOn ) ),
SendPropEHandle( SENDINFO( m_hWeapon ) ),

#else

RecvPropBool( RECVINFO( m_bIsOn ) ),
RecvPropEHandle( RECVINFO( m_hWeapon ) ),

#endif

END_NETWORK_TABLE( )

static const char *g_pLaserDotThink = "LaserThinkContext";

#ifndef CLIENT_DLL

// a list of laser dots to search quickly
CEntityClassList<CLaserDot> g_LaserDotList;
template<> CLaserDot *CEntityClassList<CLaserDot>::m_pClassList = NULL;

CLaserDot *GetLaserDotList( )
{
	return g_LaserDotList.m_pClassList;
}

#endif

//-----------------------------------------------------------------------------
// Finds missiles in cone
//-----------------------------------------------------------------------------
#ifndef CLIENT_DLL
CBaseEntity *CreateLaserDot( const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot )
{
	return CLaserDot::Create( origin, pOwner, bVisibleDot );
}
#endif

void SetLaserDotTarget( CBaseEntity *pLaserDot, CBaseEntity *pTarget )
{
	CLaserDot *pDot = assert_cast<CLaserDot *>( pLaserDot );
	pDot->SetTargetEntity( pTarget );
}

void EnableLaserDot( CBaseEntity *pLaserDot, bool bEnable )
{
	CLaserDot *pDot = assert_cast<CLaserDot *>( pLaserDot );
	if( bEnable )
		pDot->TurnOn( );
	else
		pDot->TurnOff( );
}

//=============================================================================
// Laser Dot
//=============================================================================

LINK_ENTITY_TO_CLASS( env_laserdot, CLaserDot );

#ifndef CLIENT_DLL

BEGIN_DATADESC( CLaserDot )
DEFINE_FIELD( m_vecSurfaceNormal, FIELD_VECTOR ),
DEFINE_FIELD( m_hTargetEnt, FIELD_EHANDLE ),
DEFINE_FIELD( m_bVisibleLaserDot, FIELD_BOOLEAN ),
DEFINE_FIELD( m_bIsOn, FIELD_BOOLEAN ),
DEFINE_FIELD( m_hWeapon, FIELD_EHANDLE ),

DEFINE_THINKFUNC( LaserThink ),
END_DATADESC( )

void CLaserDot::Precache( )
{
	BaseClass::Precache( );

	PrecacheModel( LASER_SPRITE );
}

#endif

CLaserDot::CLaserDot( )
{
	m_hTargetEnt = NULL;
	m_bIsOn = true;

#ifndef CLIENT_DLL

	g_LaserDotList.Insert( this );

#endif

}

CLaserDot::~CLaserDot( )
{

#ifndef CLIENT_DLL

	g_LaserDotList.Remove( this );

#endif

}

#ifndef CLIENT_DLL
CLaserDot *CLaserDot::Create( const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot )
{
	CLaserDot *pLaserDot = static_cast<CLaserDot *>(
		CBaseEntity::Create("env_laserdot", origin, QAngle(0, 0, 0), pOwner)
	);
	if( pLaserDot == NULL )
		return NULL;

	pLaserDot->m_bVisibleLaserDot = bVisibleDot;
	pLaserDot->SetMoveType( MOVETYPE_NONE );
	pLaserDot->AddSolidFlags( FSOLID_NOT_SOLID );
	pLaserDot->AddEffects( EF_NOSHADOW );
	pLaserDot->SetSize( vec3_origin, vec3_origin );

	//Create the graphic
	pLaserDot->SpriteInit( LASER_SPRITE, origin );

	pLaserDot->SetTransparency( kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation );
	pLaserDot->SetScale( 0.5f );

	pLaserDot->SetOwnerEntity( pOwner );

	pLaserDot->SetContextThink( &CLaserDot::LaserThink, gpGlobals->curtime + 0.1f, g_pLaserDotThink );
	pLaserDot->SetSimulatedEveryTick( true );

	if( !bVisibleDot )
		pLaserDot->TurnOff( );

	return pLaserDot;
}
#endif

void CLaserDot::LaserThink( )
{
	SetNextThink( gpGlobals->curtime + 0.05f, g_pLaserDotThink );

	if( GetOwnerEntity( ) == NULL )
		return;

	Vector viewDir = GetAbsOrigin( ) - GetOwnerEntity( )->GetAbsOrigin( );
	float dist = VectorNormalize( viewDir );

	float scale = RemapVal( dist, 32, 1024, 0.01f, 0.5f );
	float scaleOffs = random->RandomFloat( -scale * 0.25f, scale * 0.25f );

	scale = clamp( scale + scaleOffs, 0.1f, 32.0f );

	SetScale( scale );
}

void CLaserDot::SetLaserPosition( const Vector &origin, const Vector &normal )
{
	SetAbsOrigin( origin );
	m_vecSurfaceNormal = normal;
}

Vector CLaserDot::GetChasePosition( )
{
	return GetAbsOrigin( ) - m_vecSurfaceNormal * 10;
}

void CLaserDot::TurnOn( )
{
	m_bIsOn = true;

	if( m_bVisibleLaserDot )
		BaseClass::TurnOn( );
}

void CLaserDot::TurnOff( )
{
	m_bIsOn = false;

	if( m_bVisibleLaserDot )
		BaseClass::TurnOff( );
}
