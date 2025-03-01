#include "cbase.h"
#include "npc_combines.h"
#include "weapon_rpg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_RebelSoldier
//=========================================================
ConVar	sk_rebelsoldier_health("sk_rebelsoldier_health", "0");

class CNPC_RebelSoldier : public CNPC_CombineS
{
	DECLARE_DATADESC();
	DECLARE_CLASS( CNPC_RebelSoldier, CNPC_CombineS );
	DEFINE_CUSTOM_AI;

public:
	void Spawn();
	virtual void Precache( );

	virtual Class_T Classify( );

	//void DeathSound( const CTakeDamageInfo & ) { }
	//void PainSound( const CTakeDamageInfo & ) { }
	//void IdleSound( ) { }
	//void AlertSound( ) { }
	//void LostEnemySound( ) { }
	//void FoundEnemySound( ) { }
	//void AnnounceAssault( ) { }
	//void AnnounceEnemyType( CBaseEntity * ) { }
	//void AnnounceEnemyKill( CBaseEntity * ) { }
	//void NotifyDeadFriend( CBaseEntity * ) { }

	void StartTask(const Task_t *pTask);
	void RunTask(const Task_t *pTask);
	int TranslateSchedule(int scheduleType);
	Activity NPC_TranslateActivity(Activity activity);

	virtual int	PlaySentence(CombineSentence_t sentence,
		SentencePriority_t priority = SENTENCE_PRIORITY_NORMAL,
		SentenceCriteria_t criteria = SENTENCE_CRITERIA_IN_SQUAD);

private:
	bool m_bRPGAvoidPlayer;

	enum {
		SCHED_REBEL_RANGE_ATTACK1_RPG = BaseClass::NEXT_SCHEDULE,

		TASK_REBEL_RPG_AUGER = BaseClass::NEXT_TASK,
	};
};

BEGIN_DATADESC(CNPC_RebelSoldier)
	DEFINE_FIELD(m_bRPGAvoidPlayer, FIELD_BOOLEAN),
	
END_DATADESC();

LINK_ENTITY_TO_CLASS( npc_rebel_soldier, CNPC_RebelSoldier );

void CNPC_RebelSoldier::Spawn()
{
	BaseClass::Spawn();

	int n_body_bodygroup = FindBodygroupByName("body");
	int n_gloves_bodygroup = FindBodygroupByName("gloves");

	SetBodygroup(n_body_bodygroup, RandomInt(0, 2));
	SetBodygroup(n_gloves_bodygroup, RandomInt(0, 2));

	m_nSkin = RandomInt(0, 3);

	m_iHealth = sk_rebelsoldier_health.GetFloat();

	m_bRPGAvoidPlayer = false;

	CWeaponRPG *pRPG = dynamic_cast<CWeaponRPG *>(GetActiveWeapon());
	if (pRPG)
	{
		CapabilitiesRemove(bits_CAP_USE_SHOT_REGULATOR);
		pRPG->StopGuiding();
	}
}

void CNPC_RebelSoldier::Precache( )
{
	if( !GetModelName( ) )
		SetModelName( MAKE_STRING( "models/hdtf/characters/rebels/rebel_male.mdl" ) );

	BaseClass::Precache( );
}

Class_T CNPC_RebelSoldier::Classify( )
{
	return CLASS_CITIZEN_REBEL;
}

