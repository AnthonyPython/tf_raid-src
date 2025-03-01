#include "cbase.h"
#include "weapon_basemelee.h"
#include "in_buttons.h"

#ifndef CLIENT_DLL

#include "hdtf_player.h"
#include "te_effect_dispatch.h"
#include "ilagcompensationmanager.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( BaseMeleeWeapon, DT_BaseMeleeWeapon )

BEGIN_NETWORK_TABLE( CBaseMeleeWeapon, DT_BaseMeleeWeapon )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CBaseMeleeWeapon )
END_PREDICTION_DATA( )

#define MELEE_HULL_DIM 16

static const Vector g_meleeMins( -MELEE_HULL_DIM, -MELEE_HULL_DIM, -MELEE_HULL_DIM );
static const Vector g_meleeMaxs( MELEE_HULL_DIM, MELEE_HULL_DIM, MELEE_HULL_DIM );

CBaseMeleeWeapon::CBaseMeleeWeapon( )
{
	m_bFiresUnderwater = true;
}

Activity CBaseMeleeWeapon::GetPrimaryAttackActivity( )
{
	return ACT_VM_HITCENTER;
}

Activity CBaseMeleeWeapon::GetPrimaryMissActivity( )
{
	return ACT_VM_HITCENTER;
}

Activity CBaseMeleeWeapon::GetSecondaryAttackActivity( )
{
	return ACT_VM_MISSCENTER;
}

Activity CBaseMeleeWeapon::GetSecondaryMissActivity( )
{
	return ACT_VM_MISSCENTER2;
}

float CBaseMeleeWeapon::GetFireRate( )
{
	return 0.4f;
}

float CBaseMeleeWeapon::GetRange( )
{
	return 75.0f;
}

float CBaseMeleeWeapon::GetDamageForActivity( Activity hitActivity )
{
	return 25.0f;
}

int CBaseMeleeWeapon::GetDamageType( bool isSecondary )
{
	return DMG_CLUB;
}

void CBaseMeleeWeapon::Spawn( )
{
	m_fMinRange1 = 0.0f;
	m_fMinRange2 = 0.0f;
	m_fMaxRange1 = 64.0f;
	m_fMaxRange2 = 64.0f;

	//Call base class first
	BaseClass::Spawn( );
}

void CBaseMeleeWeapon::PrimaryAttack( )
{

#ifndef CLIENT_DLL

	CHDTF_Player *pPlayer = ToHDTFPlayer( GetOwner( ) );
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, pPlayer->GetCurrentCommand( ) );

#endif

	Swing( false, true );

#ifndef CLIENT_DLL

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( pPlayer );

#endif

}

void CBaseMeleeWeapon::SecondaryAttack( )
{

#ifndef CLIENT_DLL

	CHDTF_Player *pPlayer = ToHDTFPlayer( GetOwner( ) );
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, pPlayer->GetCurrentCommand( ) );

#endif

	Swing( true, true );

#ifndef CLIENT_DLL

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( pPlayer );

#endif

}

void CBaseMeleeWeapon::ItemPostFrame( )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	if( ( pOwner->m_nButtons & IN_ATTACK ) != 0 && m_flNextPrimaryAttack <= gpGlobals->curtime )
		PrimaryAttack( );
	else if( ( pOwner->m_nButtons & IN_ATTACK2 ) != 0 && m_flNextSecondaryAttack <= gpGlobals->curtime )
		SecondaryAttack( );
	else
		WeaponIdle( );

	if ((pOwner->m_nButtons & IN_RELOAD) != 0)
	{
		m_bIsReloadKeyDown = true;
#ifndef CLIENT_DLL
		m_flHudRevealTimer += gpGlobals->frametime;
		if (m_flHudRevealTimer >= HDTF_HUD_REVEAL_TIMER)
		{
			CRecipientFilter filter;
			filter.AddRecipient(pOwner);
			UserMessageBegin(filter, "RevealHud");
			MessageEnd();
			m_flHudRevealTimer = 0.f;
		}
#endif
	}
	else
	{
		m_bIsReloadKeyDown = false;
		m_flHudRevealTimer = 0.f;
	}
}

void CBaseMeleeWeapon::ImpactEffect( trace_t &traceHit, bool bIsSecondary )
{
	// See if we hit water (we don't do the other impact effects in this case)
	if( ImpactWater( traceHit.startpos, traceHit.endpos ) )
		return;

	//FIXME: need new decals
	UTIL_ImpactTrace( &traceHit, GetDamageType( bIsSecondary ) & ~DMG_DIRECT );
}

bool CBaseMeleeWeapon::ImpactWater( const Vector &start, const Vector &end )
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...

	// We must start outside the water
	if( ( UTIL_PointContents( start ) & ( CONTENTS_WATER | CONTENTS_SLIME ) ) != 0 )
		return false;

	// We must end inside of water
	if( ( UTIL_PointContents( end ) & ( CONTENTS_WATER | CONTENTS_SLIME ) ) == 0 )
		return false;

	trace_t	waterTrace;
	UTIL_TraceLine( start, end, CONTENTS_WATER | CONTENTS_SLIME, GetOwner( ), COLLISION_GROUP_NONE, &waterTrace );

