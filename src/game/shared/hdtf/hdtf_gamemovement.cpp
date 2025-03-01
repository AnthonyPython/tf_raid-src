#include "cbase.h"
#include "hdtf_gamemovement.h"
#include "in_buttons.h"
#include "movevars_shared.h"
#include "hdtf_player_shared.h"
#include "hdtf_gamerules.h"
#include "takedamageinfo.h"
#include "weapon_basehdtfcombat.h"
#include "weapon_parkour.h"

#if defined( CLIENT_DLL ) && defined( HDTF_SINGLEPLAYER )

#include "input.h"

#endif

extern ConVar hdtf_pronespeed;
extern ConVar hdtf_walkspeed;
extern ConVar hdtf_normspeed;
extern ConVar hdtf_sprintspeed;

#ifdef HDTF_SINGLEPLAYER

#define PLAYER_SPEED_PRONE hdtf_pronespeed.GetFloat( )
#define PLAYER_SPEED_WALK hdtf_walkspeed.GetFloat( )
#define PLAYER_SPEED_RUN hdtf_normspeed.GetFloat( )
#define PLAYER_SPEED_SPRINT hdtf_sprintspeed.GetFloat( )

#else

#define PLAYER_SPEED_PRONE 50
#define PLAYER_SPEED_WALK 82
#define PLAYER_SPEED_RUN 190
#define PLAYER_SPEED_SPRINT 320

#endif

ConVar hdtf_disable_sliding("hdtf_disable_sliding", "1", FCVAR_ARCHIVE | FCVAR_REPLICATED);

extern bool g_bMovementOptimizations;