int CNPC_RebelSoldier::PlaySentence(CombineSentence_t sentence, SentencePriority_t priority, SentenceCriteria_t criteria)
{
	const char *pSentenceName = NULL;

	const char* pnull = "\0";

	switch (sentence)
	{
	case COMBINE_SPEECH_DIE:
		
		if (V_strcmp(m_strSpeechDie.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechDie);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_DIE";
		}

		break;

	case COMBINE_SPEECH_PAIN:
		
		if (V_strcmp(m_strSpeechPain.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechPain);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_PAIN";
		}

		break;

	case COMBINE_SPEECH_KICK:
		if (V_strcmp(m_strSpeechKick.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechKick);
		}
		else
		{
			pSentenceName = "COMBINE_KICK";
		}

		break;

	case COMBINE_SPEECH_THROW_GRENADE:
		if (V_strcmp(m_strSpeechThrow_Nade.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechThrow_Nade);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_THROW_GRENADE";
		}

		break;

	case COMBINE_SPEECH_REFIND_ENEMY:
		if (V_strcmp(m_strSpeechRefind_Enemy.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechRefind_Enemy);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_REFIND_ENEMY";
		}

		break;

	case COMBINE_SPEECH_MONST_CITIZENS:
		if (V_strcmp(m_strSpeechMonst_Citizens.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechMonst_Citizens);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_SPOT_GENERAL";
		}

		break;

	case COMBINE_SPEECH_MONST_CHARACTER:
		if (V_strcmp(m_strSpeechMonst_Character.ToCStr() ,pnull))
		{
			pSentenceName = STRING(m_strSpeechMonst_Character);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_SPOT_GENERAL";
		}

		break;

	case COMBINE_SPEECH_ALERT:
		if (V_strcmp(m_strSpeechAlert.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechAlert);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_ALERT";
		}

		break;

	case COMBINE_SPEECH_ANNOUNCE:
		if (V_strcmp(m_strSpeechAnnounce.ToCStr() ,pnull))
		{
			pSentenceName = STRING(m_strSpeechAnnounce);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_ANNOUNCE";
		}

		break;

	case COMBINE_SPEECH_PLAYERHIT:
		if (V_strcmp(m_strSpeechPlayerHit.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechPlayerHit);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_PLAYERHIT";
		}

		break;
	case COMBINE_SPEECH_KILL_MONST:
		if (V_strcmp(m_strSpeechKillMonst.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechKillMonst);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_KILL_ENEMY";
		}

		break;
	case COMBINE_SPEECH_KILL_PLAYER:
		if (V_strcmp(m_strSpeechKillPlayer.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechKillPlayer);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_KILL_ENEMY";
		}

		break;

	case COMBINE_SPEECH_ASSAULT:
		if (V_strcmp(m_strSpeechAssault.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechAssault);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_ASSAULT";
		}

		break;

	case COMBINE_SPEECH_MONST:
		if (V_strcmp(m_strSpeechMonst.ToCStr() ,pnull))
		{
			pSentenceName = STRING(m_strSpeechMonst);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_SPOT_GENERAL";
		}

		break;

	case COMBINE_SPEECH_MONST_BUGS:
		if (V_strcmp(m_strSpeechMonstBugs.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechMonstBugs);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_SPOT_BUGS";
		}

		break;

	case COMBINE_SPEECH_PARASITES:
		if (V_strcmp(m_strSpeechParasites.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechParasites);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_SPOT_PARASITES";
		}

		break;

	case COMBINE_SPEECH_MONST_ZOMBIES:
		if (V_strcmp(m_strSpeechMonstZombies.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechMonstZombies);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_SPOT_ZOMBIES";
		}

		break;

	case COMBINE_SPEECH_DANGER:
		if (V_strcmp(m_strSpeechDanger.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechDanger);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_DANGER";
		}

		break;

	case COMBINE_SPEECH_GREN:
		if (V_strcmp(m_strSpeechGren.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechGren);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_SPOT_GRENADE";
		}

		break;

	case COMBINE_SPEECH_MAN_DOWN:
		if (V_strcmp(m_strSpeechManDown.ToCStr() ,pnull))
		{
			pSentenceName = STRING(m_strSpeechManDown);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_MAN_DOWN";
		}

		break;

	case COMBINE_SPEECH_GO_ALERT:
		if (V_strcmp(m_strSpeechGoAlert.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechGoAlert);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_GO_ALERT";
		}

		break;
	case COMBINE_SPEECH_FLANK:
		if (V_strcmp(m_strSpeechFlank.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechFlank);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_FLANK";
		}

		break;

	case COMBINE_SPEECH_COVER:
		if (V_strcmp(m_strSpeechCover.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechCover);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_COVER";
		}

		break;

	case COMBINE_SPEECH_LOST_LONG:
		if (V_strcmp(m_strSpeechLostLong.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechLostLong);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_ENEMY_LOST";
		}

		break;
	case COMBINE_SPEECH_LOST_SHORT:
		if (V_strcmp(m_strSpeechLostShort.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechLostShort);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_ENEMY_LOST";
		}

		break;

	case COMBINE_SPEECH_LAST_OF_SQUAD:
		if (V_strcmp(m_strSpeechLastOfSquad.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechLastOfSquad);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_LAST_MAN";
		}

		break;

	case COMBINE_SPEECH_IDLE:
		if (V_strcmp(m_strSpeechIdle.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechIdle);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_IDLE";
		}

		break;

	case COMBINE_SPEECH_TAUNT:
		if (V_strcmp(m_strSpeechTaunt.ToCStr() ,pnull))
		{
			pSentenceName = STRING(m_strSpeechTaunt);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_TAUNT";
		}

		break;
	case COMBINE_SPEECH_CHECK:
		if (V_strcmp(m_strSpeechCheck.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechCheck);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_IDLE_CHECK";
		}

		break;
	case COMBINE_SPEECH_QUEST:
		if (V_strcmp(m_strSpeechQuest.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechQuest);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_IDLE_QUESTION";
		}

		break;
	case COMBINE_SPEECH_CLEAR:
		if (V_strcmp(m_strSpeechClear.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechClear);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_IDLE_ANSWER_CLEAR";
		}

		break;
	case COMBINE_SPEECH_ANSWER:
		if (V_strcmp(m_strSpeechAnswer.ToCStr(),pnull))
		{
			pSentenceName = STRING(m_strSpeechAnswer);
		}
		else
		{
			pSentenceName = "REBEL_SOLDIER_IDLE_ANSWER";
		}

		break;

	default:
		break;
	}

	

	if (pSentenceName == NULL)
		return 0;

	return m_Sentences.Speak(pSentenceName, priority, criteria);
}

void CNPC_RebelSoldier::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_REBEL_RPG_AUGER:
		m_bRPGAvoidPlayer = false;
		SetWait(15.0); // maximum time auger before giving up
		break;
	default:
		BaseClass::StartTask(pTask);
	}
}

