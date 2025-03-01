#include "cbase.h"
#include "weapon_basemachinegun.h"

#ifdef CLIENT_DLL

#define CWeaponSkorpion C_WeaponSkorpion

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponSkorpion : public CMachineGun
{
public:
	DECLARE_CLASS(CWeaponSkorpion, CMachineGun);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponSkorpion();

	int GetMinBurst()
	{
		return 1;
	}

	int GetMaxBurst()
	{
		return 3;
	}

	float GetFireRate()
	{
		//return 0.065f;
		return 0.0521f;
	}

	Activity GetPrimaryAttackActivity()
	{
		if (IsIronsightsEnabled())
			return ACT_VM_PRIMARYATTACK_IRONSIGHTS;

		return ACT_VM_PRIMARYATTACK;
	}

	void AddViewKick();

	bool Reload();

private:
	CWeaponSkorpion(const CWeaponSkorpion &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponSkorpion, DT_WeaponSkorpion)

BEGIN_NETWORK_TABLE(CWeaponSkorpion, DT_WeaponSkorpion)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponSkorpion)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_skorpion, CWeaponSkorpion);
PRECACHE_WEAPON_REGISTER(weapon_skorpion);

CWeaponSkorpion::CWeaponSkorpion()
{}

void CWeaponSkorpion::AddViewKick()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return;

	float flPitch = random->RandomFloat(0.35f, 0.65f);
	float flYaw = random->RandomFloat(-0.25f, 0.25f);

	pPlayer->ViewPunch(QAngle(-flPitch, flYaw, 0));
}

bool CWeaponSkorpion::Reload()
{
	return DefaultReload(GetMaxClip1(), GetMaxClip2(),
		m_iClip1 == 0 ? ACT_VM_RELOAD_EMPTY : ACT_VM_RELOAD);;
}
