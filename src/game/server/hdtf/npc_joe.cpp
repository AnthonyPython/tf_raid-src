#include "cbase.h"
#include "npc_barney.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define AE_JOE_RELOAD 2

//=========================================================
//	>> CNPC_Joe
//=========================================================
class CNPC_Joe : public CNPC_Barney
{
	DECLARE_CLASS( CNPC_Joe, CNPC_Barney );

public:
	virtual void SelectModel( );

	virtual void HandleAnimEvent(animevent_t *pEvent);

	virtual Class_T Classify( );
};

LINK_ENTITY_TO_CLASS( npc_joe, CNPC_Joe );

void CNPC_Joe::SelectModel( )
{
	if (!GetModelName())
		SetModelName(MAKE_STRING("models/Protagonists/joe.mdl"));
}
	

Class_T CNPC_Joe::Classify( )
{
	return CLASS_PLAYER_ALLY_VITAL;
}

void CNPC_Joe::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case AE_JOE_RELOAD:
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
