#include "cbase.h"
#include "npc_citizen17.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_Crew
//=========================================================
class CNPC_Crew : public CNPC_Citizen
{
	DECLARE_CLASS( CNPC_Crew, CNPC_Citizen );

public:
	virtual void SelectModel( );

	virtual Class_T Classify( );
};

LINK_ENTITY_TO_CLASS( npc_crew, CNPC_Crew );

void CNPC_Crew::SelectModel( )
{
	if( !CBaseEntity::GetModelName())
		SetModelName( MAKE_STRING( "models/humans/citizen_crew.mdl" ) );
}

Class_T CNPC_Crew::Classify( )
{
	return CLASS_CREW;
}
