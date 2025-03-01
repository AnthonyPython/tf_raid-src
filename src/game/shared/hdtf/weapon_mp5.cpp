#include "cbase.h"
#include "weapon_basemachinegun.h"
#include "npcevent.h"
#include "gamestats.h"
#include "hdtf_player_shared.h"

#ifdef CLIENT_DLL

#define CWeaponMP5 C_WeaponMP5
#define CWeaponMP5_alt C_WeaponMP5_alt

#else

#include "grenade_frag.h"
#include "ai_basenpc.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define GRENADE_TIMER 1.5f //Seconds
#define GRENADE_RADIUS 4.0f // inches

#define	COMBINE_MIN_GRENADE_CLEAR_DIST 256

#define	EASY_DAMPEN 0.5f
#define	MAX_VERTICAL_KICK 1.0f //Degrees
#define	SLIDE_LIMIT 2.0f //Seconds

class CWeaponMP5 : public CMachineGun
{
public:
	DECLARE_CLASS( CWeaponMP5, CMachineGun );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );
	DECLARE_DATADESC( );

#endif

	CWeaponMP5( );

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
		return 0.075f; // 13.3hz
	}

	Activity GetPrimaryAttackActivity( )
	{
		if (IsIronsightsEnabled())
			return ACT_VM_PRIMARYATTACK_IRONSIGHTS;

		return ACT_VM_PRIMARYATTACK;
	}

	void AddViewKick( );
	void SecondaryAttack( );

	void Equip( CBaseCombatCharacter *pOwner );
	bool Reload( );

	void EnableIronsights();
	void DisableIronsights(bool ignoreAnimation = false);

#ifndef CLIENT_DLL

	int CapabilitiesGet( )
	{
		return bits_CAP_WEAPON_RANGE_ATTACK1;
	}

	int WeaponRangeAttack2Condition( float flDot, float flDist );

	const WeaponProficiencyInfo_t *GetProficiencyValues( );

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

#endif

	void CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );

	void TickThink( );

	virtual float GetIronSpeed() { return 3.5f; }

#ifdef CLIENT_DLL
	virtual EScopeType GetScopeType() { return EScopeType::SCOPE_COLLIMATOR; }
	virtual char *GetScopeReticleTextureName() { return "reticle/mp5"; }
#endif

protected:
	Vector m_vecTossVelocity;
	float m_flNextGrenadeCheck;

private:
	CWeaponMP5( const CWeaponMP5 & );
};

class CWeaponMP5_alt : public CMachineGun
{
public:
	DECLARE_CLASS(CWeaponMP5_alt, CMachineGun);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();

#endif

	CWeaponMP5_alt();

	int GetMinBurst()
	{
		return 2;
	}

	int GetMaxBurst()
	{
		return 5;
	}

	float GetFireRate()
	{
		return 0.075f; // 13.3hz
	}

	Activity GetPrimaryAttackActivity()
	{
		if (IsIronsightsEnabled())
			return ACT_VM_PRIMARYATTACK_IRONSIGHTS;

		return ACT_VM_PRIMARYATTACK;
	}

	void AddViewKick();
	void SecondaryAttack();

	void Equip(CBaseCombatCharacter* pOwner);
	bool Reload();

	void EnableIronsights();
	void DisableIronsights(bool ignoreAnimation = false);

#ifndef CLIENT_DLL

	int CapabilitiesGet()
	{
		return bits_CAP_WEAPON_RANGE_ATTACK1;
	}

	int WeaponRangeAttack2Condition(float flDot, float flDist);

	const WeaponProficiencyInfo_t* GetProficiencyValues();

	void FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir);
	void Operator_ForceNPCFire(CBaseCombatCharacter* pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);

#endif

	void CheckThrowPosition(CBasePlayer* pPlayer, const Vector& vecEye, Vector& vecSrc);

	void TickThink();

	virtual float GetIronSpeed() { return 3.5f; }

protected:
	Vector m_vecTossVelocity;
	float m_flNextGrenadeCheck;

private:
	CWeaponMP5_alt(const CWeaponMP5_alt&);
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP5, DT_WeaponMP5 )
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP5_alt, DT_WeaponMP5_alt)

