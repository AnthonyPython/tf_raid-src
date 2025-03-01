#include "cbase.h"
#include "hdtf_gamerules.h"
#include "hdtf_player_shared.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

REGISTER_GAMERULES_CLASS( CHDTF );

BEGIN_NETWORK_TABLE_NOBASE( CHDTF, DT_HDTFGameRules )

#ifdef CLIENT_DLL

	RecvPropBool( RECVINFO( m_bMegaPhysgun ) ),

#else

	SendPropBool( SENDINFO( m_bMegaPhysgun ) ),

#endif

END_NETWORK_TABLE( )

LINK_ENTITY_TO_CLASS( hdtf_gamerules, CHDTFProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( HDTFProxy, DT_HDTFProxy )

#ifdef CLIENT_DLL

void RecvProxy_HDTFGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
{
	CHDTF *pRules = HDTFGameRules( );
	Assert( pRules );
	*pOut = pRules;
}

BEGIN_RECV_TABLE( CHDTFProxy, DT_HDTFProxy )
RecvPropDataTable( "hdtf_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_HDTFGameRules ), RecvProxy_HDTFGameRules )
END_RECV_TABLE( )

#else

void *SendProxy_HDTFGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
{
	CHDTF *pRules = HDTFGameRules( );
	Assert( pRules );
	pRecipients->SetAllRecipients( );
	return pRules;
}

BEGIN_SEND_TABLE( CHDTFProxy, DT_HDTFProxy )
SendPropDataTable( "hdtf_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_HDTFGameRules ), SendProxy_HDTFGameRules )
END_SEND_TABLE( )

#endif

ConVar sk_plr_dmg_9mm( "sk_plr_dmg_9mm", "0", FCVAR_REPLICATED );
ConVar sk_npc_dmg_9mm( "sk_npc_dmg_9mm", "0", FCVAR_REPLICATED );
ConVar sk_max_9mm( "sk_max_9mm", "0", FCVAR_REPLICATED );

ConVar sk_plr_dmg_44cal( "sk_plr_dmg_44cal", "0", FCVAR_REPLICATED );
ConVar sk_npc_dmg_44cal( "sk_npc_dmg_44cal", "0", FCVAR_REPLICATED );
ConVar sk_max_44cal( "sk_max_44cal", "0", FCVAR_REPLICATED );

ConVar sk_plr_dmg_45cal( "sk_plr_dmg_45cal", "0", FCVAR_REPLICATED );
ConVar sk_npc_dmg_45cal( "sk_npc_dmg_45cal", "0", FCVAR_REPLICATED );
ConVar sk_max_45cal( "sk_max_45cal", "0", FCVAR_REPLICATED );

ConVar sk_plr_dmg_556mm( "sk_plr_dmg_556mm", "0", FCVAR_REPLICATED );
ConVar sk_npc_dmg_556mm( "sk_npc_dmg_556mm", "0", FCVAR_REPLICATED );
ConVar sk_max_556mm( "sk_max_556mm", "0", FCVAR_REPLICATED );

ConVar sk_plr_dmg_762mm( "sk_plr_dmg_762mm", "0", FCVAR_REPLICATED );
ConVar sk_npc_dmg_762mm( "sk_npc_dmg_762mm", "0", FCVAR_REPLICATED );
ConVar sk_max_762mm( "sk_max_762mm", "0", FCVAR_REPLICATED );

ConVar sk_plr_dmg_molotov( "sk_plr_dmg_molotov", "0", FCVAR_REPLICATED );
ConVar sk_npc_dmg_molotov( "sk_npc_dmg_molotov", "0", FCVAR_REPLICATED );
ConVar sk_max_molotov( "sk_max_molotov", "0", FCVAR_REPLICATED );

ConVar sk_max_claymore("sk_max_claymore", "0", FCVAR_REPLICATED);
ConVar sk_max_flare( "sk_max_flare", "0", FCVAR_REPLICATED );
ConVar sk_max_painkillers("sk_max_painkillers", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_50cal("sk_plr_dmg_50cal", "0", FCVAR_REPLICATED);
ConVar sk_npc_dmg_50cal("sk_npc_dmg_50cal", "0", FCVAR_REPLICATED);
ConVar sk_max_50cal("sk_max_50cal", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_792mm( "sk_plr_dmg_792mm", "0", FCVAR_REPLICATED );
ConVar sk_npc_dmg_792mm( "sk_npc_dmg_792mm", "0", FCVAR_REPLICATED );
ConVar sk_max_792mm( "sk_max_792mm", "0", FCVAR_REPLICATED );

static CHDTFViewVectors g_HDTFViewVectors(
	Vector( 0, 0, 64 ),			//VEC_VIEW (m_vView)

	Vector( -16, -16, 0 ),		//VEC_HULL_MIN (m_vHullMin)
	Vector( 16, 16, 72 ),		//VEC_HULL_MAX (m_vHullMax)

	Vector( -16, -16, 0 ),		//VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 16, 16, 54 ),		//VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 46 ),			//VEC_DUCK_VIEW		(m_vDuckView)

	Vector( -10, -10, -10 ),	//VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10, 10, 10 ),		//VEC_OBS_HULL_MAX	(m_vObsHullMax)

	Vector( 0, 0, 14 ),			//VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector( -16, -16, 0 ),		//VEC_PRONE_HULL_MIN (m_vProneHullMin)
	Vector( 16, 16, 24 ),		//VEC_PRONE_HULL_MAX (m_vProneHullMax)
	Vector( 0, 0, 10 )			//VEC_PRONE_VIEW (m_vProneView)
);

const CHDTFViewVectors *CHDTF::GetHDTFViewVectors( ) const
{
	return &g_HDTFViewVectors;
}

bool CHDTF::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon )
{
	//Always take a loaded gun if we have nothing else
	if( pPlayer->GetActiveWeapon( ) == NULL )
		return true;

	// The given weapon must allow autoswitching to it from another weapon.
	if( !pWeapon->AllowsAutoSwitchTo( ) )
		return false;

	// The active weapon must allow autoswitching from it.
	if( !pPlayer->GetActiveWeapon( )->AllowsAutoSwitchFrom( ) )
		return false;

	//Don't switch if our current gun doesn't want to be holstered
	if( !pPlayer->GetActiveWeapon( )->CanHolster( ) )
		return false;

	//Only switch if the weapon is better than what we're using
	if( pWeapon != pPlayer->GetActiveWeapon( ) && pWeapon->GetWeight( ) < pPlayer->GetActiveWeapon( )->GetWeight( ) )
		return false;

	return true;
}

extern ConVar sk_plr_parkour_roll_threshold;

float CHDTF::FlPlayerFallDamage( CBasePlayer *pPlayer )
{
	CHDTF_Player *ply = dynamic_cast<CHDTF_Player *>(pPlayer);

	if (ply && ply->m_pParkourController.TriggerRoll())
	{
		pPlayer->m_Local.m_flFallVelocity -= sk_plr_parkour_roll_threshold.GetFloat() * 1.25f;
		return pPlayer->m_Local.m_flFallVelocity * DAMAGE_FOR_FALL_SPEED * 1.25f;
	}

	pPlayer->m_Local.m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
	return pPlayer->m_Local.m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
}

#ifndef CLIENT_DLL

void CHDTF::InitDefaultAIRelationships( )
{
	//  Allocate memory for default relationships
	CBaseCombatCharacter::AllocateDefaultRelationships( );

	// --------------------------------------------------------------
	// First initialize table so we can report missing relationships
	// --------------------------------------------------------------
	for( int i = 0; i < NUM_AI_CLASSES; ++i )
		for( int j = 0; j < NUM_AI_CLASSES; ++j )
			// By default all relationships are neutral of priority zero
			CBaseCombatCharacter::SetDefaultRelationship(
				static_cast<Class_T>( i ), static_cast<Class_T>( j ), D_NU, 0
			);

	// ------------------------------------------------------------
	//	> CLASS_ANTLION
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_BULLSQUID,			D_HT, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_COMBINE_HUNTER,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_HOUNDEYE,				D_HT, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_MANHACK,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_SCANNER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_STALKER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_PROTOSNIPER,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_ANTLION,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ANTLION,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );

	// ------------------------------------------------------------
	//	> CLASS_BARNACLE
	//
	//  In this case, the relationship D_HT indicates which characters
	//  the barnacle will try to eat.
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_BARNACLE,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_BULLSQUID,		    D_NU, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_HOUNDEYE,				D_HT, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_MANHACK,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_STALKER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_EARTH_FAUNA,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BARNACLE,			CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );

	// ------------------------------------------------------------
	//	> CLASS_BULLSEYE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_PLAYER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_ANTLION,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_BULLSQUID,			D_NU, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_CITIZEN_REBEL,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_COMBINE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_HEADCRAB,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship (CLASS_BULLSEYE,			CLASS_HOUNDEYE,			    D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_VORTIGAUNT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_ZOMBIE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_PLAYER_ALLY,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_PLAYER_ALLY_VITAL,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_HACKED_ROLLERMINE,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_BLACK_OPS,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BULLSEYE,			CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );
	
	// ------------------------------------------------------------
	//	> CLASS_BULLSQUID
	// ------------------------------------------------------------
		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_NONE,					D_NU, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER,				D_HT, 0);			
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_ANTLION,				D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BARNACLE,				D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BULLSEYE,				D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BULLSQUID,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CITIZEN_PASSIVE,		D_HT, 0);	
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CITIZEN_REBEL,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE,				D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE_GUNSHIP,		D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE_HUNTER,		D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CONSCRIPT,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_FLARE,				D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HEADCRAB,				D_HT, 1);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HOUNDEYE,				D_HT, 1);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MANHACK,				D_FR, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_METROPOLICE,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MILITARY,				D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MISSILE,				D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_SCANNER,				D_NU, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_STALKER,				D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_VORTIGAUNT,			D_HT, 0);		
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_ZOMBIE,				D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PROTOSNIPER,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_EARTH_FAUNA,			D_NU, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER_ALLY,			D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER_ALLY_VITAL,	D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HACKED_ROLLERMINE,    D_HT, 0);
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );	
	

	// ------------------------------------------------------------
	//	> CLASS_CITIZEN_PASSIVE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_BARNACLE,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_BULLSQUID,			D_FR, 0	);	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_CITIZEN_REBEL,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE_HUNTER,		D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_HEADCRAB,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_HOUNDEYE,			    D_FR, 0	);	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_MANHACK,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_MISSILE,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_VORTIGAUNT,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_ZOMBIE,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER_ALLY,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER_ALLY_VITAL,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_HACKED_ROLLERMINE,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_BLACK_OPS,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER_COMBINE_ALLY,	D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_FR, 0 );

	// ------------------------------------------------------------
	//	> CLASS_CITIZEN_REBEL
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_BARNACLE,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_BULLSQUID,			D_FR, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_CITIZEN_REBEL,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_COMBINE_HUNTER,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_HOUNDEYE,				D_HT, 0	);	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_MANHACK,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_MISSILE,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_SCANNER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_STALKER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_VORTIGAUNT,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_PLAYER_ALLY,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_PLAYER_ALLY_VITAL,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_HACKED_ROLLERMINE,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_BLACK_OPS,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CITIZEN_REBEL,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );

	// ------------------------------------------------------------
	//	> CLASS_COMBINE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_BARNACLE,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_BULLSQUID,			D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_COMBINE,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_COMBINE_GUNSHIP,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_COMBINE_HUNTER,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_COMBINE_GUNSHIP
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_BULLSQUID,			D_HT, 0	);	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_COMBINE,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_COMBINE_GUNSHIP,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_COMBINE_HUNTER,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_HEADCRAB,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_MISSILE,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_GUNSHIP,	CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_COMBINE_HUNTER
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_BULLSQUID,			D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_COMBINE,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_COMBINE_GUNSHIP,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_COMBINE_HUNTER,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_COMBINE_HUNTER,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_CONSCRIPT
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_PLAYER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_BARNACLE,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_BULLSQUID,			D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_CITIZEN_REBEL,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_COMBINE_HUNTER,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_MANHACK,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_SCANNER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_STALKER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_VORTIGAUNT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_PLAYER_ALLY,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_PLAYER_ALLY_VITAL,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_HACKED_ROLLERMINE,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CONSCRIPT,			CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );
	
	// ------------------------------------------------------------
	//	> CLASS_FLARE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_PLAYER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_ANTLION,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_BULLSQUID,			D_NU, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_CITIZEN_REBEL,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_COMBINE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_HEADCRAB,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_HOUNDEYE,				D_NU, 0	);	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_VORTIGAUNT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_ZOMBIE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_PLAYER_ALLY,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_PLAYER_ALLY_VITAL,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_HACKED_ROLLERMINE,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_BLACK_OPS,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_FLARE,			CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_HEADCRAB
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_BULLSQUID,			D_FR, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_HEADCRAB,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_HOUNDEYE,				D_NU, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_ZOMBIE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_HACKED_ROLLERMINE,	D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HEADCRAB,			CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );
	
	// ------------------------------------------------------------
	//	> CLASS_HOUNDEYE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_NONE,					D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_PLAYER,				D_HT, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_BULLSQUID,			D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_CITIZEN_PASSIVE,		D_HT, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_COMBINE_HUNTER,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_HEADCRAB,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_HOUNDEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_SCANNER,				D_NU, 0 );		
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_STALKER,				D_NU, 0 );		
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_VORTIGAUNT,			D_HT, 0 );		
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_ZOMBIE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HOUNDEYE,			CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );	
		
	// ------------------------------------------------------------
	//	> CLASS_MANHACK
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_BULLSQUID,			D_HT, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_COMBINE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_HEADCRAB,				D_HT,-1 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_HOUNDEYE,				D_HT,-1 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MANHACK,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_METROPOLICE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_BULLSQUID,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_COMBINE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_HOUNDEYE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_METROPOLICE,	CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_MILITARY
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_BULLSQUID,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_COMBINE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MILITARY,			CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_MISSILE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_BULLSQUID,			D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_COMBINE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_MISSILE,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_NONE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_NONE,					D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_PLAYER,				D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_ANTLION,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_BULLSQUID,			D_NU, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_CITIZEN_PASSIVE,		D_NU, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_CITIZEN_REBEL,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_COMBINE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_HEADCRAB,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_HOUNDEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_SCANNER,				D_NU, 0 );		
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_STALKER,				D_NU, 0 );		
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_VORTIGAUNT,			D_NU, 0 );		
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_ZOMBIE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_PLAYER_ALLY,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_PLAYER_ALLY_VITAL,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_HACKED_ROLLERMINE,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_BLACK_OPS,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_NONE,				CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_PLAYER
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_PLAYER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_BARNACLE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_BULLSEYE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_BULLSQUID,			D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_CITIZEN_PASSIVE,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_COMBINE_GUNSHIP,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_COMBINE_HUNTER,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_HOUNDEYE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_MANHACK,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_SCANNER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_STALKER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_VORTIGAUNT,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_PROTOSNIPER,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_PLAYER_ALLY,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_PLAYER_ALLY_VITAL,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_HACKED_ROLLERMINE,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_CREW,					D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_SOLDIER,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_PLAYER_COMBINE_ALLY,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER,			CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_LI, 0 );

	// ------------------------------------------------------------
	//	> CLASS_PLAYER_ALLY
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_NONE,					D_NU, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_PLAYER,				D_LI, 0 );			
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_BARNACLE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_BULLSQUID,			D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_CITIZEN_PASSIVE,		D_NU, 0 );	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_CITIZEN_REBEL,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_COMBINE_HUNTER,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_HEADCRAB,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_MANHACK,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_SCANNER,				D_HT, 0 );		
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_STALKER,				D_HT, 0 );		
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_VORTIGAUNT,			D_LI, 0 );		
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_ZOMBIE,				D_FR, 1 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_PROTOSNIPER,			D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_PLAYER_ALLY,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_PLAYER_ALLY_VITAL,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_HACKED_ROLLERMINE,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );

	// ------------------------------------------------------------
	//	> CLASS_PLAYER_ALLY_VITAL
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_BARNACLE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_BULLSQUID,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_CITIZEN_REBEL,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE_HUNTER,		D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_HOUNDEYE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_MANHACK,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_SCANNER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_STALKER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_VORTIGAUNT,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_PROTOSNIPER,			D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER_ALLY,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER_ALLY_VITAL,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_HACKED_ROLLERMINE,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );

	// ------------------------------------------------------------
	//	> CLASS_SCANNER
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_BULLSQUID,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_COMBINE,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_COMBINE_GUNSHIP,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_COMBINE_HUNTER,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_HEADCRAB,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_HOUNDEYE,				D_NU, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_MANHACK,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_METROPOLICE,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_MILITARY,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_SCANNER,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_STALKER,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_ZOMBIE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_PROTOSNIPER,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_BLACK_OPS,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SCANNER,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_STALKER
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_BULLSQUID,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_COMBINE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_HEADCRAB,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_HOUNDEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_ZOMBIE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_STALKER,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_VORTIGAUNT
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_PLAYER,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_BARNACLE,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_BULLSQUID,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_CITIZEN_PASSIVE,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_CITIZEN_REBEL,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_COMBINE_HUNTER,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_HOUNDEYE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_MANHACK,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_SCANNER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_STALKER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_VORTIGAUNT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_PLAYER_ALLY,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_PLAYER_ALLY_VITAL,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_HACKED_ROLLERMINE,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_VORTIGAUNT,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );

	// ------------------------------------------------------------
	//	> CLASS_ZOMBIE
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_BULLSQUID,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_COMBINE_HUNTER,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_HEADCRAB,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_HOUNDEYE,				D_NU, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_MANHACK,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_MILITARY,				D_FR, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_ZOMBIE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_ZOMBIE,			CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );

	// ------------------------------------------------------------
	//	> CLASS_PROTOSNIPER
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_BULLSQUID,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_COMBINE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_HOUNDEYE,				D_NU, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_MISSILE,				D_NU, 5 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PROTOSNIPER,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_EARTH_FAUNA
	//
	// Hates pretty much everything equally except other earth fauna.
	// This will make the critter choose the nearest thing as its enemy.
	// ------------------------------------------------------------	
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_NONE,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_BULLSQUID,			D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_COMBINE_GUNSHIP,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_COMBINE_HUNTER,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_FLARE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_MANHACK,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_MISSILE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_SCANNER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_STALKER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_PROTOSNIPER,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_HACKED_ROLLERMINE,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_EARTH_FAUNA,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );

	// ------------------------------------------------------------
	//	> CLASS_HACKED_ROLLERMINE
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_BARNACLE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_BULLSEYE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_BULLSQUID,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_CITIZEN_PASSIVE,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_CITIZEN_REBEL,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_HOUNDEYE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_MISSILE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_STALKER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_VORTIGAUNT,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_ZOMBIE,				D_HT, 1 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_EARTH_FAUNA,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER_ALLY,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER_ALLY_VITAL,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_HACKED_ROLLERMINE,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );

	// ------------------------------------------------------------
	//	> CLASS_BLACK_OPS
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_PLAYER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_BARNACLE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_BULLSEYE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_BULLSQUID,			D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_COMBINE_GUNSHIP,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_COMBINE_HUNTER,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_MANHACK,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_MISSILE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_SCANNER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_STALKER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_PROTOSNIPER,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_PLAYER_ALLY,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_PLAYER_ALLY_VITAL,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_CREW,					D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_BLACK_OPS,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_SOLDIER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_BLACK_OPS,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );

	// ------------------------------------------------------------
	//	> CLASS_CREW
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_PLAYER,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_BARNACLE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_BULLSEYE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_BULLSQUID,			D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_COMBINE,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_COMBINE_GUNSHIP,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_COMBINE_HUNTER,		D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_MANHACK,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_METROPOLICE,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_MISSILE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_SCANNER,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_STALKER,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_PROTOSNIPER,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_PLAYER_ALLY,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_PLAYER_ALLY_VITAL,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_CREW,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_SOLDIER
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_PLAYER,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_BARNACLE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_BULLSEYE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_BULLSQUID,			D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_COMBINE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_COMBINE_GUNSHIP,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_COMBINE_HUNTER,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_CONSCRIPT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_MANHACK,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_METROPOLICE,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_MILITARY,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_MISSILE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_SCANNER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_STALKER,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_PROTOSNIPER,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_PLAYER_ALLY,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_PLAYER_ALLY_VITAL,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_PLAYER_COMBINE_ALLY,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_SOLDIER,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_HT, 0 );

	// ------------------------------------------------------------
	//	> CLASS_PLAYER_COMBINE_ALLY
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_PLAYER,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_BARNACLE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_BULLSQUID,			D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_BULLSEYE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_COMBINE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_MISSILE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_PLAYER_ALLY,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_PLAYER_ALLY_VITAL,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );

	// ------------------------------------------------------------
	//	> CLASS_PLAYER_COMBINE_ALLY_VITAL
	// ------------------------------------------------------------
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_NONE,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_PLAYER,				D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_ANTLION,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_BARNACLE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_BULLSEYE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_BULLSQUID,			D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_CITIZEN_PASSIVE,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_CITIZEN_REBEL,		D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_COMBINE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_COMBINE_GUNSHIP,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_COMBINE_HUNTER,		D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_CONSCRIPT,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_FLARE,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_HEADCRAB,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_HOUNDEYE,				D_HT, 0	);
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_MANHACK,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_METROPOLICE,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_MILITARY,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_MISSILE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_SCANNER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_STALKER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_VORTIGAUNT,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_ZOMBIE,				D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_PROTOSNIPER,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_EARTH_FAUNA,			D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_PLAYER_ALLY,			D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_PLAYER_ALLY_VITAL,	D_LI, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_HACKED_ROLLERMINE,	D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_BLACK_OPS,			D_HT, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_CREW,					D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_SOLDIER,				D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_PLAYER_COMBINE_ALLY,	D_NU, 0 );
	CBaseCombatCharacter::SetDefaultRelationship( CLASS_PLAYER_COMBINE_ALLY_VITAL,		CLASS_PLAYER_COMBINE_ALLY_VITAL,	D_NU, 0 );
}

