#include "cbase.h"
#include "hdtf_player_shared.h"
#include "weapon_basehdtfcombat.h"
#include "in_buttons.h"
#include "npcevent.h"

#ifndef CLIENT_DLL

#include "ai_basenpc.h"
#include "soundent.h"

#endif

#ifdef CLIENT_DLL

#define CWeaponAWP C_WeaponAWP

#endif

ConVar sk_plr_dmg_awp("sk_plr_dmg_awp", "100", FCVAR_REPLICATED);
ConVar sk_npc_dmg_awp("sk_npc_dmg_awp", "35", FCVAR_REPLICATED);

class CWeaponAWP : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponAWP, CBaseHDTFCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE();

#endif

	CWeaponAWP() 
	{
		m_fMinRange1 = 65.f;
		m_fMaxRange1 = 4096.f;
	};

	int GetMinBurst()
	{
		return 1;
	}

	int GetMaxBurst()
	{
		return 1;
	}

	float GetMinRestTime()
	{
		// slightly longer than fire rate to simulate
		// reloading and aiming delay before shot
		return 1.25f;
	}

	float GetMaxRestTime()
	{
		// takes some time to aim...
		return 2.00f;
	}

	float GetFireRate()
	{
		return 1.15f;
	}

	float GetIronSensitivity()
	{
		return 0.30f;
	}

	void PrimaryAttack();
	void ItemPostFrame();

	void EnableIronsights();
	void DisableIronsights(bool ignoreAnimation = false);

	virtual float GetIronSpeed() { return 1.5f; }

#ifdef CLIENT_DLL
	virtual EScopeType GetScopeType() { return EScopeType::SCOPE_SNIPER; }
	virtual char *GetScopeReticleTextureName() { return "reticle/awp"; }
#endif

#ifndef CLIENT_DLL
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	void	FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void	Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	virtual void ModifyDamageViewPunch(float& flOutPunch)
	{
		if (IsIronsightsEnabled())
		{
			flOutPunch = -0.3f;
		}
	}
#endif

private:
	CWeaponAWP(const CWeaponAWP &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponAWP, DT_WeaponAWP)

BEGIN_NETWORK_TABLE(CWeaponAWP, DT_WeaponAWP)

#ifdef CLIENT_DLL

#else

#endif

END_NETWORK_TABLE()

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA(CWeaponAWP)
END_PREDICTION_DATA()

#endif

LINK_ENTITY_TO_CLASS(weapon_awp, CWeaponAWP);
PRECACHE_WEAPON_REGISTER(weapon_awp);

#ifndef CLIENT_DLL

acttable_t	CWeaponAWP::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },		// FIXME: hook to AR2 unique

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },

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
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	false },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },		// FIXME: hook to AR2 unique
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_AR2_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },		// FIXME: hook to AR2 unique
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
	//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },
};

IMPLEMENT_ACTTABLE(CWeaponAWP);

#endif

void CWeaponAWP::PrimaryAttack()
{
	if (!CanPrimaryAttack())
		return;

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);

	pPlayer->DoMuzzleFlash();

	m_fShotLast = gpGlobals->curtime;
	DoSmokeBarrel();

	SendWeaponAnim(GetPrimaryAttackActivity());

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_iClip1 -= 1;

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	FireBulletsInfo_t info(1, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;
	info.m_flDamage = sk_plr_dmg_awp.GetFloat(); // override damage

	pPlayer->FireBullets(info);

	QAngle punch;
	punch.Init(SharedRandomFloat("doublebarrelpax", -2, -1), SharedRandomFloat("doublebarrelpay", -2, 2), 0);
	pPlayer->ViewPunch(punch);

	if (m_iClip1 == 0 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
}

void CWeaponAWP::ItemPostFrame()
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner == NULL)
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
			if (Reload())
				m_bDelayedReload = false;
		}

		return;
	}

	CheckLoweredCondition();

	if (!m_bLowered || IsIronsightsEnabled())
	{
		UpdateAutoFire();

		//Track the duration of the fire
		//FIXME: Check for IN_ATTACK2 and IN_ATTACK3 as well?
		//FIXME: What if we're calling ItemBusyFrame?
		m_fFireDuration = primaryButton ? m_fFireDuration + gpGlobals->frametime : 0.0f;

		if (UsesClipsForAmmo1())
			CheckReload();

		// Special attack has priority
		if (specialButton && m_flNextSpecialAttack <= gpGlobals->curtime)
			SpecialAttack();

		if (primaryButton && m_flNextPrimaryAttack <= gpGlobals->curtime)
			PrimaryAttack();

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
	}

	if (!m_bInReload && m_flNextPrimaryAttack < gpGlobals->curtime)
	{
		if (secondaryButton && !IsIronsightsEnabled())
			EnableIronsights();
		else if (!secondaryButton && IsIronsightsEnabled())
			DisableIronsights();
	}

	// -----------------------
	//  No buttons down, reloading or lowered weapon
	// -----------------------
	if (m_bLowered || isSprinting ||
		(!primaryButton && !secondaryButton && !specialButton &&
		(!CanReload() || !reloadButton) && !ReloadOrSwitchWeapons() && !m_bInReload))
		WeaponIdle();

	// Check if we should lean from corner
#ifndef CLIENT_DLL
	ProcessCornerLean();
#endif
}

#ifndef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAWP::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;

	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);

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

	WeaponSound(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

	FireBulletsInfo_t info(1, vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_iTracerFreq = 2;
	info.m_pAttacker = pOperator;

	// if not used against the player - drastically increase the damage
	if(npc->GetEnemy() && !npc->GetEnemy()->IsPlayer())
		info.m_flDamage = sk_plr_dmg_awp.GetFloat();
	else
		info.m_flDamage = sk_npc_dmg_awp.GetFloat();

	pOperator->FireBullets(info);

	// NOTENOTE: This is overriden on the client-side
	// pOperator->DoMuzzleFlash();

	//m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAWP::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	FireNPCPrimaryAttack(pOperator, true);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAWP::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_AR2:
	{
		FireNPCPrimaryAttack(pOperator, false);
	}
	break;

	default:
		CBaseCombatWeapon::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

#endif

void CWeaponAWP::EnableIronsights()
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

void CWeaponAWP::DisableIronsights(bool ignoreAnimation)
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
