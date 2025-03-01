#include "cbase.h"
#include "weapon_basehdtfcombat.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "hdtf_player_shared.h"

#ifndef CLIENT_DLL

#include "weapon_flaregun.h"
#include "props.h"
#include "te_effect_dispatch.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FLARE_RADIUS 4.0f

#define FLARE_PAUSED_NO 0
#define FLARE_PAUSED_PRIMARY 1
#define FLARE_PAUSED_SECONDARY 2

#define RETHROW_DELAY 0.5f

#define HDTF_FLARE_DURATION 900.f

#ifdef CLIENT_DLL

#define CWeaponFlare C_WeaponFlare

#endif

class CWeaponFlare : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponFlare, CBaseHDTFCombatWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	CWeaponFlare( );

	void Precache( );
	void PrimaryAttack( );
	void SecondaryAttack( );
	bool Deploy();
	bool Reload();
	void DecrementAmmo();

	void CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc);

	void ItemPostFrame();

#ifndef CLIENT_DLL
	CFlare *CreateFlareEntity(Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, float lifetime);
	void SpawnFlare(bool bIsSecondary);

	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
#endif

private:
	CNetworkVar(bool, m_bRedraw);
	CWeaponFlare( const CWeaponFlare & );
};

#ifndef CLIENT_DLL

acttable_t CWeaponFlare::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_GRENADE, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_GRENADE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_GRENADE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_GRENADE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_GRENADE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_GRENADE, false },
};

IMPLEMENT_ACTTABLE( CWeaponFlare );

#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponFlare, DT_WeaponFlare )

BEGIN_NETWORK_TABLE( CWeaponFlare, DT_WeaponFlare )
#ifndef CLIENT_DLL
	SendPropBool(SENDINFO(m_bRedraw)),
#else
	RecvPropBool(RECVINFO(m_bRedraw)),
#endif
END_NETWORK_TABLE( )

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponFlare )
END_PREDICTION_DATA( )

#endif

LINK_ENTITY_TO_CLASS( weapon_flare, CWeaponFlare );
PRECACHE_WEAPON_REGISTER( weapon_flare );

CWeaponFlare::CWeaponFlare( )
{ }

void CWeaponFlare::Precache( )
{
	BaseClass::Precache( );

	PrecacheScriptSound( "Flare.Touch" );
	PrecacheScriptSound( "WeaponFlare.BurnLoop" );
	PrecacheParticleSystem("hdtf_flare_ignition");

	PrecacheModel( "models/weapons/w_flare.mdl" );

#ifndef CLIENT_DLL

	UTIL_PrecacheOther( "env_flare" );

#endif

}

bool CWeaponFlare::Deploy()
{
	m_bRedraw = false;

	return BaseClass::Deploy();
}

void CWeaponFlare::SecondaryAttack( )
{
	if ( m_bRedraw )
		return;

	if( !HasPrimaryAmmo( ) )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	SendWeaponAnim( ACT_VM_PULLBACK_LOW );

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flTimeWeaponIdle = FLT_MAX;

	m_bRedraw = true;
}

void CWeaponFlare::PrimaryAttack( )
{
	if ( m_bRedraw )
		return;

	if( !HasPrimaryAmmo( ) )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner( ) );
	if( pOwner == NULL )
		return;

	m_fShotLast = gpGlobals->curtime;
	DoSmokeBarrel();

	SendWeaponAnim( ACT_VM_PULLBACK_HIGH );

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flTimeWeaponIdle = FLT_MAX;

	m_bRedraw = true;
}

void CWeaponFlare::DecrementAmmo()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

void CWeaponFlare::ItemPostFrame()
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner == NULL)
		return;

	bool primaryButton = (pOwner->m_nButtons & IN_ATTACK) != 0,
		secondaryButton = (pOwner->m_nButtons & IN_ATTACK2) != 0,
		isSprinting = pOwner->IsSprinting();

	if (primaryButton && m_flNextPrimaryAttack <= gpGlobals->curtime)
		PrimaryAttack();

	if (secondaryButton && m_flNextPrimaryAttack <= gpGlobals->curtime)
		SecondaryAttack();

	if (m_bRedraw && IsViewModelSequenceFinished())
		Reload();

	if (m_bLowered || isSprinting ||
		(!primaryButton && !secondaryButton &&
		(!m_bRedraw) && !ReloadOrSwitchWeapons() && !m_bInReload))
		WeaponIdle();
}

bool CWeaponFlare::Reload()
{
	if (!HasPrimaryAmmo())
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (pPlayer != NULL)
			pPlayer->SwitchToNextBestWeapon(this);

		return false;
	}

	if (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		SendWeaponAnim(ACT_VM_DRAW);

		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		m_flTimeWeaponIdle = m_flNextPrimaryAttack;

		m_bRedraw = false;
	}

	return true;
}

