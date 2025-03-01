#include "cbase.h"
#include "npc_citizen17.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_Roosevelt
//=========================================================
class CNPC_Roosevelt : public CNPC_Citizen
{
	DECLARE_CLASS( CNPC_Roosevelt, CNPC_Citizen );

public:
	virtual void SelectModel( );

	virtual Class_T Classify( );
};

LINK_ENTITY_TO_CLASS( npc_roosevelt, CNPC_Roosevelt );

void CNPC_Roosevelt::SelectModel( )
{
	if( !CBaseEntity::GetModelName())
		SetModelName( MAKE_STRING( "models/characters/capt_roosevelt.mdl" ) );
}

Class_T CNPC_Roosevelt::Classify( )
{
	return CLASS_PLAYER_ALLY_VITAL;
}
