#include "cbase.h"
#include "weapon_basemachinegun.h"
#include "hdtf_player_shared.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "in_buttons.h"
#include "npcevent.h"

#ifdef CLIENT_DLL

#include "c_te_effect_dispatch.h"

#else

#include "te_effect_dispatch.h"
#include "grenade_ar2.h"
#include "soundent.h"
#include "ai_basenpc.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL

#define CWeaponM16_Carbine C_WeaponM16_Carbine

#endif

//#define	EASY_DAMPEN 0.1f
//#define	MAX_VERTICAL_KICK 0.1f //Degrees
//#define	SLIDE_LIMIT 0.1f //Seconds

#define	EASY_DAMPEN 0.5f
#define	MAX_VERTICAL_KICK 1.0f //Degrees
#define	SLIDE_LIMIT 2.0f //Seconds

extern ConVar sk_plr_dmg_smg1_grenade;

class CWeaponM16_Carbine : public CMachineGun
{
public:
	DECLARE_CLASS( CWeaponM16_Carbine, CMachineGun );
	DECLARE_NETWORKCLASS( );

#ifdef CLIENT_DLL

	DECLARE_PREDICTABLE( );

#else

	DECLARE_DATADESC( );
	DECLARE_ACTTABLE( );

#endif

	CWeaponM16_Carbine( );

	int GetMinBurst()
	{
		return 2;
	}

	int GetMaxBurst()
	{
		return 5;
	}

	float GetFireRate( )
	{
	//	return 0.1f;
		return 0.0705f;
	}

	bool HasIronsights() const
	{
		return true;
	}

	void AddViewKick( );

#ifndef CLIENT_DLL

	int CapabilitiesGet()
	{
		return bits_CAP_WEAPON_RANGE_ATTACK1;
	}

	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	const WeaponProficiencyInfo_t *GetProficiencyValues( );

#endif

	virtual float GetIronSpeed() { return 3.5f; }

#ifdef CLIENT_DLL
	virtual EScopeType GetScopeType() { return EScopeType::SCOPE_SNIPER; }
	virtual char *GetScopeReticleTextureName() { return "reticle/M16_Carbine"; }
#endif

private:
	CWeaponM16_Carbine( const CWeaponM16_Carbine & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponM16_Carbine, DT_WeaponM16_Carbine )

BEGIN_NETWORK_TABLE( CWeaponM16_Carbine, DT_WeaponM16_Carbine )
END_NETWORK_TABLE( )

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponM16_Carbine )
END_PREDICTION_DATA( )

#else

BEGIN_DATADESC( CWeaponM16_Carbine )
END_DATADESC( )

#endif

LINK_ENTITY_TO_CLASS( weapon_m16_carbine, CWeaponM16_Carbine );
PRECACHE_WEAPON_REGISTER( weapon_m16_carbine );

#ifndef CLIENT_DLL

acttable_t CWeaponM16_Carbine::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY,					true },		// FIXME: hook to AR2 unique

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

IMPLEMENT_ACTTABLE( CWeaponM16_Carbine );

#endif

CWeaponM16_Carbine::CWeaponM16_Carbine( )
{
	m_fMinRange1 = 65.0f;
	m_fMaxRange1 = 2048.0f;

	m_fMinRange2 = 256.0f;
	m_fMaxRange2 = 1024.0f;

	m_nShotsFired = 0;
}

void CWeaponM16_Carbine::AddViewKick( )
{
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}

#ifndef CLIENT_DLL

void CWeaponM16_Carbine::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir)
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);

	pOperator->DoMuzzleFlash();
	m_iClip1--;
}

void CWeaponM16_Carbine::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	++m_iClip1;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
	AngleVectors(angShootDir, &vecShootDir);
	FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
}

void CWeaponM16_Carbine::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_AR2:
	{
		Vector vecShootOrigin, vecShootDir;
		QAngle angDiscard;

		// Support old style attachment point firing
		if (pEvent->options == NULL || pEvent->options[0] == '\0' || !pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard))
			vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);

		break;
	}

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

const WeaponProficiencyInfo_t *CWeaponM16_Carbine::GetProficiencyValues( )
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0/3.0, 0.75	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE( proficiencyTable ) == WEAPON_PROFICIENCY_PERFECT + 1 );

	return proficiencyTable;
}

#endif
