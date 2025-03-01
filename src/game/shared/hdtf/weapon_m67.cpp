#include "cbase.h"
#include "weapon_basehdtfcombat.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "hl2_player_shared.h"
#include "hdtf_player_shared.h"

#ifndef CLIENT_DLL

#include "grenade_frag.h"
#include "weapon_ar2.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define GRENADE_TIMER 2.5f //Seconds

#define GRENADE_PAUSED_NO 0
#define GRENADE_PAUSED_PRIMARY 1
#define GRENADE_PAUSED_SECONDARY 2

#define GRENADE_RADIUS 4.0f // inches

#define GRENADE_DAMAGE_RADIUS 250.0f

#define RETHROW_DELAY 0.5f

#ifdef CLIENT_DLL

#define CWeaponM67 C_WeaponM67

#endif

class CWeaponM67 : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponM67, CBaseHDTFCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE();

#endif

	CWeaponM67();

	void Precache();
	void PrimaryAttack();
	void SecondaryAttack();
	void DecrementAmmo(CBaseCombatCharacter *pOwner);
	void ItemPostFrame();

	bool Deploy();
	bool Holster(CBaseHDTFCombatWeapon *pSwitchingTo = NULL);

	bool Reload();

#ifndef CLIENT_DLL

	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

#endif

	void ThrowGrenade(CBasePlayer *pPlayer);

	bool IsPrimed(bool)
	{
		return m_AttackPaused != 0;
	}

private:
	CWeaponM67(const CWeaponM67 &);

	void RollGrenade(CBasePlayer *pPlayer);
	void LobGrenade(CBasePlayer *pPlayer);
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc);

	CNetworkVar(bool, m_bRedraw);	//Draw the weapon again after throwing a grenade

	CNetworkVar(int, m_AttackPaused);
	CNetworkVar(bool, m_fDrawbackFinished);
};

#ifndef CLIENT_DLL

acttable_t CWeaponM67::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_GRENADE, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_GRENADE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_GRENADE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_GRENADE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_GRENADE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_GRENADE, false },
};

IMPLEMENT_ACTTABLE(CWeaponM67);

#endif

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponM67, DT_WeaponM67)

BEGIN_NETWORK_TABLE(CWeaponM67, DT_WeaponM67)

#ifdef CLIENT_DLL

RecvPropBool(RECVINFO(m_bRedraw)),
RecvPropBool(RECVINFO(m_fDrawbackFinished)),
RecvPropInt(RECVINFO(m_AttackPaused)),

#else

SendPropBool(SENDINFO(m_bRedraw)),
SendPropBool(SENDINFO(m_fDrawbackFinished)),
SendPropInt(SENDINFO(m_AttackPaused)),

#endif

END_NETWORK_TABLE()

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA(CWeaponM67)
DEFINE_PRED_FIELD(m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_fDrawbackFinished, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_AttackPaused, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()

#endif

LINK_ENTITY_TO_CLASS(weapon_m67, CWeaponM67);
PRECACHE_WEAPON_REGISTER(weapon_m67);

CWeaponM67::CWeaponM67()
{
	m_bRedraw = false;
}

void CWeaponM67::Precache()
{
	BaseClass::Precache();

#ifndef CLIENT_DLL

	UTIL_PrecacheOther("grenade_m67");

#endif
}

#ifndef CLIENT_DLL

void CWeaponM67::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	bool fThrewGrenade = false;

	switch (pEvent->event)
	{
	case EVENT_WEAPON_SEQUENCE_FINISHED:
		m_fDrawbackFinished = true;
		break;

	case EVENT_WEAPON_THROW:
		ThrowGrenade(pOwner);
		DecrementAmmo(pOwner);
		fThrewGrenade = true;
		break;

	case EVENT_WEAPON_THROW2:
		RollGrenade(pOwner);
		DecrementAmmo(pOwner);
		fThrewGrenade = true;
		break;

	case EVENT_WEAPON_THROW3:
		LobGrenade(pOwner);
		DecrementAmmo(pOwner);
		fThrewGrenade = true;
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}

	if (fThrewGrenade)
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + RETHROW_DELAY;
		m_flNextSecondaryAttack = gpGlobals->curtime + RETHROW_DELAY;
		m_flTimeWeaponIdle = FLT_MAX; //NOTE: This is set once the animation has finished up!
	}
}

#endif

bool CWeaponM67::Deploy()
{
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	return BaseClass::Deploy();
}