void CWeaponFlare::CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc)
{
	trace_t tr;

	UTIL_TraceHull(vecEye, vecSrc, -Vector(1.7f, 1.7f, 4.2f), Vector(1.7f, 1.7f, 4.2f),
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr);

	if (tr.DidHit())
	{
		vecSrc = tr.endpos;
	}
}

#ifndef CLIENT_DLL
CFlare *CWeaponFlare::CreateFlareEntity(Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, float lifetime)
{
	CFlare *pFlare = (CFlare *)CreateEntityByName("env_flare");

	if (pFlare == NULL)
		return NULL;

	UTIL_SetOrigin(pFlare, vecOrigin);

	pFlare->AddSpawnFlags(SF_FLARE_CUSTOM_PARTICLE);
	pFlare->SetLocalAngles(vecAngles);
	pFlare->Spawn();
	pFlare->SetTouch(&CFlare::FlareTouch);
	pFlare->SetThink(&CFlare::FlareThink);

	//Start up the flare
	pFlare->Start(lifetime);

	//Don't start sparking immediately
	pFlare->SetNextThink(gpGlobals->curtime + 0.5f);

	//Burn out time
	pFlare->m_flTimeBurnOut = gpGlobals->curtime + lifetime;

	pFlare->RemoveSolidFlags(FSOLID_NOT_SOLID);
	pFlare->AddSolidFlags(FSOLID_NOT_STANDABLE);

	pFlare->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);

	pFlare->SetOwnerEntity(pOwner);
	pFlare->m_pOwner = pOwner;

	return pFlare;
}

void CWeaponFlare::SpawnFlare(bool bIsSecondary)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	CPhysicsProp *pProp = dynamic_cast<CPhysicsProp *>(CreateEntityByName("prop_physics"));

	Assert(pProp);

	QAngle angEye = pOwner->EyeAngles();
	Vector vecEye = pOwner->EyePosition();
	Vector vForward, vRight;

	pOwner->EyeVectors(&vForward, &vRight, NULL);
	Vector vecSrc;

	if (bIsSecondary)
	{
		vecSrc = vecEye - Vector(0, 0, 8);
		angEye.y -= 45.f;
	}
	else
	{
		vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
		angEye.y -= 90.f;
	}

	CheckThrowPosition(pOwner, vecEye, vecSrc);

	UTIL_SetOrigin(pProp, vecSrc);
	pProp->SetAbsAngles(angEye);

	pProp->KeyValue("model", "models/weapons/w_flare.mdl");
	pProp->Precache();
	DispatchSpawn(pProp);
	pProp->Activate();
	pProp->SetCollisionGroup(COLLISION_GROUP_WEAPON);
	pProp->AddEffects(EF_NOSHADOW);
	pProp->SetName(MAKE_STRING("globals.flare"));

	pProp->SetBodygroup(pProp->FindBodygroupByName("cap"), 1);

	pProp->SetFriction(1.f);

	CFlare *pFlare = CreateFlareEntity(pProp->GetAbsOrigin(),
		pProp->GetAbsAngles(), 
		pOwner, 
		HDTF_FLARE_DURATION);

	if (pFlare)
	{
		pFlare->m_bPropFlare = true;
		pFlare->m_bSmoke = false; // we use particle instead

		int iAttachment = pProp->LookupAttachment("fuse");

		Vector vOrigin;
		QAngle vAngle;
		pProp->GetAttachment(iAttachment, vOrigin, vAngle);

		pFlare->SetMoveType(MOVETYPE_NONE);
		pFlare->SetSolid(SOLID_NONE);
		pFlare->SetRenderMode(kRenderTransAlpha);
		pFlare->SetRenderColorA(1);
		pFlare->SetLocalOrigin(vOrigin);
		pFlare->SetAbsAngles(vAngle);
		pFlare->SetParent(pProp, iAttachment);
		pFlare->m_flScale = 2.f;

		AddEntityToDarknessCheck(pFlare);
	}

	Vector forward;
	pOwner->EyeVectors(&forward);

	if (bIsSecondary)
	{
		pProp->VPhysicsGetObject()->ApplyForceCenter(forward * 100.f);
	}
	else
	{
		pProp->VPhysicsGetObject()->ApplyForceCenter(forward * 900.f);
	}
}

void CWeaponFlare::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_THROW:
		DecrementAmmo();
		SpawnFlare(false);
		break;

	case EVENT_WEAPON_THROW2:
		DecrementAmmo();
		SpawnFlare(true);
		break;

	case EVENT_WEAPON_MISSILE_FIRE:
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		if (pOwner)
		{
			CBaseViewModel *pVM = pOwner->GetViewModel();
			DispatchParticleEffect("hdtf_flare_ignition", PATTACH_POINT_FOLLOW, pVM, "muzzle", true);
			break;
		}
	}

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif