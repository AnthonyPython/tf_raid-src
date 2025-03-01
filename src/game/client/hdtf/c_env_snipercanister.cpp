//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "fx_explosion.h"
#include "tempentity.h"
#include "c_tracer.h"
#include "iviewrender_beams.h"
#include "beam_shared.h"
#include "beamdraw.h"
#include "env_snipercanister_shared.h"
#include "baseparticleentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Headcrab canister Class (Client-side only!)
//-----------------------------------------------------------------------------
class C_EnvSniperCanister : public C_BaseAnimating
{
	DECLARE_CLASS(C_EnvSniperCanister, C_BaseAnimating);
	DECLARE_CLIENTCLASS();

public:
	//-------------------------------------------------------------------------
	// Initialization/Destruction
	//-------------------------------------------------------------------------
	C_EnvSniperCanister();
	~C_EnvSniperCanister();

	virtual void	OnDataChanged(DataUpdateType_t updateType);
	virtual void	ClientThink();

private:
	C_EnvSniperCanister(const C_EnvSniperCanister &);

	CEnvSniperCanisterShared m_Shared;
	CNetworkVar(bool, m_bLanded);
	CNetworkVar(bool, m_bActive);
};


EXTERN_RECV_TABLE(DT_EnvSniperCanisterShared);

IMPLEMENT_CLIENTCLASS_DT(C_EnvSniperCanister, DT_EnvSniperCanister, CEnvSniperCanister)
RecvPropDataTable(RECVINFO_DT(m_Shared), 0, &REFERENCE_RECV_TABLE(DT_EnvSniperCanisterShared)),
RecvPropBool(RECVINFO(m_bLanded)),
RecvPropBool(RECVINFO(m_bActive)),
END_RECV_TABLE()


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
C_EnvSniperCanister::C_EnvSniperCanister()
{
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
C_EnvSniperCanister::~C_EnvSniperCanister()
{
}

//-----------------------------------------------------------------------------
// On data update
//-----------------------------------------------------------------------------
void C_EnvSniperCanister::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	if (updateType == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}

	// Stop client-side simulation on landing
	if (m_bLanded)
	{
		SetNextClientThink(CLIENT_THINK_NEVER);
	}
}


//-----------------------------------------------------------------------------
// Compute position
//-----------------------------------------------------------------------------
void C_EnvSniperCanister::ClientThink()
{
	Vector vecEndPosition;
	QAngle vecEndAngles;
	m_Shared.GetPositionAtTime(gpGlobals->curtime, vecEndPosition, vecEndAngles);
	SetAbsOrigin(vecEndPosition);
	SetAbsAngles(vecEndAngles);
}
