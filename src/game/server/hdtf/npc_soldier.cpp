#include "cbase.h"
#include "npc_combines.h"
#include "grenade_frag.h"
#include "npcevent.h"
#include "weapon_rpg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SOLDIER_GRENADE_TIMER		3.5

#define SOLDIER_AE_GREN_TOSS		( 7 )
#define SOLDIER_AE_GREN_LAUNCH		( 8 )
#define SOLDIER_AE_GREN_DROP		( 9 )

//=========================================================
//	>> CNPC_Soldier
//=========================================================
class CNPC_Soldier : public CNPC_CombineS
{
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;
	DECLARE_CLASS( CNPC_Soldier, CNPC_CombineS );

public:
	virtual void Precache( );

	virtual Class_T Classify( );

	void Spawn();

	

	// this allow player to interact with us
	virtual int ObjectCaps() 
	{
		int baseCaps = BaseClass::ObjectCaps();
		baseCaps |= FCAP_IMPULSE_USE;
		return baseCaps;
	}

	bool CanSpeakResponse();
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	void Event_Killed(const CTakeDamageInfo &info);

	void HandleAnimEvent(animevent_t *pEvent);

	void StartTask(const Task_t *pTask);
	void RunTask(const Task_t *pTask);
	int TranslateSchedule(int scheduleType);
	Activity NPC_TranslateActivity(Activity activity);

	virtual int	PlaySentence(CombineSentence_t sentence,
		SentencePriority_t priority = SENTENCE_PRIORITY_NORMAL,
		SentenceCriteria_t criteria = SENTENCE_CRITERIA_IN_SQUAD);

	virtual void Touch(CBaseEntity *pOther);


	

private:
	float m_flTimeNextUse;
	bool m_bRPGAvoidPlayer;

	enum {
		SCHED_SOLDIER_RANGE_ATTACK1_RPG = BaseClass::NEXT_SCHEDULE,

		TASK_SOLDIER_RPG_AUGER = BaseClass::NEXT_TASK,
	};
};

BEGIN_DATADESC(CNPC_Soldier)
	DEFINE_FIELD(m_flTimeNextUse, FIELD_TIME),
	DEFINE_FIELD(m_bRPGAvoidPlayer, FIELD_BOOLEAN),
END_DATADESC();

LINK_ENTITY_TO_CLASS(npc_soldier, CNPC_Soldier);

void CNPC_Soldier::Precache( )
{
	if( !GetModelName( ) )
		SetModelName( MAKE_STRING( "models/Humans/citizen_soldier.mdl" ) );

	PrecacheModel("models/weapons/w_m67.mdl");

	BaseClass::Precache( );
}

Class_T CNPC_Soldier::Classify( )
{
	return CLASS_SOLDIER;
}

void CNPC_Soldier::Spawn()
{
	BaseClass::Spawn();

	m_flTimeNextUse = 0.f;
	m_bRPGAvoidPlayer = false;

	CWeaponRPG *pRPG = dynamic_cast<CWeaponRPG *>(GetActiveWeapon());
	if (pRPG)
	{
		CapabilitiesRemove(bits_CAP_USE_SHOT_REGULATOR);
		pRPG->StopGuiding();
	}
}

