#include "cbase.h"
#include "weapon_basehdtfcombat.h"
#include "in_buttons.h"

#ifndef CLIENT_DLL
#include "props.h"

#define ATTACK_THINK_CONTEXT "Parkour_AttackThink"
#endif

#ifdef CLIENT_DLL
#define CWeaponOneShotBarrel C_WeaponOneShotBarrel
#endif

class CWeaponOneShotBarrel : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponOneShotBarrel, CBaseHDTFCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE();

#endif

	CWeaponOneShotBarrel();

	virtual const Vector &GetBulletSpread()
	{
		static Vector cone = VECTOR_CONE_15DEGREES;
		return cone;
	}

	void ItemPostFrame();
	void PrimaryAttack();
	void Equip(CBaseCombatCharacter *pOwner);

	bool ShouldDrawAmmo() { return false; }

	virtual bool CanHolster() const;

	void AdjustTimeScale(float scale = 1.f);

#ifndef CLIENT_DLL
	void CommitAttack();
	void AttackThink();
	void CreateBlankWeapon(CBasePlayer *pOwner);
#endif

	void CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc);

private:
	CWeaponOneShotBarrel(const CWeaponOneShotBarrel &);

#ifndef CLIENT_DLL
	CBaseCombatWeapon *pPreviousWeapon;
#endif

	bool m_bPreparing;

	CNetworkVar(bool, m_bReady);
	CNetworkVar(float, m_flWeaponDrop);
	CNetworkVar(float, m_flIntroFinish);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponOneShotBarrel, DT_WeaponOneShotBarrel)

BEGIN_NETWORK_TABLE(CWeaponOneShotBarrel, DT_WeaponOneShotBarrel)

#ifdef CLIENT_DLL

RecvPropBool(RECVINFO(m_bReady)),
RecvPropFloat(RECVINFO(m_flWeaponDrop)),
RecvPropFloat(RECVINFO(m_flIntroFinish)),

#else

SendPropBool(SENDINFO(m_bReady)),
SendPropFloat(SENDINFO(m_flWeaponDrop)),
SendPropFloat(SENDINFO(m_flIntroFinish)),

#endif

END_NETWORK_TABLE()

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA(CWeaponOneShotBarrel)
DEFINE_PRED_FIELD(m_bReady, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flWeaponDrop, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flIntroFinish, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()

#endif

LINK_ENTITY_TO_CLASS(weapon_oneshotbarrel, CWeaponOneShotBarrel);
PRECACHE_WEAPON_REGISTER(weapon_oneshotbarrel);

#ifndef CLIENT_DLL

acttable_t CWeaponOneShotBarrel::m_acttable[] =
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

IMPLEMENT_ACTTABLE(CWeaponOneShotBarrel);

#endif

CWeaponOneShotBarrel::CWeaponOneShotBarrel()
{
#ifndef CLIENT_DLL
	pPreviousWeapon = NULL;
#endif

	m_bPreparing = false;

	m_bReady = false;
	m_flWeaponDrop = -1.f;
	m_flIntroFinish = -1.f;
}

//-----------------------------------------------------------------------------
// Purpose: hack to update cheat-protected host_timescale
//-----------------------------------------------------------------------------
void CWeaponOneShotBarrel::AdjustTimeScale(float scale)
{
	// for now there is no way to slow the game
	// without turning the sv_cheats on
	ConVar* sv_cheats = cvar->FindVar("sv_cheats");
	ConVar* host_timesale = cvar->FindVar("host_timescale");
	sv_cheats->SetValue(1);
	host_timesale->SetValue(scale);
	
	if (scale == 1.f)
		sv_cheats->SetValue(0);
}

//-----------------------------------------------------------------------------
// Purpose: play equip animation and switch to another weapon
//-----------------------------------------------------------------------------
void CWeaponOneShotBarrel::Equip(CBaseCombatCharacter *pOwner)
{
	BaseClass::Equip(pOwner);

#ifndef CLIENT_DLL
	pPreviousWeapon = pOwner->GetActiveWeapon();

	// this happens before we actualy set equiped weapon
	// as active one, so we have to switch it manualy
	pOwner->SetActiveWeapon(this);
#endif

	SendWeaponAnim(ACT_VM_PICKUP);

	m_flIntroFinish = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: disallow weapon holster if we're doing the animation
//-----------------------------------------------------------------------------
bool CWeaponOneShotBarrel::CanHolster() const
{
	return !m_bReady;
}

//-----------------------------------------------------------------------------
// Purpose: called via console command to initiate sequence
//-----------------------------------------------------------------------------
#ifndef CLIENT_DLL
void CWeaponOneShotBarrel::CommitAttack()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL || m_bReady || m_flWeaponDrop != -1.f || m_bPreparing)
		return;

	// if we have an active weapon we need to holster it first!
	CBaseCombatWeapon *pWep = pPlayer->GetActiveWeapon();
	if (pWep != NULL && pWep != this)
	{
		if (!pWep->CanHolster())
			return;

		m_bPreparing = true;

		pPreviousWeapon = pWep;

		pWep->SendWeaponAnim(ACT_VM_HOLSTER);

		float flDuration = pWep->SequenceDuration();

		// if this weapon doesn't have a holster animation we can skip this logic
		if (flDuration && flDuration != 0.f)
		{
			SetContextThink(
				&CWeaponOneShotBarrel::AttackThink, 
				gpGlobals->curtime + flDuration,
				ATTACK_THINK_CONTEXT);

			return;
		}

	}

	AttackThink();
}

