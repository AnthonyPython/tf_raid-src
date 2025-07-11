//========= Copyright Valve Corporation, All rights reserved. ============//
// tf_bot_guard_area.cpp
// Defend an area against intruders
// Michael Booth, October 2009

#include "cbase.h"

#ifdef TF_RAID_MODE

#include "tf_player.h"
#include "bot/tf_bot.h"
#include "team_control_point_master.h"
#include "econ_entity_creation.h"
#include "bot/behavior/tf_bot_retreat_to_cover.h"
#include "bot/behavior/sniper/tf_bot_sniper_attack.h"
#include "bot/behavior/engineer/tf_bot_engineer_build.h"
#include "bot/behavior/medic/tf_bot_medic_heal.h"
#include "bot/behavior/scenario/raid/tf_bot_wander.h"
#include "bot/behavior/scenario/raid/tf_bot_guard_area.h"
#include "bot/behavior/tf_bot_attack.h"
#include "bot/behavior/demoman/tf_bot_prepare_stickybomb_trap.h"
#include "raid/tf_raid_logic.h"
#include "tf_wearable_weapons.h"

#include "nav_mesh.h"

extern ConVar tf_bot_path_lookahead_range;
ConVar tf_bot_guard_aggro_range( "tf_bot_guard_aggro_range", "750", FCVAR_CHEAT );
//ConVar tf_bot_guard_give_up_range( "tf_bot_guard_give_up_range", "1250", FCVAR_CHEAT );

ConVar tf_raid_special_vocalize_min_interval( "tf_raid_special_vocalize_min_interval", "10", FCVAR_CHEAT );
ConVar tf_raid_special_vocalize_max_interval( "tf_raid_special_vocalize_max_interval", "15", FCVAR_CHEAT );


