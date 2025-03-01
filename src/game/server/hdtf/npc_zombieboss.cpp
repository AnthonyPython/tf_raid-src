#include "cbase.h"
#include "hdtf_boss.h"
#include "npc_fastzombie.h"

class CNPC_ZombieBoss : public CHDTF_Boss<CFastZombie>
{
	DECLARE_CLASS( CNPC_ZombieBoss, CHDTF_Boss<CFastZombie> );
	DECLARE_SERVERCLASS( );
	DECLARE_DATADESC( );
};

LINK_ENTITY_TO_CLASS( npc_zombieboss, CNPC_ZombieBoss );
LINK_ENTITY_TO_CLASS( npc_zombieboss_torso, CNPC_ZombieBoss );

IMPLEMENT_SERVERCLASS_ST( CNPC_ZombieBoss, DT_NPC_ZombieBoss )
END_SEND_TABLE( )

BEGIN_DATADESC( CNPC_ZombieBoss )
END_DATADESC( )

#ifdef DEBUG

CON_COMMAND( zombieboss_sethealth, "BLAH" )
{
	CNPC_ZombieBoss *pBoss = static_cast<CNPC_ZombieBoss *>( gEntList.FindEntityByClassname( NULL, "npc_zombieboss" ) );
	if( pBoss == NULL || args.ArgC( ) < 2 )
		return;

	int nHealth = V_atoi( args[1] );
	pBoss->SetHealth( nHealth );

	CRecipientFilter filter;
	filter.AddAllPlayers( );

	if( nHealth > 0 )
	{
		UserMessageBegin( filter, "BossHealthUpdate" );
		WRITE_LONG( pBoss->GetHealth( ) );
		MessageEnd( );
	}
	else
	{
		UserMessageBegin( filter, "BossHealthFinish" );
		MessageEnd( );
	}
}

#endif
