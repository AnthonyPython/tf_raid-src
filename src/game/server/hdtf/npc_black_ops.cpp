#include "cbase.h"
#include "npc_combines.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_BlackOps
//=========================================================
class CNPC_BlackOps : public CNPC_CombineS
{
	DECLARE_CLASS( CNPC_BlackOps, CNPC_CombineS );

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

LINK_ENTITY_TO_CLASS( npc_black_ops, CNPC_BlackOps );

void CNPC_BlackOps::Precache( )
{
	if( !GetModelName( ) )
		SetModelName( MAKE_STRING( "models/hdtf/characters/black_ops/black_ops1.mdl" ) );

	BaseClass::Precache( );
}

Class_T CNPC_BlackOps::Classify( )
{
	return CLASS_BLACK_OPS;
}
