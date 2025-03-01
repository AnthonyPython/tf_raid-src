#include "cbase.h"
#include "weapon_basehdtfcombat.h"
#include "hl2_player_shared.h"
#include "in_buttons.h"
#include "model_types.h"
#include "tempentity.h"
#include "laser_dot.h"
#include "beam_shared.h"
#include "hdtf_player_shared.h"

#ifdef CLIENT_DLL

#include "beamdraw.h"
#include "iviewrender_beams.h"
#include "prediction.h"
#include "c_baseviewmodel.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>

#else

#include "ehandle.h"
#include "globalstate.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	LASER_BEAM_SPRITE "effects/laser1.vmt"
#define	LASER_BEAM_SPRITE_NOZ "effects/laser1_noz.vmt"

#define HIDEWEAPON_THINK_CONTEXT "BaseCombatWeapon_HideThink"

#define IRONSIGHT_TIME 0.2f

#ifndef CLIENT_DLL
static ConVar hdtf_autolean("hdtf_autolean", "1", FCVAR_ARCHIVE, "Lean to nearest corner when facing the wall while aiming");
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( BaseHDTFCombatWeapon, DT_HDTFBaseCombatWeapon )

BEGIN_NETWORK_TABLE( CBaseHDTFCombatWeapon, DT_HDTFBaseCombatWeapon )

#ifndef CLIENT_DLL

SendPropFloat( SENDINFO( m_flNextSpecialAttack ) ),
SendPropVector( SENDINFO( m_vecNPCLaserDot ) ),
SendPropBool( SENDINFO( m_bLaserActive ) ),
SendPropEHandle( SENDINFO( m_hLaserDot ) ),
SendPropBool( SENDINFO( m_bVisibleInWeaponSelection ) ),
SendPropBool( SENDINFO( m_bIsIronsighted ) ),
SendPropFloat( SENDINFO( m_flIronsightedTime ) ),
SendPropBool( SENDINFO( m_bIsSpecialEnabled ) ),
SendPropBool( SENDINFO( m_bIsCornerLeaning ) ),
SendPropBool( SENDINFO( m_bIsReloadKeyDown ) ),
SendPropBool( SENDINFO( m_bDelayedReload ) ),

#else

RecvPropFloat( RECVINFO( m_flNextSpecialAttack ) ),
RecvPropVector( RECVINFO( m_vecNPCLaserDot ) ),
RecvPropBool( RECVINFO( m_bLaserActive ) ),
RecvPropEHandle( RECVINFO( m_hLaserDot ) ),
RecvPropBool( RECVINFO( m_bVisibleInWeaponSelection ) ),
RecvPropBool( RECVINFO( m_bIsIronsighted ) ),
RecvPropFloat( RECVINFO( m_flIronsightedTime ) ),
RecvPropBool( RECVINFO( m_bIsSpecialEnabled ) ),
RecvPropBool( RECVINFO( m_bIsCornerLeaning ) ),
RecvPropBool( RECVINFO( m_bIsReloadKeyDown ) ),
RecvPropBool( RECVINFO( m_bDelayedReload ) ),

#endif

END_NETWORK_TABLE( )


#ifndef CLIENT_DLL

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CBaseHDTFCombatWeapon )
DEFINE_FIELD( m_flNextSpecialAttack, FIELD_TIME ),
DEFINE_FIELD( m_vecNPCLaserDot, FIELD_VECTOR ),
DEFINE_FIELD( m_bLaserActive, FIELD_BOOLEAN ),
DEFINE_FIELD( m_hLaserDot, FIELD_EHANDLE ),
DEFINE_FIELD( m_bVisibleInWeaponSelection, FIELD_BOOLEAN ),
DEFINE_FIELD( m_bIsIronsighted, FIELD_BOOLEAN ),
DEFINE_FIELD( m_flIronsightedTime, FIELD_TIME),
DEFINE_FIELD( m_bIsSpecialEnabled, FIELD_BOOLEAN ),
DEFINE_FIELD( m_bIsCornerLeaning, FIELD_BOOLEAN ),
DEFINE_FIELD( m_bIsReloadKeyDown, FIELD_BOOLEAN ),
DEFINE_FIELD( m_flHudRevealTimer, FIELD_FLOAT ),
DEFINE_FIELD( m_bDelayedReload, FIELD_BOOLEAN ),
END_DATADESC( )

#else

