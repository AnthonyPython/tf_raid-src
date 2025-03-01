#include "cbase.h"
#include "npc_metropolice.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_GordonFreeman
//=========================================================
class CNPC_GordonFreeman : public CNPC_MetroPolice
{
	DECLARE_CLASS( CNPC_GordonFreeman, CNPC_MetroPolice );

public:
	virtual void Precache( );

	virtual Class_T Classify( );
};

LINK_ENTITY_TO_CLASS( npc_gordon_freeman, CNPC_GordonFreeman );

void CNPC_GordonFreeman::Precache( )
{
	if (!CBaseEntity::GetModelName())
		SetModelName(MAKE_STRING("models/player/lenoax_gordon.mdl"));
	

	BaseClass::Precache( );
}

Class_T CNPC_GordonFreeman::Classify( )
{
	return CLASS_CITIZEN_REBEL;
}