const char *CHDTF::AIClassText( int classType )
{
	switch( classType )
	{
	case CLASS_BLACK_OPS: return "CLASS_BLACK_OPS";
	case CLASS_CREW: return "CLASS_CREW";
	case CLASS_SOLDIER: return "CLASS_SOLDIER";
	case CLASS_PLAYER_COMBINE_ALLY: return "CLASS_PLAYER_COMBINE_ALLY";
	case CLASS_PLAYER_COMBINE_ALLY_VITAL: return "CLASS_PLAYER_COMBINE_ALLY_VITAL";

	default:
		return BaseClass::AIClassText( classType );
	}
}

ConVar hdtf_smg1_pickable("hdtf_smg1_pickable", "0", FCVAR_ARCHIVE);
ConVar hdtf_m16_carbine_pickable("hdtf_m16_carbine_pickable", "0", FCVAR_ARCHIVE);
ConVar hdtf_spas_pickable("hdtf_spas_picakble", "0", FCVAR_ARCHIVE);
ConVar hdtf_oneshotbarrel_pickable("hdtf_oneshotbarrel_picakble", "0", FCVAR_ARCHIVE|FCVAR_HIDDEN);

//m3sa: added all other weapons
bool CHDTF::CanHavePlayerItem(CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon)
{
	if (FClassnameIs(pWeapon, "weapon_357")
		|| FClassnameIs(pWeapon, "weapon_alyxgun")
		|| FClassnameIs(pWeapon, "weapon_annabelle")
		|| FClassnameIs(pWeapon, "weapon_brickbat")
		|| FClassnameIs(pWeapon, "weapon_bugbait")
		|| FClassnameIs(pWeapon, "weapon_crossbow")
		|| FClassnameIs(pWeapon, "weapon_rpg")
		|| FClassnameIs(pWeapon, "weapon_stunstick"))
	{
		return false;
	}

	// NOTE(m3sa): temporary hidden the new m16 carbine behind convar until it's ready
	if (FClassnameIs(pWeapon, "weapon_shotgun") && !hdtf_spas_pickable.GetBool())
	{
		return false;
	}

	// NOTE(m3sa): perminantly hidden the old one shot shotgun behind convar
	if (FClassnameIs(pWeapon, "weapon_oneshotbarrel") && !hdtf_oneshotbarrel_pickable.GetBool())
	{
		return false;
	}
	
	// NOTE(wheatley): temporary hidden the new smg1 behind convar until it's ready
	if (FClassnameIs(pWeapon, "weapon_smg1") && !hdtf_smg1_pickable.GetBool())
	{
		return false;
	}

	// NOTE(m3sa): temporary hidden the new m16 carbine behind convar until it's ready
	if (FClassnameIs(pWeapon, "weapon_m16_carbine") && !hdtf_m16_carbine_pickable.GetBool())
	{
		return false;
	}

	return true;
}