BEGIN_PREDICTION_DATA( CBaseHDTFCombatWeapon )
DEFINE_PRED_FIELD( m_flNextSpecialAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_vecNPCLaserDot, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bLaserActive, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_hLaserDot, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bVisibleInWeaponSelection, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bIsIronsighted, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_flIronsightedTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bIsSpecialEnabled, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bIsCornerLeaning, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bIsReloadKeyDown, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bDelayedReload, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA( )
#endif

#ifdef CLIENT_DLL

CON_COMMAND_F( cl_reload_weapon_scripts, "Reloads weapon scripts", FCVAR_DEVELOPMENTONLY )

#else

CON_COMMAND_F( sv_reload_weapon_scripts, "Reloads weapon scripts", FCVAR_DEVELOPMENTONLY )

#endif

{
	ResetFileWeaponInfoDatabase( );
	PrecacheFileWeaponInfoDatabase( filesystem, g_pGameRules->GetEncryptionKey( ) );
}

CBaseHDTFCombatWeapon::CBaseHDTFCombatWeapon( )
{
	m_flNextSpecialAttack = 0.0f;

	m_bLaserActive = false;
	m_hLaserDot = NULL;

	m_bVisibleInWeaponSelection = false;

	m_bIsIronsighted = false;
	m_flIronsightedTime = 0.0f;

	m_bIsCornerLeaning = false;

	m_flHudRevealTimer = 0.f;

	m_bDelayedReload = false;

#ifdef CLIENT_DLL
	m_flViewBobMultiplier = 1.f;
	m_iScopeReticleTextureId = -1;
#endif
}

CBaseHDTFCombatWeapon::~CBaseHDTFCombatWeapon( )
{

#ifndef CLIENT_DLL

	if( m_hLaserDot != NULL )
	{
		UTIL_Remove( m_hLaserDot.Get( ) );
		m_hLaserDot = NULL;
	}

#endif

}

const Vector &CBaseHDTFCombatWeapon::GetBulletSpread( )
{
	if (GetOwner()->IsNPC())
	{
		return GetWpnData().vSpreads[SPREAD_NPC];
	}

	CHDTF_Player *pPlayer = ToHDTFPlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return vec3_origin;

	int spreadtype = SPREAD_STANDING;
	if( pPlayer->IsProning( ) )
		spreadtype = IsIronsightsEnabled( ) ? SPREAD_PRONING_IRONSIGHTS : SPREAD_PRONING;
	else if( pPlayer->IsDucking( ) )
		spreadtype = IsIronsightsEnabled( ) ? SPREAD_DUCKING_IRONSIGHTS : SPREAD_DUCKING;
	else if( pPlayer->GetAbsVelocity( ).LengthSqr( ) > 0.0f )
		spreadtype = IsIronsightsEnabled( ) ? SPREAD_WALKING_IRONSIGHTS : SPREAD_WALKING;
	else if( IsIronsightsEnabled( ) )
		spreadtype = SPREAD_STANDING_IRONSIGHTS;

	return GetWpnData( ).vSpreads[spreadtype];
}

Vector CBaseHDTFCombatWeapon::GetBulletSpread(WeaponProficiency_t proficiency)
{
	// for NPC use.
	Vector baseSpread = BaseClass::GetBulletSpread(proficiency);

	const WeaponProficiencyInfo_t *pProficiencyValues = GetProficiencyValues();
	float flModifier = (pProficiencyValues)[proficiency].spreadscale;
	return (baseSpread * flModifier);
}

void CBaseHDTFCombatWeapon::Activate( )
{
	BaseClass::Activate( );
}

void CBaseHDTFCombatWeapon::Precache( )
{
	BaseClass::Precache( );

	PrecacheModel( LASER_BEAM_SPRITE );
	PrecacheModel( LASER_BEAM_SPRITE_NOZ );

	PrecacheParticleSystem("hd_.44mm_muzzleflash");
	PrecacheParticleSystem("hd_.45mm_muzzleflash");
	PrecacheParticleSystem("hd_5.56mm_muzzleflash");
	PrecacheParticleSystem("hd_7.62mm_muzzleflash");
	PrecacheParticleSystem("hd_9mm_muzzleflash");
	PrecacheParticleSystem("hd_ar2_primary_muzzleflash");
	PrecacheParticleSystem("hd_buckshot_muzzleflash");

	PrecacheModel("models/weapons/ammo_1911.mdl");
	PrecacheModel("models/weapons/ammo_ak47.mdl");
	PrecacheModel("models/weapons/ammo_hl2pistol.mdl");
	PrecacheModel("models/weapons/ammo_m9.mdl");
	PrecacheModel("models/weapons/ammo_m16.mdl");
	PrecacheModel("models/weapons/ammo_mp5.mdl");
	PrecacheModel("models/weapons/ammo_sigxi.mdl");
	PrecacheModel("models/weapons/ammo_skorpion.mdl");
	PrecacheModel("models/weapons/ammo_tmp.mdl");

	// precache all hand models
	for (int i = 0; i < HDTF_PLAYER_ARM_SKIN_MAX; ++i)
	{
		if (GetWpnData().aArmModels[i] != '\0')
			PrecacheModel(GetWpnData().aArmModels[i]);
	}
}

Activity CBaseHDTFCombatWeapon::GetPrimaryAttackActivity( )
{
	if( IsSpecialEnabled( ) )
		return IsIronsightsEnabled( ) ? ACT_VM_PRIMARYATTACK_IRONSIGHTS_SPECIAL : ACT_VM_PRIMARYATTACK_SPECIAL;

	return IsIronsightsEnabled( ) ? ACT_VM_PRIMARYATTACK_IRONSIGHTS : ACT_VM_PRIMARYATTACK;
}

bool CBaseHDTFCombatWeapon::Deploy()
{
	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());
	

	const bool allowed = BaseClass::Deploy();

	if (pPlayer && this->m_iClip1 == 0 && allowed)
		pPlayer->GetViewModel()->SetPoseParameter("pose_depleted", 1.0f);

	return allowed;
}