//---------------------------------------------------------------------------------------------
ActionResult< CTFBot >	CTFBotGuardArea::OnStart( CTFBot *me, Action< CTFBot > *priorAction )
{
	m_chasePath.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );


	// give this guy a hat!
	CItemSelectionCriteria criteria;
	criteria.SetItemLevel(AE_USE_SCRIPT_VALUE);
	criteria.SetQuality(AE_USE_SCRIPT_VALUE);
	criteria.SetEquipRegions("Hat");
	//criteria.vecAbsOrigin = me->GetAbsOrigin();
	//criteria.vecAbsAngles = vec3_angle;
	
	/*switch (me->GetPlayerClass()->GetClassIndex())
	{
	case TF_CLASS_SCOUT:
	{
		//criteria.BSerializeToMsg
		break;
	}		
	case TF_CLASS_SNIPER:		criteria.SetTags("Sniper Hat 1");		break;
	case TF_CLASS_SOLDIER:		criteria.SetTags("Soldier Pot Hat");	break;
	case TF_CLASS_DEMOMAN:		criteria.SetTags("Demo Top Hat");		break;
	case TF_CLASS_MEDIC:		criteria.SetTags("Medic Hat 1");		break;
	case TF_CLASS_HEAVYWEAPONS:	criteria.SetTags("Heavy Ushanka Hat");	break;
	case TF_CLASS_PYRO:			criteria.SetTags("Pyro Chicken Hat");	break;
	case TF_CLASS_SPY:			criteria.SetTags("Spy Derby Hat");		break;
	case TF_CLASS_ENGINEER:		criteria.SetTags("Engineer Hat 1");	break;
	default:					criteria.SetTags("");					break;
	}*/

	CBaseEntity *hat = ItemGeneration()->GenerateRandomItem( &criteria, me->GetAbsOrigin(), vec3_angle);
	if ( hat )
	{
		// Fake global id
		static int s_nFakeID = 1;
		static_cast< CEconEntity * >( hat )->GetAttributeContainer()->GetItem()->SetItemID( s_nFakeID++ );

		DispatchSpawn( hat );
		//static_cast< CEconEntity * >( hat )->GetAttributeContainer()->GetItem()->GenerateAttributes();
		static_cast< CEconEntity * >( hat )->GiveTo( me );
	}
	else
	{
		Msg( "Failed to create hat\n" );
	}
	CBaseCombatWeapon* pWeapon = me->Weapon_GetSlot(TF_WPN_TYPE_PRIMARY);
	CBaseCombatWeapon* pWeapon2 = me->Weapon_GetSlot(TF_WPN_TYPE_SECONDARY);
	if (pWeapon)
	{

		if (!me->IsPlayerClass(TF_CLASS_SNIPER) )
		{
			if (pWeapon)
			{
				if (pWeapon == me->GetActiveWeapon())
					pWeapon->Holster();

				me->Weapon_Detach(pWeapon);
				UTIL_Remove(pWeapon);
			}
			else if (me->IsWearable())
			{
				CEconWearable* pWearable = dynamic_cast<CEconWearable*>(pWeapon);
				me->RemoveWearable(pWearable);
			}
			else
			{
				Assert(false);
				UTIL_Remove(pWeapon);
			}
		}

	}

	if (me->IsPlayerClass(TF_CLASS_SPY))
	{

		if (pWeapon2)
		{
			if (pWeapon2 == me->GetActiveWeapon())
				pWeapon2->Holster();

			me->Weapon_Detach(pWeapon2);
			UTIL_Remove(pWeapon2);
		}
		else if (me->IsWearable())
		{
			CEconWearable* pWearable = dynamic_cast<CEconWearable*>(pWeapon2);
			me->RemoveWearable(pWearable);
		}
		else
		{
			Assert(false);
			UTIL_Remove(pWeapon2);
		}
	}

	CBaseCombatWeapon* meleeWeapon = me->Weapon_GetSlot(TF_WPN_TYPE_MELEE);
	if (meleeWeapon)
	{

		me->Weapon_Switch(meleeWeapon);

	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
class CFindVantagePoint : public ISearchSurroundingAreasFunctor
{
public:
	CFindVantagePoint( void )
	{
		m_vantageArea = NULL;
	}

	virtual bool operator() ( CNavArea *baseArea, CNavArea *priorArea, float travelDistanceSoFar )
	{
		if ( travelDistanceSoFar > 2000.0f )
			return false;

		CTFNavArea *area = (CTFNavArea *)baseArea;

		CTeam *raidingTeam = GetGlobalTeam( TF_TEAM_BLUE );
		for( int i=0; i<raidingTeam->GetNumPlayers(); ++i )
		{
			CTFPlayer *player = (CTFPlayer *)raidingTeam->GetPlayer(i);

			if ( !player->IsAlive() || !player->GetLastKnownArea() )
				continue;

			CTFNavArea *playerArea = (CTFNavArea *)player->GetLastKnownArea();
			if ( playerArea->IsCompletelyVisible( area ) )
			{
				// nearby area from which we can see the enemy team
				m_vantageArea = area;
				return false;
			}
		}

		return true;
	}

	CTFNavArea *m_vantageArea;
};


//---------------------------------------------------------------------------------------------
ActionResult< CTFBot >	CTFBotGuardArea::Update( CTFBot *me, float interval )
{
	CBaseCombatWeapon* meleeWeapon = me->Weapon_GetSlot(TF_WPN_TYPE_MELEE);
	if (meleeWeapon)
	{

		if (!me->IsPlayerClass(TF_CLASS_SNIPER) && !me->IsPlayerClass(TF_CLASS_SPY) && !me->IsPlayerClass(TF_CLASS_MEDIC) && !me->IsPlayerClass(TF_CLASS_DEMOMAN) && !me->IsPlayerClass(TF_CLASS_ENGINEER))
		{
			me->Weapon_Switch(meleeWeapon);
		}
		
	}


	/*CTeam* raidingTeam = GetGlobalTeam(TF_TEAM_BLUE);

	if (me->HasAttribute(CTFBot::AGGRESSIVE))
	{
		// I'm a mob rusher - pick a random raider and attack them!
		CTFPlayer* victim = TFGameRules()->GetRaidLogic()->SelectRaiderToAttack();
		if (victim)
		{
			return SuspendFor(new CTFBotMobRush(victim), "Rushing a raider");
		}
	}*/
	// emit vocalizations to warn players we're in the area
	if ( m_vocalizeTimer.IsElapsed() )
	{
		m_vocalizeTimer.Start( RandomFloat( tf_raid_special_vocalize_min_interval.GetFloat(), tf_raid_special_vocalize_max_interval.GetFloat() ) );
		me->SpeakConceptIfAllowed( MP_CONCEPT_PLAYER_JEERS );
	}

	if ( me->IsPlayerClass( TF_CLASS_MEDIC ) )
	{
		return SuspendFor( new CTFBotMedicHeal );
	}

	if ( me->IsPlayerClass( TF_CLASS_ENGINEER ) )
	{
		return SuspendFor( new CTFBotEngineerBuild );
	}

	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
	if ( threat && threat->IsVisibleRecently() )
	{
		m_pathToVantageArea.Invalidate();

		CTFNavArea *myArea = (CTFNavArea *)me->GetLastKnownArea();
		CTFNavArea *threatArea = (CTFNavArea *)threat->GetLastKnownArea();
		if ( myArea && threatArea )
		{
			if ( threatArea->GetIncursionDistance( TF_TEAM_BLUE ) < myArea->GetIncursionDistance( TF_TEAM_BLUE ) )
			{
				if ( me->IsRangeGreaterThan( threat->GetLastKnownPosition(), tf_bot_guard_aggro_range.GetFloat() ) )
				{
					// threat is far off and hasn't reached us yet - hide until they are closer
					return SuspendFor( new CTFBotRetreatToCover, "Hiding until threat gets closer" );
				}
			}
		}

		// attack!
		return SuspendFor( new CTFBotAttack, "Attacking nearby threat" );
	}
	else
	{
		// no enemy is visible
		Vector moveTo = me->GetAbsOrigin();

		// if point is being captured, move to it
		CTeamControlPoint *point = me->GetMyControlPoint();
		if ( point && point->LastContestedAt() > 0.0f && ( gpGlobals->curtime - point->LastContestedAt() ) < 5.0f )
		{
			// the point is, or was very recently, contested - defend it!
			moveTo = point->GetAbsOrigin();
		}
		else if ( me->GetHomeArea() )
		{
			// no enemy is visible - return to our home position
			moveTo = me->GetHomeArea()->GetCenter();
		}

		if ( !m_pathToPoint.IsValid() || m_repathTimer.IsElapsed() )
		{
			CTFBotPathCost cost( me, FASTEST_ROUTE );
			m_pathToPoint.Compute( me, moveTo, cost );
			m_repathTimer.Start( RandomFloat( 2.0f, 3.0f ) );
		}

		if ( ( me->GetAbsOrigin() - moveTo ).IsLengthGreaterThan( 25.0f ) )
		{
			m_pathToPoint.Update( me );
		}

		if ( me->GetHomeArea() == me->GetLastKnownArea() )
		{
			// at home
			if ( CTFBotPrepareStickybombTrap::IsPossible( me ) )
			{
				return SuspendFor( new CTFBotPrepareStickybombTrap, "Laying sticky bombs!" );
			}
		}

/*
		// no enemy is visible - move to where we can see them
		if ( !m_pathToVantageArea.IsValid() )
		{
			CTFNavArea *vantageArea = me->FindVantagePoint();
			if ( vantageArea )
			{
				CTFBotPathCost cost( me, FASTEST_ROUTE );
				m_pathToVantageArea.Compute( me, vantageArea->GetCenter(), cost );
			}
		}

		m_pathToVantageArea.Update( me );
*/
	}


	return Continue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CTFBot > CTFBotGuardArea::OnStuck( CTFBot *me )
{
	m_chasePath.Invalidate();
	m_pathToPoint.Invalidate();
	m_pathToVantageArea.Invalidate();

	return TryContinue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CTFBot > CTFBotGuardArea::OnMoveToSuccess( CTFBot *me, const Path *path )
{
	return TryContinue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CTFBot > CTFBotGuardArea::OnMoveToFailure( CTFBot *me, const Path *path, MoveToFailureType reason )
{
	return TryContinue();
}


//---------------------------------------------------------------------------------------------
QueryResultType	CTFBotGuardArea::ShouldRetreat( const INextBot *me ) const
{
	return ANSWER_NO;
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CTFBot > CTFBotGuardArea::OnCommandApproach( CTFBot *me, const Vector &pos, float range )
{
	return TryContinue();
}

#endif // TF_RAID_MODE
