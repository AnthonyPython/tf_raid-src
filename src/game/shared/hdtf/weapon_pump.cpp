#include "cbase.h"
#include "weapon_basehdtfcombat.h"
#include "hdtf_player_shared.h"
#include "in_buttons.h"
#include "npcevent.h"

#ifdef CLIENT_DLL

#define CWeaponPump C_WeaponPump

#else

#include "ai_basenpc.h"

#endif

ConVar sk_plr_dmg_buckshot_870("sk_plr_dmg_buckshot_870", "9", FCVAR_REPLICATED);

extern ConVar sk_auto_reload_time;
extern ConVar sk_plr_num_shotgun_pellets;

class CWeaponPump : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponPump, CBaseHDTFCombatWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	CWeaponPump( );

	Activity GetPrimaryAttackActivity()
	{
		if (IsIronsightsEnabled())
			return ACT_VM_PRIMARYATTACK_IRONSIGHTS;

		return ACT_VM_PRIMARYATTACK;
	}

	Activity GetShotgunPumpActivity()
	{
		if (IsIronsightsEnabled())
			return ACT_VM_RELOAD_SPECIAL;

		return ACT_SHOTGUN_PUMP;
	}

	int GetMinBurst( )
	{
		return 1;
	}

	int GetMaxBurst( )
	{
		return 3;
	}

	float GetFireRate( )
	{
		return 0.7f;
	}

	bool StartReload( );
	bool Reload( );
	void FillClip( );
	void FinishReload( );
	void Pump( );
	void ItemHolsterFrame( );
	void ItemPostFrame( );
	void PrimaryAttack( );
	void DryFire( );

#ifndef CLIENT_DLL
	bool BusyFrameForcesUnLean() const { return false; }

	int CapabilitiesGet()
	{
		return bits_CAP_WEAPON_RANGE_ATTACK1;
	}

	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
#endif

private:
	CWeaponPump( const CWeaponPump & );

	CNetworkVar( bool, m_bNeedPump );		// When emptied completely
	CNetworkVar( bool, m_bDelayedFire1 );	// Fire primary when finished reloading
	CNetworkVar( bool, m_bDelayedReload );	// Reload when finished pump
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPump, DT_WeaponPump )

BEGIN_NETWORK_TABLE( CWeaponPump, DT_WeaponPump )

#ifdef CLIENT_DLL

RecvPropBool( RECVINFO( m_bNeedPump ) ),
RecvPropBool( RECVINFO( m_bDelayedFire1 ) ),
RecvPropBool( RECVINFO( m_bDelayedReload ) ),

#else

SendPropBool( SENDINFO( m_bNeedPump ) ),
SendPropBool( SENDINFO( m_bDelayedFire1 ) ),
SendPropBool( SENDINFO( m_bDelayedReload ) ),

#endif

END_NETWORK_TABLE( )

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponPump )
DEFINE_PRED_FIELD( m_bNeedPump, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bDelayedFire1, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bDelayedReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA( )

#endif

LINK_ENTITY_TO_CLASS( weapon_pump, CWeaponPump );
PRECACHE_WEAPON_REGISTER( weapon_pump );

#ifndef CLIENT_DLL

acttable_t	CWeaponPump::m_acttable[] =
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },	// FIXME: hook to shotgun unique

	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SHOTGUN,					false },
	{ ACT_WALK,						ACT_WALK_RIFLE,						true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SHOTGUN,				true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SHOTGUN_RELAXED,		false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SHOTGUN_STIMULATED,	false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_SHOTGUN_AGITATED,		false },//always aims

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

	{ ACT_WALK_AIM,					ACT_WALK_AIM_SHOTGUN,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,				true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,			true },
	{ ACT_RUN,						ACT_RUN_RIFLE,						true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_SHOTGUN,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,				true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,			true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SHOTGUN_LOW,		true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SHOTGUN_LOW,				false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SHOTGUN,			false },
};

IMPLEMENT_ACTTABLE( CWeaponPump );

#endif