void CBaseHDTFCombatWeapon::InventoryDeploy()
{
	//Edgecase for when we use the pose depleted param
	//-Nbc66
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	int poseparamid = LookupPoseParameter("pose_depleted");

	if (m_iClip1 == 0 && pPlayer && pPlayer->GetViewModel()->GetPoseParameter(poseparamid) == 1.0f)
	{
		SendWeaponAnim(ACT_VM_DRAW);
	}
	else
	{
		SendWeaponAnim(ACT_VM_DEPLOY);
	}

	if (pPlayer && m_iClip1 == 0)
	{
		DevMsg( 2, "Pose Param Returned value of %.2f\n", pPlayer->GetViewModel()->GetPoseParameter(poseparamid) );
	}
}

bool CBaseHDTFCombatWeapon::CanPrimaryAttack( )
{
	// Only the player fires this way so we can cast
	CHDTF_Player *pPlayer = ToHDTFPlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return false;

	// Clip empty? Or out of ammo on a no-clip weapon?
	if( !IsMeleeWeapon( ) &&
		( ( UsesClipsForAmmo1( ) && m_iClip1 <= 0 ) || ( !UsesClipsForAmmo1( ) && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 ) ) )
	{
		HandleFireOnEmpty( );
		return false;
	}
	else if( pPlayer->GetWaterLevel( ) == 3 && !m_bFiresUnderwater )
	{
		// This weapon doesn't fire underwater
		WeaponSound( EMPTY );
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
		return false;
	}

	if (pPlayer->IsSprinting() && !IsIronsightsEnabled() 
		&& ( GetActivity() == ACT_VM_SPRINT_IDLE || GetActivity() == ACT_VM_SPRINT_IDLE_SPECIAL ))
	{
		SendWeaponAnim(IsSpecialEnabled() ? ACT_VM_SPRINT_LEAVE_SPECIAL : ACT_VM_SPRINT_LEAVE);
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		return false;
	}

	// TODO: Fix this not resetting m_flNextPrimaryAttack when going from m_bLowered and pressing down primary attack to not m_bLowered

	//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
	//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
	//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
	//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
	//			first shot.  Right now that's too much of an architecture change -- jdw

	// If the firing button was just pressed, or the alt-fire just released, reset the firing time
	if( ( pPlayer->m_afButtonPressed & IN_ATTACK ) != 0 || ( pPlayer->m_afButtonReleased & IN_ATTACK2 ) != 0 )
		m_flNextPrimaryAttack = gpGlobals->curtime;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Primary fire button attack
//-----------------------------------------------------------------------------
void CBaseHDTFCombatWeapon::PrimaryAttack( )
{
	if( !CanPrimaryAttack( ) )
		return;

	if (m_bDelayedReload)
		return;

	// If my clip is empty (and I use clips) start reload
	if( UsesClipsForAmmo1( ) && m_iClip1 == 0 )
	{
		Reload( );
		return;
	}

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	pPlayer->DoMuzzleFlash( );

	m_fShotLast = gpGlobals->curtime;
	DoSmokeBarrel();
	SendWeaponAnim( GetPrimaryAttackActivity( ) );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	FireBulletsInfo_t info;
	info.m_vecSrc = pPlayer->Weapon_ShootPosition( );
	info.m_vecDirShooting = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	info.m_iShots = 0;

	float fireRate = GetFireRate( );

	while( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		// MUST call sound before removing a round from the clip of a CMachineGun
		WeaponSound( SINGLE, m_flNextPrimaryAttack );
		m_flNextPrimaryAttack += fireRate;
		++info.m_iShots;

		if( fireRate == 0.0f )
			break;
	}

	// Make sure we don't fire more than the amount in the clip
	if( UsesClipsForAmmo1( ) )
	{
		info.m_iShots = Min( info.m_iShots, m_iClip1.Get( ) );
		m_iClip1 -= info.m_iShots;
	}
	else
	{
		info.m_iShots = Min( info.m_iShots, pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) );
		pPlayer->RemoveAmmo( info.m_iShots, m_iPrimaryAmmoType );
	}

	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;
	info.m_vecSpread = GetBulletSpread( );
	pPlayer->FireBullets( info );

	if( m_iClip1 == 0 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );

	if (m_iClip1 == 0 && pPlayer)
	{
		pPlayer->GetViewModel()->SetPoseParameter("pose_depleted", 1.f);
	}

	//Add our view kick in
	AddViewKick( );
}

void CBaseHDTFCombatWeapon::SecondaryAttack( )
{ }

void CBaseHDTFCombatWeapon::SpecialAttack( )
{ }

