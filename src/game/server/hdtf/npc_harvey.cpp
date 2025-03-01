#include "cbase.h"
#include "npc_citizen17.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_Harvey
//=========================================================
class CNPC_Harvey : public CNPC_Citizen
{
	DECLARE_CLASS( CNPC_Harvey, CNPC_Citizen );

public:
	virtual void SelectModel( );

	virtual Class_T Classify( );
};

LINK_ENTITY_TO_CLASS( npc_harvey, CNPC_Harvey );

void CNPC_Harvey::SelectModel( )
{
	// Allow multiple models , but default to harvey.mdl
	char* szModel = (char*)STRING(CBaseEntity::GetModelName());
	if (!szModel || !*szModel)
	{
		szModel = "models/characters/harvey.mdl";
		SetModelName(AllocPooledString(szModel));
	}
	
}

Class_T CNPC_Harvey::Classify( )
{
	return CLASS_PLAYER_ALLY_VITAL;
}
