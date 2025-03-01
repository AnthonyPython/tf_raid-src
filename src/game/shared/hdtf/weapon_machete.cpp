#include "cbase.h"
#include "weapon_basemelee.h"

#ifndef CLIENT_DLL

#include "hdtf_player.h"
#include "ilagcompensationmanager.h"
#include "npcevent.h"

#else

#define CWeaponMachete C_WeaponMachete

#endif

ConVar sk_plr_dmg_machete_primary( "sk_plr_dmg_machete_primary", "15", FCVAR_REPLICATED );
ConVar sk_plr_dmg_machete_secondary( "sk_plr_dmg_machete_secondary", "30", FCVAR_REPLICATED );
ConVar sk_npc_dmg_machete_primary( "sk_npc_dmg_machete_primary", "15", FCVAR_REPLICATED );
ConVar sk_npc_dmg_machete_secondary( "sk_npc_dmg_machete_secondary", "30", FCVAR_REPLICATED );
ConVar sk_machete_primary_slice_chance( "sk_machete_primary_slice_chance", "0.1", FCVAR_CHEAT | FCVAR_REPLICATED, "Chance of slicing a zombie in half with the machete with a primary attack" );
ConVar sk_machete_secondary_slice_chance( "sk_machete_secondary_slice_chance", "0.6", FCVAR_CHEAT | FCVAR_REPLICATED, "Chance of slicing a zombie in half with the machete with a secondary attack" );

class CWeaponMachete : public CBaseMeleeWeapon
{
public:
	DECLARE_CLASS( CWeaponMachete, CBaseMeleeWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );
	DECLARE_DATADESC( );

#endif

	CWeaponMachete( );

	Activity GetPrimaryAttackActivity( )
	{
		return ACT_VM_PRIMARYATTACK;
	}

	Activity GetSecondaryAttackActivity( )
	{
		return ACT_VM_SECONDARYATTACK;
	}

	float GetRange( )
	{
		return 75.0f;
	}

	float GetFireRate( )
	{
		return 0.6f;
	}

	void ImpactSound( bool bIsWorld )
	{ }

	float GetDamageForActivity( Activity hitActivity );

	int GetDamageType( bool bIsSecondary )
	{
		return m_nDamageType;
	}

	void AddViewKick( );

	void PrimaryAttack( );
	void SecondaryAttack( );

#ifndef CLIENT_DLL
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
#endif

private:
	int m_nDamageType;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMachete, DT_WeaponMachete )

BEGIN_NETWORK_TABLE( CWeaponMachete, DT_WeaponMachete )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CWeaponMachete )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_machete, CWeaponMachete );
PRECACHE_WEAPON_REGISTER( weapon_machete );

#ifndef CLIENT_DLL

BEGIN_DATADESC( CWeaponMachete )
END_DATADESC( )

acttable_t CWeaponMachete::m_acttable[] =
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

IMPLEMENT_ACTTABLE( CWeaponMachete );

#endif

CWeaponMachete::CWeaponMachete( )
{
	m_nDamageType = DMG_SLASH;
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponMachete::GetDamageForActivity( Activity hitActivity )
{
	CBaseCombatCharacter *pOwner = GetOwner( );
	if( pOwner != NULL && pOwner->IsPlayer( ) )
		return ( hitActivity == GetPrimaryAttackActivity( ) ? sk_plr_dmg_machete_primary : sk_plr_dmg_machete_secondary ).GetFloat( );

	return ( hitActivity == GetPrimaryAttackActivity( ) ? sk_npc_dmg_machete_primary : sk_npc_dmg_machete_secondary ).GetFloat( );
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponMachete::AddViewKick( )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	QAngle punchAng;
	punchAng.x = SharedRandomFloat( "machetepax", 2.0f, 4.0f );
	punchAng.y = SharedRandomFloat( "machetepay", -2.0f, -1.0f );
	punchAng.z = 0.0f;

	pPlayer->ViewPunch( punchAng );
}

void CWeaponMachete::PrimaryAttack( )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	m_nDamageType = DMG_SLASH;
	if( SharedRandomFloat( "machetedt", 0.0f, 1.0f ) <= sk_machete_secondary_slice_chance.GetFloat( ) )
		m_nDamageType |= DMG_DIRECT;

	pOwner->SetAnimation( PLAYER_ATTACK1 );

	SendWeaponAnim(GetPrimaryAttackActivity());
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration() - 0.25f;
}

void CWeaponMachete::SecondaryAttack( )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	m_nDamageType = DMG_SLASH;
	if( SharedRandomFloat( "machetedt", 0.0f, 1.0f ) <= sk_machete_secondary_slice_chance.GetFloat( ) )
		m_nDamageType |= DMG_DIRECT;

	pOwner->SetAnimation( PLAYER_ATTACK1 );

	SendWeaponAnim(GetSecondaryAttackActivity());
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Handle animation events
//-----------------------------------------------------------------------------
void CWeaponMachete::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_MELEE_HIT:
		Swing(false, false);
		break;

	case EVENT_WEAPON_MELEE_SWISH:
		Swing(true, false);
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif
