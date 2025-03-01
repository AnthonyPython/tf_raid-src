#include "cbase.h"
#include "npc_combines.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_npc_synth_health("sk_npc_synth_health", "120", FCVAR_ARCHIVE);

ConVar	sk_npc_synth_death_on_headshot("sk_npc_synth_death_on_headshot", "0", FCVAR_ARCHIVE);

#define SYNTH_UNOPTIMIZED

//=========================================================
//	>> CNPC_Synth
//=========================================================
class CNPC_Synth : public CNPC_CombineS
{
	DECLARE_CLASS(CNPC_Synth, CNPC_CombineS);

public:
	virtual void Precache();
	virtual void Spawn();

	void		 DeathSound(const CTakeDamageInfo &info);
	void		 PainSound(const CTakeDamageInfo &info);
	void		 IdleSound();
	void		 AlertSound() {}
	void		 LostEnemySound() {}
	void		 FoundEnemySound() {}
	void		 AnnounceAssault() {}
	void		 AnnounceEnemyType(CBaseEntity *pEnemy) {}
	void		 AnnounceEnemyKill(CBaseEntity *pEnemy) {}
	void		 NotifyDeadFriend(CBaseEntity* pFriend) {}
	int			 OnTakeDamage_Alive(const CTakeDamageInfo& info);

	bool		IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos) const;

	bool   ShouldHeadshotKill() const { return sk_npc_synth_death_on_headshot.GetBool(); }
	//null char
	const char* pnull = "\0";
	const char* pSentenceName = NULL;

	virtual int	PlaySentence(CombineSentence_t sentence,
		SentencePriority_t priority = SENTENCE_PRIORITY_NORMAL,
		SentenceCriteria_t criteria = SENTENCE_CRITERIA_IN_SQUAD);

	void Event_Killed(const CTakeDamageInfo &info);
};

LINK_ENTITY_TO_CLASS(npc_synth, CNPC_Synth);

void CNPC_Synth::Spawn()
{
	BaseClass::Spawn();

	SetHealth(sk_npc_synth_health.GetInt());
	SetMaxHealth(sk_npc_synth_health.GetInt());
	SetBloodColor(BLOOD_COLOR_MECH);

	// HACK(wheatley): synth should never drop a grenade because it uses a very custom grenade
	// which is unavailable to the players!
	AddSpawnFlags(SF_COMBINE_NO_GRENADEDROP);

	SetModelScale(1.85f, 0.f);
	m_nSkin = RandomInt(0, 2);
}

void CNPC_Synth::Precache()
{
	if (!GetModelName())
		SetModelName(MAKE_STRING("models/hdtf/npcs/synth.mdl"));

	PrecacheModel("models/hdtf/npcs/synth_ragdoll.mdl");

	BaseClass::Precache();
}

void CNPC_Synth::DeathSound(const CTakeDamageInfo &info)
{
	if (V_strcmp(m_strSpeechDie.ToCStr(), pnull))
	{
		pSentenceName = STRING(m_strSpeechDie);
	}
	else
	{
		pSentenceName = "SYNTH_DIE";
	}
	GetSentences()->Speak(pSentenceName, SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS);
}

void CNPC_Synth::PainSound(const CTakeDamageInfo &info)
{
	if (GetFlags() & FL_DISSOLVING)
		return;

	if (gpGlobals->curtime > m_flNextPainSoundTime)
	{
		if (V_strcmp(m_strSpeechPain.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechDie);
		}
		else
		{
			pSentenceName = "SYNTH_PAIN";
		}
		GetSentences()->Speak(pSentenceName, SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS);
		m_flNextPainSoundTime = gpGlobals->curtime + 1;
	}
}

void CNPC_Synth::IdleSound()
{
	if (V_strcmp(m_strSpeechIdle.ToCStr(), pnull))
	{
		pSentenceName = STRING(m_strSpeechDie);
	}
	else
	{
		pSentenceName = "SYNTH_IDLE";
	}
	GetSentences()->Speak(pSentenceName, SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS);
}

void CNPC_Synth::Event_Killed(const CTakeDamageInfo &info)
{
	const int seq = GetSequence();
	const float cycle = GetCycle();

	SetModel("models/hdtf/npcs/synth_ragdoll.mdl");
	SetSequence(seq);
	SetCycle(cycle);

#ifdef SYNTH_UNOPTIMIZED
	// currently, our synth model is way too highpoly and has no LODs.
	// leaving this guys on the ground after death is total fps waste
	// undef SYNTH_UNOPTIMIZED when we have LODs on them.
	CTakeDamageInfo dmg(info);
	//dmg.SetDamageType(DMG_DISSOLVE);
	dmg.SetDamageType(DMG_NEVERGIB);	//m3sa hack
	BaseClass::Event_Killed(dmg);
#else
	BaseClass::Event_Killed(info);
#endif
}

