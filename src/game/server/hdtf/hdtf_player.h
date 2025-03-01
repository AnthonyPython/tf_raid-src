#ifndef HDTF_PLAYER_H
#define HDTF_PLAYER_H

#ifdef _WIN32
#pragma once
#endif

#include "hl2_player.h"
#include "singleplayer_animstate.h"
#include "hdtf_parkour_controller.h"

class CHDTF_Player : public CHL2_Player
{
public:
	DECLARE_CLASS( CHDTF_Player, CHL2_Player );
	DECLARE_SERVERCLASS( );
	DECLARE_DATADESC( );

	CHDTF_Player( );
	~CHDTF_Player( );

	void Precache( );

	void Spawn( );

	void PostThink();

	void PreThink();

	bool PassesDamageFilter( const CTakeDamageInfo &info );

	void SetArmorValue( int value );
	void IncrementArmorValue( int nCount, int nMaxValue );

	Vector EyePosition( );
	const QAngle &LocalEyeAngles( );

	const Vector &GetLocalEyeOffset( ) const;
	const QAngle &GetLocalEyeAngles( ) const;

	void SetLocalEyeOffset( const Vector &viewOffset );
	void SetLocalEyeAngles( const QAngle &viewAngles );

	void ApplyGasMask( );
	void RemoveGasMask( );
	bool IsGasMaskActive( ) const;
	bool CanSwitchGasMask( ) const;

	void ApplyNightVision( );
	void RemoveNightVision( );
	bool IsNightVisionActive( ) const;
	bool CanSwitchNightVision( ) const;

	bool CanFastNightvision() const { return m_iFastNVGState == EFastNVGState::NONE; }
	void FastNightvision();
	void FastNightvisionThink();

	void SetBinocularsActive(bool state) { m_bIsBinocularsActive = state; };
	bool IsBinocularsActive() const;

	void Weapon_SetLast(CBaseCombatWeapon *pWeapon);
	bool Weapon_ShouldSetLast( CBaseCombatWeapon *pOldWeapon, CBaseCombatWeapon *pNewWeapon );
	void Weapon_Equip( CBaseCombatWeapon *pWeapon );

	CBaseCombatWeapon *GetWeaponByName( const char *name ) const;

	void SetAnimation( PLAYER_ANIM playerAnim );

	int GiveAmmo( int nCount, int nAmmoIndex, bool bSuppressSound );
	void PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize );

	bool CanSprint( ) const;

	bool CanChangePosition( ) const;

	bool CanProne( ) const;
	bool CanUnProne( ) const;
	bool IsProne( ) const;
	bool IsProning( ) const;
	bool IsGoingProne( ) const;
	bool IsGettingUpFromProne( ) const;
	void StartGoingProne( );
	void StandUpFromProne( );

	float GetLeaningPercentage( ) const;
	Vector GetLeaningVector( bool right, float perc = 1.0f ) const;
	bool CanLean( bool right ) const;

	bool CanSlide( ) const;
	bool IsSliding( ) const;

	void FastSwitchToParkour(bool bInstant);
	void WeaponHolsterThink();

	int GetArmsType() const;
	void SetArmsType(int iType = 0) { m_iArmsType = iType; }

	void UpdateClientData();
	void UpdatePlayerModel();
	virtual void OnRestore();

	void HandleAdmireAnim();
	void InputPlayAdmireAnimation( inputdata_t &inputdata );

	// inputs for the game-end sequence, where player should
	// have 200 max health
	void InputEnterFinalStage( inputdata_t &inputdata );
	void InputExitFinalStage( inputdata_t &inputdata );

	bool IsInFinalStage() { return m_bInFinalStage; }

	void RemoveAllItems(bool removeSuit, bool removeParkour);

	int GetWeaponCount();

	bool IsInventoryEnabled();
	void SetInventoryEnabled(const bool isEnabled) { m_bIsInventoryEnabled = isEnabled; }

	virtual void PlayerUse();

public:
	CNetworkVarEmbedded(CParkourController, m_pParkourController);

private:
	CHDTF_Player( const CHDTF_Player & );

	CNetworkVector( m_vLocalViewOffset );
	CNetworkQAngle( m_vLocalViewAngles );
	CNetworkVar( float, m_fJumpAllowTime );
	CNetworkVar( bool, m_bGasMaskActive );
	CNetworkVar( bool, m_bNightVisionActive );
	CNetworkVar( bool, m_bProne );
	CNetworkVar( float, m_flUnProneTime );
	CNetworkVar( float, m_flGoProneTime );
	float m_flNextProneCheck;
	CNetworkVar( LEANING_STATE, m_eLeaningState );
	CNetworkVar( float, m_fLeaningStart );
	CSinglePlayerAnimState *m_pPlayerAnimState;
	QAngle m_angEyeAngles;
	Vector m_lastStandingPos;
	CNetworkVar( bool, m_bSliding );
	CNetworkVar( float, m_flSlidingVelocity );

	CNetworkVar(bool, m_bIsBinocularsActive);

	CNetworkVar(int, m_iArmsType);

	CNetworkVar( bool, m_bIsInventoryEnabled );

	bool m_bInFinalStage;

	bool m_bHolsteringWeapon;

	enum class EFastNVGState : int
	{
		NONE,
		WAITING_FOR_NVG_DEPLOY,
		NVG_ACTING,
		SWITCHING_TO_PREV_WEAPON,
	};
	EFastNVGState m_iFastNVGState;
	bool m_bFastNVGMustSwitchAway;

	float m_flAdmireAnimStart;
	float m_flAdmireAnimTime;

public:
	friend class CHDTFGameMovement;
	friend class CBasePlayer;
};

inline CHDTF_Player *ToHDTFPlayer( CBaseEntity *pEntity )
{
	if( pEntity == NULL || !pEntity->IsPlayer( ) )
		return NULL;

	Assert( dynamic_cast<CHDTF_Player *>( pEntity ) != NULL );
	return static_cast<CHDTF_Player *>( pEntity );
}

#endif //HDTF_PLAYER_H
