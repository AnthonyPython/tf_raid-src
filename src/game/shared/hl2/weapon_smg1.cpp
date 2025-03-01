//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_basemachinegun.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "ammodef.h"

#ifndef CLIENT_DLL

#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "game.h"
#include "grenade_ar2.h"
#include "ai_memory.h"
#include "soundent.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar hdtf_smg1_shoots_grenades("hdtf_smg1_shoots_grenades", "0", FCVAR_ARCHIVE, "Changes special attack of smg1 to grenade launcher like seen in HL2.\n"
	"Sorry M3SA, I love it and I don't care that it doesn't make any sense.");

extern ConVar    sk_plr_dmg_smg1_grenade;	
extern bool UTIL_ItemCanBeTouchedByPlayer(CBaseEntity *pItem, CBasePlayer *pPlayer);

#ifdef CLIENT_DLL

#define CWeaponSMG1 C_WeaponSMG1

#endif

class CWeaponSMG1 : public CMachineGun
{
public:
	DECLARE_CLASS( CWeaponSMG1, CMachineGun);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();
#endif

	CWeaponSMG1();

#ifndef CLIENT_DLL
	void	Precache( void );
	void	AddViewKick( void );
	void	SpecialAttack( void );

	int		GetMinBurst() { return 2; }
	int		GetMaxBurst() { return 5; }

	virtual bool Deploy();
	virtual void Equip( CBaseCombatCharacter *pOwner );
	bool	Reload( void );

	float	GetFireRate( void ) { return 0.075f; }	// 13.3hz

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	//virtual const Vector& GetBulletSpread( void )
	//{
	//	static Vector cone = VECTOR_CONE_5DEGREES;

	//	if (m_bScopeAttached && IsIronsightsEnabled())
	//		cone = VECTOR_CONE_3DEGREES;
	//	else if (IsIronsightsEnabled())
	//		cone = VECTOR_CONE_4DEGREES;
	//	
	//	return cone;
	//}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	void ItemPostFrame();

	virtual void EnableIronsights();

#endif

	bool HasFlashlight() { return m_bIsFlashlightEnabled; }

	enum ScopeTransitionState_t
	{
		NONE,
		INTRO,
		HOLD,
		OUTRO,
	};

protected:

#ifndef CLIENT_DLL
	void	LaunchGrenade();
#endif

	void	ChangeScopeState(const bool bAttached);
	void	ChangeFlashlightState(const bool bAttached);

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
	bool	m_bScopeAttached;
	ScopeTransitionState_t m_ScopeTransitionState;
	bool	m_bDesiredFlashlightState;
	CNetworkVar(bool, m_bIsFlashlightEnabled);

private:
	CWeaponSMG1( const CWeaponSMG1 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponSMG1, DT_WeaponSMG1)

BEGIN_NETWORK_TABLE(CWeaponSMG1, DT_WeaponSMG1)

#ifdef CLIENT_DLL
RecvPropBool(RECVINFO(m_bIsFlashlightEnabled))
#else
SendPropBool(SENDINFO(m_bIsFlashlightEnabled))
#endif

END_NETWORK_TABLE()

#ifndef CLIENT_DLL

BEGIN_DATADESC( CWeaponSMG1 )

	DEFINE_FIELD( m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( m_ScopeTransitionState, FIELD_INTEGER ),
	DEFINE_FIELD( m_bScopeAttached, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDesiredFlashlightState, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bIsFlashlightEnabled, FIELD_BOOLEAN ),

END_DATADESC()

#endif

BEGIN_PREDICTION_DATA(CWeaponSMG1)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_smg1, CWeaponSMG1);
PRECACHE_WEAPON_REGISTER(weapon_smg1);

#ifndef CLIENT_DLL

acttable_t	CWeaponSMG1::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SMG1,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true  },
	
// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_SMG1_LOW,			false },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
};

IMPLEMENT_ACTTABLE(CWeaponSMG1);

#endif

