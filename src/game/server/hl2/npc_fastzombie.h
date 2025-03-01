//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef NPC_FASTZOMBIE_H
#define NPC_FASTZOMBIE_H

#include "npc_BaseZombie.h"

#ifdef HL2_EPISODIC
#include "episodic/ai_behavior_passenger_zombie.h"
#endif	// HL2_EPISODIC

//=========================================================
//=========================================================
class CFastZombie : public CNPC_BaseZombie
{
	DECLARE_CLASS( CFastZombie, CNPC_BaseZombie );

public:
	void Spawn( void );
	void Precache( void );

	void SetZombieModel( void );
	bool CanSwatPhysicsObjects( void ) { return false; }

	int	TranslateSchedule( int scheduleType );

	Activity NPC_TranslateActivity( Activity baseAct );

	void LeapAttackTouch( CBaseEntity *pOther );
	void ClimbTouch( CBaseEntity *pOther );

	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );
	int SelectSchedule( void );
	void OnScheduleChange( void );

	void PrescheduleThink( void );

	float InnateRange1MaxRange( void );
	int RangeAttack1Conditions( float flDot, float flDist );
	int MeleeAttack1Conditions( float flDot, float flDist );

	virtual float GetClawAttackRange() const { return 50; }

	bool ShouldPlayFootstepMoan( void ) { return false; }

	void HandleAnimEvent( animevent_t *pEvent );

	void PostNPCInit( void );

	void LeapAttack( void );
	void LeapAttackSound( void );

	void BecomeTorso( const Vector &vecTorsoForce, const Vector &vecLegsForce );

	bool IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos) const;
	bool MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost );
	bool ShouldFailNav( bool bMovementFailed );

#ifdef HDTF

	bool CanBecomeLiveTorso()
	{
		return !m_fIsHeadless;
	}

#endif

	int	SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );

	const char *GetMoanSound( int nSound );

	void OnChangeActivity( Activity NewActivity );
	void OnStateChange( NPC_STATE OldState, NPC_STATE NewState );
	void Event_Killed( const CTakeDamageInfo &info );
	bool ShouldBecomeTorso( const CTakeDamageInfo &info, float flDamageThreshold );

	virtual Vector GetAutoAimCenter() { return WorldSpaceCenter() - Vector( 0, 0, 12.0f ); }

	void PainSound( const CTakeDamageInfo &info );
	void DeathSound( const CTakeDamageInfo &info ); 
	void AlertSound( void );
	void IdleSound( void );
	void AttackSound( void );
	void AttackHitSound( void );
	void AttackMissSound( void );
	void FootstepSound( bool fRightFoot );
	void FootscuffSound( bool fRightFoot ) {}; // fast guy doesn't scuff
	void StopLoopingSounds( void );

	void SoundInit( void );
	void SetIdleSoundState( void );
	void SetAngrySoundState( void );

	void BuildScheduleTestBits( void );

	void BeginNavJump( void );
	void EndNavJump( void );

	bool IsNavJumping( void ) { return m_fIsNavJumping; }
	void OnNavJumpHitApex( void );

	void BeginAttackJump( void );
	void EndAttackJump( void );

	float		MaxYawSpeed( void );

	virtual const char *GetHeadcrabClassname( void );
	virtual const char *GetHeadcrabModel( void );
	virtual const char *GetLegsModel( void );
	virtual const char *GetTorsoModel( void );

//=============================================================================
#ifdef HL2_EPISODIC

public:
	virtual bool	CreateBehaviors( void );
	virtual void	VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	virtual	void	UpdateEfficiency( bool bInPVS );
	virtual bool	IsInAVehicle( void );
	void			InputAttachToVehicle( inputdata_t &inputdata );
	void			VehicleLeapAttackTouch( CBaseEntity *pOther );

private:
	void			VehicleLeapAttack( void );
	bool			CanEnterVehicle( CPropJeepEpisodic *pVehicle );

	CAI_PassengerBehaviorZombie		m_PassengerBehavior;

#endif	// HL2_EPISODIC
//=============================================================================

protected:

	static const char *pMoanSounds[];

	// Sound stuff
	float			m_flDistFactor; 
	unsigned char	m_iClimbCount; // counts rungs climbed (for sound)
	bool			m_fIsNavJumping;
	bool			m_fIsAttackJumping;
	bool			m_fHitApex;
	mutable float	m_flJumpDist;

	bool			m_fHasScreamed;

private:
	float	m_flNextMeleeAttack;
	bool	m_fJustJumped;
	float	m_flJumpStartAltitude;
	float	m_flTimeUpdateSound;

	CSoundPatch	*m_pLayer2; // used for climbing ladders, and when jumping (pre apex)

public:
	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();
};

#endif // NPC_FASTZOMBIE_H