CWeaponPump::CWeaponPump( )
{
	m_bReloadsSingly = true;

	m_bNeedPump = false;
	m_bDelayedFire1 = false;

	m_fMinRange1 = 0.0;
	m_fMaxRange1 = 500;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
//-----------------------------------------------------------------------------
bool CWeaponPump::StartReload( )
{
	if (m_bNeedPump || m_flNextPrimaryAttack > gpGlobals->curtime)
		return false;

	CBaseCombatCharacter *pOwner = GetOwner( );
	if( pOwner == NULL )
		return false;

	if( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return false;

	if( m_iClip1 >= GetMaxClip1( ) )
		return false;

	int j = Min( 1, pOwner->GetAmmoCount( m_iPrimaryAmmoType ) );
	if( j <= 0 )
		return false;

	if( m_iClip1 == 0 )
		m_bNeedPump = true;

	if (IsIronsightsEnabled() && !m_bDelayedReload)
	{
		SendWeaponAnim(ACT_VM_IRONSIGHTS_TO_IDLE);
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

		m_bDelayedReload = true;
		m_bIsIronsighted = false;
		m_bNeedPump = false;

		if (pOwner)
			pOwner->SetNextAttack(m_flNextPrimaryAttack);

		return false;
	}

	SendWeaponAnim( ACT_SHOTGUN_RELOAD_START );

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );

	m_bIsIronsighted = false;
	m_bDelayedReload = false;
	m_bInReload = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
//-----------------------------------------------------------------------------
bool CWeaponPump::Reload( )
{
	// Check that StartReload was called first
	if( !m_bInReload )
		Warning( "ERROR: Shotgun Reload called incorrectly!\n" );

	CBaseCombatCharacter *pOwner = GetOwner( );
	if( pOwner == NULL )
		return false;

	if( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return false;

	if( m_iClip1 >= GetMaxClip1( ) )
		return false;

	int j = Min( 1, pOwner->GetAmmoCount( m_iPrimaryAmmoType ) );
	if( j <= 0 )
		return false;

	FillClip( );
	SendWeaponAnim( ACT_VM_RELOAD );

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
//-----------------------------------------------------------------------------
void CWeaponPump::FinishReload( )
{
	CBaseCombatCharacter *pOwner = GetOwner( );
	if( pOwner == NULL )
		return;

	m_bInReload = false;

	// Finish reload animation
	if (m_bNeedPump)
	{
		SendWeaponAnim(ACT_VM_RELOAD_EMPTY);
		m_bNeedPump = false;
	}
	else
		SendWeaponAnim(ACT_SHOTGUN_RELOAD_FINISH);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
//-----------------------------------------------------------------------------
void CWeaponPump::FillClip( )
{
	CBaseCombatCharacter *pOwner = GetOwner( );
	if( pOwner == NULL )
		return;

	// Add them to the clip
	if( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) > 0 && Clip1( ) < GetMaxClip1( ) )
	{
		++m_iClip1;
		pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
	}
}

void CWeaponPump::Pump( )
{
	CBaseCombatCharacter *pOwner = GetOwner( );
	if( pOwner == NULL )
		return;

	m_bNeedPump = false;

	// Finish reload animation
	SendWeaponAnim( GetShotgunPumpActivity() );

	pOwner->m_flNextAttack = gpGlobals->curtime + SequenceDuration( );
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
}

void CWeaponPump::DryFire( )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
}

void CWeaponPump::PrimaryAttack( )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound( SINGLE );

	pPlayer->DoMuzzleFlash( );
	m_fShotLast = gpGlobals->curtime;
	DoSmokeBarrel();

	SendWeaponAnim( GetPrimaryAttackActivity() );

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
	m_iClip1 -= 1;

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = pPlayer->Weapon_ShootPosition( );
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	FireBulletsInfo_t info( 7, vecSrc, vecAiming, GetBulletSpread( ), MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
	info.m_flDamage = sk_plr_dmg_buckshot_870.GetFloat();
	info.m_pAttacker = pPlayer;

	// Fire the bullets, and force the first shot to be perfectly accuracy
	pPlayer->FireBullets( info );

	QAngle punch;
	punch.Init( SharedRandomFloat( "doublebarrelpax", -2, -1 ), SharedRandomFloat( "doublebarrelpay", -2, 2 ), 0 );
	pPlayer->ViewPunch( punch );

	if( m_iClip1 == 0 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );

	m_bNeedPump = true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do mulitple reloads in a row
//-----------------------------------------------------------------------------
void CWeaponPump::ItemPostFrame( )
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner == NULL)
		return;

	if( m_bNeedPump && !m_bInReload && ( pOwner->m_nButtons & IN_RELOAD ) != 0 )
		m_bDelayedReload = true;

	CheckLoweredCondition();

	if( m_bInReload )
	{
		// If I'm primary firing and have one round stop reloading and fire
		if( ( pOwner->m_nButtons & IN_ATTACK ) != 0 && m_iClip1 >= 1 )
		{
			m_bInReload = false;
			m_bNeedPump = false;
			m_bDelayedFire1 = true;
		}
		else if( m_flNextPrimaryAttack <= gpGlobals->curtime )
		{
			// If out of ammo end reload
			if( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
			{
				FinishReload( );
				return;
			}

			// If clip not full reload again
			if( m_iClip1 < GetMaxClip1( ) )
			{
				Reload( );
				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload( );
				return;
			}
		}
		return;
	}

	if( m_bNeedPump && m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		Pump( );
		return;
	}

	if( ( m_bDelayedFire1 || ( pOwner->m_nButtons & IN_ATTACK ) != 0 ) && m_flNextPrimaryAttack <= gpGlobals->curtime && !m_bLowered )
	{
		m_bDelayedFire1 = false;
		if( ( m_iClip1 <= 0 && UsesClipsForAmmo1( ) ) || ( !UsesClipsForAmmo1( ) && pOwner->GetAmmoCount( m_iPrimaryAmmoType ) == 0 ) )
		{
			if( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) == 0 )
				DryFire( );
			else
				StartReload( );
		}
		// Fire underwater?
		else if( pOwner->GetWaterLevel( ) == 3 && m_bFiresUnderwater == false )
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			// If the firing button was just pressed, reset the firing time
			if( ( pOwner->m_afButtonPressed & IN_ATTACK ) != 0 )
				m_flNextPrimaryAttack = gpGlobals->curtime;

			PrimaryAttack( );
		}
	}

		bool reloadButton = ( pOwner->m_nButtons & IN_RELOAD ) != 0;

	if (reloadButton)
	{
#ifndef CLIENT_DLL
		m_flHudRevealTimer += gpGlobals->frametime;
		if (m_flHudRevealTimer >= HDTF_HUD_REVEAL_TIMER)
		{
			CRecipientFilter filter;
			filter.AddRecipient(pOwner);
			UserMessageBegin(filter, "RevealHud");
			MessageEnd();
		}
#endif
	}
	else if( !reloadButton && m_bIsReloadKeyDown && (m_flHudRevealTimer < HDTF_HUD_REVEAL_TIMER)
		&& !m_bInReload && !m_bNeedPump && m_flNextPrimaryAttack <= gpGlobals->curtime || m_bDelayedReload )
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		m_flHudRevealTimer = 0.f;
		StartReload( );
	}
	else
	{
		// no fire buttons down
		m_bFireOnEmpty = false;

		if( !HasAnyAmmo( ) && m_flNextPrimaryAttack < gpGlobals->curtime )
		{
			// weapon isn't useable, switch.
			if( ( GetWeaponFlags( ) & ITEM_FLAG_NOAUTOSWITCHEMPTY ) == 0 && pOwner->SwitchToNextBestWeapon( this ) )
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
				return;
			}
		}
		else
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if( m_iClip1 <= 0 && ( GetWeaponFlags( ) & ITEM_FLAG_NOAUTORELOAD ) == 0 && m_flNextPrimaryAttack < gpGlobals->curtime && StartReload( ) )
				// if we've successfully started to reload, we're done
				return;
		}

		if (!m_bInReload && !m_bNeedPump && !m_bDelayedReload)
		{
			bool secondaryButton = (pOwner->m_nButtons & IN_ATTACK2) != 0;

			if (secondaryButton && !IsIronsightsEnabled())
				EnableIronsights();
			else if (!secondaryButton && IsIronsightsEnabled())
				DisableIronsights();
		}
		
		WeaponIdle( );
	}

	m_bIsReloadKeyDown = reloadButton;
	if (!reloadButton)
		m_flHudRevealTimer = 0.f;

	// Check if we should lean from corner
