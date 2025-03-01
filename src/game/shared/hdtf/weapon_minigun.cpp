#include "cbase.h"
#include "weapon_basemachinegun.h"
#include "hdtf_player_shared.h"
#include "soundenvelope.h"
#include "in_buttons.h"
#include "ammodef.h"
#include "soundenvelope.h"

#ifndef CLIENT_DLL
#include "props.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_minigun_damage("sk_minigun_damage", "24", FCVAR_REPLICATED);

#ifdef CLIENT_DLL
#define CWeaponMinigun C_WeaponMinigun
#endif

class CWeaponMinigun : public CMachineGun
{
public:
	DECLARE_CLASS(CWeaponMinigun, CMachineGun);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CWeaponMinigun();
	~CWeaponMinigun();

	bool CanBeDeselected() const { return false; }
	virtual bool CanHolster() const;

	void Equip(CBaseCombatCharacter *pOwner);

	bool CanPrimaryAttack();
	void PrimaryAttack();

	void ItemPostFrame();
	void ItemBusyFrame();
	void Precache();

	bool ShouldDrawAmmo() { return false; }

#ifndef CLIENT_DLL
	void CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc);
	void CreateBlankWeapon(CBasePlayer *pOwner);
#endif

	float GetFireRate() { return 0.050f; }

	void InputDrop( inputdata_t &inputData );

private:
	CWeaponMinigun(const CWeaponMinigun &);

	CBaseCombatWeapon *m_pWeaponLast;

	bool m_bHolsterPermitted;

	bool m_bSpin;
	CNetworkVar(float, m_flSpinSpeed);
	CNetworkVar(float, m_flDelayedHolster);
	float m_flSpin;
	int m_iBarrelPose;
	CSoundPatch *m_pSpinSound;
};

#ifndef CLIENT_DLL
BEGIN_DATADESC(CWeaponMinigun)
	DEFINE_FIELD(m_flSpinSpeed, FIELD_FLOAT),
	DEFINE_FIELD(m_flSpin, FIELD_FLOAT),
	DEFINE_FIELD(m_flDelayedHolster, FIELD_TIME),
	DEFINE_FIELD(m_pWeaponLast, FIELD_EHANDLE),

	DEFINE_INPUTFUNC(FIELD_VOID, "ReleaseWeapon", InputDrop),
END_DATADESC()
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponMinigun, DT_WeaponMinigun)

BEGIN_NETWORK_TABLE(CWeaponMinigun, DT_WeaponMinigun)

#ifdef CLIENT_DLL
	RecvPropFloat(RECVINFO(m_flSpinSpeed)),
	RecvPropFloat(RECVINFO(m_flDelayedHolster))
#else
	SendPropFloat(SENDINFO(m_flSpinSpeed)),
	SendPropFloat(SENDINFO(m_flDelayedHolster))
#endif

END_NETWORK_TABLE()

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA(CWeaponMinigun)
DEFINE_PRED_FIELD(m_flSpinSpeed, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flDelayedHolster, FIELD_TIME, FTYPEDESC_INSENDTABLE)
END_PREDICTION_DATA()

#endif

LINK_ENTITY_TO_CLASS(weapon_minigun, CWeaponMinigun);
PRECACHE_WEAPON_REGISTER(weapon_minigun);

CWeaponMinigun::CWeaponMinigun()
{
	m_flSpinSpeed = 0.f;
	m_flSpin = 0.f;
	m_iBarrelPose = -1;
	m_flDelayedHolster = -1.f;
	m_bSpin = false;
	m_pWeaponLast = NULL;
	m_bHolsterPermitted = false;
}

CWeaponMinigun::~CWeaponMinigun()
{
	if (m_pSpinSound != NULL)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_pSpinSound);
	}
}

void CWeaponMinigun::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound("WeaponMinigun.SpinStart");
	PrecacheScriptSound("WeaponMinigun.SpinEnd");
}

