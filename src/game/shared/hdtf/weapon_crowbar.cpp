#include "cbase.h"
#include "weapon_basemelee.h"
#include "npcevent.h"

#if defined( CLIENT_DLL )

#define CWeaponCrowbar C_WeaponCrowbar

#else

#include "ai_basenpc.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponCrowbar : public CBaseMeleeWeapon
{
public:
	DECLARE_CLASS( CWeaponCrowbar, CBaseMeleeWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	CWeaponCrowbar( )
	{ }

	float GetRange( )
	{
		return 75.0f;
	}

	float GetFireRate( )
	{
		return 0.4f;
	}

	float GetDamageForActivity( Activity hitActivity )
	{
		return 25.0f;
	}

	void SecondaryAttack( )
	{ }

	void AddViewKick( );

	void Drop( const Vector &vecVelocity );

#ifndef CLIENT_DLL

	// Animation event
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	int WeaponMeleeAttack1Condition( float flDot, float flDist );

#endif

private:
	CWeaponCrowbar( const CWeaponCrowbar & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponCrowbar, DT_WeaponCrowbar )

BEGIN_NETWORK_TABLE( CWeaponCrowbar, DT_WeaponCrowbar )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CWeaponCrowbar )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_crowbar, CWeaponCrowbar );
PRECACHE_WEAPON_REGISTER( weapon_crowbar );

#ifndef CLIENT_DLL

acttable_t CWeaponCrowbar::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_MELEE, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_MELEE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_MELEE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_MELEE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_MELEE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_MELEE, false },
};

IMPLEMENT_ACTTABLE( CWeaponCrowbar );

#endif

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponCrowbar::AddViewKick( )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	QAngle punchAng;
	punchAng.x = SharedRandomFloat( "crowbarpax", 1.0f, 2.0f );
	punchAng.y = SharedRandomFloat( "crowbarpay", -2.0f, -1.0f );
	punchAng.z = 0.0f;

	pPlayer->ViewPunch( punchAng );
}

void CWeaponCrowbar::Drop( const Vector &vecVelocity )
{

#ifndef CLIENT_DLL

	UTIL_Remove( this );

#endif

}

#ifndef CLIENT_DLL

//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponCrowbar::HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors( GetAbsAngles( ), &vecDirection );

	Vector vecEnd;
	VectorMA( pOperator->Weapon_ShootPosition( ), 50.0f, vecDirection, vecEnd );
	CBaseEntity *pHurt = pOperator->CheckTraceHullAttack( pOperator->Weapon_ShootPosition( ), vecEnd,
		Vector( -16.0f, -16.0f, -16.0f ), Vector( 36.0f, 36.0f, 36.0f ), GetDamageForActivity( GetActivity( ) ), DMG_CLUB, 0.75f );

	// did I hit someone?
	if( pHurt != NULL )
	{
		// play sound
		WeaponSound( MELEE_HIT );

		// Fake a trace impact, so the effects work out like a player's crowbaw
		trace_t traceHit;
		UTIL_TraceLine( pOperator->Weapon_ShootPosition( ), pHurt->GetAbsOrigin( ), MASK_SHOT_HULL, pOperator, COLLISION_GROUP_NONE, &traceHit );
		ImpactEffect( traceHit, false );
	}
	else
	{
		WeaponSound( MELEE_MISS );
	}
}

//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponCrowbar::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_MELEE_HIT:
		HandleAnimEventMeleeHit( pEvent, pOperator );
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

//-----------------------------------------------------------------------------
// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
//-----------------------------------------------------------------------------
ConVar sk_crowbar_lead_time( "sk_crowbar_lead_time", "0.9" );

int CWeaponCrowbar::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC = GetOwner( )->MyNPCPointer( );
	CBaseEntity *pEnemy = pNPC->GetEnemy( );
	if( pEnemy == NULL )
		return COND_NONE;

	// Project where the enemy will be in a little while
	float dt = sk_crowbar_lead_time.GetFloat( );
	dt += SharedRandomFloat( "crowbarmelee1", -0.3f, 0.2f );
	if( dt < 0.0f )
		dt = 0.0f;

	Vector vecVelocity = pEnemy->GetSmoothedVelocity( );
	Vector vecExtrapolatedPos;
	VectorMA( pEnemy->WorldSpaceCenter( ), dt, vecVelocity, vecExtrapolatedPos );

	Vector vecDelta;
	VectorSubtract( vecExtrapolatedPos, pNPC->WorldSpaceCenter( ), vecDelta );

	if( fabs( vecDelta.z ) > 70.0f )
		return COND_TOO_FAR_TO_ATTACK;

	Vector vecForward = pNPC->BodyDirection2D( );
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize( vecDelta.AsVector2D( ) );
	if( flDist > 64.0f && flExtrapolatedDist > 64.0f )
		return COND_TOO_FAR_TO_ATTACK;

	float flExtrapolatedDot = DotProduct2D( vecDelta.AsVector2D( ), vecForward.AsVector2D( ) );
	if( flDot < 0.7f && flExtrapolatedDot < 0.7f )
		return COND_NOT_FACING_ATTACK;

	return COND_CAN_MELEE_ATTACK1;
}

#endif
