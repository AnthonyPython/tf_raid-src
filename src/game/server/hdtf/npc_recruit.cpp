#include "cbase.h"
#include "npc_citizen17.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_Recruit
//=========================================================
class CNPC_Recruit : public CNPC_Citizen
{
	DECLARE_CLASS( CNPC_Recruit, CNPC_Citizen );

public:
	virtual void SelectModel( );

	virtual Class_T Classify( );
};

LINK_ENTITY_TO_CLASS( npc_recruit, CNPC_Recruit );

void CNPC_Recruit::SelectModel( )
{
	if( !CBaseEntity::GetModelName())
		SetModelName( MAKE_STRING( "models/characters/soldier_jungle.mdl" ) );
}

Class_T CNPC_Recruit::Classify( )
{
	return CLASS_PLAYER_ALLY;
}