void CWeaponMinigun::Equip(CBaseCombatCharacter *pOwner)
{
	m_pWeaponLast = pOwner->GetActiveWeapon();
	SetVisibleInWeaponSelection(true);

	BaseClass::Equip(pOwner);
}

bool CWeaponMinigun::CanHolster() const
{
	return m_bHolsterPermitted;
}

bool CWeaponMinigun::CanPrimaryAttack()
{
	// Only the player fires this way so we can cast
	CHDTF_Player *pPlayer = ToHDTFPlayer(GetOwner());
	if (pPlayer == NULL)
		return false;

	if (pPlayer->GetWaterLevel() == 3 && !m_bFiresUnderwater)
	{
		// This weapon doesn't fire underwater
		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
		return false;
	}

	if (pPlayer->IsSprinting() && !IsIronsightsEnabled()
		&& (GetActivity() == ACT_VM_SPRINT_IDLE || GetActivity() == ACT_VM_SPRINT_IDLE_SPECIAL))
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
	if ((pPlayer->m_afButtonPressed & IN_ATTACK) != 0 || (pPlayer->m_afButtonReleased & IN_ATTACK2) != 0)
		m_flNextPrimaryAttack = gpGlobals->curtime;

	return true;
}

void CWeaponMinigun::PrimaryAttack()
{
	if (!CanPrimaryAttack())
		return;

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	++m_nShotsFired;

	pPlayer->DoMuzzleFlash();

	m_fShotLast = gpGlobals->curtime;
	DoSmokeBarrel();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;
	float fireRate = GetFireRate();

	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		// MUST call sound before removing a round from the clip of a CHLMachineGun
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack += fireRate;
		++iBulletsToFire;
	}

	// Fire the bullets
	FireBulletsInfo_t info;
	info.m_iShots = iBulletsToFire;
	info.m_vecSrc = pPlayer->Weapon_ShootPosition();
	info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	info.m_vecSpread = GetBulletSpread();
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;
	info.m_flDamage = sk_minigun_damage.GetFloat();
	FireBullets(info);

	//Factor in the view kick
	AddViewKick();

	SendWeaponAnim(GetPrimaryAttackActivity());
	pPlayer->SetAnimation(PLAYER_ATTACK1);
}

void CWeaponMinigun::ItemPostFrame()
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (!pOwner)
		return;

	bool primaryButton = (pOwner->m_nButtons & IN_ATTACK) != 0;

	if (primaryButton)
	{
		if (!m_bSpin)
		{
			if (!m_pSpinSound)
			{
				CPASAttenuationFilter filter(this);
				m_pSpinSound = CSoundEnvelopeController::GetController().SoundCreate(filter, entindex(), "WeaponMinigun.SpinStart");
				CSoundEnvelopeController::GetController().Play(m_pSpinSound, 0.0, 100, m_flSpinSpeed * 0.432f);
			}

			if (m_pSpinSound)
			{
				CSoundEnvelopeController::GetController().Play(m_pSpinSound, 0.0, 100, m_flSpinSpeed * 0.432f);
				CSoundEnvelopeController::GetController().SoundChangeVolume(m_pSpinSound, 1.f, 0.25f);
			}

			m_bSpin = true;
		}
		m_flSpinSpeed.GetForModify() = Approach(1.f, m_flSpinSpeed, gpGlobals->frametime * 2.5f);
	}
	else
	{
		if (m_bSpin)
		{
			if (m_pSpinSound)
				CSoundEnvelopeController::GetController().SoundFadeOut(m_pSpinSound, 0.25f);

			if(m_flSpinSpeed > 0.75)
				EmitSound("WeaponMinigun.SpinEnd");
			
			m_bSpin = false;
		}
		m_flSpinSpeed.GetForModify() = Approach(0.f, m_flSpinSpeed, gpGlobals->frametime * 0.375f);
	}

	if (m_flSpinSpeed > 0.f)
	{
		m_flSpin = (m_flSpin + gpGlobals->frametime * 1020.f * m_flSpinSpeed);
		if (m_flSpin > 360.f)
			m_flSpin -= 360.f;

		if (m_iBarrelPose == -1)
		{
			m_iBarrelPose = pOwner->GetViewModel()->LookupPoseParameter("barrel_spin");
		}

		pOwner->GetViewModel()->SetPoseParameter(m_iBarrelPose, m_flSpin);
	}

	if (m_flSpinSpeed == 1.f && m_flNextPrimaryAttack < gpGlobals->curtime)
	{
		m_flNextPrimaryAttack = gpGlobals->curtime;
		PrimaryAttack();
	}

	if (!primaryButton)
		WeaponIdle();
}