// Camera Bob
static ConVar cl_viewbob_enabled( "cl_viewbob_enabled", "0", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Toggle oscillation", true, 0, true, 1 );
static ConVar cl_viewbob_intensity("cl_viewbob_intensity", "1.0", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Scales the strength of viewbob effect", true, 0.f, true, 1.f);

// Walk
static ConVar cl_viewbob_timer( "cl_viewbob_timer", "3", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Speed of oscillation" );
static ConVar cl_viewbob_scale_x( "cl_viewbob_scale_x", "0.0", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Magnitude of oscillation on the X axis" );
static ConVar cl_viewbob_scale_y( "cl_viewbob_scale_y", "0.01", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Magnitude of oscillation on the Y axis" );
static ConVar cl_viewbob_scale_z( "cl_viewbob_scale_z", "0.03", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Magnitude of oscillation on the Z axis" );

// Sprint
static ConVar cl_viewbob_timer_sprint( "cl_viewbob_timer_sprint", "7.5", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Speed of oscillation while sprinting" );
static ConVar cl_viewbob_scale_x_sprint( "cl_viewbob_scale_x_sprint", "0.0", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Magnitude of oscillation on the X axis while sprinting" );
static ConVar cl_viewbob_scale_y_sprint( "cl_viewbob_scale_y_sprint", "0.025", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Magnitude of oscillation on the Y axis while sprinting" );
static ConVar cl_viewbob_scale_z_sprint( "cl_viewbob_scale_z_sprint", "0.0525", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Magnitude of oscillation on the Z axis while sprinting" );

// Low health multiplier
static ConVar cl_viewbob_lowhealth_lerp( "cl_viewbob_lowhealth_lerp", "1", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Lerp the viewbob multiplier rather than snap to it" );
static ConVar cl_viewbob_lowhealth_scaler( "cl_viewbob_lowhealth_scaler", "4", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Multiply viewbob values by this when under a health threshold. (lerps from health to zero)" );
static ConVar cl_viewbob_lowhealth_percentage( "cl_viewbob_lowhealth_percentage", "25", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Health value to lerp viewbob multiplier from" );

static ConVar sv_jump_cooldown_enabled( "sv_jump_cooldown_enabled", "1", FCVAR_CHEAT | FCVAR_REPLICATED | FCVAR_ARCHIVE, "Prevent bunny-hopping by putting the jump on a cooldown" );
// NOTE: I maintain that this is a bad idea; speedrunning can keep a game in the public conciousness long after release,
//		 and this just hampers them. I'll implement it anyway, but this cvar above will disable the behavior. -sam 19/08/2016

static ConVar sv_jump_cooldown_timer( "sv_jump_cooldown_timer", "0.1", FCVAR_CHEAT | FCVAR_REPLICATED | FCVAR_ARCHIVE, "How long after landing before we can jump again?" );
// TODO: Play with this variable until it feels right; enough to stop bhops but not enough to become restrictive to normal players.

static ConVar sv_jump_cooldown_debug( "sv_jump_cooldown_debug", "0", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED, "Debug the jump cooldown system" );

static ConVar cl_show_speed( "cl_show_speed", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "spam console with local player speed" );

static ConVar hdtf_auto_crouch_jump("hdtf_auto_crouch_jump", "0", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Enables automatic crouch-jumping");

// Expose our interface.
static CHDTFGameMovement g_GameMovement;
IGameMovement *g_pGameMovement = (IGameMovement *)&g_GameMovement;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CHDTFGameMovement, IGameMovement, INTERFACENAME_GAMEMOVEMENT, g_GameMovement );

// ---------------------------------------------------------------------------------------- //
// CHDTFGameMovement.
// ---------------------------------------------------------------------------------------- //

CHDTFGameMovement::CHDTFGameMovement( )
{
	// Don't set any member variables here, or you'll get an access
	// violation exception on LoadLibrary, and will have to stay up til 
	// 3 in the morning figuring out where you did bad things.

	//m_bUnProneToDuck = false;
}

CHDTFGameMovement::~CHDTFGameMovement( )
{ }

void CHDTFGameMovement::SetPlayerSpeed( )
{
	if( m_pHDTFPlayer->IsProne( ) &&
		!m_pHDTFPlayer->IsGettingUpFromProne( ) &&
		m_pHDTFPlayer->GetGroundEntity( ) != NULL )
	{
		mv->m_flClientMaxSpeed = PLAYER_SPEED_PRONE; //Base prone speed 
	}
	else //not prone, dead or deployed - standing or crouching and possibly moving
	{
		CBaseHDTFCombatWeapon *pWeapon = 
			dynamic_cast<CBaseHDTFCombatWeapon *>(m_pHDTFPlayer->GetActiveWeapon());

		if( ( mv->m_nButtons & IN_DUCK ) != 0 )
		{
			mv->m_flClientMaxSpeed = PLAYER_SPEED_RUN; //gets cut in fraction later
		}
		else if( ( mv->m_nButtons & IN_WALK ) != 0 
			|| (pWeapon != NULL && pWeapon->IsIronsightsEnabled()) && m_pHDTFPlayer->GetGroundEntity() != NULL )
		{
			mv->m_flClientMaxSpeed = PLAYER_SPEED_WALK;
		}
		else
		{
			float flMaxSpeed = PLAYER_SPEED_RUN; //jogging

			float stamina = m_pHDTFPlayer->m_HL2Local.m_flSuitPower;
			if( ( mv->m_nButtons & IN_SPEED ) != 0 && stamina > 0.0f && ( mv->m_nButtons & IN_FORWARD ) != 0 )
				flMaxSpeed = PLAYER_SPEED_SPRINT; //sprinting

			mv->m_flClientMaxSpeed = flMaxSpeed - 100.0f + stamina;
		}
	}

	if( m_pHDTFPlayer->GetGroundEntity( ) != NULL )
	{
		if( m_pHDTFPlayer->IsGoingProne( ) )
		{
			float maxSpeed = mv->m_flClientMaxSpeed;
			if( m_bUnProneToDuck )
				maxSpeed *= 0.33f;

			//interp to prone speed
			float flProneFraction = SimpleSpline( ( m_pHDTFPlayer->m_flGoProneTime - gpGlobals->curtime ) / TIME_TO_PRONE );

			mv->m_flClientMaxSpeed = ( 1.0f - flProneFraction ) * PLAYER_SPEED_PRONE + flProneFraction * maxSpeed;
		}
		else if( m_pHDTFPlayer->IsGettingUpFromProne( ) )
		{
			float maxSpeed = mv->m_flClientMaxSpeed;
			if( m_bUnProneToDuck )
				maxSpeed *= 0.33f;

			//interp to regular speed speed
			float flProneFraction = SimpleSpline( ( m_pHDTFPlayer->m_flUnProneTime - gpGlobals->curtime ) / TIME_TO_PRONE );

			mv->m_flClientMaxSpeed = flProneFraction * PLAYER_SPEED_PRONE + ( 1.0f - flProneFraction ) * maxSpeed;
		}
	}
}

void CHDTFGameMovement::CheckParameters( )
{
	SetPlayerSpeed( );

	BaseClass::CheckParameters( );

	if( cl_show_speed.GetBool( ) )
		Msg( "player speed %.1f\n", m_pHDTFPlayer->GetAbsVelocity( ).Length2DSqr( ) );
}

void CHDTFGameMovement::CheckFalling( )
{
	if( player->GetGroundEntity( ) != NULL && player->m_Local.m_flFallVelocity > 0.0f )
	{
		// Set our jump cooldown time here! Set this even if the mode is disabled (by cheat)
		m_pHDTFPlayer->m_fJumpAllowTime = gpGlobals->curtime + sv_jump_cooldown_timer.GetFloat( );
		if( sv_jump_cooldown_debug.GetBool( ) )
			DevMsg( "Player jump cooldown reset, player can jump again in %f secs\n", sv_jump_cooldown_timer.GetFloat( ) );
	}

	// if we landed on the ground
	if( player->GetGroundEntity( ) != NULL && !IsDead( ) && player->m_Local.m_flFallVelocity >= 200.0f )
	{
		CPASFilter filter( player->GetAbsOrigin( ) );
		filter.UsePredictionRules( );
		player->EmitSound( filter, player->entindex( ), "Player.JumpLanding" );
		player->ViewPunch(QAngle(5, 0, 0));
	}

	BaseClass::CheckFalling( );
}

void CHDTFGameMovement::ProcessMovement( CBasePlayer *pBasePlayer, CMoveData *pMove )
{
	//Store the player pointer
	m_pHDTFPlayer = ToHDTFPlayer( pBasePlayer );
	Assert( m_pHDTFPlayer != NULL );

	BaseClass::ProcessMovement( pBasePlayer, pMove );
}

void CHDTFGameMovement::HandleLeaning( )
{
	LEANING_STATE leaning = m_pHDTFPlayer->m_eLeaningState;
	int buttons = player->m_nButtons;

	Vector eyeOrigin = m_pHDTFPlayer->GetLocalEyeOffset( );
	QAngle eyeAngles = m_pHDTFPlayer->GetLocalEyeAngles( );

	bool nolean = m_pHDTFPlayer->IsProning( );

	if (player->GetActiveWeapon() && player->GetActiveWeapon()->ClassMatches("weapon_parkour"))
		nolean = true;

	switch( leaning )
	{
	case LEANING_NONE:
		if( nolean )
			break;

		if( ( buttons & IN_LEANLEFT ) != 0 && m_pHDTFPlayer->CanLean( false ) )
		{
			m_pHDTFPlayer->m_eLeaningState = LEANING_LEFT;
			m_pHDTFPlayer->m_fLeaningStart = gpGlobals->curtime;
		}
		else if( ( buttons & IN_LEANRIGHT ) != 0 && m_pHDTFPlayer->CanLean( true ) )
		{
			m_pHDTFPlayer->m_eLeaningState = LEANING_RIGHT;
			m_pHDTFPlayer->m_fLeaningStart = gpGlobals->curtime;
		}

		break;

	case LEANING_LEFT:
	case LEANING_FULLY_LEFT:
		if( ( buttons & IN_LEANLEFT ) == 0 || nolean || !m_pHDTFPlayer->CanLean( false ) )
		{
			m_pHDTFPlayer->m_eLeaningState = LEANING_BACK_LEFT;

			if( leaning == LEANING_FULLY_LEFT )
				m_pHDTFPlayer->m_fLeaningStart = gpGlobals->curtime;
			else
				m_pHDTFPlayer->m_fLeaningStart = gpGlobals->curtime - TIME_TO_LEAN * 0.5f;
		}
		else if( leaning == LEANING_LEFT )
		{
			if( gpGlobals->curtime >= m_pHDTFPlayer->m_fLeaningStart + TIME_TO_LEAN )
			{
				m_pHDTFPlayer->m_eLeaningState = LEANING_FULLY_LEFT;
				m_pHDTFPlayer->m_fLeaningStart = 0.0f;

				eyeOrigin = m_pHDTFPlayer->GetLeaningVector( false );
				eyeAngles.z = -LEANING_DEGREES;
			}
			else
			{
				float perc = m_pHDTFPlayer->GetLeaningPercentage( );
				eyeOrigin = m_pHDTFPlayer->GetLeaningVector( false, perc );
				eyeAngles.z = -LEANING_DEGREES * perc;
			}
		}

		break;

	case LEANING_BACK_LEFT:
		if( ( buttons & IN_LEANLEFT ) != 0 && !nolean && m_pHDTFPlayer->CanLean( false ) )
		{
			m_pHDTFPlayer->m_eLeaningState = LEANING_LEFT;
			m_pHDTFPlayer->m_fLeaningStart = gpGlobals->curtime - TIME_TO_LEAN * 0.5f;
		}
		else if( gpGlobals->curtime >= m_pHDTFPlayer->m_fLeaningStart + TIME_TO_LEAN )
		{
			m_pHDTFPlayer->m_eLeaningState = LEANING_NONE;
			m_pHDTFPlayer->m_fLeaningStart = 0.0f;

			eyeOrigin.Init( );
			eyeAngles.z = 0.0f;
		}
		else
		{
			float perc = 1.0f - m_pHDTFPlayer->GetLeaningPercentage( );
			eyeOrigin = m_pHDTFPlayer->GetLeaningVector( false, perc );
			eyeAngles.z = -LEANING_DEGREES * perc;
		}

		break;

	case LEANING_RIGHT:
	case LEANING_FULLY_RIGHT:
		if( ( buttons & IN_LEANRIGHT ) == 0 || nolean || !m_pHDTFPlayer->CanLean( true ) )
		{
			m_pHDTFPlayer->m_eLeaningState = LEANING_BACK_RIGHT;

			if( leaning == LEANING_FULLY_RIGHT )
				m_pHDTFPlayer->m_fLeaningStart = gpGlobals->curtime;
			else
				m_pHDTFPlayer->m_fLeaningStart = gpGlobals->curtime - TIME_TO_LEAN * 0.5f;
		}
		else if( leaning == LEANING_RIGHT )
		{
			if( gpGlobals->curtime >= m_pHDTFPlayer->m_fLeaningStart + TIME_TO_LEAN )
			{
				m_pHDTFPlayer->m_eLeaningState = LEANING_FULLY_RIGHT;
				m_pHDTFPlayer->m_fLeaningStart = 0.0f;

				eyeOrigin = m_pHDTFPlayer->GetLeaningVector( true );
				eyeAngles.z = LEANING_DEGREES;
			}
			else
			{
				float perc = m_pHDTFPlayer->GetLeaningPercentage( );
				eyeOrigin = m_pHDTFPlayer->GetLeaningVector( true, perc );
				eyeAngles.z = LEANING_DEGREES * perc;
			}
		}

		break;

	case LEANING_BACK_RIGHT:
		if( ( buttons & IN_LEANRIGHT) != 0 && !nolean && m_pHDTFPlayer->CanLean( true ) )
		{
			m_pHDTFPlayer->m_eLeaningState = LEANING_RIGHT;
			m_pHDTFPlayer->m_fLeaningStart = gpGlobals->curtime - TIME_TO_LEAN * 0.5f;
		}
		else if( gpGlobals->curtime >= m_pHDTFPlayer->m_fLeaningStart + TIME_TO_LEAN )
		{
			m_pHDTFPlayer->m_eLeaningState = LEANING_NONE;
			m_pHDTFPlayer->m_fLeaningStart = 0.0f;

			eyeOrigin.Init( );
			eyeAngles.z = 0.0f;
		}
		else
		{
			float perc = 1.0f - m_pHDTFPlayer->GetLeaningPercentage( );
			eyeOrigin = m_pHDTFPlayer->GetLeaningVector( true, perc );
			eyeAngles.z = LEANING_DEGREES * perc;
		}

		break;
	}

	m_pHDTFPlayer->SetLocalEyeOffset( eyeOrigin );
	m_pHDTFPlayer->SetLocalEyeAngles( eyeAngles );
}

void CHDTFGameMovement::HandleSliding()
{
	if (hdtf_disable_sliding.GetBool())
		return;

	if ((player->m_nButtons & IN_FORWARD) != 0 &&
		(player->m_nButtons & IN_DUCK) != 0 &&
		player->GetGroundEntity() != NULL &&
		player->GetWaterLevel() <= 1 &&
		player->GetAbsVelocity().LengthSqr() >= 150.0f * 150.0f)
	{
		if (!m_pHDTFPlayer->IsSliding())
		{
			m_pHDTFPlayer->m_bSliding = true;
			m_pHDTFPlayer->m_flSlidingVelocity = 70.0f * Clamp(player->GetAbsVelocity().Length() / 500.0f, 0.0f, 1.0f);
		}

		QAngle angle(0.0f, player->EyeAngles().y, 0.0f);
		Vector dir;
		AngleVectors(angle, &dir);

		Vector pos = player->GetAbsOrigin() + Vector(0.0f, 0.0f, 45.0f);
		trace_t tr;
		UTIL_TraceLine(pos, pos + dir * 35.0f, MASK_SOLID, player, COLLISION_GROUP_NONE, &tr);
		trace_t tr_up;
		UTIL_TraceLine(pos - Vector(0.0f, 0.0f, 40.0f), pos - Vector(0.0f, 0.0f, 43.0f) + dir * 25.0f, MASK_SOLID, player, COLLISION_GROUP_NONE, &tr_up);
		trace_t tr_down;
		UTIL_TraceLine(pos - Vector(0.0f, 0.0f, 40.0f), pos - Vector(0.0f, 0.0f, 48.0f) + dir * 25.0f, MASK_SOLID, player, COLLISION_GROUP_NONE, &tr_down);

		if (!tr_down.DidHit())
			m_pHDTFPlayer->m_flSlidingVelocity = Approach(100.0f, m_pHDTFPlayer->m_flSlidingVelocity, 0.5f);
		else
			m_pHDTFPlayer->m_flSlidingVelocity = Approach(0.0f, m_pHDTFPlayer->m_flSlidingVelocity, 0.5f);

		if (tr_up.DidHit())
			m_pHDTFPlayer->m_flSlidingVelocity = Approach(0.0f, m_pHDTFPlayer->m_flSlidingVelocity, 2.0f);

		if (tr.DidHit() && m_pHDTFPlayer->m_flSlidingVelocity > 0.0f)
		{

#ifndef CLIENT_DLL

			/*if( tr.DidHitNonWorldEntity( ) && ( tr.m_pEnt->IsPlayer( ) || tr.m_pEnt->IsNPC( ) ) )
			{
				CTakeDamageInfo dmg( player, player, 70.0f * Clamp( player->GetAbsVelocity( ).Length( ) / 1000.0f, 0.0f, 1.0f ), DMG_CRUSH );
				tr.m_pEnt->TakeDamage( dmg );
			}*/

#endif

			player->ViewPunch(QAngle(m_pHDTFPlayer->m_flSlidingVelocity / 5.0f, 0.3f, -1.0f));
			m_pHDTFPlayer->m_flSlidingVelocity = 0.0f;
			return;
		}

		player->ViewPunch(QAngle(0.0f, 0.3f, -1.0f));
		mv->m_vecVelocity += dir * m_pHDTFPlayer->m_flSlidingVelocity;
	}
	else if (m_pHDTFPlayer->IsSliding())
	{
		m_pHDTFPlayer->m_bSliding = false;
		m_pHDTFPlayer->m_flSlidingVelocity = 0.0f;
	}
}

void CHDTFGameMovement::PlayerMove( )
{
	BaseClass::PlayerMove( );

	if( player->GetMoveType( ) != MOVETYPE_ISOMETRIC &&
		player->GetMoveType( ) != MOVETYPE_NOCLIP &&
		player->GetMoveType( ) != MOVETYPE_OBSERVER )
	{
		// Cap actual player speed, fix wall running
		float spd = mv->m_vecVelocity[0] * mv->m_vecVelocity[0] + mv->m_vecVelocity[1] * mv->m_vecVelocity[1];

		if( spd > 0.0f && spd > mv->m_flMaxSpeed * mv->m_flMaxSpeed )
		{
			float fRatio = mv->m_flMaxSpeed / sqrt( spd );
			mv->m_vecVelocity[0] *= fRatio;
			mv->m_vecVelocity[1] *= fRatio;
		}
	}

	HandleLeaning( );

	HandleSliding( );
}

void CHDTFGameMovement::WalkMove( )
{
	if( cl_viewbob_enabled.GetBool( ) && !engine->IsPaused( ) )
	{
		CHLMoveData *pMoveData = static_cast<CHLMoveData *>( mv );
		CBaseHDTFCombatWeapon *pWeapon = dynamic_cast<CBaseHDTFCombatWeapon *>(player->GetActiveWeapon());
		Vector vel = player->GetAbsVelocity();
		bool bSprinting = pMoveData->m_bIsSprinting && (!pWeapon || !pWeapon->IsIronsightsEnabled());
		float fCurTime = gpGlobals->curtime,
			fTimer = bSprinting ? cl_viewbob_timer_sprint.GetFloat( ) : cl_viewbob_timer.GetFloat( ),
			fSpeed = vel.Length( ),
			fSine = sin( 2.0f * fCurTime * fTimer ) * fSpeed,

			fScaleX = bSprinting ? cl_viewbob_scale_x_sprint.GetFloat( ) : cl_viewbob_scale_x.GetFloat( ),
			fScaleY = bSprinting ? cl_viewbob_scale_y_sprint.GetFloat( ) : cl_viewbob_scale_y.GetFloat( ),
			fScaleZ = bSprinting ? cl_viewbob_scale_z_sprint.GetFloat( ) : cl_viewbob_scale_z.GetFloat( ),
			
			fMultiplier = 0.0f;

		if( !bSprinting )
		{
			float fPlayerHP = player->GetHealth( ),
				fLowHealthPerc = cl_viewbob_lowhealth_percentage.GetFloat( ),
				fScaler = cl_viewbob_lowhealth_scaler.GetFloat( );

			fMultiplier = Clamp( cl_viewbob_lowhealth_lerp.GetBool( ) ?
				Lerp( fPlayerHP / fLowHealthPerc, fScaler, 1.0f ) :
				( fLowHealthPerc > fPlayerHP ? fScaler : 1.0f ), 0.0f, fScaler );
		}
		else
			fMultiplier = cl_viewbob_lowhealth_scaler.GetFloat( );

		float fOffsetX = fSine * fScaleX / 400.0f * fMultiplier,
			fOffsetY = fSine * fScaleY / 400.0f * fMultiplier,
			fOffsetZ = fSine * fScaleZ / 400.0f * fMultiplier;

		vel.z = 0.f; // ignore falling speed

		Vector vForward, vRight, vUp;
		AngleVectors(pMoveData->m_vecAngles, &vForward, &vRight, &vUp);

		float flSpeedPerc = vel.Length() / hdtf_sprintspeed.GetFloat();

		fOffsetX += sin(fCurTime * 14.5f) * 0.035f * flSpeedPerc;

		if(bSprinting)
			fOffsetY += sin(fCurTime * 12.f) * 0.25f * flSpeedPerc;

		if (bSprinting)
			fOffsetX += 0.60f;

		const float fIntensity = cl_viewbob_intensity.GetFloat();
		fOffsetX *= fIntensity;
		fOffsetY *= fIntensity;
		fOffsetZ *= fIntensity;

		player->ViewPunch( QAngle( fOffsetX, fOffsetY, fOffsetZ ) );
	}

#ifndef CLIENT_DLL

	if( ( mv->m_nButtons & IN_SPEED ) != 0 &&
		( mv->m_nButtons & IN_FORWARD ) != 0 &&
		( mv->m_nButtons & IN_DUCK ) == 0 &&
		!m_pHDTFPlayer->IsProne( ) &&
		!player->IsDucking( ) &&
		player->GetAbsVelocity( ).Length2DSqr( ) > 80.0f * 80.0f )
	{
		if( !m_pHDTFPlayer->IsSprinting( ) )
			m_pHDTFPlayer->StartSprinting( );
	}
	else if( m_pHDTFPlayer->IsSprinting( ) )
		m_pHDTFPlayer->StopSprinting( );

#endif

	BaseClass::WalkMove( );
}

bool CHDTFGameMovement::CheckJumpButton( )
{
	if( m_pHDTFPlayer->pl.deadflag )
	{
		mv->m_nOldButtons |= IN_JUMP;	// don't jump again until released
		return false;
	}

	if( m_pHDTFPlayer->IsProning( ) )
	{
		mv->m_nOldButtons |= IN_JUMP;
		return false;
	}

	// See if we are waterjumping.  If so, decrement count and return.
	float flWaterJumpTime = player->GetWaterJumpTime( );
	if( flWaterJumpTime > 0.0f )
	{
		flWaterJumpTime -= gpGlobals->frametime;
		if( flWaterJumpTime < 0.0f )
			flWaterJumpTime = 0.0f;

		player->SetWaterJumpTime( flWaterJumpTime );

		return false;
	}

	// If we are in the water most of the way...
	if( m_pHDTFPlayer->GetWaterLevel( ) >= 2 )
	{
		// swimming, not jumping
		SetGroundEntity( NULL );

		if( m_pHDTFPlayer->GetWaterType( ) == CONTENTS_WATER )    // We move up a certain amount
			mv->m_vecVelocity[2] = 100.0f;
		else if( m_pHDTFPlayer->GetWaterType( ) == CONTENTS_SLIME )
			mv->m_vecVelocity[2] = 80.0f;

		// play swiming sound
		if( player->GetSwimSoundTime( ) <= 0.0f )
		{
			// Don't play sound again for 1 second
			player->SetSwimSoundTime( 1000.0f );
			PlaySwimSound( );
		}

		return false;
	}

	// No more effect
	if( m_pHDTFPlayer->GetGroundEntity( ) == NULL )
	{
		mv->m_nOldButtons |= IN_JUMP;
		return false;		// in air, so no effect
	}

	// Jump timer (to prevent speedrunning)
	if( sv_jump_cooldown_enabled.GetBool( ) )
	{
		if( gpGlobals->curtime < m_pHDTFPlayer->m_fJumpAllowTime )
		{
			if( sv_jump_cooldown_debug.GetBool( ) )
				DevMsg( "Jump hasn't cooled down yet!\n" );

			return false; // Cooldown hasn't expired, we can't jump. Sorry speedrunners :'C
		}
		else
		{
			if( sv_jump_cooldown_debug.GetBool( ) )
				DevMsg( "Jump cooled down, player will leap.\n" );
		}
	}

	if( ( mv->m_nOldButtons & IN_JUMP ) != 0 )
		return false; // don't pogo stick

	// In the air now.
	SetGroundEntity( NULL );

	m_pHDTFPlayer->PlayStepSound( (Vector &)mv->GetAbsOrigin( ), player->GetSurfaceData( ), 1.0, true );

	// make the jump sound
	CPASFilter filter( m_pHDTFPlayer->GetAbsOrigin( ) );
	filter.UsePredictionRules( );
	m_pHDTFPlayer->EmitSound( filter, m_pHDTFPlayer->entindex( ), "Player.Jump" );
	player->ViewPunch(QAngle(-3, 0, 0));

	float flGroundFactor = 1.0f;
	if( player->GetSurfaceData( ) != NULL )
		flGroundFactor = player->GetSurfaceData( )->game.jumpFactor;

	Assert( sv_gravity.GetFloat( ) == 800.0f );

	// Accelerate upward
	// If we are ducking...
	float startz = mv->m_vecVelocity[2];
	if( m_pHDTFPlayer->m_Local.m_bDucking || ( m_pHDTFPlayer->GetFlags( ) & FL_DUCKING ) != 0 )
		// d = 0.5 * g * t ^ 2		- distance traveled with linear accel
		// t = sqrt( 2.0 * 45 / g )	- how long to fall 45 units
		// v = g * t				- velocity at the end (just invert it to jump up that high)
		// v = g * sqrt( 2.0 * 45 / g )
		// v ^ 2 = g * g * 2.0 * 45 / g
		// v = sqrt( g * 2.0 * 45 )

		mv->m_vecVelocity[2] = flGroundFactor * 237.0f /*268.3281572999747f*/; // flJumpSpeed of 45
		//mv->m_vecVelocity[2] = flGroundFactor * sqrt( 2 * 800 * flJumpSpeed ); // 2 * gravity * height
	else
		mv->m_vecVelocity[2] += flGroundFactor * 237.0f /*268.3281572999747f*/;	// flJumpSpeed of 45
		//mv->m_vecVelocity[2] += flGroundFactor * sqrt( 2 * 800 * flJumpSpeed ); // 2 * gravity * height

	FinishGravity( );

	mv->m_outWishVel.z += mv->m_vecVelocity[2] - startz;
	mv->m_outStepHeight += 0.1f;

	// Flag that we jumped.
	mv->m_nOldButtons |= IN_JUMP;	// don't jump again until released

	if (hdtf_auto_crouch_jump.GetBool())
	{
		FinishDuck();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Limit speed if we are ducking
//-----------------------------------------------------------------------------
void CHDTFGameMovement::HandleDuckingSpeedCrop( )
{
	/*
	if( ( mv->m_nButtons & IN_DUCK ) != 0 || player->m_Local.m_bDucking || ( player->GetFlags( ) & FL_DUCKING ) != 0 )
	{
		float frac = 0.33333333f;
		mv->m_flForwardMove *= frac;
		mv->m_flSideMove *= frac;
		mv->m_flUpMove *= frac;
	}
	*/

	BaseClass::HandleDuckingSpeedCrop( );
}

CHDTFGameMovement::UnproneResult_t CHDTFGameMovement::CanUnprone( )
{
	Vector newOrigin = ComputeUnproneOrigin(false);

	bool saveprone = m_pHDTFPlayer->IsProne();
	bool saveducked = player->IsDucked();

	// pretend we're not prone
	m_pHDTFPlayer->m_bProne = false;
	if ((mv->m_nButtons & IN_DUCK) != 0)
		player->m_Local.m_bDucked = true;

	trace_t trace;
	TracePlayerBBox(mv->GetAbsOrigin(), newOrigin, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);

	// NOTE(wheatley): we cannot unprone to standing and we're not holding the duck key
	// try unproning into ducked state before considering it impossible
	bool bUnproneToDuck = false;
	if (((mv->m_nButtons & IN_DUCK) == 0) && (trace.startsolid || trace.fraction != 1.f))
	{
		player->m_Local.m_bDucked = true;
		newOrigin = ComputeUnproneOrigin(true);
		TracePlayerBBox(mv->GetAbsOrigin(), newOrigin, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);
		bUnproneToDuck = true;
	}

	// revert to reality
	m_pHDTFPlayer->m_bProne = saveprone;
	player->m_Local.m_bDucked = saveducked;

	if( trace.startsolid || trace.fraction != 1.0f )
		return CHDTFGameMovement::UnproneResult_t::UNPRONE_RESULT_STUCK;

	if(bUnproneToDuck)
		return CHDTFGameMovement::UnproneResult_t::UNPRONE_RESULT_DUCK;

	return CHDTFGameMovement::UnproneResult_t::UNPRONE_RESULT_STAND;
}

Vector CHDTFGameMovement::ComputeUnproneOrigin(bool bForceDuck)
{
	Vector newOrigin;
	VectorCopy(mv->GetAbsOrigin(), newOrigin);

	Vector vecMins, vecMaxs;
	if ((mv->m_nButtons & IN_DUCK) != 0 || bForceDuck)
	{
		vecMins = VEC_DUCK_HULL_MIN;
		vecMaxs = VEC_DUCK_HULL_MAX;
	}
	else
	{
		vecMins = VEC_HULL_MIN;
		vecMaxs = VEC_HULL_MAX;
	}

	if (player->GetGroundEntity() != NULL)
	{
		newOrigin += VEC_PRONE_HULL_MIN - vecMins;
	}
	else
	{
		// If in air an letting go of crouch, make sure we can offset origin to make
		//  up for uncrouching

		Vector hullSizeNormal = vecMaxs - vecMins;
		Vector hullSizeProne = VEC_PRONE_HULL_MAX - VEC_PRONE_HULL_MIN;
		Vector viewDelta = -0.5f * (hullSizeNormal - hullSizeProne);

		VectorAdd(newOrigin, viewDelta, newOrigin);
	}

	return newOrigin;
}

bool CHDTFGameMovement::CanUnduck( )
{
	/*
	trace_t trace;
	Vector newOrigin;

	VectorCopy( mv->GetAbsOrigin( ), newOrigin );

	if( player->GetGroundEntity( ) != NULL )
	{
		newOrigin += VEC_DUCK_HULL_MIN - VEC_HULL_MIN;
	}
	else
	{
		// If in air an letting go of croush, make sure we can offset origin to make
		//  up for uncrouching
		Vector hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
		Vector hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;

		Vector viewDelta = -0.5f * ( hullSizeNormal - hullSizeCrouch );

		VectorAdd( newOrigin, viewDelta, newOrigin );
	}

	UTIL_TraceHull( mv->GetAbsOrigin( ), newOrigin, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, player, COLLISION_GROUP_PLAYER_MOVEMENT, &trace );

	if( trace.startsolid || ( trace.fraction != 1.0f ) )
		return false;

	return true;
	*/

	if (hdtf_auto_crouch_jump.GetBool() && mv->m_vecVelocity.z > 0.f)
	{
		return false;
	}

	return BaseClass::CanUnduck( );
}

//-----------------------------------------------------------------------------
// Purpose: Finish ducking
//-----------------------------------------------------------------------------
void CHDTFGameMovement::FinishDuck( )
{
	//Fix quantum crouch --AlexEpisode
	/*if (player->GetFlags() & FL_DUCKING)
		return;
	*/

	player->AddFlag(FL_DUCKING);
	player->m_Local.m_bDucked = true;
	player->m_Local.m_bDucking = false;

	player->SetViewOffset(GetPlayerViewOffset(true));

	// HACKHACK - Fudge for collision bug - no time to fix this properly
	if (player->GetGroundEntity() != NULL)
	{
		for (int i = 0; i < 3; i++)
		{
			Vector org = mv->GetAbsOrigin();
			org[i] -= (VEC_DUCK_HULL_MIN_SCALED(player)[i] - VEC_HULL_MIN_SCALED(player)[i]);
			mv->SetAbsOrigin(org);
		}
	}
	else
	{
		Vector hullSizeNormal = VEC_HULL_MAX_SCALED(player) - VEC_HULL_MIN_SCALED(player);
		Vector hullSizeCrouch = VEC_DUCK_HULL_MAX_SCALED(player) - VEC_DUCK_HULL_MIN_SCALED(player);
		Vector viewDelta = (hullSizeNormal - hullSizeCrouch);
		Vector out;
		VectorAdd(mv->GetAbsOrigin(), viewDelta, out);

		// NOTE(wheatley): if going out of prone this might cause us to get stuck so don't do
		// it if we ended up stuck in the ceiling
		trace_t trace;
		TracePlayerBBox(out, out, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace);
		if(!trace.startsolid)
			mv->SetAbsOrigin(out);

#ifdef CLIENT_DLL
#ifdef STAGING_ONLY
		if (debug_latch_reset_onduck.GetBool())
		{
			player->ResetLatched();
		}
#else
		player->ResetLatched();
#endif
#endif // CLIENT_DLL
	}

	// See if we are stuck?
	FixPlayerCrouchStuck(true);

	// Recategorize position since ducking can change origin
	CategorizePosition();
}

void CHDTFGameMovement::SetProneEyeOffset( float proneFraction )
{
	Vector vecPropViewOffset = VEC_PRONE_VIEW;
	Vector vecStandViewOffset = GetPlayerViewOffset( player->m_Local.m_bDucked );

	Vector temp = player->GetViewOffset( );
	temp.z = SimpleSplineRemapVal( proneFraction, 1.0, 0.0, vecPropViewOffset.z, vecStandViewOffset.z );

	player->SetViewOffset( temp );
}

void CHDTFGameMovement::FinishUnProne( )
{
	m_pHDTFPlayer->m_flUnProneTime = 0.0f;

	SetProneEyeOffset( 0.0f );

	if( m_bUnProneToDuck )
	{
		FinishDuck( );
	}
	else
	{
		CategorizePosition( );

		if( ( mv->m_nButtons & IN_DUCK ) != 0 && ( player->GetFlags( ) & FL_DUCKING ) == 0 )
		{
			// Use 1 second so super long jump will work
			player->m_Local.m_flDucktime = 1000;
			player->m_Local.m_bDucking = true;
		}
	}
}

void CHDTFGameMovement::FinishProne( )
{
	m_pHDTFPlayer->m_bProne = true;
	m_pHDTFPlayer->m_flGoProneTime = 0.0f;

	FinishUnDuck( );	// clear ducking

	SetProneEyeOffset( 1.0f );

	FixPlayerCrouchStuck( true );

	CategorizePosition( );
}

//-----------------------------------------------------------------------------
// Purpose: See if duck button is pressed and do the appropriate things
//-----------------------------------------------------------------------------
void CHDTFGameMovement::Duck( )
{
	int buttonsChanged = mv->m_nOldButtons ^ mv->m_nButtons;	// These buttons have changed this frame
	int buttonsPressed = buttonsChanged & mv->m_nButtons;			// The changed ones still down are "pressed"
	int buttonsReleased = buttonsChanged & mv->m_nOldButtons;		// The changed ones which were previously down are "released"

	if( ( mv->m_nButtons & IN_DUCK ) != 0 )
		mv->m_nOldButtons |= IN_DUCK;
	else
		mv->m_nOldButtons &= ~IN_DUCK;

	if( !player->IsAlive( ) )
	{
		if( m_pHDTFPlayer->IsProne( ) )
			FinishUnProne( );

		// Unduck
		if( player->m_Local.m_bDucking || player->m_Local.m_bDucked )
			FinishUnDuck( );

		return;
	}

	// Prone / UnProne - we don't duck or deploy if this is happening
	if( m_pHDTFPlayer->IsGettingUpFromProne( ) )
	{
		float pronetime = m_pHDTFPlayer->m_flUnProneTime - gpGlobals->curtime;
		if( pronetime < 0.0f )
		{
			FinishUnProne( );

			if( !m_bUnProneToDuck && ( mv->m_nButtons & IN_DUCK ) != 0 )
			{
				buttonsPressed |= IN_DUCK;
				mv->m_nOldButtons &= ~IN_DUCK;
			}
		}
		else
			SetProneEyeOffset( pronetime / TIME_TO_PRONE );

		// Set these, so that as soon as we stop unproning, we don't pop to standing
		// the information that we let go of the duck key has been lost by now.
		if( m_bUnProneToDuck )
		{
			player->m_Local.m_flDucktime = 1000;
			player->m_Local.m_bDucking = true;
		}

		//don't deal with ducking while we're proning
		return;
	}
	else if( m_pHDTFPlayer->IsGoingProne( ) )
	{
		float pronetime = m_pHDTFPlayer->m_flGoProneTime - gpGlobals->curtime;
		if( pronetime < 0.0f )
			FinishProne( );
		else
			SetProneEyeOffset( ( TIME_TO_PRONE - pronetime ) / TIME_TO_PRONE );

		//don't deal with ducking while we're proning
		return;
	}

	if( gpGlobals->curtime > m_pHDTFPlayer->m_flNextProneCheck )
	{
#ifndef CLIENT_DLL
		const bool bMustUnprone = (m_pHDTFPlayer->m_afButtonDisabled & IN_PRONE) != 0 && m_pHDTFPlayer->IsProne();
#else
		const bool bMustUnprone = false;
#endif
		if ((buttonsPressed & IN_PRONE) != 0 && !m_pHDTFPlayer->IsGoingProne() && !m_pHDTFPlayer->IsGettingUpFromProne() || bMustUnprone)
		{
			if (!m_pHDTFPlayer->IsProne())
			{
				CBaseCombatWeapon* pActiveWeapon = m_pHDTFPlayer->GetActiveWeapon();
				if (pActiveWeapon && pActiveWeapon->ClassMatches("weapon_parkour"))
				{
					CWeaponParkour* pActiveParkour = (CWeaponParkour*)m_pHDTFPlayer->GetActiveWeapon();
					if (pActiveParkour && !pActiveParkour->AllowCustomizedMovement())
						return;
				}

				m_pHDTFPlayer->StartGoingProne();

				// clear unprone to duck state or it might mess up player_speedmod prone restrictions
				m_bUnProneToDuck = false;
			}
			else
			{
				CHDTFGameMovement::UnproneResult_t result = CanUnprone();
				if (result != CHDTFGameMovement::UnproneResult_t::UNPRONE_RESULT_STUCK)
				{
					m_pHDTFPlayer->StandUpFromProne();

					m_bUnProneToDuck = ((mv->m_nButtons & IN_DUCK) != 0) || (result == CHDTFGameMovement::UnproneResult_t::UNPRONE_RESULT_DUCK);
					if (m_bUnProneToDuck)
						player->m_Local.m_bDucked = true;
				}
			}

			m_pHDTFPlayer->m_flNextProneCheck = gpGlobals->curtime + 1.0f;
			return;
		}
	}

	if( m_pHDTFPlayer->IsProne( ) &&
		!m_pHDTFPlayer->IsGoingProne( ) &&
		!m_pHDTFPlayer->IsGettingUpFromProne( ) &&
		( buttonsPressed & IN_DUCK ) != 0 &&
		(CanUnprone( ) != CHDTFGameMovement::UnproneResult_t::UNPRONE_RESULT_STUCK))	// BUGBUG - even calling this will unzoom snipers.
	{
		// If the player presses duck while prone,
		// unprone them to the duck position
		m_pHDTFPlayer->m_bProne = false;
		m_pHDTFPlayer->StandUpFromProne( );

		m_bUnProneToDuck = true;

		// simulate a duck that was pressed while we were prone
		player->AddFlag( FL_DUCKING );
		player->m_Local.m_bDucked = true;
		player->m_Local.m_flDucktime = 1000;
		player->m_Local.m_bDucking = true;
	}

	// no ducking or unducking while deployed or prone
	if( m_pHDTFPlayer->IsProning( ) )
		return;

	HandleDuckingSpeedCrop( );

	if( ( player->GetFlags( ) & FL_DUCKING ) == 0 && player->m_Local.m_bDucked )
		player->m_Local.m_bDucked = false;

	// Holding duck, in process of ducking or fully ducked?
	if( ( mv->m_nButtons & IN_DUCK ) != 0 || player->m_Local.m_bDucking || ( player->GetFlags( ) & FL_DUCKING ) != 0 )
	{
		if( ( mv->m_nButtons & IN_DUCK ) != 0 )
		{
			bool alreadyDucked = ( player->GetFlags( ) & FL_DUCKING ) != 0;

			if( ( buttonsPressed & IN_DUCK ) != 0 && ( player->GetFlags( ) & FL_DUCKING ) == 0 )
			{
				// Use 1 second so super long jump will work
				player->m_Local.m_flDucktime = 1000;
				player->m_Local.m_bDucking = true;
			}

			float duckmilliseconds = max( 0.0f, 1000.0f - player->m_Local.m_flDucktime );
			float duckseconds = duckmilliseconds / 1000.0f;

			if( player->m_Local.m_bDucking )
			{
				// Finish ducking immediately if duck time is over or not on ground
				if( duckseconds > TIME_TO_DUCK ||
					player->GetGroundEntity( ) == NULL ||
					alreadyDucked )
				{
					FinishDuck( );
				}
				else
				{
					// Calc parametric time
					float flDuckFraction = SimpleSpline( duckseconds / TIME_TO_DUCK );
					SetDuckedEyeOffset( flDuckFraction );
				}
			}
		}
		else
		{
			// Try to unduck unless automovement is not allowed
			// NOTE: When not onground, you can always unduck
			if( player->m_Local.m_bAllowAutoMovement || player->GetGroundEntity( ) == NULL )
			{
				if( ( buttonsReleased & IN_DUCK ) != 0 && ( player->GetFlags( ) & FL_DUCKING ) != 0 )
				{
					// Use 1 second so super long jump will work
					player->m_Local.m_flDucktime = 1000;
					player->m_Local.m_bDucking = true;  // or unducking
				}

				float duckmilliseconds = max( 0.0f, 1000.0f - (float)player->m_Local.m_flDucktime );
				float duckseconds = duckmilliseconds / 1000.0f;

				if( CanUnduck( ) )
				{
					if( player->m_Local.m_bDucking ||
						player->m_Local.m_bDucked ) // or unducking
					{
						// Finish ducking immediately if duck time is over or not on ground
						if( duckseconds > TIME_TO_UNDUCK ||
							player->GetGroundEntity( ) == NULL )
						{
							FinishUnDuck( );
						}
						else
						{
							// Calc parametric time
							float duckFraction = SimpleSpline( 1.0f - duckseconds / TIME_TO_UNDUCK );
							SetDuckedEyeOffset( duckFraction );
						}
					}
				}
				else
				{
					// Still under something where we can't unduck, so make sure we reset this timer so
					//  that we'll unduck once we exit the tunnel, etc.
					player->m_Local.m_flDucktime = 1000;
				}
			}
		}
	}
}

Vector CHDTFGameMovement::GetPlayerMins( ) const
{
	if( player == NULL )
		return vec3_origin;

	if( player->IsObserver( ) )
		return VEC_OBS_HULL_MIN;
	else if( player->IsDucked( ) )
		return VEC_DUCK_HULL_MIN;
	else if( m_pHDTFPlayer->IsProne( ) )
		return VEC_PRONE_HULL_MIN;

	return VEC_HULL_MIN;
}

Vector CHDTFGameMovement::GetPlayerMaxs( ) const
{
	if( player == NULL )
		return vec3_origin;

	if( player->IsObserver( ) )
		return VEC_OBS_HULL_MAX;
	else if( player->IsDucked( ) )
		return VEC_DUCK_HULL_MAX;
	else if( m_pHDTFPlayer->IsProne( ) )
		return VEC_PRONE_HULL_MAX;

	return VEC_HULL_MAX;
}