BEGIN_NETWORK_TABLE( CWeaponMP5, DT_WeaponMP5 )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CWeaponMP5 )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_mp5, CWeaponMP5 );
PRECACHE_WEAPON_REGISTER( weapon_mp5 );


// ALt mp5

BEGIN_NETWORK_TABLE(CWeaponMP5_alt, DT_WeaponMP5_alt)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponMP5_alt)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_mp5_noscope, CWeaponMP5_alt);
PRECACHE_WEAPON_REGISTER(weapon_mp5_noscope);


#ifndef CLIENT_DLL

BEGIN_DATADESC( CWeaponMP5 )
DEFINE_FIELD( m_vecTossVelocity, FIELD_VECTOR ),
DEFINE_FIELD( m_flNextGrenadeCheck, FIELD_TIME ),
DEFINE_THINKFUNC( TickThink ),
END_DATADESC( )

acttable_t CWeaponMP5::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, true },
	{ ACT_RELOAD, ACT_RELOAD_AR2, true },
	{ ACT_IDLE, ACT_IDLE_AR2, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_AR2, true },

	{ ACT_WALK, ACT_WALK_AR2, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_AR2, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_AR2_RELAXED, false }, //never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_AR2_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_AR2, false }, //always aims

	{ ACT_WALK_RELAXED, ACT_WALK_AR2_RELAXED, false }, //never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_AR2_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_AR2, false }, //always aims

	{ ACT_RUN_RELAXED, ACT_RUN_AR2_RELAXED, false }, //never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_AR2_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false }, //always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_AR2_RELAXED, false }, //never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_AR2, false }, //always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_AR2_RELAXED, false }, //never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_AR2_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_AR2, false }, //always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_AR2_RELAXED, false }, //never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false }, //always aims
//End readiness activities

	{ ACT_WALK_AIM, ACT_WALK_AIM_AR2, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_RIFLE, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SMG1, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SMG1_LOW, true },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_SMG1_LOW, false },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SMG1, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SMG1, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SMG1, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SMG1, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SMG1, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, false },
};

IMPLEMENT_ACTTABLE( CWeaponMP5 );

#endif

CWeaponMP5::CWeaponMP5( )
{
	m_fMinRange1 = 0; // No minimum range. 
	m_fMaxRange1 = 1400;
	m_iClip2 = 5;

	m_bAltFiresUnderwater = false;
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponMP5::Equip( CBaseCombatCharacter *pOwner )
{

#ifndef CLIENT_DLL

	m_fMaxRange1 = pOwner->Classify( ) == CLASS_PLAYER_ALLY ? 3000 : 1400;

#endif

	BaseClass::Equip( pOwner );
}

#ifndef CLIENT_DLL

void CWeaponMP5::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin( ), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy( ) );
	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex( ), 0 );

	pOperator->DoMuzzleFlash( );
	m_iClip1 = m_iClip1 - 1;
}

void CWeaponMP5::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	++m_iClip1;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
	AngleVectors( angShootDir, &vecShootDir );
	FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
}

void CWeaponMP5::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_SMG1:
	{
		Vector vecShootOrigin, vecShootDir;
		QAngle angDiscard;

		// Support old style attachment point firing
		if( pEvent->options == NULL || pEvent->options[0] == '\0' || !pOperator->GetAttachment( pEvent->options, vecShootOrigin, angDiscard ) )
			vecShootOrigin = pOperator->Weapon_ShootPosition( );

		CAI_BaseNPC *npc = pOperator->MyNPCPointer( );
		ASSERT( npc != NULL );
		vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

		FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );

		break;
	}

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