bool CBaseHDTFCombatWeapon::WeaponShouldBeLowered( )
{
	// Can't be in the middle of another animation
	if( GetIdealActivity( ) != ACT_VM_IDLE_LOWERED && GetIdealActivity( ) != ACT_VM_IDLE_LOWERED_SPECIAL &&
		GetIdealActivity( ) != ACT_VM_IDLE && GetIdealActivity( ) != ACT_VM_IDLE_SPECIAL &&
		GetIdealActivity( ) != ACT_VM_IDLE_TO_LOWERED && GetIdealActivity( ) != ACT_VM_IDLE_TO_LOWERED_SPECIAL &&
		GetIdealActivity( ) != ACT_VM_LOWERED_TO_IDLE && GetIdealActivity( ) != ACT_VM_LOWERED_TO_IDLE_SPECIAL &&
		GetIdealActivity( ) != ACT_VM_SPRINT_ENTER && GetIdealActivity( ) != ACT_VM_SPRINT_ENTER_SPECIAL &&
		GetIdealActivity( ) != ACT_VM_SPRINT_IDLE && GetIdealActivity( ) != ACT_VM_SPRINT_IDLE_SPECIAL &&
		GetIdealActivity( ) != ACT_VM_SPRINT_LEAVE && GetIdealActivity( ) != ACT_VM_SPRINT_LEAVE_SPECIAL )
		return false;

	if( m_bLowered )
		return true;

#ifndef CLIENT_DLL

	if( GlobalEntity_GetState( "friendly_encounter" ) == GLOBAL_ON )
		return true;

#endif

	return false;
}

void CBaseHDTFCombatWeapon::CheckLoweredCondition()
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner == NULL)
		return;

	bool primaryButton = (pOwner->m_nButtons & IN_ATTACK) != 0,
		walkButton = (pOwner->m_nButtons & IN_WALK) != 0,
		objectInWay = ObjectInWay();

	if (!m_bLowered && (walkButton || (objectInWay && !IsIronsightsEnabled())))
	{
		m_bLowered = true;
	}
	else if (m_bLowered && !walkButton && (!objectInWay || IsIronsightsEnabled()))
	{
		if (primaryButton && m_flNextPrimaryAttack < gpGlobals->curtime)
			m_flNextPrimaryAttack = gpGlobals->curtime;

		m_bLowered = false;
	}
}

void CBaseHDTFCombatWeapon::ItemPostFrame( )
{
	CHDTF_Player *pOwner = ToHDTFPlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	bool primaryButton = (pOwner->m_nButtons & IN_ATTACK) != 0,
		secondaryButton = (pOwner->m_nButtons & IN_ATTACK2) != 0,
		specialButton = (pOwner->m_nButtons & IN_ATTACK3) != 0,
		reloadButton = (pOwner->m_nButtons & IN_RELOAD) != 0,
		isSprinting = pOwner->IsSprinting();

	// we're currently moving from IRONSIGHT to IDLE state
	// wait until we're done
	if (m_bDelayedReload)
	{
		if (m_flNextPrimaryAttack < gpGlobals->curtime)
		{
			if(Reload())
				m_bDelayedReload = false;
		}

		return;
	}

	CheckLoweredCondition();

	if (UsesClipsForAmmo1())
		CheckReload();

	if( !m_bLowered || IsIronsightsEnabled() )
	{
		UpdateAutoFire( );

		//Track the duration of the fire
		//FIXME: Check for IN_ATTACK2 and IN_ATTACK3 as well?
		//FIXME: What if we're calling ItemBusyFrame?
		m_fFireDuration = primaryButton ? m_fFireDuration + gpGlobals->frametime : 0.0f;

		// Special attack has priority
		if( specialButton && m_flNextSpecialAttack <= gpGlobals->curtime )
			SpecialAttack( );

		if( primaryButton && m_flNextPrimaryAttack <= gpGlobals->curtime )
			PrimaryAttack( );
	}

	// We should be able to reload even if weapon is lowered!!!
	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
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
		&& UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
		m_fFireDuration = 0.0f;
		m_flHudRevealTimer = 0.f;
	}

	m_bIsReloadKeyDown = reloadButton;
	if (!reloadButton)
		m_flHudRevealTimer = 0.f;

	// Reload() method also called from HandleFireOnEmpty, which is called
	// from PrimaryAttack so we should check twice
	if (m_bDelayedReload)
		return;

	if (secondaryButton && !IsIronsightsEnabled())
		EnableIronsights();
	else if (!secondaryButton && IsIronsightsEnabled())
		DisableIronsights();

	// -----------------------
	//  No buttons down, reloading or lowered weapon
	// -----------------------
	if( m_bLowered || IsIronsightsEnabled() || isSprinting ||
		( !primaryButton && !secondaryButton && !specialButton &&
		( !CanReload( ) || !reloadButton ) && !ReloadOrSwitchWeapons( ) && !m_bInReload ) )
		WeaponIdle( );

	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer && m_iClip1 == 0 && pPlayer->GetAmmoCount(pPlayer->GetActiveWeapon()->GetPrimaryAmmoType()) == 0)
	{
		pPlayer->GetViewModel()->SetPoseParameter("pose_depleted", 1.f);
	}

	// Check if we should lean from corner
#ifndef CLIENT_DLL
	ProcessCornerLean();
#endif
}

void CBaseHDTFCombatWeapon::Drop( const Vector &vecVelocity )
{
	DisableIronsights( );
	BaseClass::Drop( vecVelocity );
}

