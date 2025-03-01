#include "cbase.h"
#include "npc_combines.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_HECU
//=========================================================
class CNPC_HECU : public CNPC_CombineS
{
	DECLARE_CLASS( CNPC_HECU, CNPC_CombineS );

public:
	virtual void Precache( );

	virtual Class_T Classify( );

	void DeathSound( const CTakeDamageInfo & ) { }
	void PainSound( const CTakeDamageInfo & ) { }
	void IdleSound( ) { }
	void AlertSound( ) { }
	void LostEnemySound( ) { }
	void FoundEnemySound( ) { }
	void AnnounceAssault( ) { }
	void AnnounceEnemyType( CBaseEntity * ) { }
	void AnnounceEnemyKill( CBaseEntity * ) { }
	void NotifyDeadFriend( CBaseEntity * ) { }
};

LINK_ENTITY_TO_CLASS( npc_hecu, CNPC_HECU );

void CNPC_HECU::Precache( )
{
	if( !GetModelName( ) )
		SetModelName( MAKE_STRING( "models/HDTF/cinematic/marine_with_gasmask.mdl" ) );

	BaseClass::Precache( );
}

Class_T CNPC_HECU::Classify( )
{
	return CLASS_SOLDIER;
}