int CWeaponMP5::WeaponRangeAttack2Condition( float flDot, float flDist )
{
	CAI_BaseNPC *npcOwner = GetOwner( )->MyNPCPointer( );

	return COND_NONE;

	// --------------------------------------------------------
	// Assume things haven't changed too much since last time
	// --------------------------------------------------------
	/*
	if( gpGlobals->curtime < m_flNextGrenadeCheck )
	return m_lastGrenadeCondition;
	*/

	// -----------------------
	// If moving, don't check.
	// -----------------------
	if( npcOwner->IsMoving( ) )
		return COND_NONE;

	CBaseEntity *pEnemy = npcOwner->GetEnemy( );
	if( !pEnemy )
		return COND_NONE;

	Vector vecEnemyLKP = npcOwner->GetEnemyLKP( );
	if( !( pEnemy->GetFlags( ) & FL_ONGROUND ) && pEnemy->GetWaterLevel( ) == 0 && vecEnemyLKP.z > GetAbsOrigin( ).z + WorldAlignMaxs( ).z )
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		return COND_NONE;

	// --------------------------------------
	//  Get target vector
	// --------------------------------------
	Vector vecTarget;
	if( random->RandomInt( 0, 1 ) )
		// magically know where they are
		vecTarget = pEnemy->WorldSpaceCenter( );
	else
		// toss it to where you last saw them
		vecTarget = vecEnemyLKP;

	// vecTarget = m_vecEnemyLKP + pEnemy->BodyTarget( GetLocalOrigin() - pEnemy->GetLocalOrigin() );
	// estimate position
	// vecTarget = vecTarget + pEnemy->m_vecVelocity * 2;

	if( ( vecTarget - npcOwner->GetLocalOrigin( ) ).Length2D( ) <= COMBINE_MIN_GRENADE_CLEAR_DIST )
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return COND_NONE;
	}

	// ---------------------------------------------------------------------
	// Are any friendlies near the intended grenade impact area?
	// ---------------------------------------------------------------------
	CBaseEntity *pTarget = NULL;
	while( ( pTarget = gEntList.FindEntityInSphere( pTarget, vecTarget, COMBINE_MIN_GRENADE_CLEAR_DIST ) ) != NULL )
		//Check to see if the default relationship is hatred, and if so intensify that
		if( npcOwner->IRelationType( pTarget ) == D_LI )
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
			return COND_WEAPON_BLOCKED_BY_FRIEND;
		}

	// ---------------------------------------------------------------------
	// Check that throw is legal and clear
	// ---------------------------------------------------------------------
	// FIXME: speed is based on difficulty...

	Vector vecToss = VecCheckThrow( this, npcOwner->GetLocalOrigin( ) + Vector( 0, 0, 60 ), vecTarget, 600.0, 0.5 );
	if( vecToss != vec3_origin )
	{
		m_vecTossVelocity = vecToss;

		// don't check again for a while.
		// JAY: HL1 keeps checking - test?
		//m_flNextGrenadeCheck = gpGlobals->curtime;
		m_flNextGrenadeCheck = gpGlobals->curtime + 0.3; // 1/3 second.
		return COND_CAN_RANGE_ATTACK2;
	}
	else
	{
		// don't check again for a while.
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return COND_WEAPON_SIGHT_OCCLUDED;
	}
}

const WeaponProficiencyInfo_t *CWeaponMP5::GetProficiencyValues( )
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0f, 0.75f },
		{ 5.0f, 0.75f },
		{ 10.0f / 3.0f, 0.75f },
		{ 5.0f / 3.0f, 0.75f },
		{ 1.0f, 1.0f },
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE( proficiencyTable ) == WEAPON_PROFICIENCY_PERFECT + 1 );

	return proficiencyTable;
}

#endif

bool CWeaponMP5::Reload( )
{
	if (IsIronsightsEnabled() && CanReload())
	{
		DisableIronsights();
		m_bDelayedReload = true;
		return false;
	}

	bool fRet = DefaultReload( GetMaxClip1( ), GetMaxClip2( ), 
		m_iClip1 == 0 ? ACT_VM_RELOAD_EMPTY : ACT_VM_RELOAD );

	return fRet;
}

void CWeaponMP5::AddViewKick( )
{
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	float flPitch = random->RandomFloat(0.35f, 0.75f);
	float flYaw = random->RandomFloat(-0.25f, 0.25f);

	pPlayer->ViewPunch(QAngle(-flPitch, flYaw, 0));
}

void CWeaponMP5::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc )
{
	trace_t tr;
	UTIL_TraceHull( vecEye, vecSrc, -Vector( GRENADE_RADIUS + 2, GRENADE_RADIUS + 2, GRENADE_RADIUS + 2 ), Vector( GRENADE_RADIUS + 2, GRENADE_RADIUS + 2, GRENADE_RADIUS + 2 ),
		pPlayer->PhysicsSolidMaskForEntity( ), pPlayer, pPlayer->GetCollisionGroup( ), &tr );

	if( tr.DidHit( ) )
		vecSrc = tr.endpos;
}