bool CBaseHDTFCombatWeapon::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if( BaseClass::Holster( pSwitchingTo ) )
	{
		DisableIronsights( );
		SetWeaponVisible( false );

		if (m_bIsCornerLeaning)
		{
#ifndef CLIENT_DLL
			CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());
			if (pPlayer)
			{
				pPlayer->UnforceButtons(IN_LEANRIGHT);
				pPlayer->UnforceButtons(IN_LEANLEFT);
			}
#endif
			
			m_bIsCornerLeaning = false;
		}

		m_flHolsterTime = gpGlobals->curtime;
		return true;
	}

	return false;
}

void CBaseHDTFCombatWeapon::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );
}

void CBaseHDTFCombatWeapon::WeaponIdle( )
{
	CHDTF_Player *pPlayer = ToHDTFPlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	// we should not idle while reloading
	if (m_bInReload)
		return;

	Activity currentAct = GetActivity( );

	//See if we should idle high or low
	if( WeaponShouldBeLowered( ) )
	{

#ifndef CLIENT_DLL

		pPlayer->Weapon_Lower( );

#endif

		// Move to lowered position if we're not there yet
		if( ( currentAct != ACT_VM_IDLE_TO_LOWERED && currentAct != ACT_VM_IDLE_LOWERED &&
			currentAct != ACT_VM_IDLE_TO_LOWERED_SPECIAL && currentAct != ACT_VM_IDLE_LOWERED_SPECIAL &&
			currentAct != ACT_VM_SPRINT_ENTER && currentAct != ACT_VM_SPRINT_ENTER_SPECIAL &&
			currentAct != ACT_TRANSITION ) ||
			HasWeaponIdleTimeElapsed( ) )
			switch( currentAct )
			{
			case ACT_VM_SPRINT_ENTER:
			case ACT_VM_SPRINT_IDLE:
				SendWeaponAnim(ACT_VM_IDLE_TO_LOWERED);
				break;

			case ACT_VM_IDLE_TO_LOWERED:
			case ACT_VM_IDLE_LOWERED:
				SendWeaponAnim( ACT_VM_IDLE_LOWERED );
				break;

			case ACT_VM_IDLE_TO_LOWERED_SPECIAL:
			case ACT_VM_SPRINT_ENTER_SPECIAL:
			case ACT_VM_IDLE_LOWERED_SPECIAL:
			case ACT_VM_SPRINT_IDLE_SPECIAL:
				SendWeaponAnim( ACT_VM_IDLE_LOWERED_SPECIAL );
				break;

			default:
				if( IsSpecialEnabled( ) )
					SendWeaponAnim( ACT_VM_IDLE_TO_LOWERED_SPECIAL );
				else
					SendWeaponAnim( ACT_VM_IDLE_TO_LOWERED );

				break;
			}
	}
	else
	{
		// ironsights has the priority over sprinting
		if (pPlayer->IsSprinting() && !IsIronsightsEnabled() 
			&& m_flNextPrimaryAttack < gpGlobals->curtime
			&& m_flNextSecondaryAttack < gpGlobals->curtime
			&& m_flNextSpecialAttack < gpGlobals->curtime
			&& (pPlayer->m_nButtons & IN_ATTACK) == 0
			&& (pPlayer->m_nButtons & IN_ATTACK2) == 0
			&& (pPlayer->m_nButtons & IN_ATTACK3) == 0
			&& pPlayer->GetMoveType() == MOVETYPE_WALK
			&& pPlayer->GetGroundEntity() != NULL)
		{
			if (IsSpecialEnabled())
			{
				if (currentAct != ACT_VM_SPRINT_IDLE_SPECIAL
					&& currentAct != ACT_VM_SPRINT_ENTER_SPECIAL)
					SendWeaponAnim(ACT_VM_SPRINT_ENTER_SPECIAL);
				else if (HasWeaponIdleTimeElapsed())
					SendWeaponAnim(ACT_VM_SPRINT_IDLE_SPECIAL);
			}
			else
			{
				if (currentAct != ACT_VM_SPRINT_IDLE
					&& currentAct != ACT_VM_SPRINT_ENTER)
					SendWeaponAnim(ACT_VM_SPRINT_ENTER);
				else if (HasWeaponIdleTimeElapsed())
					SendWeaponAnim(ACT_VM_SPRINT_IDLE);
			}

			return;
		}

		// See if we need to raise immediately
		if( ( currentAct == ACT_VM_IDLE_LOWERED ||
				currentAct == ACT_VM_IDLE_LOWERED_SPECIAL ||
				currentAct == ACT_VM_SPRINT_IDLE ||
				currentAct == ACT_VM_SPRINT_IDLE_SPECIAL ) ||
			HasWeaponIdleTimeElapsed( ) )
			switch( currentAct )
			{
			case ACT_VM_IDLE_LOWERED:
				SendWeaponAnim( ACT_VM_LOWERED_TO_IDLE );
				break;

			case ACT_VM_IDLE_TO_LOWERED:
				SendWeaponAnim(ACT_VM_LOWERED_TO_IDLE);
				break;

			case ACT_VM_LOWERED_TO_IDLE:
				SendWeaponAnim( ACT_VM_IDLE );
				break;

			case ACT_VM_IDLE_LOWERED_SPECIAL:
				SendWeaponAnim( ACT_VM_LOWERED_TO_IDLE_SPECIAL );
				break;

			case ACT_VM_LOWERED_TO_IDLE_SPECIAL:
				SendWeaponAnim( ACT_VM_IDLE_SPECIAL );
				break;

			case ACT_VM_SPRINT_ENTER:
			case ACT_VM_SPRINT_IDLE:
				SendWeaponAnim( ACT_VM_SPRINT_LEAVE );
				break;

			case ACT_VM_SPRINT_LEAVE:
				SendWeaponAnim( ACT_VM_IDLE );
				break;

			case ACT_VM_SPRINT_ENTER_SPECIAL:
			case ACT_VM_SPRINT_IDLE_SPECIAL:
				SendWeaponAnim( ACT_VM_SPRINT_LEAVE_SPECIAL );
				break;

			case ACT_VM_SPRINT_LEAVE_SPECIAL:
				SendWeaponAnim( ACT_VM_IDLE_SPECIAL );
				break;

			default:
				if( IsSpecialEnabled( ) )
					SendWeaponAnim( IsIronsightsEnabled( ) ? ACT_VM_IDLE_IRONSIGHTS_SPECIAL : ACT_VM_IDLE_SPECIAL );
				else
					SendWeaponAnim( IsIronsightsEnabled( ) ? ACT_VM_IDLE_IRONSIGHTS : ACT_VM_IDLE );

				break;
			}
	}
}

