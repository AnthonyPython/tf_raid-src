#include "cbase.h"
#include "npc_citizen17.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_DrillSergeant
//=========================================================
class CNPC_DrillSergeant : public CNPC_Citizen
{
	DECLARE_CLASS( CNPC_DrillSergeant, CNPC_Citizen );

public:
	virtual void SelectModel( );

	virtual Class_T Classify( );
};

LINK_ENTITY_TO_CLASS( npc_drill_sergeant, CNPC_DrillSergeant );

void CNPC_DrillSergeant::SelectModel( )
{
	if( !CBaseEntity::GetModelName())
		SetModelName( MAKE_STRING( "models/characters/drill_sergeant.mdl" ) );
}

Class_T CNPC_DrillSergeant::Classify( )
{
	return CLASS_PLAYER_ALLY_VITAL;
}
