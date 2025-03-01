#include "cbase.h"
#include "c_ai_basenpc.h"
#include "hdtf_boss.h"

class C_NPC_ZombieBoss : public C_HDTF_Boss<C_AI_BaseNPC>
{
	DECLARE_CLASS( C_NPC_ZombieBoss, C_HDTF_Boss<C_AI_BaseNPC> );
	DECLARE_CLIENTCLASS( );

public:
	void Spawn( );
};

IMPLEMENT_CLIENTCLASS_DT( C_NPC_ZombieBoss, DT_NPC_ZombieBoss, CNPC_ZombieBoss )
END_RECV_TABLE( )

void C_NPC_ZombieBoss::Spawn( )
{
	SetMaxHealth( 100 );
	SetHealth( 100 );

	BaseClass::Spawn( );
}
