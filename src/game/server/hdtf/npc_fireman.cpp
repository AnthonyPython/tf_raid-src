#include "cbase.h"
#include "npc_barney.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_Fireman
//=========================================================
class CNPC_Fireman : public CNPC_Barney
{
	DECLARE_CLASS( CNPC_Fireman, CNPC_Barney );

public:
	virtual void SelectModel( );

	virtual Class_T Classify( );
};

LINK_ENTITY_TO_CLASS( npc_fireman, CNPC_Fireman );

void CNPC_Fireman::SelectModel( )
{
	if (!GetModelName())
		SetModelName(MAKE_STRING("models/humans/citizen_fireman.mdl"));
	
}

Class_T CNPC_Fireman::Classify( )
{
	return CLASS_PLAYER_ALLY;
}
