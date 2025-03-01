#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"
#include "eventlist.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------
// Applies ammo quantity scale.
//---------------------------------------------------------
int ITEM_GiveAmmo(CBasePlayer *pPlayer, float flCount, const char *pszAmmoName, bool bSuppressSound = false)
{
	int iAmmoType = GetAmmoDef()->Index(pszAmmoName);
	if (iAmmoType == -1)
	{
		Msg("ERROR: Attempting to give unknown ammo type (%s)\n", pszAmmoName);
		return 0;
	}

	flCount *= g_pGameRules->GetAmmoQuantityScale(iAmmoType);

	// Don't give out less than 1 of anything.
	flCount = MAX(1.0f, flCount);

	return pPlayer->GiveAmmo(flCount, iAmmoType, bSuppressSound);
}

//---------------------------------------------------------
// 45CAL 1911
//---------------------------------------------------------
class CItem_45CAL_1911 : public CItem
{
public:
	DECLARE_CLASS(CItem_45CAL_1911, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_1911.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_1911.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_45CAL_1911, "45cal"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_45cal_1911, CItem_45CAL_1911);

//---------------------------------------------------------
// 7.62 AK
//---------------------------------------------------------
class CItem_762_AK : public CItem
{
public:
	DECLARE_CLASS(CItem_762_AK, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_ak47.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_ak47.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_762_AK, "7_62mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_762_ak, CItem_762_AK);

//---------------------------------------------------------
// 7.62 SIG 552 XI
//---------------------------------------------------------
class CItem_762_SIGXI : public CItem
{
public:
	DECLARE_CLASS(CItem_762_SIGXI, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_sigxi.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_sigxi.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_762_SIGXI, "7_62mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_762_sigxi, CItem_762_AK);

//---------------------------------------------------------
// 5.56 M16A2
//---------------------------------------------------------
class CItem_556_M16 : public CItem
{
public:
	DECLARE_CLASS(CItem_556_M16, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_m16.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_m16.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_556_M16, "5_56mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_556_m16, CItem_556_M16);

//---------------------------------------------------------
// 7.92 KAR98
//---------------------------------------------------------
class CItem_792_KAR98 : public CItem
{
public:
	DECLARE_CLASS(CItem_792_KAR98, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_k98.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_k98.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_792_KAR98, "7_92mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_792_kar98, CItem_792_KAR98);

//---------------------------------------------------------
// 9MM M9
//---------------------------------------------------------
class CItem_9MM_M9 : public CItem
{
public:
	DECLARE_CLASS(CItem_9MM_M9, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_m9.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_m9.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_9MM_M9, "9mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_9mm_m9, CItem_9MM_M9);

//---------------------------------------------------------
// 9MM HL2 PISTOL
//---------------------------------------------------------
class CItem_9MM_HL2P : public CItem
{
public:
	DECLARE_CLASS(CItem_9MM_HL2P, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_hl2pistol.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_hl2pistol.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_9MM_HL2P, "9mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_9mm_hl2pistol, CItem_9MM_HL2P);

//---------------------------------------------------------
// 9MM SKORPION
//---------------------------------------------------------
class CItem_9MM_SKORPION : public CItem
{
public:
	DECLARE_CLASS(CItem_9MM_SKORPION, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_skorpion.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_skorpion.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_9MM_SKORPION, "9mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_9mm_skorpion, CItem_9MM_SKORPION);

//---------------------------------------------------------
// 9MM MP5
//---------------------------------------------------------
class CItem_9MM_MP5 : public CItem
{
public:
	DECLARE_CLASS(CItem_9MM_MP5, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_mp5.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_mp5.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_9MM_MP5, "9mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_9mm_mp5, CItem_9MM_MP5);

//---------------------------------------------------------
// 9MM TMP
//---------------------------------------------------------
class CItem_9MM_TMP : public CItem
{
public:
	DECLARE_CLASS(CItem_9MM_TMP, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_tmp.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_tmp.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_9MM_TMP, "9mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_9mm_tmp, CItem_9MM_TMP);

//---------------------------------------------------------
// Buckshot
//---------------------------------------------------------
class CItem_AmmoBuckshot : public CItem
{
public:
	DECLARE_CLASS(CItem_AmmoBuckshot, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_buckshot.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_buckshot.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_BUCKSHOT_HDTF, "Buckshot"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_buckshot, CItem_AmmoBuckshot);

//---------------------------------------------------------
// 44CAL MAGNUM
//---------------------------------------------------------
class CItem_44CAL_MAGNUM : public CItem
{
public:
	DECLARE_CLASS(CItem_44CAL_MAGNUM, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_magnum.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_magnum.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_44CAL_MAGNUM, "44cal"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_44cal_magnum, CItem_44CAL_MAGNUM);

//---------------------------------------------------------
// AR2 PULSE AMMO
//---------------------------------------------------------
class CItem_AmmoAR2 : public CItem
{
public:
	DECLARE_CLASS(CItem_AmmoAR2, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_ar2.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_ar2.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_AR2_HDTF, "AR2"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_ammo_ar2, CItem_AmmoAR2);

//---------------------------------------------------------
// AR2 ENERGY BALL ROUND
//---------------------------------------------------------
class CItem_AmmoAR2AltFireRound : public CItem
{
public:
	DECLARE_CLASS(CItem_AmmoAR2AltFireRound, CItem);

	void Precache(void)
	{
		PrecacheParticleSystem("combineball");
		PrecacheModel("models/weapons/ammo_ar2_alt.mdl");
	}

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_ar2_alt.mdl");
		BaseClass::Spawn();
	}

	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_AR2_ALT, "AR2AltFire"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_ar2_altfire, CItem_AmmoAR2AltFireRound);

//---------------------------------------------------------
// M203 AMMO
//---------------------------------------------------------
class CItem_AmmoM203 : public CItem
{
public:
	DECLARE_CLASS(CItem_AmmoM203, CItem);

	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_m203.mdl");
	}

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_m203.mdl");
		BaseClass::Spawn();
	}

	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_M203, "SMG1_Grenade"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_m203, CItem_AmmoM203);

//---------------------------------------------------------
// 9MM BOX
//---------------------------------------------------------
class CItem_9MM_BOX : public CItem
{
public:
	DECLARE_CLASS(CItem_9MM_BOX, CItem);

	void Spawn(void)
	{
		Precache();
		//SetModel("models/items/ammoboxes/ammobox_9mm.mdl");
		SetModel("models/weapons/hdtf_ammobox.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		//PrecacheModel("models/items/ammoboxes/ammobox_9mm.mdl");
		PrecacheModel("models/weapons/hdtf_ammobox.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 150, "9mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_9mm_box, CItem_9MM_BOX);

//---------------------------------------------------------
// .44 BOX
//---------------------------------------------------------
class CItem_44cal_BOX : public CItem
{
public:
	DECLARE_CLASS(CItem_44cal_BOX, CItem);

	void Spawn(void)
	{
		Precache();
		//SetModel("models/items/ammoboxes/ammobox_44cal.mdl");
		SetModel("models/weapons/hdtf_ammobox.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		//PrecacheModel("models/items/ammoboxes/ammobox_44cal.mdl");
		PrecacheModel("models/weapons/hdtf_ammobox.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 150, "44cal"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_44cal_box, CItem_44cal_BOX);

//---------------------------------------------------------
// .45 BOX
//---------------------------------------------------------
class CItem_45cal_BOX : public CItem
{
public:
	DECLARE_CLASS(CItem_45cal_BOX, CItem);

	void Spawn(void)
	{
		Precache();
		//SetModel("models/items/ammoboxes/ammobox_45cal.mdl");
		SetModel("models/weapons/hdtf_ammobox.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		//PrecacheModel("models/items/ammoboxes/ammobox_45cal.mdl");
		PrecacheModel("models/weapons/hdtf_ammobox.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 150, "45cal"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_45cal_box, CItem_45cal_BOX);


//---------------------------------------------------------
// 4.6mm BOX
//---------------------------------------------------------
class CItem_46mm_BOX : public CItem
{
public:
	DECLARE_CLASS(CItem_46mm_BOX, CItem);

	void Spawn(void)
	{
		Precache();
		//SetModel("models/items/ammoboxes/ammobox_46mm.mdl");
		SetModel("models/weapons/hdtf_ammobox.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		//PrecacheModel("models/items/ammoboxes/ammobox_46mm.mdl");
		PrecacheModel("models/weapons/hdtf_ammobox.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 150, "SMG1"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_46mm_box, CItem_46mm_BOX);

//---------------------------------------------------------
// 556 BOX
//---------------------------------------------------------
class CItem_556mm_BOX : public CItem
{
public:
	DECLARE_CLASS(CItem_556mm_BOX, CItem);

	void Spawn(void)
	{
		Precache();
		//SetModel("models/items/ammoboxes/ammobox_556.mdl");
		SetModel("models/weapons/hdtf_ammobox.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		//PrecacheModel("models/items/ammoboxes/ammobox_556.mdl");
		PrecacheModel("models/weapons/hdtf_ammobox.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 150, "5_56mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_556mm_box, CItem_556mm_BOX);

//---------------------------------------------------------
// .45 BOX
//---------------------------------------------------------
class CItem_762mm_BOX : public CItem
{
public:
	DECLARE_CLASS(CItem_762mm_BOX, CItem);

	void Spawn(void)
	{
		Precache();
		//SetModel("models/items/ammoboxes/ammobox_762.mdl");
		SetModel("models/weapons/hdtf_ammobox.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		//PrecacheModel("models/items/ammoboxes/ammobox_762.mdl");
		PrecacheModel("models/weapons/hdtf_ammobox.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 150, "7_62mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_762mm_box, CItem_762mm_BOX);

//---------------------------------------------------------
// 792 BOX
//---------------------------------------------------------
class CItem_792mm_BOX : public CItem
{
public:
	DECLARE_CLASS(CItem_792mm_BOX, CItem);

	void Spawn(void)
	{
		Precache();
		//SetModel("models/items/ammoboxes/ammobox_792.mdl");
		SetModel("models/weapons/hdtf_ammobox.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		//PrecacheModel("models/items/ammoboxes/ammobox_792.mdl");
		PrecacheModel("models/weapons/hdtf_ammobox.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 150, "7_92mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_792mm_box, CItem_792mm_BOX);

//---------------------------------------------------------
// 12GA BOX
//---------------------------------------------------------
class CItem_12GA_BOX : public CItem
{
public:
	DECLARE_CLASS(CItem_12GA_BOX, CItem);

	void Spawn(void)
	{
		Precache();
		//SetModel("models/items/ammoboxes/ammobox_buckshot.mdl");
		SetModel("models/weapons/hdtf_ammobox.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		//PrecacheModel("models/items/ammoboxes/ammobox_buckshot.mdl");
		PrecacheModel("models/weapons/hdtf_ammobox.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 50, "Buckshot"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_12GA_box, CItem_12GA_BOX);

//---------------------------------------------------------
// AWP BOX
//---------------------------------------------------------
class CItem_AWP_BOX : public CItem
{
public:
	DECLARE_CLASS(CItem_AWP_BOX, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/ammo_awp.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/ammo_awp.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 6, "7_62mm"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_762_awp, CItem_AWP_BOX);

//---------------------------------------------------------
// 40mm BOX
//---------------------------------------------------------
class CItem_40mm_BOX : public CItem
{
public:
	DECLARE_CLASS(CItem_40mm_BOX, CItem);

	void Spawn(void)
	{
		Precache();
		//SetModel("models/items/ammoboxes/ammobox_40mm.mdl");
		SetModel("models/weapons/hdtf_ammobox.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		//PrecacheModel("models/items/ammoboxes/ammobox_40mm.mdl");
		PrecacheModel("models/weapons/hdtf_ammobox.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 22, "SMG1_Grenade"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_40mm_box, CItem_40mm_BOX);

//---------------------------------------------------------
// 40mm BOX
//---------------------------------------------------------
class CItem_AR2_BOX : public CItem
{
public:
	DECLARE_CLASS(CItem_AR2_BOX, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/weapons/hdtf_ammobox.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/weapons/hdtf_ammobox.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 150, "AR2"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_ammo_AR2_box, CItem_AR2_BOX);