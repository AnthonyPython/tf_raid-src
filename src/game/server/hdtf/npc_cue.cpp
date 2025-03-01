#include "cbase.h"
#include "npc_barney.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define AE_CUE_RELOAD 2

//=========================================================
//	>> CNPC_Cue
//=========================================================
class CNPC_Cue : public CNPC_Barney
{
	DECLARE_CLASS( CNPC_Cue, CNPC_Barney );

public:
	virtual void SelectModel( );

	virtual Class_T Classify( );

	virtual void HandleAnimEvent(animevent_t *pEvent);
};

LINK_ENTITY_TO_CLASS( npc_cue, CNPC_Cue );

void CNPC_Cue::SelectModel( )
{
	if (!CBaseEntity::GetModelName())
		SetModelName(MAKE_STRING("models/characters/cue.mdl"));
	
}

Class_T CNPC_Cue::Classify( )
{
	return CLASS_PLAYER_ALLY_VITAL;
}

void CNPC_Cue::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case AE_CUE_RELOAD:
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