int CNPC_Soldier::PlaySentence(CombineSentence_t sentence, SentencePriority_t priority, SentenceCriteria_t criteria)
{
	// HACK!!!
	// larry should not speak any sentences.
	if (FStrEq(GetEntityName().ToCStr(), "Larry"))
		return 0;

	if (FStrEq(GetEntityName().ToCStr(), "npc_soldier_unique_1"))
		return 0;

	if (FStrEq(GetEntityName().ToCStr(), "npc_soldier_unique_2"))
		return 0;

	if (FStrEq(GetEntityName().ToCStr(), "npc_soldier_unique_3"))
		return 0;

	if (FStrEq(GetEntityName().ToCStr(), "npc_soldier_unique_4"))
		return 0;

	if (FStrEq(GetEntityName().ToCStr(), "npc_soldier_unique_5"))
		return 0;

	if (m_flTimeNextUse >= gpGlobals->curtime)
		return 0;

	const char *pSentenceName = NULL;
	switch (sentence)
	{
	case COMBINE_SPEECH_DIE:
		if (V_strcmp(m_strSpeechDie.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechDie);
		}
		else
		{
			pSentenceName = "SOLDIER_DIE";
		}
		
		break;

	case COMBINE_SPEECH_PAIN:
		if (V_strcmp(m_strSpeechPain.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechPain);
		}
		else
		{
			pSentenceName = "SOLDIER_PAIN";
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
			pSentenceName = "SOLDIER_GRENADE_ANNOUNCE";
		}
		
		break;

	case COMBINE_SPEECH_REFIND_ENEMY:
		if (V_strcmp(m_strSpeechRefind_Enemy.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechRefind_Enemy);
		}
		else
		{
			pSentenceName = "SOLDIER_REFIND_ENEMY";
		}
		
		break;

	case COMBINE_SPEECH_MONST_CITIZENS:
		if (V_strcmp(m_strSpeechMonst_Citizens.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechMonst_Citizens);
		}
		else
		{
			pSentenceName = "SOLDIER_ALERT";
		}
		
		break;

	case COMBINE_SPEECH_MONST_CHARACTER:
		if (V_strcmp(m_strSpeechMonst_Character.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechMonst_Character);
		}
		else
		{
			pSentenceName = "SOLDIER_ALERT";
		}
		
		break;

	case COMBINE_SPEECH_ALERT:
		if (V_strcmp(m_strSpeechAlert.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechAlert);
		}
		else
		{
			pSentenceName = "SOLDIER_ALERT";
		}
		
		break;

	case COMBINE_SPEECH_ANNOUNCE:
		if (V_strcmp(m_strSpeechAnnounce.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechAnnounce);
		}
		else
		{
			pSentenceName = "SOLDIER_ALERT";
		}
		
		break;

	case COMBINE_SPEECH_PLAYERHIT:
		if (V_strcmp(m_strSpeechPlayerHit.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechPlayerHit);
		}
		else
		{
			pSentenceName = "SOLDIER_KILLENEMY";
		}
		
		break;
	case COMBINE_SPEECH_KILL_MONST:
		if (V_strcmp(m_strSpeechKillMonst.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechKillMonst);
		}
		else
		{
			pSentenceName = "SOLDIER_KILLENEMY";
		}
		
		break;
	case COMBINE_SPEECH_KILL_PLAYER:
		if (V_strcmp(m_strSpeechKillPlayer.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechKillPlayer);
		}
		else
		{
			pSentenceName = "SOLDIER_KILLENEMY";
		}
		
		break;

	case COMBINE_SPEECH_ASSAULT:
		if (V_strcmp(m_strSpeechAssault.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechAssault);
		}
		else
		{
			pSentenceName = "SOLDIER_ASSAULT";
		}
		
		break;

	case COMBINE_SPEECH_MONST:
		if (V_strcmp(m_strSpeechMonst.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechMonst);
		}
		else
		{
			pSentenceName = "SOLDIER_ALERT";
		}
		
		break;

	case COMBINE_SPEECH_MONST_BUGS:
		if (V_strcmp(m_strSpeechMonstBugs.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechMonstBugs);
		}
		else
		{
			pSentenceName = "SOLDIER_ALERT";
		}
		
		break;

	case COMBINE_SPEECH_PARASITES:
		if (V_strcmp(m_strSpeechParasites.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechParasites);
		}
		else
		{
			pSentenceName = "SOLDIER_ALERT";
		}
		
		break;
	
	case COMBINE_SPEECH_MONST_ZOMBIES:
		if (V_strcmp(m_strSpeechMonstZombies.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechMonstZombies);
		}
		else
		{
			pSentenceName = "SOLDIER_MONST_ZOMBIES";
		}
		
		break;

	case COMBINE_SPEECH_DANGER:
		if (V_strcmp(m_strSpeechDanger.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechDanger);
		}
		else
		{
			pSentenceName = "SOLDIER_DANGER";
		}
		
		break;

	case COMBINE_SPEECH_GREN:
		if (V_strcmp(m_strSpeechGren.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechGren);
		}
		else
		{
			pSentenceName = "SOLDIER_GRENADE";
		}
		
		break;

	case COMBINE_SPEECH_MAN_DOWN:
		if (V_strcmp(m_strSpeechManDown.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechManDown);
		}
		else
		{
			pSentenceName = "SOLDIER_MANDOWN";
		}
		
		break;

	case COMBINE_SPEECH_GO_ALERT:
		if (V_strcmp(m_strSpeechGoAlert.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechGoAlert);
		}
		else
		{
			pSentenceName = "SOLDIER_ORDER";
		}
		
		break;
	case COMBINE_SPEECH_FLANK:
		if (V_strcmp(m_strSpeechFlank.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechFlank);
		}
		else
		{
			pSentenceName = "SOLDIER_ORDER";
		}
		
		break;
	
	case COMBINE_SPEECH_COVER:
		if (V_strcmp(m_strSpeechCover.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechCover);
		}
		else
		{
			pSentenceName = "SOLDIER_FALLBACK";
		}
		
		break;

	case COMBINE_SPEECH_LOST_LONG:
		if (V_strcmp(m_strSpeechLostLong.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechLostLong);
		}
		else
		{
			pSentenceName = "SOLDIER_ENEMYLOST";
		}
		
		break;
	case COMBINE_SPEECH_LOST_SHORT:
		if (V_strcmp(m_strSpeechLostShort.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechLostShort);
		}
		else
		{
			pSentenceName = "SOLDIER_ENEMYLOST";
		}
		
		break;
	
	case COMBINE_SPEECH_LAST_OF_SQUAD:
		if (V_strcmp(m_strSpeechLastOfSquad.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechLastOfSquad);
		}
		else
		{
			pSentenceName = "SOLDIER_LAST_MAN";
		}
		
		break;
	
	case COMBINE_SPEECH_IDLE:
		if (V_strcmp(m_strSpeechIdle.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechIdle);
		}
		else
		{
			pSentenceName = "SOLDIER_IDLE";
		}
		
		break;

	case COMBINE_SPEECH_TAUNT:
		if (V_strcmp(m_strSpeechTaunt.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechTaunt);
		}
		else
		{
			pSentenceName = "SOLDIER_ANSWER";
		}
		
		break;
	case COMBINE_SPEECH_CHECK:
		if (V_strcmp(m_strSpeechCheck.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechCheck);
		}
		else
		{
			pSentenceName = "SOLDIER_ANSWER";
		}

		break;
	case COMBINE_SPEECH_QUEST:
		if (V_strcmp(m_strSpeechQuest.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechQuest);
		}
		else
		{
			pSentenceName = "SOLDIER_ANSWER";
		}
		
		break;
	case COMBINE_SPEECH_CLEAR:
		if (V_strcmp(m_strSpeechClear.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechClear);
		}
		else
		{
			pSentenceName = "SOLDIER_ANSWER";
		}
		
		break;
	case COMBINE_SPEECH_ANSWER:
		if (V_strcmp(m_strSpeechAnswer.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechAnswer);
		}
		else
		{
			pSentenceName = "SOLDIER_ANSWER";
		}
		
		break;

	default:
		break;
	}

	if (pSentenceName == NULL)
		return 0;

	int index = m_Sentences.Speak(pSentenceName, priority, criteria);
	float duration = engine->SentenceLength(index);
	m_flTimeNextUse = gpGlobals->curtime + duration + 2.5f;
	return duration;
}