void CWeaponMP5::SecondaryAttack( )
{
	SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	SetThink( &CWeaponMP5::TickThink );
	SetNextThink( gpGlobals->curtime + 1.5f );
	m_flNextPrimaryAttack = gpGlobals->curtime + 1.5f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 5.0f;
}

void CWeaponMP5::TickThink( )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	//Must have ammo
	if( pPlayer->GetAmmoCount( m_iSecondaryAmmoType ) <= 0 || pPlayer->GetWaterLevel( ) == 3 )
	{
		SendWeaponAnim( ACT_VM_DRYFIRE );
		BaseClass::WeaponSound( EMPTY );
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	if( m_bInReload )
		m_bInReload = false;

	// MUST call sound before removing a round from the clip of a CMachineGun
	BaseClass::WeaponSound( WPN_DOUBLE );

	Vector vecSrc = pPlayer->Weapon_ShootPosition( );
	Vector vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors( pPlayer->EyeAngles( ) + pPlayer->GetPunchAngle( ), &vecThrow );
	VectorScale( vecThrow, 1000.0f, vecThrow );

	VectorScale( vecThrow, 0.1f * 1200, vecThrow );

	Vector vecEye = pPlayer->EyePosition( );
	Vector vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
	vForward[2] += 0.1f;

#ifndef CLIENT_DLL

	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 1200;
	Fraggrenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse( 600, random->RandomInt( -1200, 1200 ), 0 ), pPlayer, GRENADE_TIMER, false );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin( ), 1000, 0.2, GetOwner( ), SOUNDENT_CHANNEL_WEAPON );

	// Register a muzzleflash for the AI.
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

	gamestats->Event_WeaponFired( pPlayer, false, GetClassname( ) );

#endif

	// Decrease ammo
	pPlayer->RemoveAmmo( 1, m_iSecondaryAmmoType );

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;

	--m_iClip2;
}

void CWeaponMP5::EnableIronsights()
{
	if (!HasIronsights() || IsIronsightsEnabled() || m_bInReload)
		return;

	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner != NULL)
	{
		m_bIsIronsighted = true;
		pOwner->EmitSound("Player.IronsightsIn");
		pOwner->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;
		pOwner->SetFOV(this, CalcViewCorrectedFov(70), 0.42f);

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

void CWeaponMP5::DisableIronsights(bool ignoreAnimation)
{
	if (!HasIronsights() || !IsIronsightsEnabled())
		return;

	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner != NULL)
	{
		m_bIsIronsighted = false;
		pOwner->EmitSound("Player.IronsightsOut");
		pOwner->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
		pOwner->SetFOV(this, 0, 0.42f);

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


#ifndef CLIENT_DLL

BEGIN_DATADESC(CWeaponMP5_alt)
DEFINE_FIELD(m_vecTossVelocity, FIELD_VECTOR),
DEFINE_FIELD(m_flNextGrenadeCheck, FIELD_TIME),
DEFINE_THINKFUNC(TickThink),
END_DATADESC()

acttable_t CWeaponMP5_alt::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, true },
	{ ACT_RELOAD, ACT_RELOAD_AR2, true },
	{ ACT_IDLE, ACT_IDLE_AR2, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_AR2, true },

	{ ACT_WALK, ACT_WALK_AR2, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_AR2, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_AR2_RELAXED, false }, //never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_AR2_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_AR2, false }, //always aims

	{ ACT_WALK_RELAXED, ACT_WALK_AR2_RELAXED, false }, //never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_AR2_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_AR2, false }, //always aims

	{ ACT_RUN_RELAXED, ACT_RUN_AR2_RELAXED, false }, //never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_AR2_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false }, //always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_AR2_RELAXED, false }, //never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_AR2, false }, //always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_AR2_RELAXED, false }, //never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_AR2_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_AR2, false }, //always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_AR2_RELAXED, false }, //never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false }, //always aims
//End readiness activities

	{ ACT_WALK_AIM, ACT_WALK_AIM_AR2, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_RIFLE, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SMG1, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SMG1_LOW, true },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_SMG1_LOW, false },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SMG1, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SMG1, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SMG1, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SMG1, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SMG1, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, false },
};

IMPLEMENT_ACTTABLE(CWeaponMP5_alt);

#endif

