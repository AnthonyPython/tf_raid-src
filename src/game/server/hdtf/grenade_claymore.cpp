#include "cbase.h"
#include "grenade_claymore.h"
#include "ai_basenpc.h"
#include "entitylist.h"
#include "explode.h"
#include "te_effect_dispatch.h"

ConVar sk_plr_dmg_claymore("sk_plr_dmg_claymore", "175");
ConVar sk_npc_dmg_claymore("sk_npc_dmg_claymore", "75");
ConVar sk_claymore_radius("sk_claymore_radius", "120");

#define READY_DELAY 0.5f
#define WAKE_RADIUS 52.f

#define SF_NO_SOUND			(1 << 0)
#define SF_NO_PARTICLES		(1 << 1)

class CGrenadeClaymore : public CBaseAnimating
{
	DECLARE_CLASS(CGrenadeClaymore, CBaseAnimating);

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif

public:
	void	Spawn();
	void	Precache();
	bool	CreateVPhysics();
	int		OnTakeDamage(const CTakeDamageInfo &inputInfo);
	void	ExplodeThink();
	void	Explode();
	void	SetEnabled(bool enabled = true) { m_bEnabled = enabled; }

	void	SetOwner(CBaseCombatCharacter *pOwner) { m_hOwner = pOwner; }
	CBaseCombatCharacter *GetOwner() { return m_hOwner; }

	COutputEvent	m_OnExplode;
	void			InputEnable(inputdata_t &input);
	void			InputDisable(inputdata_t &input);

	float	FindNearestTarget();

	static string_t gm_iszFloorTurretClassname;
	static string_t gm_iszGroundTurretClassname;

protected:
	bool m_bEnabled;
	float m_flDamage;
	float m_flDmgRadius;
	CHandle<CBaseCombatCharacter> m_hOwner;
};

LINK_ENTITY_TO_CLASS(grenade_claymore, CGrenadeClaymore);

BEGIN_DATADESC(CGrenadeClaymore)

DEFINE_KEYFIELD(m_bEnabled, FIELD_BOOLEAN, "enabled"),

DEFINE_OUTPUT(m_OnExplode, "OnExplode"),

DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),

DEFINE_FIELD(m_flDamage, FIELD_FLOAT),
DEFINE_FIELD(m_flDmgRadius, FIELD_FLOAT),
DEFINE_FIELD(m_hOwner, FIELD_EHANDLE),

DEFINE_THINKFUNC(ExplodeThink),

END_DATADESC()

string_t CGrenadeClaymore::gm_iszFloorTurretClassname;
string_t CGrenadeClaymore::gm_iszGroundTurretClassname;

void CGrenadeClaymore::Precache()
{
	PrecacheModel("models/weapons/w_claymore.mdl");
	PrecacheParticleSystem("hdtf_claymore");

	BaseClass::Precache();
}

void CGrenadeClaymore::Spawn()
{
	Precache();

	SetModel("models/weapons/w_claymore.mdl");

	SetOwner(static_cast<CBaseCombatCharacter *>(GetOwnerEntity()));

	// reset owner entity because otherwise
	// player won't be able to shot the claymore
	SetOwnerEntity(NULL);

	if (GetOwner() && GetOwner()->IsPlayer())
	{
		m_flDamage = sk_plr_dmg_claymore.GetFloat();
	}
	else
		m_flDamage = sk_npc_dmg_claymore.GetFloat();

	m_flDmgRadius = sk_claymore_radius.GetFloat();

	m_takedamage = DAMAGE_YES;
	m_iHealth = 1;

	SetSize(-Vector(2.5f, 9, 0), Vector(2.5f, 9, 16));
	SetCollisionGroup(COLLISION_GROUP_WEAPON);
	CreateVPhysics();

	AddSolidFlags(FSOLID_NOT_STANDABLE);

	SetThink(&CGrenadeClaymore::ExplodeThink);
	SetNextThink(gpGlobals->curtime + READY_DELAY);

	BaseClass::Spawn();
}

bool CGrenadeClaymore::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, 0, false);
	SetMoveType(MOVETYPE_NONE);
	
	return true;
}