bool CWeaponM67::Holster(CBaseHDTFCombatWeapon *pSwitchingTo)
{
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	return BaseClass::Holster(pSwitchingTo);
}

bool CWeaponM67::Reload()
{
	if (!HasPrimaryAmmo())
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (pPlayer != NULL)
			pPlayer->SwitchToNextBestWeapon(this);

		return false;
	}

	if (m_bRedraw && m_flNextPrimaryAttack <= gpGlobals->curtime && m_flNextSecondaryAttack <= gpGlobals->curtime)
	{
		//Redraw the weapon
		SendWeaponAnim(ACT_VM_DRAW);

		//Update our times
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

		//Mark this as done
		m_bRedraw = false;
	}

	return true;
}

void CWeaponM67::SecondaryAttack()
{
	if (m_bRedraw || !HasPrimaryAmmo())
		return;

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	// Note that this is a secondary attack and prepare the grenade attack to pause.
	m_AttackPaused = GRENADE_PAUSED_SECONDARY;
	SendWeaponAnim(ACT_VM_PULLBACK_LOW);

	// Don't let weapon idle interfere in the middle of a throw!
	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextPrimaryAttack = FLT_MAX;
	m_flNextSecondaryAttack = FLT_MAX;

	// If I'm now out of ammo, switch away
	if (!HasPrimaryAmmo())
		pPlayer->SwitchToNextBestWeapon(this);
}

void CWeaponM67::PrimaryAttack()
{
	if (m_bRedraw)
		return;

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	// Note that this is a primary attack and prepare the grenade attack to pause.
	m_AttackPaused = GRENADE_PAUSED_PRIMARY;
	SendWeaponAnim(ACT_VM_PULLBACK_HIGH);

	// Put both of these off indefinitely. We do not know how long
	// the player will hold the grenade.
	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextPrimaryAttack = FLT_MAX;
	m_flNextSecondaryAttack = FLT_MAX;

	// If I'm now out of ammo, switch away
	if (!HasPrimaryAmmo())
		pPlayer->SwitchToNextBestWeapon(this);
}

void CWeaponM67::DecrementAmmo(CBaseCombatCharacter *pOwner)
{
	pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
}

void CWeaponM67::ItemPostFrame()
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (!pOwner)
		return;

	if (m_fDrawbackFinished)
	{
		switch (m_AttackPaused)
		{
		case GRENADE_PAUSED_PRIMARY:
			if ((pOwner->m_nButtons & IN_ATTACK) == 0)
			{
				SendWeaponAnim(ACT_VM_THROW);
				m_fDrawbackFinished = false;
			}

			break;

		case GRENADE_PAUSED_SECONDARY:
			if ((pOwner->m_nButtons & IN_ATTACK2) == 0)
			{
				//See if we're ducking
				if (pOwner->m_nButtons & IN_DUCK)
					//Send the weapon animation
					SendWeaponAnim(ACT_VM_SECONDARYATTACK);
				else
					//Send the weapon animation
					SendWeaponAnim(ACT_VM_HAULBACK);

				m_fDrawbackFinished = false;
			}

			break;

		default:
			break;
		}
	}

	bool primaryButton = (pOwner->m_nButtons & IN_ATTACK) != 0,
		secondaryButton = (pOwner->m_nButtons & IN_ATTACK2) != 0,
		isSprinting = pOwner->IsSprinting();

	if (primaryButton && m_flNextPrimaryAttack <= gpGlobals->curtime)
		PrimaryAttack();

	if (secondaryButton && m_flNextSecondaryAttack <= gpGlobals->curtime)
		SecondaryAttack();

	if (m_bRedraw && IsViewModelSequenceFinished())
		Reload();

	if (m_bLowered || isSprinting ||
		(!primaryButton && !secondaryButton &&
		(!m_bRedraw) && !ReloadOrSwitchWeapons() && !m_bInReload))
		WeaponIdle();
}

// check a throw from vecSrc. If not valid, move the position back along the line to vecEye
void CWeaponM67::CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc)
{
	trace_t tr;
	UTIL_TraceHull(vecEye, vecSrc, -Vector(GRENADE_RADIUS + 2, GRENADE_RADIUS + 2, GRENADE_RADIUS + 2), Vector(GRENADE_RADIUS + 2, GRENADE_RADIUS + 2, GRENADE_RADIUS + 2),
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr);

	if (tr.DidHit())
		vecSrc = tr.endpos;
}