#ifdef CLIENT_DLL

#define	RPG_GUIDE_ATTACHMENT 2
#define	RPG_GUIDE_ATTACHMENT_3RD 4

#define	LASER_BEAM_LENGTH 128

RenderGroup_t CBaseHDTFCombatWeapon::GetRenderGroup( )
{
	return RENDER_GROUP_TWOPASS;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the attachment point on either the world or viewmodel
//			This should really be worked into the CBaseCombatWeapon class!
//-----------------------------------------------------------------------------
void CBaseHDTFCombatWeapon::GetWeaponAttachment( int attachmentId, Vector &outVector, Vector *dir /*= NULL*/ )
{
	QAngle angles;

	if( ShouldDrawUsingViewModel( ) )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
		if( pOwner != NULL )
		{
			pOwner->GetViewModel( )->GetAttachment( attachmentId, outVector, angles );
			::FormatViewModelAttachment( outVector, true );
		}
	}
	else
	{
		// We offset the IDs to make them correct for our world model
		BaseClass::GetAttachment( attachmentId, outVector, angles );
	}

	// Supply the direction, if requested
	if( dir != NULL )
		AngleVectors( angles, dir, NULL, NULL );
}

#endif

inline Vector BodyDirection2D( CBaseCombatCharacter *pOwner )
{
	QAngle angles = pOwner->GetAbsAngles( );

	Vector vBodyDir;
	AngleVectors( angles, &vBodyDir );

	vBodyDir.z = 0;
	vBodyDir.AsVector2D( ).NormalizeInPlace( );

	return vBodyDir;
}

bool CBaseHDTFCombatWeapon::ObjectInWay( ) const
{
	CBaseCombatCharacter *pOwner = GetOwner( );
	if( pOwner == NULL )
		return false;

	Vector vecSrc = pOwner->Weapon_ShootPosition( );
	Vector vecAiming = BodyDirection2D( pOwner );
	Vector vecEnd = vecSrc + vecAiming * 32;

	trace_t tr;
	UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );

	// Don't block on a living creature
	return tr.fraction < 1.0f && ToBaseCombatCharacter( tr.m_pEnt ) == NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon can be deselected in the inventory
//-----------------------------------------------------------------------------
bool CBaseHDTFCombatWeapon::CanBeDeselected() const
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon can be selected via the weapon selection
//-----------------------------------------------------------------------------
bool CBaseHDTFCombatWeapon::CanBeSelected( )
{
	return VisibleInWeaponSelection( );
}

//-----------------------------------------------------------------------------
// Purpose: Return true if this weapon should be seen, and hence be selectable, in the weapon selection
//-----------------------------------------------------------------------------
bool CBaseHDTFCombatWeapon::VisibleInWeaponSelection( )
{
	return m_bVisibleInWeaponSelection;
}

//-----------------------------------------------------------------------------
// Purpose: Sets if this weapon should be seen, and hence be selectable, in the weapon selection
//-----------------------------------------------------------------------------
void CBaseHDTFCombatWeapon::SetVisibleInWeaponSelection( bool visible )
{
	m_bVisibleInWeaponSelection = visible;
}

bool CBaseHDTFCombatWeapon::HasIronsights( ) const
{
	return true; // default yes; override and return false for weapons with no ironsights (like weapon_crowbar)
}

bool CBaseHDTFCombatWeapon::IsIronsightsEnabled( ) const
{
	return m_bIsIronsighted;
}

