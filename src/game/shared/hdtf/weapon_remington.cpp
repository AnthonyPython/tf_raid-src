#include "cbase.h"
#include "hdtf_player_shared.h"
#include "weapon_basehdtfcombat.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL

#define CWeaponRemington C_WeaponRemington

#endif

// done for convenience and readability. we really don't need
// to add new activities just for one weapon
#define ACT_VM_PRONE_MOVE ACT_VM_SPRINT_IDLE_SPECIAL
#define ACT_VM_IDLE_TO_COLDWALK ACT_VM_SECONDARYATTACK
#define ACT_VM_IDLE_COLDWALK ACT_VM_IDLE_SPECIAL
#define ACT_VM_COLDWALK_TO_IDLE ACT_VM_SECONDARYATTACK_SPECIAL

#define K98_TIME_TO_COLDWALK 1.25f

extern ConVar sk_auto_reload_time;
extern ConVar sk_plr_num_shotgun_pellets;

ConVar sk_plr_dmg_remington("sk_plr_dmg_remington", "75", FCVAR_REPLICATED);

class CWeaponRemington : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponRemington, CBaseHDTFCombatWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );
	DECLARE_DATADESC();

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	CWeaponRemington( );

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

	float GetIronSensitivity()
	{
		return 0.30f;
	}

	bool StartReload( );
	bool Reload( );
	void FillClip( );
	void FinishReload( );
	void ItemHolsterFrame( );
	void ItemPostFrame( );
	void PrimaryAttack( );
	void DryFire( );

	bool Deploy();
	bool Holster(CBaseCombatWeapon *pSwitchTo);

	void WeaponIdle();

	void InventoryDeploy();

	void EnableIronsights();
	void DisableIronsights(bool ignoreAnimation = false);

	virtual float GetIronSpeed() { return 1.7f; }

#ifdef CLIENT_DLL
	virtual EScopeType GetScopeType() { return EScopeType::SCOPE_SNIPER; }
	virtual char *GetScopeReticleTextureName() { return "reticle/k98"; }
#endif

#ifdef GAME_DLL
	virtual void ModifyDamageViewPunch(float& flOutPunch)
	{
		if (IsIronsightsEnabled())
		{
			DevMsg("we are modifying viewpunch rn\n");
			flOutPunch = -0.3f;
		}
	}
#endif // GAME_DLL


private:
	CWeaponRemington( const CWeaponRemington & );

	CNetworkVar( bool, m_bDelayedFire1 );	// Fire primary when finished reloading
	CNetworkVar( bool, m_bDelayedReload );
	CNetworkVar( float, m_flNextIdleTime );
	float m_flTimeIdle;
};

BEGIN_DATADESC(CWeaponRemington)

DEFINE_FIELD(m_flTimeIdle, FIELD_FLOAT),

END_DATADESC()

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRemington, DT_WeaponRemington )

BEGIN_NETWORK_TABLE( CWeaponRemington, DT_WeaponRemington )

#ifdef CLIENT_DLL

RecvPropBool( RECVINFO( m_bDelayedFire1 ) ),
RecvPropBool( RECVINFO( m_bDelayedReload ) ),
RecvPropFloat( RECVINFO( m_flNextIdleTime ) ),

#else

SendPropBool( SENDINFO( m_bDelayedFire1 ) ),
SendPropBool( SENDINFO( m_bDelayedReload ) ),
SendPropFloat( SENDINFO( m_flNextIdleTime ) )

#endif

END_NETWORK_TABLE( )

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponRemington )
DEFINE_PRED_FIELD( m_bDelayedFire1, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bDelayedReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_flNextIdleTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE )
END_PREDICTION_DATA( )

#endif

LINK_ENTITY_TO_CLASS( weapon_kar98, CWeaponRemington );
PRECACHE_WEAPON_REGISTER( weapon_kar98 );

#ifndef CLIENT_DLL

acttable_t CWeaponRemington::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SHOTGUN, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SHOTGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_SHOTGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SHOTGUN, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, false },
};

IMPLEMENT_ACTTABLE( CWeaponRemington );

#endif

CWeaponRemington::CWeaponRemington( )
{
	m_bReloadsSingly = true;

	m_bDelayedFire1 = false;
	m_bDelayedReload = false;

	m_fMinRange1 = 0.0;
	m_fMaxRange1 = 500;

	m_flTimeIdle = 0.f;
	m_flNextIdleTime = 0.f;
}

bool CWeaponRemington::Deploy()
{
	const bool ok = BaseClass::Deploy();

	if (ok)
	{
		m_flNextIdleTime = gpGlobals->curtime;
	}

	return ok;
}

bool CWeaponRemington::Holster(CBaseCombatWeapon *pSwitchTo)
{
	const bool ok = BaseClass::Holster(pSwitchTo);

	if (ok)
	{
		m_bInReload = false;
	}

	return ok;
}

