#pragma once

class CEnvSniperCanister : public CBaseAnimating
{
	DECLARE_CLASS(CEnvSniperCanister, CBaseAnimating);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

public:

	// Initialization
	CEnvSniperCanister();
	~CEnvSniperCanister();

	virtual void		Precache(void);
	virtual void		Spawn(void);
	virtual void		UpdateOnRemove();

	virtual void		SetTransmit(CCheckTransmitInfo *pInfo, bool bAlways);

	void				SetPitchYawTarget(int cannon, float pitch, float yaw);

private:
	void				InputFireCanister(inputdata_t &inputdata);
	void				InputOpenCanister(inputdata_t &inputdata);
	void				InputStopSmoke(inputdata_t &inputdata);

	// Think(s)
	void				CanisterSkyboxThink(void);
	void				CanisterWorldThink(void);
	void				CanisterSkyboxOnlyThink(void);
	void				CanisterSkyboxRestartThink(void);
	void				CanisterPoseUpdateThink(void);
	void				WaitForOpenSequenceThink();

	// Place the canister in the world
	CSkyCamera*			PlaceCanisterInWorld();

	// Check for impacts
	void				TestForCollisionsAgainstEntities(const Vector &vecEndPosition);
	void				TestForCollisionsAgainstWorld(const Vector &vecEndPosition);

	// Figure out where we enter the world
	void				ComputeWorldEntryPoint(Vector *pStartPosition, QAngle *pStartAngles, Vector *pStartDirection);

	// Blows up!
	void				Detonate(void);

	// Landed!
	void				SetLanded(void);
	void				Landed(void);

	// Open!
	void				OpenCanister(void);
	void				CanisterFinishedOpening();

	// Set up the world model
	void				SetupWorldModel();

	virtual void		TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);
	void				CommitDeath();
	void				CreateSnipers();
	void				AlertSnipers(CBaseEntity *attacker);

	void				RemoveAlertEntites();

	void				WaitForDeathSequenceThink();

private:
	CNetworkVar(bool, m_bLanded);
	CNetworkVar(bool, m_bActive);

	CNetworkVarEmbedded(CEnvSniperCanisterShared, m_Shared);
	CHandle<CSpriteTrail> m_hTrail;
	CHandle<SmokeTrail>	m_hSmokeTrail;
	Vector m_vecImpactPosition;
	QAngle m_angImpactAngle;
	float m_flDamageRadius;
	float m_flDamage;
	bool m_bIncomingSoundStarted;
	bool m_bHasDetonated;
	bool m_bLaunched;
	bool m_bOpened;
	float m_flSmokeLifetime;

	int m_iDamageLevel;

	int s_nExplosionTexture = -1;

	string_t m_iszLaunchPositionName;

	COutputEHANDLE m_OnLaunched;
	COutputEvent m_OnImpacted;
	COutputEvent m_OnOpened;
	COutputEvent m_OnDestroyed;

	// Only for skybox only cannisters.
	float m_flMinRefireTime;
	float m_flMaxRefireTime;
	int m_nSkyboxCannisterCount;

	float flPitchTargets[3];
	float flYawTargets[3];

	CProtoSniper *pSnipers[3];
	CAI_BaseNPC *pDecoy;
};