CWeaponMP5_alt::CWeaponMP5_alt()
{
	m_fMinRange1 = 0; // No minimum range. 
	m_fMaxRange1 = 1400;
	m_iClip2 = 5;

	m_bAltFiresUnderwater = false;
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponMP5_alt::Equip(CBaseCombatCharacter* pOwner)
{

#ifndef CLIENT_DLL

	m_fMaxRange1 = pOwner->Classify() == CLASS_PLAYER_ALLY ? 3000 : 1400;

#endif

	BaseClass::Equip(pOwner);
}

#ifndef CLIENT_DLL

void CWeaponMP5_alt::FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir)
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

void CWeaponMP5_alt::Operator_ForceNPCFire(CBaseCombatCharacter* pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	++m_iClip1;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
	AngleVectors(angShootDir, &vecShootDir);
	FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
}

void CWeaponMP5_alt::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SMG1:
	{
		Vector vecShootOrigin, vecShootDir;
		QAngle angDiscard;

		// Support old style attachment point firing
		if (pEvent->options == NULL || pEvent->options[0] == '\0' || !pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard))
			vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC* npc = pOperator->MyNPCPointer();
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

int CWeaponMP5_alt::WeaponRangeAttack2Condition(float flDot, float flDist)
{
	CAI_BaseNPC* npcOwner = GetOwner()->MyNPCPointer();

	return COND_NONE;

	// --------------------------------------------------------
	// Assume things haven't changed too much since last time
	// --------------------------------------------------------
	/*
	if( gpGlobals->curtime < m_flNextGrenadeCheck )
	return m_lastGrenadeCondition;
	*/

	// -----------------------
	// If moving, don't check.
	// -----------------------
	if (npcOwner->IsMoving())
		return COND_NONE;

	CBaseEntity* pEnemy = npcOwner->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecEnemyLKP = npcOwner->GetEnemyLKP();
	if (!(pEnemy->GetFlags() & FL_ONGROUND) && pEnemy->GetWaterLevel() == 0 && vecEnemyLKP.z > GetAbsOrigin().z + WorldAlignMaxs().z)
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		return COND_NONE;

	// --------------------------------------
	//  Get target vector
	// --------------------------------------
	Vector vecTarget;
	if (random->RandomInt(0, 1))
		// magically know where they are
		vecTarget = pEnemy->WorldSpaceCenter();
	else
		// toss it to where you last saw them
		vecTarget = vecEnemyLKP;

	// vecTarget = m_vecEnemyLKP + pEnemy->BodyTarget( GetLocalOrigin() - pEnemy->GetLocalOrigin() );
	// estimate position
	// vecTarget = vecTarget + pEnemy->m_vecVelocity * 2;

	if ((vecTarget - npcOwner->GetLocalOrigin()).Length2D() <= COMBINE_MIN_GRENADE_CLEAR_DIST)
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return COND_NONE;
	}

	// ---------------------------------------------------------------------
	// Are any friendlies near the intended grenade impact area?
	// ---------------------------------------------------------------------
	CBaseEntity* pTarget = NULL;
	while ((pTarget = gEntList.FindEntityInSphere(pTarget, vecTarget, COMBINE_MIN_GRENADE_CLEAR_DIST)) != NULL)
		//Check to see if the default relationship is hatred, and if so intensify that
		if (npcOwner->IRelationType(pTarget) == D_LI)
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
			return COND_WEAPON_BLOCKED_BY_FRIEND;
		}

	// ---------------------------------------------------------------------
	// Check that throw is legal and clear
	// ---------------------------------------------------------------------
	// FIXME: speed is based on difficulty...

	Vector vecToss = VecCheckThrow(this, npcOwner->GetLocalOrigin() + Vector(0, 0, 60), vecTarget, 600.0, 0.5);
	if (vecToss != vec3_origin)
	{
		m_vecTossVelocity = vecToss;

		// don't check again for a while.
		// JAY: HL1 keeps checking - test?
		//m_flNextGrenadeCheck = gpGlobals->curtime;
		m_flNextGrenadeCheck = gpGlobals->curtime + 0.3; // 1/3 second.
		return COND_CAN_RANGE_ATTACK2;
	}
	else
	{
		// don't check again for a while.
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return COND_WEAPON_SIGHT_OCCLUDED;
	}
}

