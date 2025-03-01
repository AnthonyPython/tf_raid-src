#ifndef C_BASEHDTFPLAYER_H
#define C_BASEHDTFPLAYER_H

#ifdef _WIN32
#pragma once
#endif

#include "c_basehlplayer.h"
#include "colorcorrectionmgr.h"
#include "weapon_basehdtfcombat.h"
#include "hdtf_parkour_controller.h"

bool IsInEye( );

class CParkourController;
class C_HDTF_Player : public C_BaseHLPlayer
{
public:
	DECLARE_CLASS( C_HDTF_Player, C_BaseHLPlayer );
	DECLARE_CLIENTCLASS( );
	DECLARE_PREDICTABLE( );

	C_HDTF_Player( );
	~C_HDTF_Player();

	Vector EyePosition( );
	const QAngle &LocalEyeAngles( );

	const Vector &GetLocalEyeOffset( ) const;
	const QAngle &GetLocalEyeAngles( ) const;

	void SetLocalEyeOffset( const Vector &viewOffset );
	void SetLocalEyeAngles( const QAngle &viewAngles );

	virtual void			CalcPlayerView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);

	void ApplyGasMask( );
	void RemoveGasMask( );
	bool IsGasMaskActive( ) const;
	bool CanSwitchGasMask( ) const;

	void ApplyNightVision( );
	void RemoveNightVision( );
	bool IsNightVisionActive( ) const;
	bool CanSwitchNightVision( ) const;

	void Weapon_SetLast(CBaseCombatWeapon *pWeapon);
	bool Weapon_ShouldSetLast( CBaseCombatWeapon *pOldWeapon, CBaseCombatWeapon *pNewWeapon );

	void SetBinocularsActive( bool state ) { m_bIsBinocularsActive = state; };
	bool IsBinocularsActive() const;

	C_BaseCombatWeapon *GetWeaponByName( const char *name ) const;

	const Vector &GetRenderOrigin( );
	int DrawModel( int flags );

	void AddEntity( );

	bool IsWalking( ) const;

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

	void AdjustMouseSensitivity( float &sensitivity );

	void OnDataChanged(DataUpdateType_t updateType);

	void ClientThink();

	void UpdateFlashlight();
	void Flashlight();

	void CreateNightVisionLight(const Vector *overridePos = NULL, const Quaternion *overrideQuat = NULL);
	void RemoveNightVisionLight();

	void UpdateWeaponLight(C_BaseHDTFCombatWeapon *pWeapon);
	void DestroyWeaponLight();

	int GetArmsType() const;

	int GetIDTarget( ) const;
	void UpdateIDTarget( );

	int GetWeaponCount();

	bool IsInventoryEnabled();

	static C_HDTF_Player *GetLocalHDTFPlayer( );

	void ShowInventory() { m_bDesiredInventoryState = true; }
	void HideInventory() { m_bDesiredInventoryState = false; }

	// Tottery: for blur
	void UpdateIronsightDistanceFraction();
	float IronsightDistanceFraction() { return m_flIronsightDistanceFraction; };

protected:
	void UpdateInventoryState();

public:
	CNetworkVarEmbedded(CParkourController, m_pParkourController);

private:
	C_HDTF_Player( const C_HDTF_Player & );

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
	CNetworkVar( bool, m_bSliding );
	CNetworkVar( float, m_flSlidingVelocity );
	CNetworkVar( bool, m_fIsWalking );
	CNetworkVar( bool, m_bIsBinocularsActive );
	CNetworkVar( int, m_iArmsType );
	CNetworkVar( bool, m_bIsInventoryEnabled );

	ClientShadowHandle_t m_hNightVisionLight;
	ClientShadowHandle_t m_hWeaponLight;

	ClientCCHandle_t m_hNightVisionCC;

	bool m_bDesiredInventoryState;

	float m_flIronsightDistanceFraction;
public:
	CNetworkVar( GrabState, m_eGrabbyHandsState );

	friend class CHDTFGameMovement;

	int m_iIDEntIndex;
	int m_iOldArmsType;
};

inline C_HDTF_Player *ToHDTFPlayer( CBaseEntity *pEntity )
{
	if( pEntity == NULL || !pEntity->IsPlayer( ) )
		return NULL;

	Assert( dynamic_cast<C_HDTF_Player *>( pEntity ) != NULL );
	return static_cast<C_HDTF_Player *>( pEntity );
}

#endif // C_BASEHDTFPLAYER_H