void CWeaponMinigun::ItemBusyFrame()
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());

	if (pOwner)
	{
		if (m_iBarrelPose == -1)
		{
			m_iBarrelPose = pOwner->GetViewModel()->LookupPoseParameter("barrel_spin");
		}

		pOwner->GetViewModel()->SetPoseParameter(m_iBarrelPose, m_flSpin);

		if (m_flDelayedHolster != -1.f && m_flDelayedHolster < gpGlobals->curtime)
		{
			m_flDelayedHolster = -1.f;

#ifndef CLIENT_DLL
			Vector vForward, vRight;
			pOwner->EyeVectors(&vForward, &vRight);

			Vector vThrowVelocity = vRight * 24.f - vForward * 5.f;

			CreateBlankWeapon(pOwner);

			m_bHolsterPermitted = true;
			if (m_pWeaponLast)
				pOwner->Weapon_Switch(m_pWeaponLast);

			UTIL_Remove(this);
#endif

			return;
		}
	}
	
	BaseClass::ItemBusyFrame();
}

#ifndef CLIENT_DLL
void CWeaponMinigun::CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc)
{
	trace_t tr;
	UTIL_TraceHull(vecEye, vecSrc,
		-Vector(8, 8, 8),
		Vector(8, 8, 8),
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr);
	if (tr.DidHit())
		vecSrc = tr.endpos;
}

void CWeaponMinigun::CreateBlankWeapon(CBasePlayer *pOwner)
{
	// compute drop origin & velocity
	QAngle ang = pOwner->EyeAngles();
	Vector pos = pOwner->Weapon_ShootPosition(),
		forward,
		right,
		up,
		vel;

	AngleVectors(ang, &forward, &right, &up);
	vel = -forward * 100.f + right * 180.f + up * 20.f;

	Vector spawnPos = pos + right * 11.f - forward * 3.5f;
	CheckThrowPosition(pOwner, pos, spawnPos);

	CPhysicsProp *pProp = dynamic_cast<CPhysicsProp *>(CreateEntityByName("prop_physics"));
	if (pProp)
	{
		char buf[512];
		Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", spawnPos.x, spawnPos.y, spawnPos.z);
		pProp->KeyValue("origin", buf);
		Q_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", ang.x - 15.f, ang.y + 180.f, ang.z + 15.f);
		pProp->KeyValue("angles", buf);
		pProp->KeyValue("model", GetWorldModel());
		pProp->KeyValue("fademindist", "-1");
		pProp->KeyValue("fademaxdist", "0");
		pProp->KeyValue("fadescale", "1");
		pProp->KeyValue("inertiaScale", "1.0");
		pProp->KeyValue("physdamagescale", "0");
		pProp->Precache();
		DispatchSpawn(pProp);
		pProp->Activate();
		pProp->SetCollisionGroup(COLLISION_GROUP_WEAPON);
		pProp->VPhysicsGetObject()->ApplyForceCenter(vel);
	}
}
#endif

void CWeaponMinigun::InputDrop(inputdata_t &inputData)
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (!pOwner)
		return;

	if (pOwner->GetActiveWeapon() != this)
		return;

	SendWeaponAnim(ACT_VM_HOLSTER);
	m_flDelayedHolster = gpGlobals->curtime + SequenceDuration();
	pOwner->m_flNextAttack = m_flDelayedHolster + 0.15f;
}