#ifndef CLIENT_DLL

	if( waterTrace.fraction < 1.0f )
	{
		CEffectData	data;
		data.m_fFlags = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if( ( waterTrace.contents & CONTENTS_SLIME ) != 0 )
			data.m_fFlags |= FX_WATER_IN_SLIME;

		DispatchEffect( "watersplash", data );
	}

#endif

	return true;
}

void CBaseMeleeWeapon::Swing( bool bIsSecondary, bool bSendAnim )
{
	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	Vector forward;
	pOwner->EyeVectors( &forward, NULL, NULL );

	Vector swingStart = pOwner->Weapon_ShootPosition( );
	Vector swingEnd = swingStart + forward * GetRange( );

	trace_t traceHit;
	UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );

	Activity nHitActivity = bIsSecondary ? GetSecondaryAttackActivity( ) : GetPrimaryAttackActivity( );

#ifndef CLIENT_DLL

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo( this, GetOwner( ), GetDamageForActivity( nHitActivity ), GetDamageType( bIsSecondary ) );
	TraceAttackToTriggers( triggerInfo, traceHit.startpos, traceHit.endpos, vec3_origin );

#endif

	if( traceHit.fraction == 1.0 )
	{
		// Back off by hull "radius"
		swingEnd -= forward * 1.732f * MELEE_HULL_DIM; // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		UTIL_TraceHull( swingStart, swingEnd, g_meleeMins, g_meleeMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
		if( traceHit.fraction < 1.0 && traceHit.m_pEnt != NULL )
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin( ) - swingStart;
			VectorNormalize( vecToTarget );

			// YWB:  Make sure they are sort of facing the guy at least...
			if( vecToTarget.Dot( forward ) < 0.70721f )
				// Force amiss
				traceHit.fraction = 1.0f;
			else
				nHitActivity = ChooseIntersectionPointAndActivity( traceHit, g_meleeMins, g_meleeMaxs, pOwner );
		}
	}

	WeaponSound( SINGLE );

	// -------------------------
	//	Miss
	// -------------------------
	if( traceHit.fraction == 1.0f )
	{
		nHitActivity = bIsSecondary ? GetSecondaryMissActivity( ) : GetPrimaryMissActivity( );

		// We want to test the first swing again
		// See if we happened to hit water
		ImpactWater( swingStart, swingStart + forward * GetRange( ) );
	}
	else
	{
		Hit( traceHit, nHitActivity, bIsSecondary );
	}

	if( bSendAnim )
	{
		// Send the anim
		SendWeaponAnim( nHitActivity );

		pOwner->SetAnimation( PLAYER_ATTACK1 );
	}

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate( );
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration( );
}

void CBaseMeleeWeapon::Hit( trace_t &traceHit, Activity nHitActivity, bool bIsSecondary )
{
	//Do view kick
	//	AddViewKick( );

	//Apply damage to a hit target
	CBaseEntity	*pHitEntity = traceHit.m_pEnt;
	if( pHitEntity != NULL )
	{
		CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
		Vector hitDirection;
		pPlayer->EyeVectors( &hitDirection, NULL, NULL );
		VectorNormalize( hitDirection );

#ifndef CLIENT_DLL

		CTakeDamageInfo info( this, GetOwner( ), GetDamageForActivity( nHitActivity ), GetDamageType( bIsSecondary ) );

		if( pHitEntity->IsNPC( ) )
			// If bonking an NPC, adjust damage.
			info.AdjustPlayerDamageInflictedForSkillLevel( );

		CalculateMeleeDamageForce( &info, hitDirection, traceHit.endpos );

		pHitEntity->DispatchTraceAttack( info, hitDirection, &traceHit );
		ApplyMultiDamage( );

		// Now hit all triggers along the ray that... 
		TraceAttackToTriggers( info, traceHit.startpos, traceHit.endpos, hitDirection );

#endif

		if(pHitEntity->IsNPC())
			WeaponSound(MELEE_HIT);
		else
			WeaponSound(MELEE_HIT_WORLD);
	}

	// Apply an impact effect if we're not hitting ragdolls, NPCs and players.
	// This is done to prevent engine running out of indices with obscure decals on highly-detailed models.
	if (pHitEntity != NULL && !pHitEntity->ClassMatches("prop_ragdoll") && !pHitEntity->IsNPC() && !pHitEntity->IsPlayer())
	{
		ImpactEffect(traceHit, bIsSecondary);
	}
}

Activity CBaseMeleeWeapon::ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner )
{
	const float	*minmaxs[2] = { mins.Base( ), maxs.Base( ) };
	float distance = 1e6f;
	Vector vecSrc = hitTrace.startpos, vecHullEnd = vecSrc + ( hitTrace.endpos - vecSrc ) * 2, vecEnd;

	trace_t tmpTrace;
	UTIL_TraceLine( vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
	if( tmpTrace.fraction == 1.0 )
		for( int i = 0; i < 2; ++i )
			for( int j = 0; j < 2; ++j )
				for( int k = 0; k < 2; ++k )
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine( vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
					if( tmpTrace.fraction < 1.0 )
					{
						float thisDistance = ( tmpTrace.endpos - vecSrc ).Length( );
						if( thisDistance < distance )
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
	else
		hitTrace = tmpTrace;

	return ACT_VM_HITCENTER;
}