//=========================================================
CWeaponSMG1::CWeaponSMG1( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;

	m_bAltFiresUnderwater = false;
	m_ScopeTransitionState = ScopeTransitionState_t::NONE;
	m_bDesiredFlashlightState = false;
	m_bIsFlashlightEnabled = false;
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::Precache( void )
{
	UTIL_PrecacheOther("grenade_ar2");

	BaseClass::Precache();
}

bool CWeaponSMG1::Deploy()
{
	const bool result = BaseClass::Deploy();

	if (result)
	{
		if (m_ScopeTransitionState != ScopeTransitionState_t::NONE)
		{
			m_ScopeTransitionState = ScopeTransitionState_t::NONE;

			m_flNextSpecialAttack = m_flNextPrimaryAttack;
		}

		ChangeScopeState(true);
		ChangeFlashlightState(m_bIsFlashlightEnabled);
	}

	return result;
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponSMG1::Equip( CBaseCombatCharacter *pOwner )
{
#ifdef HDTF
	if( IsPlayerAlly( pOwner->Classify( ) ) )
#else
	if( pOwner->Classify() == CLASS_PLAYER_ALLY )
#endif
	{
		m_fMaxRange1 = 3000;
	}
	else
	{
		m_fMaxRange1 = 1400;
	}

	BaseClass::Equip( pOwner );

	ChangeScopeState(true);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );
	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0 );

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
	AngleVectors( angShootDir, &vecShootDir );
	FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_SMG1:
		{
			Vector vecShootOrigin, vecShootDir;
			QAngle angDiscard;

			// Support old style attachment point firing
			if ((pEvent->options == NULL) || (pEvent->options[0] == '\0') || (!pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard)))
			{
				vecShootOrigin = pOperator->Weapon_ShootPosition();
			}

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );
			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
		}
		break;

		/*//FIXME: Re-enable
		case EVENT_WEAPON_AR2_GRENADE:
		{
		CAI_BaseNPC *npc = pOperator->MyNPCPointer();

		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetShootEnemyDir( vecShootOrigin );

		Vector vecThrow = m_vecTossVelocity;

		CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create( "grenade_ar2", vecShootOrigin, vec3_angle, npc );
		pGrenade->SetAbsVelocity( vecThrow );
		pGrenade->SetLocalAngularVelocity( QAngle( 0, 400, 0 ) );
		pGrenade->SetMoveType( MOVETYPE_FLYGRAVITY ); 
		pGrenade->m_hOwner			= npc;
		pGrenade->m_pMyWeaponAR2	= this;
		pGrenade->SetDamage(sk_npc_dmg_ar2_grenade.GetFloat());

		// FIXME: arrgg ,this is hard coded into the weapon???
		m_flNextGrenadeCheck = gpGlobals->curtime + 6;// wait six seconds before even looking again to see if a grenade can be thrown.

		m_iClip2--;
		}
		break;
		*/

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponSMG1::Reload( void )
{
	if (m_ScopeTransitionState != ScopeTransitionState_t::NONE)
	{
		return false;
	}

	float fCacheTime = m_flNextSpecialAttack;

	const bool result = BaseClass::Reload();
	if (result)
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSpecialAttack = GetOwner()->m_flNextAttack = fCacheTime;
	}

	return result;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::AddViewKick( void )
{
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	float flPitch = random->RandomFloat(0.35f, 0.75f);
	float flYaw = random->RandomFloat(-0.25f, 0.25f);

	pPlayer->ViewPunch(QAngle(-flPitch, flYaw, 0));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::SpecialAttack( void )
{
	if (m_ScopeTransitionState != ScopeTransitionState_t::NONE || IsIronsightsEnabled())
		return;

	if (hdtf_smg1_shoots_grenades.GetBool())
	{
		LaunchGrenade();
		return;
	}

	m_bDesiredFlashlightState = !m_bIsFlashlightEnabled;

	SendWeaponAnim(ACT_VM_HOLSTER);
	m_ScopeTransitionState = ScopeTransitionState_t::INTRO;

	m_flNextPrimaryAttack = m_flNextSpecialAttack = FLT_MAX;
	m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();
}

void CWeaponSMG1::LaunchGrenade()
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	//Must have ammo
	if ((pPlayer->GetAmmoCount(m_iSecondaryAmmoType) <= 0) || (pPlayer->GetWaterLevel() == 3))
	{
		SendWeaponAnim(ACT_VM_DRYFIRE);
		BaseClass::WeaponSound(EMPTY);
		m_flNextSpecialAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	if (m_bInReload)
		m_bInReload = false;

	// MUST call sound before removing a round from the clip of a CMachineGun
	BaseClass::WeaponSound(WPN_DOUBLE);

	pPlayer->RumbleEffect(RUMBLE_357, 0, RUMBLE_FLAGS_NONE);

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector	vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors(pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow);
	VectorScale(vecThrow, 1000.0f, vecThrow);

	//Create the grenade
	QAngle angles;
	VectorAngles(vecThrow, angles);
	CGrenadeAR2 *pGrenade = (CGrenadeAR2 *)Create("grenade_ar2", vecSrc, angles, pPlayer);
	pGrenade->SetAbsVelocity(vecThrow);

	pGrenade->SetLocalAngularVelocity(RandomAngle(-400, 400));
	pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	pGrenade->SetThrower(GetOwner());
	pGrenade->SetDamage(sk_plr_dmg_smg1_grenade.GetFloat());

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON);

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Decrease ammo
	pPlayer->RemoveAmmo(1, m_iSecondaryAmmoType);

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextSpecialAttack = gpGlobals->curtime + 1.0f;

	// Register a muzzleflash for the AI.
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

	m_iSecondaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, false, GetClassname());
}