void CWeaponM67::ThrowGrenade(CBasePlayer *pPlayer)
{

#ifndef CLIENT_DLL

	Vector vecEye = pPlayer->EyePosition();
	Vector vForward, vRight;

	pPlayer->EyeVectors(&vForward, &vRight, NULL);
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
	CheckThrowPosition(pPlayer, vecEye, vecSrc);
	//	vForward[0] += 0.1f;
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vForward * 1200;

	CBaseGrenade *pGrenade = GrenadeM67_Create(vecSrc, vec3_angle, vecThrow, AngularImpulse(600, random->RandomInt(-1200, 1200), 0), pPlayer, GRENADE_TIMER, false);
	if (pGrenade != NULL)
	{
		if (pPlayer && pPlayer->m_lifeState != LIFE_ALIVE)
		{
			pPlayer->GetVelocity(&vecThrow, NULL);

			IPhysicsObject *pPhysicsObject = pGrenade->VPhysicsGetObject();
			if (pPhysicsObject)
				pPhysicsObject->SetVelocity(&vecThrow, NULL);
		}

		pGrenade->SetDamage(150.0f);
		pGrenade->SetDamageRadius(GRENADE_DAMAGE_RADIUS);
	}

#endif

	m_bRedraw = true;

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);
}

void CWeaponM67::LobGrenade(CBasePlayer *pPlayer)
{

#ifndef CLIENT_DLL

	Vector vecEye = pPlayer->EyePosition();
	Vector vForward, vRight;

	pPlayer->EyeVectors(&vForward, &vRight, NULL);
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f + Vector(0, 0, -8);
	CheckThrowPosition(pPlayer, vecEye, vecSrc);

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vForward * 325 + Vector(0, 0, 50);

	CBaseGrenade *pGrenade = GrenadeM67_Create(vecSrc, vec3_angle, vecThrow, AngularImpulse(200, random->RandomInt(-600, 600), 0), pPlayer, GRENADE_TIMER, false);
	if (pGrenade != NULL)
	{
		if (pPlayer && pPlayer->m_lifeState != LIFE_ALIVE)
		{
			pPlayer->GetVelocity(&vecThrow, NULL);

			IPhysicsObject *pPhysicsObject = pGrenade->VPhysicsGetObject();
			if (pPhysicsObject)
				pPhysicsObject->SetVelocity(&vecThrow, NULL);
		}

		pGrenade->SetDamage(150.0f);
		pGrenade->SetDamageRadius(GRENADE_DAMAGE_RADIUS);
	}

#endif

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_bRedraw = true;
}

void CWeaponM67::RollGrenade(CBasePlayer *pPlayer)
{

#ifndef CLIENT_DLL

	// BUGBUG: Hardcoded grenade width of 4 - better not change the model :)
	Vector vecSrc;
	pPlayer->CollisionProp()->NormalizedToWorldSpace(Vector(0.5f, 0.5f, 0.0f), &vecSrc);
	vecSrc.z += GRENADE_RADIUS;

	Vector vecFacing = pPlayer->BodyDirection2D();
	// no up/down direction
	vecFacing.z = 0;
	VectorNormalize(vecFacing);

	trace_t tr;
	UTIL_TraceLine(vecSrc, vecSrc - Vector(0, 0, 16), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction != 1.0)
	{
		// compute forward vec parallel to floor plane and roll grenade along that
		Vector tangent;
		CrossProduct(vecFacing, tr.plane.normal, tangent);
		CrossProduct(tr.plane.normal, tangent, vecFacing);
	}

	vecSrc += vecFacing * 18.0;
	CheckThrowPosition(pPlayer, pPlayer->WorldSpaceCenter(), vecSrc);

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vecFacing * 325 + Vector(0, 0, 50);

	// put it on its side
	QAngle orientation(0, pPlayer->GetLocalAngles().y, -90);

	// roll it
	AngularImpulse rotSpeed(0, 0, 720);

	CBaseGrenade *pGrenade = GrenadeM67_Create(vecSrc, orientation, vecThrow, rotSpeed, pPlayer, GRENADE_TIMER, false);
	if (pGrenade != NULL)
	{
		if (pPlayer && pPlayer->m_lifeState != LIFE_ALIVE)
		{
			pPlayer->GetVelocity(&vecThrow, NULL);

			IPhysicsObject *pPhysicsObject = pGrenade->VPhysicsGetObject();
			if (pPhysicsObject)
				pPhysicsObject->SetVelocity(&vecThrow, NULL);
		}

		pGrenade->SetDamage(150.0f);
		pGrenade->SetDamageRadius(GRENADE_DAMAGE_RADIUS);
	}

#endif

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_bRedraw = true;
}
