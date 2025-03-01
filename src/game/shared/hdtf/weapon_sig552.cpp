#include "cbase.h"
#include "weapon_basemachinegun.h"
#include "npcevent.h"

#ifdef CLIENT_DLL

#include "c_te_effect_dispatch.h"

#else

#include "te_effect_dispatch.h"
#include "ai_basenpc.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL

#define CWeaponSig552 C_WeaponSig552

#endif

#define	EASY_DAMPEN 0.5f
#define	MAX_VERTICAL_KICK 8.0f //Degrees
#define	SLIDE_LIMIT 5.0f //Seconds

class CWeaponSig552 : public CMachineGun
{
public:
	DECLARE_CLASS( CWeaponSig552, CMachineGun );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	CWeaponSig552( );

	int GetMinBurst( )
	{
		return 2;
	}

	int GetMaxBurst( )
	{
		return 5;
	}

	float GetFireRate( )
	{
		return 0.1f;
	}

#ifndef CLIENT_DLL

	int CapabilitiesGet()
	{
		return bits_CAP_WEAPON_RANGE_ATTACK1;
	}

	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

#endif

	void AddViewKick( );

	const WeaponProficiencyInfo_t *GetProficiencyValues( );

private:
	CWeaponSig552( const CWeaponSig552 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSig552, DT_WeaponSig552 )

BEGIN_NETWORK_TABLE( CWeaponSig552, DT_WeaponSig552 )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CWeaponSig552 )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_sig552, CWeaponSig552 );
PRECACHE_WEAPON_REGISTER( weapon_sig552 );


#ifndef CLIENT_DLL

acttable_t CWeaponSig552::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_AR2, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_AR2, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_AR2, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_AR2, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_AR2, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_AR2, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_AR2, false },
};

IMPLEMENT_ACTTABLE( CWeaponSig552 );

#endif

CWeaponSig552::CWeaponSig552( )
{
	m_fMinRange1 = 65.0f;
	m_fMaxRange1 = 2048.0f;

	m_fMinRange2 = 256.0f;
	m_fMaxRange2 = 1024.0f;

	m_nShotsFired = 0;
}

void CWeaponSig552::AddViewKick( )
{
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}

const WeaponProficiencyInfo_t *CWeaponSig552::GetProficiencyValues( )
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0, 0.75 },
		{ 5.00, 0.75 },
		{ 3.0, 0.85 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE( proficiencyTable ) == WEAPON_PROFICIENCY_PERFECT + 1 );

	return proficiencyTable;
}

#ifndef CLIENT_DLL

void CWeaponSig552::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir)
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

void CWeaponSig552::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	++m_iClip1;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
	AngleVectors(angShootDir, &vecShootDir);
	FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
}

void CWeaponSig552::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_AR2:
	case EVENT_WEAPON_SMG1:
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

#endif