void CWeaponSMG1::ItemPostFrame()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer && m_ScopeTransitionState != ScopeTransitionState_t::NONE)
	{
		if (HasWeaponIdleTimeElapsed())
		{
			switch (m_ScopeTransitionState)
			{
				case ScopeTransitionState_t::INTRO:
					m_ScopeTransitionState = ScopeTransitionState_t::HOLD;
					m_flTimeWeaponIdle = gpGlobals->curtime + 0.5f;

					ChangeFlashlightState(m_bDesiredFlashlightState);
					
					if (!m_bDesiredFlashlightState)
						m_bIsFlashlightEnabled = false;

					break;

				case ScopeTransitionState_t::HOLD:
					if (m_bDesiredFlashlightState)
						m_bIsFlashlightEnabled = true;

					SendWeaponAnim(ACT_VM_DRAW);
					m_ScopeTransitionState = ScopeTransitionState_t::OUTRO;
					m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();
					break;

				case ScopeTransitionState_t::OUTRO:
					m_ScopeTransitionState = ScopeTransitionState_t::NONE;
					m_flNextPrimaryAttack = m_flNextSpecialAttack = gpGlobals->curtime;
					break;
			}
		}

		return;
	}

	BaseClass::ItemPostFrame();
}

void CWeaponSMG1::ChangeScopeState(const bool bAttached)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	const int scopeBodyGroup = pPlayer->GetViewModel()->FindBodygroupByName("Sight");
	if (scopeBodyGroup != -1)
		pPlayer->GetViewModel()->SetBodygroup(scopeBodyGroup, bAttached);
}

void CWeaponSMG1::ChangeFlashlightState(const bool bAttached)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	const int flashlightBodyGroup = pPlayer->GetViewModel()->FindBodygroupByName("FlashLight");
	if (flashlightBodyGroup != -1)
		pPlayer->GetViewModel()->SetBodygroup(flashlightBodyGroup, bAttached);
}

void CWeaponSMG1::EnableIronsights()
{
	if (!HasIronsights() || IsIronsightsEnabled() || m_bInReload)
		return;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner != NULL)
	{
		m_bIsIronsighted = true;
		pOwner->EmitSound("Player.IronsightsIn");
		pOwner->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;
		SendWeaponAnim(IsSpecialEnabled() ? ACT_VM_IDLE_TO_IRONSIGHTS_SPECIAL : ACT_VM_IDLE_TO_IRONSIGHTS);
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack = m_flNextPrimaryAttack;
		m_flNextSpecialAttack = m_flNextPrimaryAttack;

		if(m_bScopeAttached)
			pOwner->SetFOV(this, 75.f, 0.35f);
	}
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponSMG1::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0/3.0, 0.75	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}

#endif