void CBaseHDTFCombatWeapon::ToggleIronsights( )
{

#if defined( CLIENT_DLL ) && !defined( HDTF_SINGLEPLAYER )

	if( !prediction->IsFirstTimePredicted( ) )
		return;

#endif

	if( IsIronsightsEnabled( ) )
		DisableIronsights( );
	else
		EnableIronsights( );

	m_flNextPrimaryAttack = m_flNextSpecialAttack = m_flNextSecondaryAttack = gpGlobals->curtime + IRONSIGHT_TIME;
}

void CBaseHDTFCombatWeapon::EnableIronsights( )
{
	if( !HasIronsights( ) || IsIronsightsEnabled( ) || m_bInReload )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner != NULL )
	{
		m_bIsIronsighted = true;
		pOwner->EmitSound( "Player.IronsightsIn" );
		pOwner->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;
		SendWeaponAnim( IsSpecialEnabled( ) ? ACT_VM_IDLE_TO_IRONSIGHTS_SPECIAL : ACT_VM_IDLE_TO_IRONSIGHTS );
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack = m_flNextPrimaryAttack;
		m_flNextSpecialAttack = m_flNextPrimaryAttack;
	}
}

void CBaseHDTFCombatWeapon::DisableIronsights( bool ignoreAnimation )
{
	if( !HasIronsights( ) || !IsIronsightsEnabled( ) )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner != NULL )
	{
		pOwner->SetFOV(this, 0, 0.35f);
		m_bIsIronsighted = false;
		pOwner->EmitSound( "Player.IronsightsOut" );
		pOwner->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;

		// Because of the way reloading works we don't want this animation to interrupt reloading one
		if (!ignoreAnimation)
		{
			SendWeaponAnim(IsSpecialEnabled() ? ACT_VM_IRONSIGHTS_TO_IDLE_SPECIAL : ACT_VM_IRONSIGHTS_TO_IDLE);
			m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
			m_flNextSecondaryAttack = m_flNextPrimaryAttack;
			m_flNextSpecialAttack = m_flNextPrimaryAttack;
		}
	}
}

void CBaseHDTFCombatWeapon::SetSpecialEnabled( bool enabled )
{
	m_bIsSpecialEnabled = enabled;
}

bool CBaseHDTFCombatWeapon::IsSpecialEnabled( ) const
{
	return m_bIsSpecialEnabled;
}

// TODO: Should special reload the secondary clip?
bool CBaseHDTFCombatWeapon::Reload( )
{
	if (IsIronsightsEnabled() && CanReload())
	{
		DisableIronsights();
		m_bDelayedReload = true;

		CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

		if (m_iClip1 == 0 && pPlayer)
		{
			pPlayer->GetViewModel()->SetPoseParameter("pose_depleted", 1.f);
		}

		return false;
	}

	bool isSpecialEnabled = IsSpecialEnabled( );
	return DefaultReload( isSpecialEnabled ? Clip1( ) : GetMaxClip1( ),
		isSpecialEnabled ? GetMaxClip2( ) : Clip2( ),
		isSpecialEnabled ? ACT_VM_RELOAD_SPECIAL : ( Clip1( ) == 0 ? ACT_VM_RELOAD_EMPTY : ACT_VM_RELOAD ) );
}

bool CBaseHDTFCombatWeapon::DefaultReload( int iClipSize1, int iClipSize2, int iActivity )
{
	if( BaseClass::DefaultReload( iClipSize1, iClipSize2, iActivity ) )
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

		if (IsIronsightsEnabled())
		{
			SendWeaponAnim(IsSpecialEnabled() ? ACT_VM_IRONSIGHTS_TO_IDLE_SPECIAL : ACT_VM_IRONSIGHTS_TO_IDLE);
			m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration() + 0.15f;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack;

			if (pPlayer)
				pPlayer->SetNextAttack(m_flNextPrimaryAttack);

			if (pPlayer && m_iClip1 != 0)
			{
				pPlayer->GetViewModel()->SetPoseParameter("pose_depleted", 0.0f);
			}
			
			return true;
		}

		if (pPlayer)
		{
			pPlayer->GetViewModel()->SetPoseParameter("pose_depleted", 0.0f);
		}

		return true;
	}

	return false;
}

bool CBaseHDTFCombatWeapon::CanReload()
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
		return false;

	const bool isSpecialEnabled = IsSpecialEnabled();
	const int iClipSize1 = isSpecialEnabled ? Clip1() : GetMaxClip1();
	const int iClipSize2 = isSpecialEnabled ? GetMaxClip2() : Clip2();

	// If I don't have any spare ammo, I can't reload
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	bool bReload = false;

	// If you don't have clips, then don't try to reload them.
	if (UsesClipsForAmmo1())
	{
		// need to reload primary clip?
		int primary = MIN(iClipSize1 - m_iClip1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));
		if (primary != 0)
		{
			bReload = true;
		}
	}

	if (UsesClipsForAmmo2())
	{
		// need to reload secondary clip?
		int secondary = MIN(iClipSize2 - m_iClip2, pOwner->GetAmmoCount(m_iSecondaryAmmoType));
		if (secondary != 0)
		{
			bReload = true;
		}
	}

	return bReload;
}