int CNPC_Synth::OnTakeDamage_Alive(const CTakeDamageInfo& info)
{

	//Kill the synth if they get shot by a sniper
	//-Nbc66
	if (info.GetDamageType() & DMG_SNIPER)
	{
		CTakeDamageInfo dmg(info);
		dmg.SetDamage(GetMaxHealth());
		return BaseClass::OnTakeDamage_Alive(dmg);
	}
	return BaseClass::OnTakeDamage_Alive(info);
}

bool CNPC_Synth::IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos) const
{
	float MAX_JUMP_RISE = 220.0f;
	float MAX_JUMP_DISTANCE = 256.0f;
	float MAX_JUMP_DROP = 384.0f;

	trace_t tr;
	UTIL_TraceHull(startPos, startPos, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);
	if (tr.startsolid)
	{
		// Trying to start a jump in solid! Consider checking for this in CAI_MoveProbe::JumpMoveLimit.
		Assert(0);
		return false;
	}

	if (BaseClass::IsJumpLegal(startPos, apex, endPos, MAX_JUMP_RISE, MAX_JUMP_DROP, MAX_JUMP_DISTANCE))
	{
		return true;
	}
	return false;
}

int CNPC_Synth::PlaySentence(CombineSentence_t sentence, SentencePriority_t priority, SentenceCriteria_t criteria)
{
	
	
	switch (sentence)
	{
	case COMBINE_SPEECH_DIE:
		if (V_strcmp(m_strSpeechDie.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechDie);
		}
		else
		{
			pSentenceName = "SYNTH_DIE";
		}

		break;

	case COMBINE_SPEECH_PAIN:
		if (V_strcmp(m_strSpeechPain.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechPain);
		}
		else
		{
			pSentenceName = "SYNTH_PAIN";
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
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_REFIND_ENEMY:
		if (V_strcmp(m_strSpeechRefind_Enemy.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechRefind_Enemy);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_MONST_CITIZENS:
		if (V_strcmp(m_strSpeechMonst_Citizens.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechMonst_Citizens);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_MONST_CHARACTER:
		if (V_strcmp(m_strSpeechMonst_Character.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechMonst_Character);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_ALERT:
		if (V_strcmp(m_strSpeechAlert.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechAlert);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_ANNOUNCE:
		if (V_strcmp(m_strSpeechAnnounce.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechAnnounce);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_PLAYERHIT:
		if (V_strcmp(m_strSpeechPlayerHit.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechPlayerHit);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;
	case COMBINE_SPEECH_KILL_MONST:
		if (V_strcmp(m_strSpeechKillMonst.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechKillMonst);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;
	case COMBINE_SPEECH_KILL_PLAYER:
		if (V_strcmp(m_strSpeechKillPlayer.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechKillPlayer);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_ASSAULT:
		if (V_strcmp(m_strSpeechAssault.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechAssault);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_MONST:
		if (V_strcmp(m_strSpeechMonst.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechMonst);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_MONST_BUGS:
		if (V_strcmp(m_strSpeechMonstBugs.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechMonstBugs);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_PARASITES:
		if (V_strcmp(m_strSpeechParasites.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechParasites);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_MONST_ZOMBIES:
		if (V_strcmp(m_strSpeechMonstZombies.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechMonstZombies);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_DANGER:
		if (V_strcmp(m_strSpeechDanger.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechDanger);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_GREN:
		if (V_strcmp(m_strSpeechGren.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechGren);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_MAN_DOWN:
		if (V_strcmp(m_strSpeechManDown.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechManDown);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_GO_ALERT:
		if (V_strcmp(m_strSpeechGoAlert.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechGoAlert);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;
	case COMBINE_SPEECH_FLANK:
		if (V_strcmp(m_strSpeechFlank.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechFlank);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_COVER:
		if (V_strcmp(m_strSpeechCover.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechCover);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_LOST_LONG:
		if (V_strcmp(m_strSpeechLostLong.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechLostLong);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;
	case COMBINE_SPEECH_LOST_SHORT:
		if (V_strcmp(m_strSpeechLostShort.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechLostShort);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_LAST_OF_SQUAD:
		if (V_strcmp(m_strSpeechLastOfSquad.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechLastOfSquad);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_IDLE:
		if (V_strcmp(m_strSpeechIdle.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechIdle);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	case COMBINE_SPEECH_TAUNT:
		if (V_strcmp(m_strSpeechTaunt.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechTaunt);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;
	case COMBINE_SPEECH_CHECK:
		if (V_strcmp(m_strSpeechCheck.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechCheck);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;
	case COMBINE_SPEECH_QUEST:
		if (V_strcmp(m_strSpeechQuest.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechQuest);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;
	case COMBINE_SPEECH_CLEAR:
		if (V_strcmp(m_strSpeechClear.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechClear);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;
	case COMBINE_SPEECH_ANSWER:
		if (V_strcmp(m_strSpeechAnswer.ToCStr(), pnull))
		{
			pSentenceName = STRING(m_strSpeechAnswer);
		}
		else
		{
			pSentenceName = "SYNTH_IDLE";
		}

		break;

	default:
		break;
	}
	

	if (pSentenceName == NULL)
		return 0;

	return m_Sentences.Speak(pSentenceName, priority, criteria);
}
