#include "cbase.h"
#include "npc_barney.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define AE_NICK_RELOAD 2

//=========================================================
//	>> CNPC_Nick
//=========================================================
class CNPC_Nick : public CNPC_Barney
{
	DECLARE_CLASS( CNPC_Nick, CNPC_Barney );

	Class_T m_Class;

public:
	virtual void PostConstructor( const char *szClassname );

	virtual void SelectModel( );

	virtual void HandleAnimEvent(animevent_t *pEvent);

	virtual Class_T Classify( );

	Activity NPC_TranslateActivity(Activity activity);
};

LINK_ENTITY_TO_CLASS( npc_nick_young, CNPC_Nick );
LINK_ENTITY_TO_CLASS( npc_nick_old, CNPC_Nick );

void CNPC_Nick::PostConstructor( const char *szClassname )
{
	BaseClass::PostConstructor( szClassname );

	m_Class =
		FClassnameIs( this, "npc_nick_young" ) ? CLASS_PLAYER_ALLY_VITAL : CLASS_PLAYER_COMBINE_ALLY_VITAL;
}

void CNPC_Nick::SelectModel( )
{
	// Allow multiple models, but default to young_nick.mdl or old_nick.mdl
	char* szModel = (char*)STRING(GetModelName());

	if (FClassnameIs(this, "npc_nick_young"))
	{
		if (!szModel || !*szModel)
		{
			szModel = "models/Protagonists/young_nick.mdl";
			SetModelName(AllocPooledString(szModel));
		}
	}
	else
	{
		if (!szModel || !*szModel)
		{
			szModel = "models/Protagonists/old_nick.mdl";
			SetModelName(AllocPooledString(szModel));
		}
	}
}

Class_T CNPC_Nick::Classify( )
{
	return m_Class;
}

void CNPC_Nick::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case AE_NICK_RELOAD:
		// We never actually run out of ammo, just need to refill the clip
		if (GetActiveWeapon())
		{
			GetActiveWeapon()->WeaponSound(RELOAD_NPC);
			GetActiveWeapon()->m_iClip1 = GetActiveWeapon()->GetMaxClip1();
			GetActiveWeapon()->m_iClip2 = GetActiveWeapon()->GetMaxClip2();
		}
		ClearCondition(COND_LOW_PRIMARY_AMMO);
		ClearCondition(COND_NO_PRIMARY_AMMO);
		ClearCondition(COND_NO_SECONDARY_AMMO);
		break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

Activity CNPC_Nick::NPC_TranslateActivity(Activity activity)
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();
	if (pWeapon && (FClassnameIs(pWeapon, "weapon_m16") || FClassnameIs(pWeapon, "weapon_M16_Carbine")))
	{
		// NOTE(wheatley): Nick has tonns of clipping body parts when using m16 so translate
		// a few animations requested by m16 into more appropriate for his model
		switch (activity)
		{
		case ACT_IDLE_ANGRY_SMG1:
			return ACT_IDLE_ANGRY_AR2;

		case ACT_IDLE_SMG1_RELAXED:
			return ACT_IDLE_AR2_RELAXED;

		case ACT_IDLE_SMG1:
		case ACT_IDLE_SMG1_STIMULATED:
			return ACT_IDLE_AR2;

		case ACT_WALK_RIFLE_RELAXED:
			return ACT_WALK_AR2_RELAXED;

		case ACT_WALK_RIFLE_STIMULATED:
			return ACT_WALK_AR2_STIMULATED;

		case ACT_WALK_AIM_RIFLE:
			return ACT_WALK_AIM_AR2;

		case ACT_RUN_RIFLE_RELAXED:
			return ACT_RUN_AR2_RELAXED;

		case ACT_RUN_RIFLE_STIMULATED:
			return ACT_RUN_AR2_STIMULATED;

		case ACT_RUN_AIM_RIFLE:
			return ACT_RUN_AIM_AR2_STIMULATED;
		}
	}

	return BaseClass::NPC_TranslateActivity(activity);
}

//Activity CNPC_Nick::NPC_TranslateActivity(Activity activity)
//{
//	CBaseCombatWeapon* pWeapon = GetActiveWeapon();
//	if (pWeapon && FClassnameIs(pWeapon, "weapon_M16_Carbine"))
//	{
//		// NOTE(wheatley): Nick has tonns of clipping body parts when using m16 so translate
//		// a few animations requested by m16 into more appropriate for his model
//		switch (activity)
//		{
//		case ACT_IDLE_ANGRY_SMG1:
//			return ACT_IDLE_ANGRY_AR2;
//
//		case ACT_IDLE_SMG1_RELAXED:
//			return ACT_IDLE_AR2_RELAXED;
//
//		case ACT_IDLE_SMG1:
//		case ACT_IDLE_SMG1_STIMULATED:
//			return ACT_IDLE_AR2;
//
//		case ACT_WALK_RIFLE_RELAXED:
//			return ACT_WALK_AR2_RELAXED;
//
//		case ACT_WALK_RIFLE_STIMULATED:
//			return ACT_WALK_AR2_STIMULATED;
//
//		case ACT_WALK_AIM_RIFLE:
//			return ACT_WALK_AIM_AR2;
//
//		case ACT_RUN_RIFLE_RELAXED:
//			return ACT_RUN_AR2_RELAXED;
//
//		case ACT_RUN_RIFLE_STIMULATED:
//			return ACT_RUN_AR2_STIMULATED;
//
//		case ACT_RUN_AIM_RIFLE:
//			return ACT_RUN_AIM_AR2_STIMULATED;
//		}
//	}
//
//	return BaseClass::NPC_TranslateActivity(activity);
//}