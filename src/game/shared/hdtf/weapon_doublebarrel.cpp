#include "cbase.h"
#include "weapon_basehdtfcombat.h"
#include "in_buttons.h"
#include "npcevent.h"
#include "util.h"

ConVar sk_plr_dmg_buckshot_db("sk_plr_dmg_buckshot_db", "13", FCVAR_REPLICATED);

#ifdef CLIENT_DLL

#define CWeaponDoubleBarrel C_WeaponDoubleBarrel

#endif

#define ACT_VM_ATTACH_FLASHLIGHT ACT_VM_ATTACH_SILENCER

class CWeaponDoubleBarrel : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponDoubleBarrel, CBaseHDTFCombatWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );
	DECLARE_DATADESC();

#endif

	CWeaponDoubleBarrel( );

	virtual const Vector &GetBulletSpread( )
	{
		static Vector cone = VECTOR_CONE_10DEGREES;
		return cone;
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

	void ItemPostFrame( );
	void PrimaryAttack( );
	void SecondaryAttack( );
	bool Reload();
	void FillClip();
	bool Deploy();
	bool Holster(CBaseCombatWeapon *pSwitchingTo);

	void InputEnableFlashlight(inputdata_t &inputData);
	void InputDisableFlashlight(inputdata_t &inputData);

	bool HasFlashlight() { return m_bHasFlashlight && m_bIsFlashlightEnabled; }

	void SetFlashlightBodygroupState(bool enabled);

#ifndef CLIENT_DLL
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
#endif

private:
	CWeaponDoubleBarrel( const CWeaponDoubleBarrel & );
	
	bool m_bDelayedFlashlight;

	CNetworkVar(bool, m_bHasFlashlight);
	CNetworkVar(bool, m_bIsFlashlightEnabled);
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponDoubleBarrel, DT_WeaponDoubleBarrel )

BEGIN_NETWORK_TABLE(CWeaponDoubleBarrel, DT_WeaponDoubleBarrel)

#ifdef CLIENT_DLL
	RecvPropBool(RECVINFO(m_bHasFlashlight)),
	RecvPropBool(RECVINFO(m_bIsFlashlightEnabled))
#else
	SendPropBool(SENDINFO(m_bHasFlashlight)),
	SendPropBool(SENDINFO(m_bIsFlashlightEnabled))
#endif

END_NETWORK_TABLE()

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA(CWeaponDoubleBarrel)
DEFINE_PRED_FIELD(m_bHasFlashlight, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bIsFlashlightEnabled, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE)
END_PREDICTION_DATA()

#endif

#ifndef CLIENT_DLL
BEGIN_DATADESC(CWeaponDoubleBarrel)
DEFINE_FIELD(m_bHasFlashlight, FIELD_BOOLEAN),
DEFINE_FIELD(m_bDelayedFlashlight, FIELD_BOOLEAN),
DEFINE_FIELD(m_bIsFlashlightEnabled, FIELD_BOOLEAN),
DEFINE_INPUTFUNC(FIELD_VOID, "EnableFlashlight", InputEnableFlashlight),
DEFINE_INPUTFUNC(FIELD_VOID, "DisableFlashlight", InputDisableFlashlight)
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( weapon_doublebarrel, CWeaponDoubleBarrel );
PRECACHE_WEAPON_REGISTER( weapon_doublebarrel );

#ifndef CLIENT_DLL

acttable_t CWeaponDoubleBarrel::m_acttable[] =
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

IMPLEMENT_ACTTABLE( CWeaponDoubleBarrel );

#endif

CWeaponDoubleBarrel::CWeaponDoubleBarrel( )
{
	m_fMinRange1 = 0.0;
	m_fMaxRange1 = 500;
	m_fMinRange2 = 0.0;
	m_fMaxRange2 = 200;

	m_bDelayedFlashlight = false;
}

bool CWeaponDoubleBarrel::Deploy()
{
	const bool result = BaseClass::Deploy();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (result && pPlayer)
	{
		if (m_bHasFlashlight && !m_bDelayedFlashlight)
		{
			m_bIsFlashlightEnabled = true;

			SetFlashlightBodygroupState(true);
		}
		else
		{
			SetFlashlightBodygroupState(false);
		}
	}

	return result;
}

bool CWeaponDoubleBarrel::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	const bool result = BaseClass::Holster(pSwitchingTo);

	if (result)
	{
		m_bIsFlashlightEnabled = false;
	}

	return result;
}

