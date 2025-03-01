#include "cbase.h"
#include "weapon_basemelee.h"
#include "npcevent.h"

#if defined( CLIENT_DLL )

#define CWeaponAxe C_WeaponAxe

#else

#include "ai_basenpc.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponAxe : public CBaseMeleeWeapon
{
public:
	DECLARE_CLASS( CWeaponAxe, CBaseMeleeWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	CWeaponAxe( )
	{ }

	float GetRange();

	float GetDamageForActivity(Activity hitActivity);

	void AddViewKick( );

	void Drop( const Vector &vecVelocity );

	void PrimaryAttack();
	void SecondaryAttack();

#ifndef CLIENT_DLL

	// Animation event
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	int WeaponMeleeAttack1Condition( float flDot, float flDist );

#endif

private:
	CWeaponAxe( const CWeaponAxe & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAxe, DT_WeaponAxe )

BEGIN_NETWORK_TABLE( CWeaponAxe, DT_WeaponAxe )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CWeaponAxe )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_axe, CWeaponAxe );
PRECACHE_WEAPON_REGISTER( weapon_axe );

#ifndef CLIENT_DLL

acttable_t CWeaponAxe::m_acttable[] =
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

IMPLEMENT_ACTTABLE( CWeaponAxe );

#endif

ConVar sk_axe_range("sk_axe_range", "75");
ConVar sk_axe_secondary_damage("sk_axe_secondary_damage", "64");
ConVar sk_axe_damage("sk_axe_damage", "25");
//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponAxe::AddViewKick( )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	QAngle punchAng;
	punchAng.x = SharedRandomFloat( "axepax", 1.0f, 2.0f );
	punchAng.y = SharedRandomFloat( "axepay", -2.0f, -1.0f );
	punchAng.z = 0.0f;

	pPlayer->ViewPunch( punchAng );
}

void CWeaponAxe::Drop( const Vector &vecVelocity )
{

#ifndef CLIENT_DLL

	UTIL_Remove( this );

#endif

}

float CWeaponAxe::GetRange()
{
	return sk_axe_range.GetFloat();
}

float CWeaponAxe::GetDamageForActivity(Activity hitActivity)
{
	if (hitActivity == ACT_VM_SECONDARYATTACK)
		return sk_axe_secondary_damage.GetFloat();

	return sk_axe_damage.GetFloat();
}
#ifndef CLIENT_DLL

//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponAxe::HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors( GetAbsAngles( ), &vecDirection );

	Vector vecEnd;
	VectorMA( pOperator->Weapon_ShootPosition( ), 50, vecDirection, vecEnd );
	CBaseEntity *pHurt = pOperator->CheckTraceHullAttack( pOperator->Weapon_ShootPosition( ), vecEnd,
		Vector( -16, -16, -16 ), Vector( 36, 36, 36 ), GetDamageForActivity( GetActivity( ) ), DMG_SLASH, 0.75 );

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
		// still need this trace because we might hit the world
		trace_t traceHit;
		UTIL_TraceLine(pOperator->Weapon_ShootPosition(), vecEnd, MASK_SHOT_HULL, pOperator, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.DidHit())
			ImpactEffect(traceHit, false);
		else
			WeaponSound(MELEE_MISS);
	}
}

//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponAxe::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
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
ConVar sk_axe_lead_time( "sk_axe_lead_time", "0.9" );


int CWeaponAxe::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC = GetOwner( )->MyNPCPointer( );
	CBaseEntity *pEnemy = pNPC->GetEnemy( );
	if( pEnemy == NULL )
		return COND_NONE;

	Vector vecVelocity = pEnemy->GetSmoothedVelocity( );

	// Project where the enemy will be in a little while
	float dt = sk_axe_lead_time.GetFloat( );
	dt += SharedRandomFloat( "axemelee1", -0.3f, 0.2f );
	if( dt < 0.0f )
		dt = 0.0f;

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

void CWeaponAxe::PrimaryAttack()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	pOwner->SetAnimation(PLAYER_ATTACK1);
	SendWeaponAnim(ACT_VM_PRIMARYATTACK);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponAxe::SecondaryAttack()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	pOwner->SetAnimation(PLAYER_ATTACK1);
	SendWeaponAnim(ACT_VM_SECONDARYATTACK);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}