#ifndef CLIENT_DLL
	ProcessCornerLean();
#endif
}

void CWeaponPump::ItemHolsterFrame( )
{
	// Must be player held
	CBaseCombatCharacter *pOwner = GetOwner( );
	if( pOwner != NULL && !pOwner->IsPlayer( ) )
		return;

	// We can't be active
	if( pOwner->GetActiveWeapon( ) == this )
		return;

	// If it's been longer than three seconds, reload
	//if( gpGlobals->curtime - m_flHolsterTime > sk_auto_reload_time.GetFloat( ) )
	//{
	//	// Reset the timer
	//	m_flHolsterTime = gpGlobals->curtime;
	//
	//	if( pOwner == NULL )
	//		return;
	//
	//	if( m_iClip1 == GetMaxClip1( ) )
	//		return;
	//
	//	// Just load the clip with no animations
	//	int ammoFill = Min( GetMaxClip1( ) - m_iClip1, pOwner->GetAmmoCount( GetPrimaryAmmoType( ) ) );
	//
	//	pOwner->RemoveAmmo( ammoFill, GetPrimaryAmmoType( ) );
	//	m_iClip1 += ammoFill;
	//}
}

#ifndef CLIENT_DLL

void CWeaponPump::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);
	WeaponSound(SINGLE_NPC);
	pOperator->DoMuzzleFlash();
	m_iClip1--;

	if (bUseWeaponAngles)
	{
		QAngle	angShootDir;
		GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
		AngleVectors(angShootDir, &vecShootDir);
	}
	else
	{
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);
	}

	pOperator->FireBullets(7, vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0, entindex(), 0);
}

void CWeaponPump::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	FireNPCPrimaryAttack(pOperator, true);
}

void CWeaponPump::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SHOTGUN_FIRE:
	{
		FireNPCPrimaryAttack(pOperator, false);

		break;
	}

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

#endif