void CWeaponDoubleBarrel::PrimaryAttack( )
{
	if (m_iClip1 <= 0)
		return;

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound( SINGLE );

	pPlayer->DoMuzzleFlash( );

	m_fShotLast = gpGlobals->curtime;
	DoSmokeBarrel();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
	m_iClip1 -= 1;

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = pPlayer->Weapon_ShootPosition( );
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	FireBulletsInfo_t info( 7, vecSrc, vecAiming, GetBulletSpread( ), MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
	info.m_flDamage = sk_plr_dmg_buckshot_db.GetFloat();
	info.m_pAttacker = pPlayer;

	// Fire the bullets, and force the first shot to be perfectly accuracy
	pPlayer->FireBullets( info );

	QAngle punch;
	punch.Init( SharedRandomFloat( "doublebarrelpax", -2, -1 ), SharedRandomFloat( "doublebarrelpay", -2, 2 ), 0 );
	pPlayer->ViewPunch( punch );

	m_bInReload = false;

	if( m_iClip1 == 0 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
}

void CWeaponDoubleBarrel::SecondaryAttack( )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	if (m_iClip1 - 2 < 0)
	{
		// we don't have enough ammo to shot two barrels
		// fire primary attack instead
		PrimaryAttack();
		return;
	}

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound( WPN_DOUBLE );

	pPlayer->DoMuzzleFlash( );

	SendWeaponAnim( ACT_VM_SECONDARYATTACK );

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
	m_iClip1 -= 2; // Shotgun uses same clip for primary and secondary attacks

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = pPlayer->Weapon_ShootPosition( );
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	FireBulletsInfo_t info( 12, vecSrc, vecAiming, GetBulletSpread( ), MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
	info.m_flDamage = sk_plr_dmg_buckshot_db.GetFloat();
	info.m_pAttacker = pPlayer;

	// Fire the bullets, and force the first shot to be perfectly accuracy
	pPlayer->FireBullets( info );
	pPlayer->ViewPunch( QAngle( SharedRandomFloat( "doublebarrelsax", -5, 5 ), 0, 0 ) );

	if( m_iClip1 == 0 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Reload
//-----------------------------------------------------------------------------
bool CWeaponDoubleBarrel::Reload()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	if (m_iClip1 == 0 && pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 1)
		SendWeaponAnim(ACT_VM_RELOAD_EMPTY);
	else
		SendWeaponAnim(ACT_VM_RELOAD);

	m_flNextPrimaryAttack = m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();
	
	m_bInReload = true;

	return true;
}

void CWeaponDoubleBarrel::FillClip()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return;

	m_iClip1 += 1;
	pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
}

#ifndef CLIENT_DLL
void CWeaponDoubleBarrel::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_RELOAD_FILL_CLIP:
		FillClip();
		break;

	case EVENT_WEAPON_MELEE_SWISH:
		m_bIsFlashlightEnabled = true;
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do mulitple reloads in a row
//-----------------------------------------------------------------------------
void CWeaponDoubleBarrel::ItemPostFrame( )
{

	bool autoreload = false;
	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if (pOwner == NULL)
		return;

	if (m_bDelayedFlashlight && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		m_bDelayedFlashlight = false;
		SendWeaponAnim(ACT_VM_ATTACH_FLASHLIGHT);
		SetFlashlightBodygroupState(true);
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

		return;
	}

	if (HasAnyAmmo() && m_iClip1 <= 0 && !m_bInReload && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		autoreload = true;
		Reload();
	}

	if(m_bInReload && m_flNextPrimaryAttack <= gpGlobals->curtime)
		m_bInReload = false;

	bool reloadButton = (pOwner->m_nButtons & IN_RELOAD) != 0;

	if ((pOwner->m_nButtons & IN_ATTACK) != 0 && (m_flNextPrimaryAttack <= gpGlobals->curtime || m_bInReload))
	{
		if (m_iClip1 > 0)
			PrimaryAttack();
		else if (HasAnyAmmo() && !m_bInReload && !autoreload)
			Reload();
		else if (!m_bInReload && pOwner->SwitchToNextBestWeapon(this))
		{
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.3f;
		}
	}
	else if ((pOwner->m_nButtons & IN_ATTACK2) != 0 && m_flNextPrimaryAttack <= gpGlobals->curtime)
		SecondaryAttack();
	else if(!reloadButton)
		WeaponIdle();

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
	else if (!reloadButton && m_bIsReloadKeyDown && (m_flHudRevealTimer < HDTF_HUD_REVEAL_TIMER) 
		&& m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		m_flHudRevealTimer = 0.f;
		Reload();
	}

	m_bIsReloadKeyDown = reloadButton;
	if (!reloadButton)
		m_flHudRevealTimer = 0.f;
}

void CWeaponDoubleBarrel::SetFlashlightBodygroupState(bool enabled)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	const int flashlightBodyGroup = pPlayer->GetViewModel()->FindBodygroupByName("Flashlight");
	if (flashlightBodyGroup != -1)
		pPlayer->GetViewModel()->SetBodygroup(flashlightBodyGroup, enabled);
}

//-----------------------------------------------------------------------------
// Purpose: Input to enable flashlight
//-----------------------------------------------------------------------------
void CWeaponDoubleBarrel::InputEnableFlashlight(inputdata_t &inputData)
{
	if (m_bHasFlashlight)
		return;
	
#ifndef CLIENT_DLL
	if (!inputData.value.Bool())
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		if (pOwner)
		{
			m_bHasFlashlight = true;

			if (pOwner->GetActiveWeapon() != this)
			{
				SetVisibleInWeaponSelection(true);
				for (int i = 0; i < pOwner->WeaponCount(); i++)
				{
					CBaseHDTFCombatWeapon *iter = dynamic_cast<CBaseHDTFCombatWeapon *>(pOwner->GetWeapon(i));
					if (iter != NULL && iter->GetSlot() == GetSlot() && iter != this)
					{
						iter->SetVisibleInWeaponSelection(false);
					}
				}

				m_bDelayedFlashlight = true;
				pOwner->Weapon_Switch(this);

				SendWeaponAnim(ACT_VM_DRAW);
				m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

				return;
			}
		}

		SendWeaponAnim(ACT_VM_ATTACH_FLASHLIGHT);
		SetFlashlightBodygroupState(true);
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Input to disable flashlight
//-----------------------------------------------------------------------------
void CWeaponDoubleBarrel::InputDisableFlashlight(inputdata_t &inputData)
{
	m_bHasFlashlight = false;
	m_bIsFlashlightEnabled = false;
}
