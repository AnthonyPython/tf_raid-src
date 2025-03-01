#include "hl_gamemovement.h"
#include "convar.h"
#include "hdtf_gamerules.h"
#include "hdtf_player_shared.h"

enum ViewAnimationType
{
	VIEW_ANIM_LINEAR_Z_ONLY,
	VIEW_ANIM_SPLINE_Z_ONLY,
	VIEW_ANIM_EXPONENTIAL_Z_ONLY,
};

//-----------------------------------------------------------------------------
// Purpose: HDTF specific movement code
//-----------------------------------------------------------------------------
class CHDTFGameMovement : public CHL2GameMovement
{
public:
	DECLARE_CLASS( CHDTFGameMovement, CHL2GameMovement );

	CHDTFGameMovement( );
	~CHDTFGameMovement( );

	void SetPlayerSpeed( );

	void ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMove );
	bool CheckJumpButton( );
	void WalkMove( );
	void CheckParameters( );
	void CheckFalling( );

	// Ducking
	void Duck( );
	void FinishDuck( );
	bool CanUnduck( );
	void HandleDuckingSpeedCrop( );

	// Prone
	enum UnproneResult_t
	{
		UNPRONE_RESULT_STUCK,
		UNPRONE_RESULT_DUCK,
		UNPRONE_RESULT_STAND,
	};

	void SetProneEyeOffset( float proneFraction );
	void FinishProne( );
	void FinishUnProne( );
	UnproneResult_t CanUnprone( );
	Vector ComputeUnproneOrigin(bool bForceDuck);

	Vector GetPlayerMins( ) const; // uses local player
	Vector GetPlayerMaxs( ) const; // uses local player

	CHDTF_Player *m_pHDTFPlayer;
	bool m_bUnProneToDuck;

protected:
	void HandleLeaning( );
	void HandleSliding( );
	void PlayerMove( );
};