void CNPC_Soldier::Touch( CBaseEntity *pOther )
{
	BaseClass::Touch(pOther);

	// Did the player touch me?
	if (pOther->IsPlayer() || (pOther->VPhysicsGetObject() && (pOther->VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD)))
	{
		// Ignore if pissed at player
		if (m_afMemory & bits_MEMORY_PROVOKED)
			return;

		TestPlayerPushing((pOther->IsPlayer()) ? pOther : AI_GetSinglePlayer());
	}
}

bool CNPC_Soldier::CanSpeakResponse()
{
	if (GetState() == NPC_STATE_DEAD)
		return false;

	if (GetState() == NPC_STATE_COMBAT)
		return false;

	if (GetState() == NPC_STATE_SCRIPT)
		return false;

	if (m_flTimeNextUse >= gpGlobals->curtime)
		return false;

	return true;
}

void CNPC_Soldier::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// if this is the player and we can speak - say something!
	if (pActivator == UTIL_GetLocalPlayer() && CanSpeakResponse())
	{
		int index = m_Sentences.Speak("SOLDIER_RESPONSE", SENTENCE_PRIORITY_MEDIUM, SENTENCE_CRITERIA_ALWAYS);
		m_flTimeNextUse = gpGlobals->curtime + engine->SentenceLength(index) + 2.5f;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Soldier::Event_Killed(const CTakeDamageInfo &info)
{
	// if I was killed before I could finish throwing my grenade, drop
	// a grenade item that the player can retrieve.
	if (GetActivity() == ACT_RANGE_ATTACK2)
	{
		if (m_iLastAnimEventHandled != SOLDIER_AE_GREN_TOSS)
		{
			// Drop the grenade as an item.
			Vector vecStart;
			GetAttachment("lefthand", vecStart);

			CBaseEntity *pItem = DropItem("weapon_m67", vecStart, RandomAngle(0, 360));

			if (pItem)
			{
				IPhysicsObject *pObj = pItem->VPhysicsGetObject();

				if (pObj)
				{
					Vector			vel;
					vel.x = random->RandomFloat(-100.0f, 100.0f);
					vel.y = random->RandomFloat(-100.0f, 100.0f);
					vel.z = random->RandomFloat(800.0f, 1200.0f);
					AngularImpulse	angImp = RandomAngularImpulse(-300.0f, 300.0f);

					vel[2] = 0.0f;
					pObj->AddVelocity(&vel, &angImp);
				}
			}
		}
	}

	CBasePlayer *pPlayer = ToBasePlayer(info.GetAttacker());
	if (pPlayer != NULL)
	{
		CHalfLife2 *pHL2GameRules = static_cast<CHalfLife2 *>(g_pGameRules);
		if (HasSpawnFlags(SF_COMBINE_NO_GRENADEDROP) == false)
		{
			// Attempt to drop a grenade
			if (pHL2GameRules->NPC_ShouldDropGrenade(pPlayer))
			{
				DropItem("weapon_m67", WorldSpaceCenter() + RandomVector(-4, 4), RandomAngle(0, 360));
				pHL2GameRules->NPC_DroppedGrenade();
			}
		}
	}

	// NOTE(wheatley): we have to bypass all standard combine definitions of this event because two
	// of our closest parent classes (CNPC_CombineS and CNPC_Combine) define unappropriate for this
	// NPC logic, like dropping AR2 altfire rounds, healthvials and grenades.
	CAI_BaseActor::Event_Killed(info);
}

void CNPC_Soldier::HandleAnimEvent(animevent_t *pEvent)
{
	bool handledEvent = false;
	switch (pEvent->event)
	{
		case SOLDIER_AE_GREN_TOSS:
		{
			Vector vecSpin;
			vecSpin.x = 1000.0f;
			vecSpin.y = 1000.0f;
			vecSpin.z = random->RandomFloat(-1000.0, 1000.0);

			Vector vecStart;
			GetAttachment("lefthand", vecStart);

			if (m_NPCState == NPC_STATE_SCRIPT)
			{
				// Use a fixed velocity for grenades thrown in scripted state.
				// Grenades thrown from a script do not count against grenades remaining for the AI to use.
				Vector forward, up, vecThrow;

				GetVectors(&forward, NULL, &up);
				vecThrow = forward * 750 + up * 175;
				GrenadeM67_Create(vecStart, vec3_angle, vecThrow, vecSpin, this, SOLDIER_GRENADE_TIMER, true);
			}
			else
			{
				// Use the Velocity that AI gave us.
				Fraggrenade_Create(vecStart, vec3_angle, m_vecTossVelocity, vecSpin, this, SOLDIER_GRENADE_TIMER, true);
				m_iNumGrenades--;
			}

			// wait six seconds before even looking again to see if a grenade can be thrown.
			m_flNextGrenadeCheck = gpGlobals->curtime + 6;
		}
		handledEvent = true;
		break;

		case SOLDIER_AE_GREN_LAUNCH:
		{
			EmitSound("NPC_Combine.GrenadeLaunch");

			CBaseEntity *pGrenade = CreateNoSpawn("npc_contactgrenade", Weapon_ShootPosition(), vec3_angle, this);
			pGrenade->KeyValue("velocity", m_vecTossVelocity);
			pGrenade->Spawn();
			pGrenade->SetModel("models/weapons/w_m67.mdl");

			if (g_pGameRules->IsSkillLevel(SKILL_HARD))
				m_flNextGrenadeCheck = gpGlobals->curtime + random->RandomFloat(2, 5);// wait a random amount of time before shooting again
			else
				m_flNextGrenadeCheck = gpGlobals->curtime + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
		}
		handledEvent = true;
		break;

		case SOLDIER_AE_GREN_DROP:
		{
			Vector vecStart;
			GetAttachment("lefthand", vecStart);

			Fraggrenade_Create(vecStart, vec3_angle, m_vecTossVelocity, vec3_origin, this, SOLDIER_GRENADE_TIMER, true);
			m_iNumGrenades--;
		}
		handledEvent = true;
		break;

		default:
			BaseClass::HandleAnimEvent(pEvent);
			break;
	}

	if (handledEvent)
	{
		m_iLastAnimEventHandled = pEvent->event;
	}
}

void CNPC_Soldier::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_SOLDIER_RPG_AUGER:
		m_bRPGAvoidPlayer = false;
		SetWait(15.0); // maximum time auger before giving up
		break;
	default:
		BaseClass::StartTask(pTask);
	}
}