//-----------------------------------------------------------------------------
// Purpose: called after we holster previous weapon and actualy initiates the attack
//-----------------------------------------------------------------------------
void CWeaponOneShotBarrel::AttackThink()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL || m_bReady)
		return;

	pPlayer->SetActiveWeapon(this);

	m_flIntroFinish = -1.f;

	m_bReady = true;

	SendWeaponAnim(ACT_VM_DRAW);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

	AdjustTimeScale(0.25f);
}
#endif

//-----------------------------------------------------------------------------
// Purpose: fire the weapon!
//-----------------------------------------------------------------------------
void CWeaponOneShotBarrel::PrimaryAttack()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if( pPlayer == NULL )
		return;

	WeaponSound(SINGLE);

	pPlayer->DoMuzzleFlash();

	m_fShotLast = gpGlobals->curtime;
	DoSmokeBarrel();

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);

	m_flWeaponDrop = gpGlobals->curtime + SequenceDuration();

	pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	FireBulletsInfo_t info(17, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;
	info.m_flDamage = 15.f;

	pPlayer->FireBullets(info);

	QAngle punch;
	punch.Init(SharedRandomFloat("doublebarrelpax", -4, -2), 
		SharedRandomFloat("doublebarrelpay", -2, 2 ), 0);

	pPlayer->ViewPunch(punch);

	AdjustTimeScale(1.f);

	m_flNextPrimaryAttack = -1.f;
}

//-----------------------------------------------------------------------------
// Purpose: timings, attack and drop logic
//-----------------------------------------------------------------------------
void CWeaponOneShotBarrel::ItemPostFrame()
{
	if (m_flIntroFinish != -1.f && m_flIntroFinish <= gpGlobals->curtime)
	{
		m_flIntroFinish = -1.f;

#ifndef CLIENT_DLL
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		if (pOwner != NULL)
			// if we have a previous weapon we can use it
			if (pPreviousWeapon != NULL)
			{
				pOwner->Weapon_Switch(pPreviousWeapon);
				pPreviousWeapon = NULL;
			}
			else
				// ... otherwise select best weapon available
				pOwner->SwitchToNextBestWeapon(this);
#endif
	}

	if (m_bReady && m_flNextPrimaryAttack != -1.f && m_flNextPrimaryAttack <= gpGlobals->curtime)
		PrimaryAttack();

	if (m_flWeaponDrop != -1.f && m_flWeaponDrop <= gpGlobals->curtime)
	{
		m_bReady = false;

		// Expected to solve this with Drop method
		// but it doesn't seem to actualy drop the
		// weapon and causes a crash when attempting 
		// to switch to another weapon

#ifndef CLIENT_DLL
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		// spawn physics prop with our world model and throw it away
		CreateBlankWeapon(pOwner);

		// if we have a previous weapon we can use it
		if (pPreviousWeapon != NULL)
		{
			pOwner->Weapon_Switch(pPreviousWeapon);
			pPreviousWeapon = NULL;
		}
		else
			// ... otherwise select best weapon available
			pOwner->SwitchToNextBestWeapon(this);

		// now we can remove the weapon
		UTIL_Remove(this);
#endif
	}
}

void CWeaponOneShotBarrel::CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc)
{
	trace_t tr;
	UTIL_TraceHull(vecEye, vecSrc,
		-Vector(8, 8, 8),
		Vector(8, 8, 8),
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr);
	if (tr.DidHit())
		vecSrc = tr.endpos;
}

//-----------------------------------------------------------------------------
// Purpose: spawns a weapon prop and throws it away
//-----------------------------------------------------------------------------
#ifndef CLIENT_DLL
void CWeaponOneShotBarrel::CreateBlankWeapon(CBasePlayer *pOwner)
{
	// compute drop origin & velocity
	QAngle ang = pOwner->EyeAngles();
	Vector pos = pOwner->Weapon_ShootPosition(),
		forward,
		right,
		up,
		vel;

	AngleVectors(ang, &forward, &right, &up);
	vel = forward * 100.f + right * 1200.f + up * 100.f;

	Vector spawnPos = pos + right * 11.f;
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

//-----------------------------------------------------------------------------
// Purpose: fire the weapon from our inventory
//-----------------------------------------------------------------------------
#ifndef CLIENT_DLL
void CC_ShotOSWeapon()
{
	CBasePlayer *pPlayer = ToBasePlayer(UTIL_GetLocalPlayer());
	if (pPlayer == NULL)
		return;

	CWeaponOneShotBarrel *pWeapon = (CWeaponOneShotBarrel*)pPlayer->Weapon_OwnsThisType("weapon_oneshotbarrel");
	
	if (pWeapon == NULL)
		return;

	pWeapon->CommitAttack();
}

static ConCommand shotosweapon(
	"shotosweapon", 
	CC_ShotOSWeapon,
	"Fires the one-shot weapon if there is one in the inventory"
	);
#endif