#endif

// ------------------------------------------------------------------------------------ //
// Global functions.
// ------------------------------------------------------------------------------------ //

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB( grains ) ( 0.002285f * ( grains ) / 16.0f )
#define BULLET_MASS_GRAINS_TO_KG( grains ) lbs2kg( BULLET_MASS_GRAINS_TO_LB( grains ) )

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION 3.5f
// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE( grains, ftpersec ) ( ( ftpersec ) * 12.0f * BULLET_MASS_GRAINS_TO_KG( grains ) * BULLET_IMPULSE_EXAGGERATION )

CAmmoDef *GetAmmoDef( )
{
	static CAmmoDef def;
	static bool bInitted = false;

	if( !bInitted )
	{
		bInitted = true;

		def.AddAmmoType( "AR2", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_ar2", "sk_npc_dmg_ar2", "sk_max_ar2", BULLET_IMPULSE( 200.0f, 1225.0f ), 0 );
		def.AddAmmoType( "AlyxGun", DMG_BULLET, TRACER_LINE, "sk_plr_dmg_alyxgun", "sk_npc_dmg_alyxgun", "sk_max_alyxgun", BULLET_IMPULSE( 200.0f, 1225.0f ), 0 );
		def.AddAmmoType( "Pistol", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_pistol", "sk_npc_dmg_pistol", "sk_max_pistol", BULLET_IMPULSE( 200.0f, 1225.0f ), 0 );
		def.AddAmmoType( "SMG1", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_smg1", "sk_npc_dmg_smg1", "sk_max_smg1", BULLET_IMPULSE( 200.0f, 1225.0f ), 0 );
		def.AddAmmoType( "357", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_357", "sk_npc_dmg_357", "sk_max_357", BULLET_IMPULSE( 800.0f, 5000.0f ), 0 );
		def.AddAmmoType( "XBowBolt", DMG_BULLET, TRACER_LINE, "sk_plr_dmg_crossbow", "sk_npc_dmg_crossbow", "sk_max_crossbow", BULLET_IMPULSE( 800.0f, 8000.0f ), 0 );

		def.AddAmmoType( "Buckshot", DMG_BULLET | DMG_BUCKSHOT, TRACER_LINE, "sk_plr_dmg_buckshot", "sk_npc_dmg_buckshot", "sk_max_buckshot", BULLET_IMPULSE( 400.0f, 1200.0f ), 0 );
		def.AddAmmoType( "RPG_Round", DMG_BURN, TRACER_NONE, "sk_plr_dmg_rpg_round", "sk_npc_dmg_rpg_round", "sk_max_rpg_round", 0.0f, 0 );
		def.AddAmmoType( "SMG1_Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_smg1_grenade", "sk_npc_dmg_smg1_grenade", "sk_max_smg1_grenade", 0.0f, 0 );
		def.AddAmmoType( "SniperRound", DMG_BULLET | DMG_SNIPER, TRACER_NONE, "sk_plr_dmg_sniper_round", "sk_npc_dmg_sniper_round", "sk_max_sniper_round", BULLET_IMPULSE( 650.0f, 6000.0f ), 0 );
		def.AddAmmoType( "SniperPenetratedRound", DMG_BULLET | DMG_SNIPER, TRACER_NONE, "sk_dmg_sniper_penetrate_plr", "sk_dmg_sniper_penetrate_npc", "sk_max_sniper_round", BULLET_IMPULSE( 150.0f, 6000.0f ), 0 );
		def.AddAmmoType( "Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_grenade", 0.0f, 0 );
		def.AddAmmoType( "Thumper", DMG_SONIC, TRACER_NONE, 10, 10, 2, 0.0f, 0 );
		def.AddAmmoType( "Gravity", DMG_CLUB, TRACER_NONE, 0, 0, 8, 0.0f, 0 );
		def.AddAmmoType( "Battery", DMG_CLUB, TRACER_NONE, NULL, NULL, NULL, 0.0f, 0 );
		def.AddAmmoType( "GaussEnergy", DMG_SHOCK, TRACER_NONE, "sk_jeep_gauss_damage", "sk_jeep_gauss_damage", "sk_max_gauss_round", BULLET_IMPULSE( 650.0f, 8000.0f ), 0 ); // hit like a 10kg weight at 400 in/s
		def.AddAmmoType( "CombineCannon", DMG_BULLET, TRACER_LINE, "sk_npc_dmg_gunship_to_plr", "sk_npc_dmg_gunship", NULL, 1.5f * 750.0f * 12.0f, 0 ); // hit like a 1.5kg weight at 750 ft/s
		def.AddAmmoType( "AirboatGun", DMG_AIRBOAT, TRACER_LINE, "sk_plr_dmg_airboat", "sk_npc_dmg_airboat", NULL, BULLET_IMPULSE( 10.0f, 600.0f ), 0 );

#ifdef HL2_EPISODIC

		def.AddAmmoType( "StriderMinigun", DMG_BULLET, TRACER_LINE, 5, 5, 15, 1.0f * 750.0f * 12.0f, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s

#else

		def.AddAmmoType( "StriderMinigun", DMG_BULLET, TRACER_LINE, 5, 15, 15, 1.0f * 750.0f * 12.0f, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s

#endif // HL2_EPISODIC

		def.AddAmmoType( "StriderMinigunDirect", DMG_BULLET, TRACER_LINE, 2, 2, 15, 1.0f * 750.0f * 12.0f, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s
		def.AddAmmoType( "HelicopterGun", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_npc_dmg_helicopter_to_plr", "sk_npc_dmg_helicopter", "sk_max_smg1", BULLET_IMPULSE( 400.0f, 1225.0f ), AMMO_FORCE_DROP_IF_CARRIED | AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER );
		def.AddAmmoType( "AR2AltFire", DMG_DISSOLVE, TRACER_NONE, 0, 0, "sk_max_ar2_altfire", 0.0f, 0 );
		def.AddAmmoType( "Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_grenade", 0.0f, 0 );

#ifdef HL2_EPISODIC

		def.AddAmmoType( "Hopwire", DMG_BLAST, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_hopwire", 0.0f, 0 );
		def.AddAmmoType( "CombineHeavyCannon", DMG_BULLET, TRACER_LINE, 40, 40, NULL, 10.0f * 750.0f * 12.0f, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 10 kg weight at 750 ft/s
		def.AddAmmoType( "ammo_proto1", DMG_BULLET, TRACER_LINE, 0, 0, 10, 0.0f, 0 );

#endif // HL2_EPISODIC

		def.AddAmmoType( "Flare", DMG_BURN, TRACER_NONE, NULL, NULL, "sk_max_flare", 0.0f, 0 );
		def.AddAmmoType( "Molotov", DMG_BURN, TRACER_NONE, "sk_plr_dmg_molotov", "sk_npc_dmg_molotov", "sk_max_molotov", 0.0f, 0 );
		def.AddAmmoType( "Painkiller", DMG_GENERIC, TRACER_NONE, 0, 0, "sk_max_painkillers", 0.0f, 0 );
		def.AddAmmoType( "Claymore", DMG_BURN, TRACER_NONE, 0, 0, "sk_max_claymore", 0.0f, 0 );

		// TODO: Change impulse values
		def.AddAmmoType( "9mm", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_9mm", "sk_npc_dmg_9mm", "sk_max_9mm", BULLET_IMPULSE( 200.0f, 1225.0f ), 0 );
		def.AddAmmoType( "45cal", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_45cal", "sk_npc_dmg_45cal", "sk_max_45cal", BULLET_IMPULSE( 200.0f, 1225.0f ), 0 );
		def.AddAmmoType( "44cal", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_44cal", "sk_npc_dmg_44cal", "sk_max_44cal", BULLET_IMPULSE( 200.0f, 1225.0f ), 0 );
		def.AddAmmoType( "5_56mm", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_556mm", "sk_npc_dmg_556mm", "sk_max_556mm", BULLET_IMPULSE( 620.0f, 1850.0f ), 0 );
		def.AddAmmoType( "7_62mm", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_762mm", "sk_npc_dmg_762mm", "sk_max_762mm", BULLET_IMPULSE( 620.0f, 1850.0f ), 0 );
		def.AddAmmoType( "50cal", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_50cal", "sk_npc_dmg_50cal", "sk_max_50cal", BULLET_IMPULSE( 1000.0f, 8000.0f ), 0 );
		def.AddAmmoType( "7_92mm", DMG_BULLET, TRACER_LINE_AND_WHIZ, "sk_plr_dmg_792mm", "sk_npc_dmg_792mm", "sk_max_792mm", BULLET_IMPULSE( 800.0f, 5000.0f ), 0 );
	}

	return &def;
}
