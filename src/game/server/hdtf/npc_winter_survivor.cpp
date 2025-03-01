#include "cbase.h"
#include "npc_barney.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_WinterSurvivor
//=========================================================
class CNPC_WinterSurvivor : public CNPC_Barney
{
	DECLARE_CLASS( CNPC_WinterSurvivor, CNPC_Barney );

public:
	virtual void SelectModel( );

	virtual Class_T Classify( );
};

LINK_ENTITY_TO_CLASS( npc_winter_survivor, CNPC_WinterSurvivor );

void CNPC_WinterSurvivor::SelectModel( )
{
	// Allow multiple models (for slaves), but default to vortigaunt.mdl
	char* szModel = (char*)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		szModel = "models/humans/citizen_wintersurvivor.mdl";
		SetModelName(AllocPooledString(szModel));
	}
	
}

Class_T CNPC_WinterSurvivor::Classify( )
{
	return CLASS_PLAYER_ALLY;
}
