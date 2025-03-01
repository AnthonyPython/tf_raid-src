#ifndef HDTF_GAMERULES_H
#define HDTF_GAMERULES_H

#ifdef _WIN32
#pragma once
#endif

#include "hl2_gamerules.h"
#include "convar.h"

#ifdef CLIENT_DLL

#define CHDTF C_HDTF
#define CHDTFProxy C_HDTFProxy

#endif

#define VEC_PRONE_HULL_MIN HDTFGameRules( )->GetHDTFViewVectors( )->m_vProneHullMin
#define VEC_PRONE_HULL_MAX HDTFGameRules( )->GetHDTFViewVectors( )->m_vProneHullMax // MUST be shorter than duck hull for deploy check
#define VEC_PRONE_VIEW HDTFGameRules( )->GetHDTFViewVectors( )->m_vProneView

#define VEC_PRONE_HULL_MIN_SCALED( player ) ( HDTFGameRules( )->GetHDTFViewVectors( )->m_vProneHullMin * player->GetModelScale( ) )
#define VEC_PRONE_HULL_MAX_SCALED( player ) ( HDTFGameRules( )->GetHDTFViewVectors( )->m_vProneHullMax * player->GetModelScale( ) )
#define VEC_PRONE_VIEW_SCALED( player ) ( HDTFGameRules( )->GetHDTFViewVectors( )->m_vProneView * player->GetModelScale( ) )

class CHDTFProxy : public CHalfLife2Proxy
{
public:
	DECLARE_CLASS( CHDTFProxy, CHalfLife2Proxy );
	DECLARE_NETWORKCLASS( );
};

class CHDTFViewVectors
{
public:
	CHDTFViewVectors( )
	{ }

	CHDTFViewVectors(
		Vector vView,
		Vector vHullMin,
		Vector vHullMax,
		Vector vDuckHullMin,
		Vector vDuckHullMax,
		Vector vDuckView,
		Vector vObsHullMin,
		Vector vObsHullMax,
		Vector vDeadViewHeight,
		Vector vProneHullMin,
		Vector vProneHullMax,
		Vector vProneView )
	{
		m_vView = vView;
		m_vHullMin = vHullMin;
		m_vHullMax = vHullMax;
		m_vDuckHullMin = vDuckHullMin;
		m_vDuckHullMax = vDuckHullMax;
		m_vDuckView = vDuckView;
		m_vObsHullMin = vObsHullMin;
		m_vObsHullMax = vObsHullMax;
		m_vDeadViewHeight = vDeadViewHeight;
		m_vProneHullMin = vProneHullMin;
		m_vProneHullMax = vProneHullMax;
		m_vProneView = vProneView;
	}

	// Height above entity position where the viewer's eye is.
	Vector m_vView;

	Vector m_vHullMin;
	Vector m_vHullMax;

	Vector m_vDuckHullMin;
	Vector m_vDuckHullMax;
	Vector m_vDuckView;

	Vector m_vObsHullMin;
	Vector m_vObsHullMax;

	Vector m_vDeadViewHeight;

	Vector m_vProneHullMin;
	Vector m_vProneHullMax;
	Vector m_vProneView;
};

class CHDTF : public CHalfLife2
{
public:
	DECLARE_CLASS( CHDTF, CHalfLife2 );

	const CHDTFViewVectors *GetHDTFViewVectors( ) const;

	bool FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon ); // overrides CSingleplayRules::FShouldSwitchWeapon
	float FlPlayerFallDamage( CBasePlayer *pPlayer ); // overrides CSingleplayRules::FlPlayerFallDamage

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE( ); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE( ); // This makes datatables able to access our private vars.

	virtual const char *GetGameDescription( )
	{
		return "Hunt Down The Freeman";
	}

	virtual void InitDefaultAIRelationships( );
	virtual const char *AIClassText( int classType );

	virtual bool CanHavePlayerItem(CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon);

#endif

};

//-----------------------------------------------------------------------------
// Gets us at the HDTF game rules
//-----------------------------------------------------------------------------
inline CHDTF *HDTFGameRules( )
{

#if !defined( HDTF )

	Assert( 0 ); // g_pGameRules is NOT an instance of CHDTF and bad things happen

#endif

	return static_cast<CHDTF *>( g_pGameRules );
}

#endif // HDTF_GAMERULES_H