void CBaseHDTFCombatWeapon::ItemBusyFrame()
{
	if (m_bInReload && IsIronsightsEnabled() && m_flNextPrimaryAttack - 0.25f <= gpGlobals->curtime)
	{
		m_bIsIronsighted = false;
		SendWeaponAnim(IsSpecialEnabled() ? ACT_VM_RELOAD_SPECIAL : 
			(Clip1() == 0 ? ACT_VM_RELOAD_EMPTY : ACT_VM_RELOAD));

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

		if (pPlayer)
		{
			if (m_iClip1 != 0 && !m_bStage_BoltRelease && !m_bStage_Empty_magin)
			{
				pPlayer->GetViewModel()->SetPoseParameter("pose_depleted", 0.0f);
			}
			pPlayer->SetNextAttack(m_flNextPrimaryAttack);
		}
	}

#ifndef CLIENT_DLL
	if(BusyFrameForcesUnLean())
		ForceUnLean();
#endif

	BaseClass::ItemBusyFrame();
}

#ifndef CLIENT_DLL
void CBaseHDTFCombatWeapon::ForceUnLean()
{
	if (!IsLeaning())
		return;

	m_bIsCornerLeaning = false;

	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	pPlayer->UnforceButtons(IN_LEANRIGHT);
	pPlayer->UnforceButtons(IN_LEANLEFT);
}

void CBaseHDTFCombatWeapon::ProcessCornerLean()
{
	if (!ShouldAutoLean() || !hdtf_autolean.GetBool())
		return;

	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	if (IsIronsightsEnabled())
	{
		Vector origin, forward, right;
		origin = pPlayer->GetAbsOrigin();
		origin += pPlayer->GetViewOffset();
		forward = pPlayer->BodyDirection2D();
		VectorRotate(forward, QAngle(0, -90, 0), right);

		trace_t tr;
		UTIL_TraceLine(origin, forward * 20.f, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

		// check if something blocks our way but ignore NPCs
		if (tr.DidHit() && ToBaseCombatCharacter(tr.m_pEnt) == NULL)
		{
			trace_t tr_l, tr_r;
			Vector origin_r = origin + right * 11.f;
			Vector origin_l = origin - right * 11.f;

			UTIL_TraceLine(origin_r, origin_r + forward * 36.f, MASK_SOLID_BRUSHONLY, pPlayer, COLLISION_GROUP_NONE, &tr_r);
			UTIL_TraceLine(origin_l, origin_l + forward * 36.f, MASK_SOLID_BRUSHONLY, pPlayer, COLLISION_GROUP_NONE, &tr_l);

			bool rState = tr_r.DidHit(), lState = tr_l.DidHit();

			// if both traces pass this means that our wall is too thin
			// and we're uncertain on what direction we should lean
			// that's why we ignore that condition -Wheatley

			if (rState || lState)
			{
				if (!rState)
				{
					// simulate right lean button
					pPlayer->ForceButtons(IN_LEANRIGHT);
					m_bIsCornerLeaning = true;
					return;
				}

				if (!lState)
				{
					// simulate left lean button
					pPlayer->ForceButtons(IN_LEANLEFT);
					m_bIsCornerLeaning = true;
					return;
				}
			}
		}
	}
	
	if (m_bIsCornerLeaning)
	{
		pPlayer->UnforceButtons(IN_LEANRIGHT);
		pPlayer->UnforceButtons(IN_LEANLEFT);

		m_bIsCornerLeaning = false;
	}
}
#endif

#if defined( CLIENT_DLL ) && ( !defined( HL2MP ) && !defined( PORTAL ) && defined( HDTF ) )

float	g_lateralBob;
float	g_verticalBob;

#define	HL2_BOB_CYCLE_MIN	1.0f
#define	HL2_BOB_CYCLE_MAX	0.45f
#define	HL2_BOB			0.002f
#define	HL2_BOB_UP		0.5f

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CBaseHDTFCombatWeapon::CalcViewmodelBob(void)
{
	// RAY: Moved to BaseViewModel
	return 0.0f;
}

void CBaseHDTFCombatWeapon::AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles)
{
	// RAY: Moved to BaseViewModel
}

void CBaseHDTFCombatWeapon::OnRestore()
{
	BaseClass::OnRestore();
}

#else

// Server stubs
float CBaseHDTFCombatWeapon::CalcViewmodelBob(void)
{
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			viewmodelindex - 
//-----------------------------------------------------------------------------
void CBaseHDTFCombatWeapon::AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles)
{
}

#endif

#ifdef CLIENT_DLL
int CBaseHDTFCombatWeapon::GetScopeReticleTextureId()
{
	if (m_iScopeReticleTextureId == -1 && GetScopeReticleTextureName() != NULL)
	{
		m_iScopeReticleTextureId = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_iScopeReticleTextureId, GetScopeReticleTextureName(), true, true);
	}

	return m_iScopeReticleTextureId;
}
#endif

int CBaseHDTFCombatWeapon::CalcViewCorrectedFov(const int desired) const
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	
	if (!pPlayer)
	{
		return desired;
	}

	return desired + (pPlayer->GetDefaultFOV() - 90);
}