const WeaponProficiencyInfo_t* CWeaponMP5_alt::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0f, 0.75f },
		{ 5.0f, 0.75f },
		{ 10.0f / 3.0f, 0.75f },
		{ 5.0f / 3.0f, 0.75f },
		{ 1.0f, 1.0f },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}

#endif

bool CWeaponMP5_alt::Reload()
{
	if (IsIronsightsEnabled() && CanReload())
	{
		DisableIronsights();
		m_bDelayedReload = true;
		return false;
	}

	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(),
		m_iClip1 == 0 ? ACT_VM_RELOAD_EMPTY : ACT_VM_RELOAD);

	return fRet;
}

void CWeaponMP5_alt::AddViewKick()
{
	//Get the view kick
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	float flPitch = random->RandomFloat(0.35f, 0.75f);
	float flYaw = random->RandomFloat(-0.25f, 0.25f);

	pPlayer->ViewPunch(QAngle(-flPitch, flYaw, 0));
}

void CWeaponMP5_alt::CheckThrowPosition(CBasePlayer* pPlayer, const Vector& vecEye, Vector& vecSrc)
{
	trace_t tr;
	UTIL_TraceHull(vecEye, vecSrc, -Vector(GRENADE_RADIUS + 2, GRENADE_RADIUS + 2, GRENADE_RADIUS + 2), Vector(GRENADE_RADIUS + 2, GRENADE_RADIUS + 2, GRENADE_RADIUS + 2),
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr);

	if (tr.DidHit())
		vecSrc = tr.endpos;
}

void CWeaponMP5_alt::SecondaryAttack()
{
	SendWeaponAnim(ACT_VM_SECONDARYATTACK);
	SetThink(&CWeaponMP5::TickThink);
	SetNextThink(gpGlobals->curtime + 1.5f);
	m_flNextPrimaryAttack = gpGlobals->curtime + 1.5f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 5.0f;
}

void CWeaponMP5_alt::TickThink()
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	//Must have ammo
	if (pPlayer->GetAmmoCount(m_iSecondaryAmmoType) <= 0 || pPlayer->GetWaterLevel() == 3)
	{
		SendWeaponAnim(ACT_VM_DRYFIRE);
		BaseClass::WeaponSound(EMPTY);
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	if (m_bInReload)
		m_bInReload = false;

	// MUST call sound before removing a round from the clip of a CMachineGun
	BaseClass::WeaponSound(WPN_DOUBLE);

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors(pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow);
	VectorScale(vecThrow, 1000.0f, vecThrow);

	VectorScale(vecThrow, 0.1f * 1200, vecThrow);

	Vector vecEye = pPlayer->EyePosition();
	Vector vForward, vRight;

	pPlayer->EyeVectors(&vForward, &vRight, NULL);
	CheckThrowPosition(pPlayer, vecEye, vecSrc);
	vForward[2] += 0.1f;

#ifndef CLIENT_DLL

	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vForward * 1200;
	Fraggrenade_Create(vecSrc, vec3_angle, vecThrow, AngularImpulse(600, random->RandomInt(-1200, 1200), 0), pPlayer, GRENADE_TIMER, false);

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON);

	// Register a muzzleflash for the AI.
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

	gamestats->Event_WeaponFired(pPlayer, false, GetClassname());

#endif

	// Decrease ammo
	pPlayer->RemoveAmmo(1, m_iSecondaryAmmoType);

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;

	--m_iClip2;
}

void CWeaponMP5_alt::EnableIronsights()
{
	if (!HasIronsights() || IsIronsightsEnabled() || m_bInReload)
		return;

	CHDTF_Player* pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner != NULL)
	{
		m_bIsIronsighted = true;
		pOwner->EmitSound("Player.IronsightsIn");
		pOwner->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;
		pOwner->SetFOV(this, CalcViewCorrectedFov(70), 0.42f);

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

void CWeaponMP5_alt::DisableIronsights(bool ignoreAnimation)
{
	if (!HasIronsights() || !IsIronsightsEnabled())
		return;

	CHDTF_Player* pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner != NULL)
	{
		m_bIsIronsighted = false;
		pOwner->EmitSound("Player.IronsightsOut");
		pOwner->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
		pOwner->SetFOV(this, 0, 0.42f);

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