#include "cbase.h"
#include "weapon_basemelee.h"
#include "npcevent.h"

#ifdef CLIENT_DLL

#define CWeaponKnife C_WeaponKnife

#else

#include "hdtf_player.h"

#endif

ConVar sk_plr_dmg_knife_primary( "sk_plr_dmg_knife_primary", "15", FCVAR_REPLICATED );
ConVar sk_plr_dmg_knife_secondary( "sk_plr_dmg_knife_secondary", "30", FCVAR_REPLICATED );
ConVar sk_npc_dmg_knife_primary( "sk_npc_dmg_knife_primary", "15", FCVAR_REPLICATED );
ConVar sk_npc_dmg_knife_secondary( "sk_npc_dmg_knife_secondary", "30", FCVAR_REPLICATED );

class CWeaponKnife : public CBaseMeleeWeapon
{
public:
	DECLARE_CLASS( CWeaponKnife, CBaseMeleeWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	Activity GetPrimaryAttackActivity()
	{
		return ACT_VM_PRIMARYATTACK;
	}

	Activity GetSecondaryAttackActivity()
	{
		return ACT_VM_SECONDARYATTACK;
	}

	float GetRange( )
	{
		return 75.0f;
	}

	float GetFireRate( )
	{
		return 0.4f;
	}

	int GetDamageType( bool bIsSecondary )
	{
		// not supposed to slice zombies in two!
		return DMG_CLUB;
	}

	void ImpactSound( bool bIsWorld )
	{ }

	bool IsAllowedToSwitch( )
	{
		return !m_bQuickAttacking;
	}

	float GetDamageForActivity( Activity hitActivity );

	void AddViewKick( );

	void PrimaryAttack();
	void SecondaryAttack();

	void ImpactEffect(trace_t &traceHit, bool bIsSecondary);

#ifndef CLIENT_DLL

	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	void QuickAttack( );

	void ItemPostFrame( );

#endif

private:
	bool m_bQuickAttacking;
	float m_fQuickAttackEnd;
	CBaseCombatWeapon *m_pLastWeapon;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponKnife, DT_WeaponKnife )

BEGIN_NETWORK_TABLE( CWeaponKnife, DT_WeaponKnife )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CWeaponKnife )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_knife, CWeaponKnife );
PRECACHE_WEAPON_REGISTER( weapon_knife );

#ifndef CLIENT_DLL

acttable_t CWeaponKnife::m_acttable[] =
{
	{ ACT_MELEE_ATTACK1, ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE, ACT_IDLE_ANGRY_MELEE, false },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_MELEE, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_MELEE, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_MELEE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_MELEE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_MELEE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_MELEE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_MELEE, false },
};

IMPLEMENT_ACTTABLE( CWeaponKnife );

#endif

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponKnife::GetDamageForActivity( Activity hitActivity )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer != NULL )
		return ( hitActivity == GetPrimaryAttackActivity( ) ? sk_plr_dmg_knife_primary : sk_plr_dmg_knife_secondary ).GetFloat( );

	return ( hitActivity == GetPrimaryAttackActivity( ) ? sk_npc_dmg_knife_primary : sk_npc_dmg_knife_secondary ).GetFloat( );
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponKnife::AddViewKick( )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	QAngle punchAng;
	punchAng.x = SharedRandomFloat( "knifepax", 2.0f, 4.0f );
	punchAng.y = SharedRandomFloat( "knifepay", -2.0f, -1.0f );
	punchAng.z = 0.0f;

	pPlayer->ViewPunch( punchAng );
}

void CWeaponKnife::PrimaryAttack()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	pOwner->SetAnimation(PLAYER_ATTACK1);

	Swing(false, false);

	SendWeaponAnim(GetPrimaryAttackActivity());
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
}

void CWeaponKnife::SecondaryAttack()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	pOwner->SetAnimation(PLAYER_ATTACK1);

	SendWeaponAnim(GetSecondaryAttackActivity());
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Handle animation events
//-----------------------------------------------------------------------------
void CWeaponKnife::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_MELEE_SWISH:
		Swing(true, false);
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

void CWeaponKnife::QuickAttack( )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	m_pLastWeapon = pPlayer->GetLastWeapon( );

	if( !pPlayer->Weapon_Switch( this ) )
		return;

	m_bQuickAttacking = true;
	PrimaryAttack( );
}

void CWeaponKnife::ItemPostFrame( )
{
	if( m_bQuickAttacking && gpGlobals->curtime >= m_flNextSecondaryAttack )
	{
		m_bQuickAttacking = false;
		m_fQuickAttackEnd = 0.0f;

		CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
		if( pPlayer != NULL )
		{
			CBaseCombatWeapon *pWeapon = pPlayer->GetLastWeapon( );
			if( pWeapon != NULL )
			{
				pPlayer->Weapon_Switch( pWeapon );
				pPlayer->Weapon_SetLast( m_pLastWeapon );
			}
		}

		m_pLastWeapon = NULL;
	}

	BaseClass::ItemPostFrame( );
}

CON_COMMAND( quick_knife, "Quickly makes a swipe with your knife (if it's equipped)." )
{
	CHDTF_Player *pPlayer = ToHDTFPlayer( UTIL_GetCommandClient( ) );
	CBaseCombatWeapon *pWeapon = pPlayer->GetWeaponByName( "weapon_knife" );
	if( pWeapon == NULL || !pWeapon->VisibleInWeaponSelection( ) )
		return;

	static_cast<CWeaponKnife *>( pWeapon )->QuickAttack( );
}

#endif

void CWeaponKnife::ImpactEffect(trace_t &traceHit, bool bIsSecondary)
{
	// See if we hit water (we don't do the other impact effects in this case)
	if (ImpactWater(traceHit.startpos, traceHit.endpos))
		return;

	// dot-shaped hit if secondary attack and slice if primary
	if(bIsSecondary)
		UTIL_ImpactTrace(&traceHit, GetDamageType(bIsSecondary) & ~DMG_DIRECT);
	else
		UTIL_DecalTrace(&traceHit, "ManhackCut");
}
