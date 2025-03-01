#include "cbase.h"
#include "npc_citizen17.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_Ranger
//=========================================================
class CNPC_Ranger : public CNPC_Citizen
{
	DECLARE_CLASS( CNPC_Ranger, CNPC_Citizen );

public:
	virtual void SelectModel( );

	virtual Class_T Classify( );
};

LINK_ENTITY_TO_CLASS( npc_ranger, CNPC_Ranger );

void CNPC_Ranger::SelectModel( )
{
	if( !CBaseEntity::GetModelName())
		SetModelName( MAKE_STRING( "models/characters/ranger.mdl" ) );
}

Class_T CNPC_Ranger::Classify( )
{
	return CLASS_PLAYER_ALLY;
}