void CGrenadeClaymore::ExplodeThink()
{
	if (m_bEnabled)
	{
		float nearest = FindNearestTarget();

		if (nearest <= WAKE_RADIUS)
		{
			Explode();
			return;
		}
	}

	SetNextThink(gpGlobals->curtime + 0.1f);
}

float CGrenadeClaymore::FindNearestTarget()
{
	float flNearest = (WAKE_RADIUS * WAKE_RADIUS) + 1.0;

	CBaseCombatCharacter *pNearest = NULL;

	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();

	for (int i = 0; i < nAIs; i++)
	{
		CAI_BaseNPC *pNPC = ppAIs[i];

		if (pNPC->IsAlive())
		{
			if (pNPC->IsEffectActive(EF_NODRAW))
				continue;

			if (pNPC->EyePosition().z < GetAbsOrigin().z)
				continue;

			if (pNPC->Classify() == CLASS_NONE)
				continue;

			if (pNPC->Classify() == CLASS_BULLSEYE)
				continue;

			if (pNPC->IsPlayerAlly())
				continue;

			if (pNPC->m_iClassname == gm_iszFloorTurretClassname 
				|| pNPC->m_iClassname == gm_iszGroundTurretClassname)
				continue;

			float flDist = (GetAbsOrigin() - pNPC->GetAbsOrigin()).LengthSqr();

			if (flDist < flNearest)
			{
				if (FVisible(pNPC, MASK_SOLID_BRUSHONLY))
				{
					flNearest = flDist;
					pNearest = pNPC;
				}
			}
		}
	}

	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

	if (pPlayer && pPlayer != GetOwner() && !(pPlayer->GetFlags() & FL_NOTARGET))
	{
		float flDist = (pPlayer->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();

		if (flDist < flNearest && FVisible(pPlayer, MASK_SOLID_BRUSHONLY))
		{
			flNearest = flDist;
			pNearest = pPlayer;
		}
	}

	return sqrt(flNearest);
}

int CGrenadeClaymore::OnTakeDamage(const CTakeDamageInfo &inputInfo)
{
	if (m_takedamage == DAMAGE_NO)
	{
		return 0;
	}

	if (inputInfo.GetDamageType() & DMG_BLAST
		|| inputInfo.GetDamageType() & DMG_BULLET
		|| inputInfo.GetDamageType() & DMG_BUCKSHOT
		|| inputInfo.GetDamageType() & DMG_SLASH
		|| inputInfo.GetDamageType() & DMG_CLUB)
	{
		Explode();
		return 1;
	}

	return 0;
}

void CGrenadeClaymore::Explode()
{
	SetThink(NULL);

	m_takedamage = DAMAGE_NO;

	int explosionFlags = SF_ENVEXPLOSION_NOPARTICLES | SF_ENVEXPLOSION_NOFIREBALL;

	if (HasSpawnFlags(SF_NO_SOUND))
	{
		explosionFlags |= SF_ENVEXPLOSION_NOSOUND;
	}

	ExplosionCreate(GetAbsOrigin(), 
		GetAbsAngles(), 
		(GetOwner()) ? dynamic_cast<CBaseEntity *>(GetOwner()) : this,
		m_flDamage,
		m_flDmgRadius,
		explosionFlags,
		0.f,
		this);

	if (!HasSpawnFlags(SF_NO_PARTICLES))
	{
		DispatchParticleEffect("hdtf_claymore", GetAbsOrigin(), GetAbsAngles(), NULL);
	}

	m_OnExplode.FireOutput(this, this, 0.f);

	UTIL_Remove(this);
}

void CGrenadeClaymore::InputEnable(inputdata_t &input)
{
	SetEnabled(true);
}

void CGrenadeClaymore::InputDisable(inputdata_t &input)
{
	SetEnabled(false);
}

CGrenadeClaymore *ClaymoreGrenadeCreate(const Vector &pos,
	const QAngle &ang,
	CBasePlayer *pOwner,
	CBaseEntity *pParent)
{
	CGrenadeClaymore *pGrenade = (CGrenadeClaymore *)CBaseEntity::Create("grenade_claymore", pos, ang, pOwner);

	if (pParent && (!pParent->IsWorld() && !pParent->ClassMatches("func_reflective_glass")) || !pParent)
		pGrenade->SetMoveType(MOVETYPE_VPHYSICS);

	pGrenade->SetEnabled(true);

	return pGrenade;
}