void CNPC_Soldier::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
		case TASK_SOLDIER_RPG_AUGER:
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

int CNPC_Soldier::TranslateSchedule(int scheduleType)
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
				return SCHED_SOLDIER_RANGE_ATTACK1_RPG;
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

Activity CNPC_Soldier::NPC_TranslateActivity(Activity activity)
{
	CBaseCombatWeapon* pWeapon = GetActiveWeapon();
	if (pWeapon && FClassnameIs(pWeapon, "weapon_M16"))
	{
		// NOTE(wheatley): Nick has tonns of clipping body parts when using m16 so translate
		// a few animations requested by m16 into more appropriate for his model
		switch (activity)
		{
		case ACT_IDLE_ANGRY_SMG1:
			return ACT_IDLE_ANGRY_AR2;

		case ACT_IDLE_SMG1_RELAXED:
			return ACT_IDLE_AR2_RELAXED;

		case ACT_IDLE_SMG1:
		case ACT_IDLE_SMG1_STIMULATED:
			return ACT_IDLE_AR2;

		case ACT_WALK_RIFLE_RELAXED:
			return ACT_WALK_AR2_RELAXED;

		case ACT_WALK_RIFLE_STIMULATED:
			return ACT_WALK_AR2_STIMULATED;

		case ACT_WALK_AIM_RIFLE:
			return ACT_WALK_AIM_AR2;

		case ACT_RUN_RIFLE_RELAXED:
			return ACT_RUN_AR2_RELAXED;

		case ACT_RUN_RIFLE_STIMULATED:
			return ACT_RUN_AR2_STIMULATED;

		case ACT_RUN_AIM_RIFLE:
			return ACT_RUN_AIM_AR2_STIMULATED;

		case ACT_RANGE_ATTACK1:
			return ACT_RANGE_ATTACK_AR2;
		}
	}

	return BaseClass::NPC_TranslateActivity(activity);
}

//Activity CNPC_Soldier::NPC_TranslateActivity(Activity activity)
//{
//	if (activity == ACT_RANGE_ATTACK1)
//		return ACT_RANGE_ATTACK_AR2;
//
//	return BaseClass::NPC_TranslateActivity(activity);
//}

AI_BEGIN_CUSTOM_NPC(npc_soldier, CNPC_Soldier)

	DECLARE_TASK(TASK_SOLDIER_RPG_AUGER)

	//=========================================================
	// > SCHED_CITIZEN_RANGE_ATTACK1_RPG
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_SOLDIER_RANGE_ATTACK1_RPG,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_FACE_ENEMY				0"
		"		TASK_ANNOUNCE_ATTACK		1"	// 1 = primary attack
		"		TASK_RANGE_ATTACK1			0"
		"		TASK_SOLDIER_RPG_AUGER		1"
		"		TASK_WAIT				  2.5"
		""
		"	Interrupts"
	)

AI_END_CUSTOM_NPC()
