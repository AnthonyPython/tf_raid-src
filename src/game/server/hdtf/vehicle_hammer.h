#pragma once

#include "vehicle_jeep.h"

#define NUM_WHEEL_EFFECTS	2

class CParticleSystem;

class CPropHammer : public CPropJeep
{
	DECLARE_CLASS(CPropHammer, CPropJeep);
	DECLARE_SERVERCLASS();

public:
	CPropHammer(void);

	virtual void	Spawn(void);
	virtual void	Think(void);
	virtual void	UpdateOnRemove(void);

	virtual void	DriveVehicle(float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased);

	virtual void	EnterVehicle(CBaseCombatCharacter *pPassenger);
	virtual void	ExitVehicle(int nRole);

	virtual bool	CanExitVehicle(CBaseEntity *pEntity);
	virtual void	Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual void	Precache(void);

	virtual Vector	BodyTarget(const Vector &posSrc, bool bNoisy = true);

	void			FireCannon(void);
	void			GetCannonAim(Vector &resultDir);
	void			AimGunAt(Vector *endPos, float flInterval);

	const char		*GetTracerType(void) { return "ApacheHelicopterTracer"; }
	void			DoImpactEffect(trace_t &tr, int nDamageType);

	virtual bool	PassengerShouldReceiveDamage(CTakeDamageInfo &info)
	{
		if (GetServerVehicle() && GetServerVehicle()->IsPassengerExiting())
			return false;

		return (info.GetDamageType() & DMG_VEHICLE) != 0;
	}

	virtual int	ObjectCaps(void) { return (BaseClass::ObjectCaps() | FCAP_NOTIFY_ON_TRANSITION); }

	virtual int	DrawDebugTextOverlays(void);

	DECLARE_DATADESC();

protected:
	virtual float			GetUprightTime(void) { return 1.0f; }
	virtual float			GetUprightStrength(void);
	virtual bool			ShouldPuntUseLaunchForces(PhysGunForce_t reason) { return (reason == PHYSGUN_FORCE_PUNTED); }
	virtual void			HandleWater(void);

	virtual AngularImpulse	PhysGunLaunchAngularImpulse(void);
	virtual Vector			PhysGunLaunchVelocity(const Vector &forward, float flMass);

private:

	void	UpdateWheelDust(void);

	void	InputLockEntrance(inputdata_t &data);
	void	InputUnlockEntrance(inputdata_t &data);
	void	InputLockExit(inputdata_t &data);
	void	InputUnlockExit(inputdata_t &data);
	void	InputDisablePhysGun(inputdata_t &data);
	void	InputEnablePhysGun(inputdata_t &data);
	void	InputDisableGun(inputdata_t &data);
	void	InputEnableGun(inputdata_t &data);
	void	CreateAvoidanceZone(void);

	bool	m_bEntranceLocked;
	bool	m_bExitLocked;

	CHandle< CParticleSystem >			m_hWheelDust[NUM_WHEEL_EFFECTS];
	CHandle< CParticleSystem >			m_hWheelWater[NUM_WHEEL_EFFECTS];

	float								m_flNextWaterSound;

	float m_flNextAvoidBroadcastTime;

	EHANDLE m_hArms;
};