void CNPC_RebelSoldier::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_REBEL_RPG_AUGER:
	{
		// Keep augering until the RPG has been destroyed
		CWeaponRPG *pRPG = dynamic_cast<CWeaponRPG *>(GetActiveWeapon());
		if (!pRPG)
		{
			TaskFail(FAIL_ITEM_NO_FIND);
			return;
		}

		// Has the RPG detonated?
		if (!pRPG->GetMissile())
		{
			pRPG->StopGuiding();
			TaskComplete();
			return;
		}

		Vector vecLaserPos = pRPG->GetNPCLaserPosition();

		if (!m_bRPGAvoidPlayer)
		{
			// Abort if we've lost our enemy
			if (!GetEnemy())
			{
				pRPG->StopGuiding();
				TaskFail(FAIL_NO_ENEMY);
				return;
			}

			// Is our enemy occluded?
			if (HasCondition(COND_ENEMY_OCCLUDED))
			{
				// Turn off the laserdot, but don't stop augering
				pRPG->StopGuiding();
				return;
			}
			else if (pRPG->IsGuiding() == false)
			{
				pRPG->StartGuiding();
			}

			Vector vecEnemyPos = GetEnemy()->BodyTarget(GetAbsOrigin(), false);

			// NOTE(wheatley): if this is the first time we're aiming snap to enemy location
			if (vecLaserPos == vec3_origin)
				vecLaserPos = vecEnemyPos;

			CBasePlayer *pPlayer = AI_GetSinglePlayer();
			if (pPlayer && pPlayer != GetEnemy() && ((vecEnemyPos - pPlayer->GetAbsOrigin()).LengthSqr() < CMissile::EXPLOSION_RADIUS * CMissile::EXPLOSION_RADIUS))
			{
				m_bRPGAvoidPlayer = true;
			}
			else
			{
				// Pull the laserdot towards the target
				Vector vecToTarget = (vecEnemyPos - vecLaserPos);
				float distToMove = VectorNormalize(vecToTarget);
				if (distToMove > 90)
					distToMove = 90;
				vecLaserPos += vecToTarget * distToMove;
			}
		}

		if (m_bRPGAvoidPlayer)
		{
			// Pull the laserdot up
			vecLaserPos.z += 90;
		}

		if (IsWaitFinished())
		{
			pRPG->StopGuiding();
			TaskFail(FAIL_NO_SHOOT);
			return;
		}
		// Add imprecision to avoid obvious robotic perfection stationary targets
		float imprecision = 18 * sin(gpGlobals->curtime);
		vecLaserPos.x += imprecision;
		vecLaserPos.y += imprecision;
		vecLaserPos.z += imprecision;
		pRPG->UpdateNPCLaserPosition(vecLaserPos);
	}
	break;

	default:
		BaseClass::RunTask(pTask);
	}
}

int CNPC_RebelSoldier::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_RANGE_ATTACK1:
		// If we have an RPG, we use a custom schedule for it
		if (GetActiveWeapon() && FClassnameIs(GetActiveWeapon(), "weapon_rpg"))
		{
			CBasePlayer *pPlayer = AI_GetSinglePlayer();
			if (pPlayer && GetEnemy() && pPlayer != GetEnemy() && ((GetEnemy()->GetAbsOrigin() -
				pPlayer->GetAbsOrigin()).LengthSqr() < CMissile::EXPLOSION_RADIUS * CMissile::EXPLOSION_RADIUS))
			{
				// Don't fire our RPG at an enemy too close to the player unless player is the enemy
				return SCHED_STANDOFF;
			}
			else
			{
				return SCHED_REBEL_RANGE_ATTACK1_RPG;
			}
		}
		break;

	case SCHED_ESTABLISH_LINE_OF_FIRE:
	case SCHED_MOVE_TO_WEAPON_RANGE:
		if (GetActiveWeapon() && FClassnameIs(GetActiveWeapon(), "weapon_rpg"))
		{
			return TranslateSchedule(SCHED_RANGE_ATTACK1);
		}
		break;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

Activity CNPC_RebelSoldier::NPC_TranslateActivity(Activity activity)
{
	if (activity == ACT_RANGE_ATTACK1)
		return ACT_RANGE_ATTACK_AR2;

	return BaseClass::NPC_TranslateActivity(activity);
}

AI_BEGIN_CUSTOM_NPC(npc_rebel_soldier, CNPC_RebelSoldier)

DECLARE_TASK(TASK_REBEL_RPG_AUGER)

//=========================================================
// > SCHED_CITIZEN_RANGE_ATTACK1_RPG
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_REBEL_RANGE_ATTACK1_RPG,

	"	Tasks"
	"		TASK_STOP_MOVING			0"
	"		TASK_FACE_ENEMY				0"
	"		TASK_ANNOUNCE_ATTACK		1"	// 1 = primary attack
	"		TASK_RANGE_ATTACK1			0"
	"		TASK_REBEL_RPG_AUGER		1"
	"		TASK_WAIT				  2.5"
	""
	"	Interrupts"
)

AI_END_CUSTOM_NPC()