void CWeaponRemington::InventoryDeploy()
{
	BaseClass::InventoryDeploy();

	m_flNextIdleTime = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
//-----------------------------------------------------------------------------
bool CWeaponRemington::StartReload( )
{
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

	if (IsIronsightsEnabled())
	{
		CBasePlayer *pPlayer = ToBasePlayer(pOwner);
		pPlayer->SetFOV(this, 0, 0.35f);
		SendWeaponAnim(ACT_VM_IRONSIGHTS_TO_IDLE);
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

		m_bDelayedReload = true;
		m_bIsIronsighted = false;

		if (pOwner)
			pOwner->SetNextAttack(m_flNextPrimaryAttack);

		return false;
	}

	SendWeaponAnim( ACT_SHOTGUN_RELOAD_START );

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );

	m_bDelayedReload = false;
	m_bInReload = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
//-----------------------------------------------------------------------------
bool CWeaponRemington::Reload( )
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
	// Play reload on different channel as otherwise steals channel away from fire sound
	SendWeaponAnim( ACT_VM_RELOAD );

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
	pOwner->m_flNextAttack = m_flNextPrimaryAttack;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
//-----------------------------------------------------------------------------
void CWeaponRemington::FinishReload( )
{
	CBaseCombatCharacter *pOwner = GetOwner( );
	if( pOwner == NULL )
		return;

	m_bInReload = false;

	// Finish reload animation
	SendWeaponAnim( ACT_SHOTGUN_RELOAD_FINISH );

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
	pOwner->m_flNextAttack = m_flNextPrimaryAttack;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
//-----------------------------------------------------------------------------
void CWeaponRemington::FillClip( )
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

void CWeaponRemington::DryFire( )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
}

void CWeaponRemington::PrimaryAttack( )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	if (GetActivity() == ACT_VM_PRONE_MOVE)
	{
		SendWeaponAnim(ACT_VM_IDLE);
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.15f;
		m_bDelayedFire1 = true;
		return;
	}

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

	FireBulletsInfo_t info( 1, vecSrc, vecAiming, GetBulletSpread( ), MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
	info.m_pAttacker = pPlayer;
	info.m_flDamage = sk_plr_dmg_remington.GetFloat(); // override damage

	// Fire the bullets, and force the first shot to be perfectly accuracy
	pPlayer->FireBullets( info );

	QAngle punch;
	punch.Init( SharedRandomFloat( "doublebarrelpax", -2, -1 ), SharedRandomFloat( "doublebarrelpay", -2, 2 ), 0 );
	pPlayer->ViewPunch( punch );

	if( m_iClip1 == 0 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do mulitple reloads in a row
//-----------------------------------------------------------------------------
void CWeaponRemington::ItemPostFrame( )
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner == NULL)
		return;

	CheckLoweredCondition();

	if (m_bDelayedReload && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		StartReload();
		return;
	}

	if( m_bInReload )
	{
		// If I'm primary firing and have one round stop reloading and fire
		if( ( pOwner->m_nButtons & IN_ATTACK ) != 0 && m_iClip1 >= 1 )
		{
			m_bInReload = false;
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

	if( ( m_bDelayedFire1 || ( pOwner->m_nButtons & IN_ATTACK ) != 0 ) && m_flNextPrimaryAttack <= gpGlobals->curtime )
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
		&& !m_bInReload && ( m_flNextPrimaryAttack <= gpGlobals->curtime || m_flTimeIdle > K98_TIME_TO_COLDWALK ) )
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
		
		// consider next primary attack too because we want
		// our shot animation to play to the end
		if (!m_bInReload && m_flNextPrimaryAttack < gpGlobals->curtime)
		{
			bool secondaryButton = (pOwner->m_nButtons & IN_ATTACK2) != 0;

			if (secondaryButton && !IsIronsightsEnabled())
				EnableIronsights();
			else if (!secondaryButton && IsIronsightsEnabled())
				DisableIronsights();
		}

		if(m_flNextIdleTime <= gpGlobals->curtime)
			WeaponIdle( );
	}

	m_bIsReloadKeyDown = reloadButton;
	if(!reloadButton)
		m_flHudRevealTimer = 0.f;

	// Check if we should lean from corner
#ifndef CLIENT_DLL
	ProcessCornerLean();
#endif
}

void CWeaponRemington::ItemHolsterFrame( )
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

void CWeaponRemington::WeaponIdle()
{
	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	// we should not idle while reloading
	if (m_bInReload)
		return;

	Activity currentAct = GetActivity();

#ifndef CLIENT_DLL
	const bool isMoving = (pPlayer->m_flForwardMove > 0 || pPlayer->m_flSideMove != 0);

	if (isMoving && !pPlayer->IsSprinting() 
		&& !IsIronsightsEnabled() && (pPlayer->m_nButtons & IN_ATTACK) == 0
		&& (pPlayer->m_nButtons & IN_RELOAD) == 0 && (pPlayer->m_nButtons & IN_ATTACK2) == 0
		// hack to prevent player from 'freezing' inside of the factory
		&& !FStrEq(STRING(gpGlobals->mapname), "hdtf_newalaska_4b"))
		m_flTimeIdle += gpGlobals->frametime;
	else
		m_flTimeIdle = 0;

	if (pPlayer->IsProne())
	{
		if (isMoving)
		{
			if((currentAct != ACT_VM_PRONE_MOVE || HasWeaponIdleTimeElapsed()) && m_flNextPrimaryAttack + 0.5f <= gpGlobals->curtime)
				SendWeaponAnim(ACT_VM_PRONE_MOVE);

			return;
		}
		else
		{
			if (currentAct == ACT_VM_PRONE_MOVE)
			{
				if (IsIronsightsEnabled())
					SendWeaponAnim(ACT_VM_IDLE_TO_IRONSIGHTS);
				else
					SendWeaponAnim(ACT_VM_IDLE);
			}

			if (currentAct == ACT_VM_COLDWALK_TO_IDLE && !HasWeaponIdleTimeElapsed())
				return;
		}
	}
	else
	{
		if (currentAct == ACT_VM_PRONE_MOVE)
		{
			if(IsIronsightsEnabled())
				SendWeaponAnim(ACT_VM_IDLE_TO_IRONSIGHTS);
			else
				SendWeaponAnim(ACT_VM_IDLE);
		}
	}
#endif

	if (m_flTimeIdle > K98_TIME_TO_COLDWALK)
	{
		if (currentAct != ACT_VM_IDLE_TO_COLDWALK
			&& currentAct != ACT_VM_IDLE_COLDWALK)
		{
			SendWeaponAnim(ACT_VM_IDLE_TO_COLDWALK);
		}
		else if(HasWeaponIdleTimeElapsed())
			SendWeaponAnim(ACT_VM_IDLE_COLDWALK);

		m_flNextPrimaryAttack = gpGlobals->curtime + 0.25f;

		return;
	}
	else
	{
		if (currentAct == ACT_VM_IDLE_TO_COLDWALK
			|| currentAct == ACT_VM_IDLE_COLDWALK)
		{
			SendWeaponAnim(ACT_VM_COLDWALK_TO_IDLE);
			currentAct = ACT_VM_COLDWALK_TO_IDLE;
			m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		}

		if (currentAct == ACT_VM_COLDWALK_TO_IDLE && !HasWeaponIdleTimeElapsed())
			return;
	}

	BaseClass::WeaponIdle();
}

void CWeaponRemington::EnableIronsights()
{
	if (!HasIronsights() || IsIronsightsEnabled() || m_bInReload)
		return;

	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner != NULL)
	{
		m_bIsIronsighted = true;
		pOwner->EmitSound("Player.IronsightsIn");
		pOwner->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;

		pOwner->SetFOV(this, CalcViewCorrectedFov(60), 0.65f);

#ifndef CLIENT_DLL
		const bool isMoving = (pOwner->m_flForwardMove > 0 || pOwner->m_flSideMove != 0);
		if (!pOwner->IsProne() || !isMoving)
		{
			SendWeaponAnim(IsSpecialEnabled() ? ACT_VM_IDLE_TO_IRONSIGHTS_SPECIAL : ACT_VM_IDLE_TO_IRONSIGHTS);

			m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
			m_flNextSecondaryAttack = m_flNextPrimaryAttack;
			m_flNextSpecialAttack = m_flNextPrimaryAttack;
		}
#endif
	}
}

void CWeaponRemington::DisableIronsights(bool ignoreAnimation)
{
	if (!HasIronsights() || !IsIronsightsEnabled())
		return;

	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner != NULL)
	{
		m_bIsIronsighted = false;
		pOwner->EmitSound("Player.IronsightsOut");
		pOwner->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
		pOwner->SetFOV(this, 0, 0.35f);

#ifndef CLIENT_DLL
		const bool isMoving = (pOwner->m_flForwardMove > 0 || pOwner->m_flSideMove != 0);
		if ((!pOwner->IsProne() || !isMoving) && !ignoreAnimation)
		{
			SendWeaponAnim(IsSpecialEnabled() ? ACT_VM_IRONSIGHTS_TO_IDLE_SPECIAL : ACT_VM_IRONSIGHTS_TO_IDLE);
			m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
			m_flNextSecondaryAttack = m_flNextPrimaryAttack;
			m_flNextSpecialAttack = m_flNextPrimaryAttack;
		}
#endif
	}
}