#pragma once
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#pragma once

#include "vstdlib/random.h"
#include "mathlib/vector.h"
#include "utlvector.h"
#include "networkvar.h"

// list of all the attachments used by the canister and snipers
static const char *s_c1Laser = "cannon_1_laser";
static const char *s_c2Laser = "cannon_2_laser";
static const char *s_c3Laser = "cannon_3_laser";

static const char *s_c1Aim = "cannon_1_aim";
static const char *s_c2Aim = "cannon_2_aim";
static const char *s_c3Aim = "cannon_3_aim";

static const char *s_c1Muzzle = "cannon_1_muzzle";
static const char *s_c2Muzzle = "cannon_2_muzzle";
static const char *s_c3Muzzle = "cannon_3_muzzle";

// shared class
class CEnvSniperCanisterShared
{
	DECLARE_CLASS_NOBASE(CEnvSniperCanisterShared);
	DECLARE_EMBEDDED_NETWORKVAR();
	DECLARE_SIMPLE_DATADESC();

public:
	CEnvSniperCanisterShared();

	// Initialization.
	void InitInWorld(float flLaunchTime, const Vector &vecStartPosition, const QAngle &vecStartAngles, const Vector &vecDirection, const Vector &vecImpactPosition, bool bLaunchedFromWithinWorld = false);
	void InitInSkybox(float flLaunchTime, const Vector &vecStartPosition, const QAngle &vecStartAngles, const Vector &vecDirection, const Vector &vecImpactPosition, const Vector &vecSkyboxOrigin, float flSkyboxScale);

	// Returns the position of the object at a given time.
	void GetPositionAtTime(float flTime, Vector &vecPosition, QAngle &vecAngles);

	// Returns whether or not the object is the the skybox
	bool IsInSkybox();

	// Returns the time at which it enters the world
	float GetEnterWorldTime() const;

	// Convert from skybox to world
	void ConvertFromSkyboxToWorld();

	// Did we impact?
	bool DidImpact(float flTime) const;

public:
	// The objects initial parametric conditions.
	CNetworkVector(m_vecStartPosition);
	CNetworkVector(m_vecEnterWorldPosition);
	CNetworkVector(m_vecDirection);
	CNetworkQAngle(m_vecStartAngles);

	CNetworkVar(float, m_flFlightTime);
	CNetworkVar(float, m_flFlightSpeed);
	CNetworkVar(float, m_flLaunchTime);

	CNetworkVar(float, m_flInitialZSpeed);
	CNetworkVar(float, m_flZAcceleration);
	CNetworkVar(float, m_flHorizSpeed);

	CNetworkVar(bool, m_bLaunchedFromWithinWorld);

	CNetworkVector(m_vecParabolaDirection);

	// The time at which the canister enters the skybox
	CNetworkVar(float, m_flWorldEnterTime);

	// Skybox data
	CNetworkVector(m_vecSkyboxOrigin);
	CNetworkVar(float, m_flSkyboxScale);
	CNetworkVar(bool, m_bInSkybox);

private:
	float	m_flLaunchHeight;

	// Calculate the enter time. (called from Init)
	void CalcEnterTime(const Vector &vecTriggerMins, const Vector &vecTriggerMaxs);

	friend class CEnvSniperCanister;
	friend class C_EnvSniperCanister;
};
