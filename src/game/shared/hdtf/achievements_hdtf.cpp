//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================


#include "cbase.h"

#ifdef GAME_DLL

#include "achievementmgr.h"
#include "baseachievement.h"
#include "iservervehicle.h"
#include "npc_antlion.h"
#include "npc_hunter.h"
#include "filesystem.h"
#include "steam_locator.h"
#include <ctime>
#include "engine/IEngineSound.h"
#include "hdtf_player.h"

// HDTF specific map event
#define DECLARE_HDTF_MAP_EVENT_ACHIEVEMENT( achievementID, achievementName, iPointValue )					\
	DECLARE_MAP_EVENT_ACHIEVEMENT_( achievementID, achievementName, "\0", iPointValue, false )

#define DECLARE_HDTF_MAP_EVENT_ACHIEVEMENT_HIDDEN( achievementID, achievementName, iPointValue )					\
	DECLARE_MAP_EVENT_ACHIEVEMENT_( achievementID, achievementName, "\0", iPointValue, true )

// shorter version
#define ACH_MAP_EVENT( achievementName, hidden ) \
	DECLARE_MAP_EVENT_ACHIEVEMENT_( achievementName, "ACHIEVEMENT_EVENT_" #achievementName, "\0", 10, hidden )


// we use enums now instead of preporcessor defines cause we can easily add way more ids faster
// then to do ex: #define Ach_something 232
// -Nbc66
enum AchievemntID
{
	START_ACH_ID = 199,
	HDTF_LOSS_SCARS,
	HDTF_ELEVATOR,
	HDTF_QUIT_GAME,
	HDTF_NO_KNIFE,
	HDTF_COOL_STICK,
	HDTF_CONE,	
	HDTF_DEVY,
	HDTF_BOOMSTICK,
	HDTF_ENDING_A,
	HDTF_ENDING_B,
	HDTF_MESS,
	HDTF_BOOT,	
	HDTF_IGWT,
	HDTF_SV_CHEATS,
	HDTF_GUNSHIP_DOWN,
	HDTF_TWO_HOURS,
	HDTF_DIE_DURING_TUTORIAL,
	HDTF_BREAK_GLASS_OFFICE,
	HDTF_DEATH_BY_CLAYMORE,
	HDTF_SHUT_UP_LARRY,		 
	HDTF_BEAT_ACT_2,		 
	HDTF_CREW_GAME_ENDED,	 
	HDTF_THE_WRONG_WAY,		 
	HDTF_DR_BREEN,			 
	HDTF_BME_CLEAR,			 
	HDTF_GET_CRUSHED_RAVEN,	 
	HDTF_PLAY_TWO_HOURS,		 
	HDTF_CRASH,				 
	HDTF_START_GAME,			 
	HDTF_PLAY_AT_3AM,		 
	HDTF_JOE,
	HDTF_GIVE_A_LITTLE_BIT,
	HDTF_100_PERCENT, 
	MAX_ACH_ID //MUST ALLWAYS BE THE LAST ONE IN THE ENUM - Nbc66
};
int NumAchivementIDs = (MAX_ACH_ID - START_ACH_ID) - 2;

// Not Used Anymore - Nbc66
/*these IDs don't have anything to do with the ones set in Steamworks
#define HDTF_LOSS_SCARS				200
#define HDTF_ELEVATOR				201
#define HDTF_QUIT_GAME				202
#define HDTF_NO_KNIFE				203
#define HDTF_COOL_STICK				204
#define HDTF_REFUND					205
#define HDTF_CONE					206
#define HDTF_DEVY					207
#define HDTF_BOOMSTICK				208
#define HDTF_ENDING_A				209
#define HDTF_ENDING_B				210
#define HDTF_MESS					211
#define HDTF_BOOT					212
#define HDTF_SV_CHEATS				213
#define HDTF_IGWT					214
#define HDTF_GUNSHIP_DOWN			215
#define HDTF_ALASKAN_CHAIR			216
#define HDTF_SPEED_CHAIR			217
#define HDTF_TWO_HOURS				218
#define HDTF_DIE_DURING_TUTORIAL	219
#define HDTF_BREAK_GLASS_OFFICE		220
#define HDTF_DEATH_BY_CLAYMORE		221
#define HDTF_SHUT_UP_LARRY			222
#define HDTF_BEAT_ACT_2				223
#define HDTF_CREW_GAME_ENDED		224
#define HDTF_THE_WRONG_WAY			225
#define HDTF_DR_BREEN				226
#define HDTF_BME_CLEAR				227
#define HDTF_GET_CRUSHED_RAVEN		228
#define HDTF_PLAY_TWO_HOURS			229
#define HDTF_CRASH					230
#define HDTF_START_GAME				231
#define HDTF_PLAY_AT_3AM			232
#define HDTF_JOE					233
*/

// achievements which are won by a map event firing once
ACH_MAP_EVENT(HDTF_LOSS_SCARS,				true);
ACH_MAP_EVENT(HDTF_ELEVATOR,				true);
ACH_MAP_EVENT(HDTF_QUIT_GAME,				true);
ACH_MAP_EVENT(HDTF_NO_KNIFE,				true);
ACH_MAP_EVENT(HDTF_COOL_STICK,				true);
ACH_MAP_EVENT(HDTF_CONE,					true);
ACH_MAP_EVENT(HDTF_DEVY,					true);
ACH_MAP_EVENT(HDTF_BOOMSTICK,				true);
ACH_MAP_EVENT(HDTF_ENDING_A,				true);
ACH_MAP_EVENT(HDTF_ENDING_B,				true);
ACH_MAP_EVENT(HDTF_MESS,					true);
ACH_MAP_EVENT(HDTF_BOOT,					true);
ACH_MAP_EVENT(HDTF_SV_CHEATS,				true);
ACH_MAP_EVENT(HDTF_IGWT,					true);
ACH_MAP_EVENT(HDTF_GUNSHIP_DOWN,			true);
ACH_MAP_EVENT(HDTF_DIE_DURING_TUTORIAL,		true);
ACH_MAP_EVENT(HDTF_BREAK_GLASS_OFFICE,		true);
ACH_MAP_EVENT(HDTF_DEATH_BY_CLAYMORE,		true);
ACH_MAP_EVENT(HDTF_SHUT_UP_LARRY,			true);
ACH_MAP_EVENT(HDTF_BEAT_ACT_2,				true);
ACH_MAP_EVENT(HDTF_CREW_GAME_ENDED,			true);
ACH_MAP_EVENT(HDTF_THE_WRONG_WAY,			true);
ACH_MAP_EVENT(HDTF_DR_BREEN,				true);
ACH_MAP_EVENT(HDTF_BME_CLEAR,				true);
ACH_MAP_EVENT(HDTF_GET_CRUSHED_RAVEN,		true);
ACH_MAP_EVENT(HDTF_JOE,						true);
ACH_MAP_EVENT(HDTF_GIVE_A_LITTLE_BIT,		true);

#define HDTF_TWO_HOURS_IN_SECONDS (60 * 60 * 2)

class CAchievementHDTFPlayForTwoHours : public CBaseAchievement
{
	DECLARE_CLASS(CAchievementHDTFPlayForTwoHours, CBaseAchievement);

public:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1);

		m_flTotalGameTime = 0.f;
	}

	virtual void ListenForEvents()
	{
		if (IsActive())
		{
			SetNextThink(0.f);
		}
	}

	virtual bool ShouldSaveGlobal()
	{
		return (((m_iFlags & ACH_SAVE_GLOBAL) > 0) || IsAchieved() || (m_iProgressShown > 0) || ShouldShowOnHUD());
	}

	virtual void Think()
	{
		BaseClass::Think();

		m_flTotalGameTime += gpGlobals->frametime;

		if (m_flTotalGameTime >= HDTF_TWO_HOURS_IN_SECONDS)
		{
			IncrementCount();
			return;
		}

		SetNextThink(0.f);
	}

	virtual void GetSettings(KeyValues *pNodeOut)
	{
		BaseClass::GetSettings(pNodeOut);

		pNodeOut->SetFloat("totaltime", m_flTotalGameTime);
	}

	virtual void ApplySettings( /* const */ KeyValues *pNodeIn)
	{
		BaseClass::ApplySettings(pNodeIn);

		m_flTotalGameTime = pNodeIn->GetFloat("totaltime");
	}

	float GetElapsedTime() { return m_flTotalGameTime; }

protected:
	float m_flTotalGameTime;
};

DECLARE_ACHIEVEMENT(CAchievementHDTFPlayForTwoHours, HDTF_PLAY_TWO_HOURS, "ACHIEVEMENT_HDTF_PLAY_TWO_HOURS", 5);

#ifdef GAME_DLL

class CAchievementHDTFPlayAt3AM : public CBaseAchievement
{
	DECLARE_CLASS(CAchievementHDTFPlayAt3AM, CBaseAchievement);

public:

	time_t ltime;
	const time_t* ptime;
	struct tm* today;

	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1);

		ltime = time(0);
		ptime = &ltime;
		today = localtime(ptime);

		SetNextThink(10.0f);
	
	}

	//The Think Function for Achievements only gets triggerd while in game
	//so you actualy have to play the game to get it 
	//-Nbc66
	virtual void Think()
	{
		BaseClass::Think();

		CHDTF_Player* pPlayer = ToHDTFPlayer(UTIL_GetLocalPlayer());

		if (pPlayer)
		{
			pPlayer->PrecacheScriptSound("General.Spooky");
		}

		if (today->tm_hour == 3 && !IsAchieved())
		{

			pPlayer->EmitSound("General.Spooky");

			AwardAchievement();

			return;
		}

		SetNextThink(10.0f);
	}

protected:
	float m_flTotalGameTime;
};

#endif

DECLARE_ACHIEVEMENT(CAchievementHDTFPlayAt3AM, HDTF_PLAY_AT_3AM, "ACHIEVEMENT_EVENT_HDTF_3AM", 5);

class CAchievementHDTFBeatGameUnderTwoHours : public CFailableAchievement
{
	DECLARE_CLASS(CAchievementHDTFBeatGameUnderTwoHours, CFailableAchievement);

public:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_MAP_EVENTS | ACH_SAVE_GLOBAL | ACH_HAS_COMPONENTS);

		static const char *szComponents[] =
		{
			"a1c1p1_hospital_1", "a1c1p2_hospital_2", "a1c2p1_downtown_1", "a1c2p2_downtown_2", "a1c2p3_office",
			"a1c3p1_canal", "a1c3p2_urban", "a1c3p3_metro_1", "a1c3p4_metro_2", "a1c3p5_metro_3", "a1c3p6_station",
			"a1c4p2_mesa", "a1c4p3_outlands_1", "a1c4p4_factory", /*"a1c4p5_outlands_2",*/ "a1c5p1_pier_1",
			"a1c5p2_pier_2", "a1c5p4_ship", "a2c1p1_alaska", "a2c1p4_distribution_center", "a2c2p1_lasers",
			"a2c2p2_factory", "a3c1p1_ship", "a3c1p2_jump", "a3c2p1_ci17-1", "a3c2p2_ci17-2", "a3c2p3_ci17-3",
			"a3c2p4_boris", "a3c3p1_blackmesaeast", "a3c3p2_ravenholm", "a3c3p3_tunnel", "a3c3p4_streetfight",
			"a3c4p1_cave", "a3c5p1_badcountry", "a3c5p2_highway", "a3c6p1_novapro", "a3c6p3_novapro",
			"a3c7p1_prison_1", "a3c7p2_prison_2", "a3c8p1_forest"
		};

		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE(szComponents);

		// NOTE(wheatley): not only we have to satisfy all the components but also beat the game in time
		// so our goal is num of components + beat in time
		SetGoal(m_iNumComponents + 1);
	}

	virtual void ListenForEvents()
	{
		// NOTE(wheatley): this method is conveniently called on level init which is the
		// perfect time to restart our think function if we should
		if (m_bActivated && !m_bFailed)
		{
			SetNextThink(0.f);
		}

		// this is also a convenient time to mark a map completed
		OnComponentEvent(m_pAchievementMgr->GetMapName());
	}

	virtual bool ShouldSaveGlobal()
	{
		return (((m_iFlags & ACH_SAVE_GLOBAL) > 0) || IsAchieved() || (m_iProgressShown > 0) || ShouldShowOnHUD());
	}

	virtual void Think()
	{
		BaseClass::Think();

		m_flTotalGameTime += gpGlobals->frametime;

		if (m_flTotalGameTime >= HDTF_TWO_HOURS_IN_SECONDS)
		{
			m_flTotalGameTime = HDTF_TWO_HOURS_IN_SECONDS;
			SetFailed();
			return;
		}

		SetNextThink(0.f);
	}

	float GetElapsedTime() { return m_flTotalGameTime; }

	virtual void OnMapEvent(const char *pEventName)
	{
		// NOTE(wheatley): overriden because we don't really care if we were activated before - any
		// activation event should reset the achievement
		if ((0 == Q_stricmp(pEventName, GetActivationEventName())))
		{
			OnActivationEvent();
		}
		else if (m_bActivated && 0 == Q_stricmp(pEventName, GetEvaluationEventName()))
		{
			OnEvaluationEvent();
		}
	}

	virtual void OnActivationEvent()
	{
		m_flTotalGameTime = 0.f;
		m_bFailed = false;
		SetComponentBits(0);

		BaseClass::OnActivationEvent();
	}

	virtual void OnEvaluationEvent()
	{
		BaseClass::OnEvaluationEvent();

		m_bActivated = false;
		ClearThink();
	}

	virtual void PreRestoreSavedGame()
	{
	}

	virtual void GetSettings(KeyValues *pNodeOut)
	{
		BaseClass::GetSettings(pNodeOut);

		pNodeOut->SetBool("isactive", m_bActivated);
		pNodeOut->SetBool("isfailed", m_bFailed);
		pNodeOut->SetFloat("totaltime", m_flTotalGameTime);
	}

	virtual void ApplySettings( /* const */ KeyValues *pNodeIn)
	{
		BaseClass::ApplySettings(pNodeIn);

		m_bActivated = pNodeIn->GetBool("isactive");
		m_bFailed = pNodeIn->GetBool("isfailed");
		m_flTotalGameTime = pNodeIn->GetFloat("totaltime");
	}

	virtual bool ShouldShowProgressNotification() { return false; }

protected:
	virtual void Activate()
	{
		BaseClass::Activate();

		SetNextThink(0.f);
	}

	// map event where achievement is activated
	virtual const char *GetActivationEventName() { return "ACHIEVEMENT_EVENT_HDTF_UNDER_TWO_HOURS_START"; }
	// map event where achievement is evaluated for success
	virtual const char *GetEvaluationEventName() { return "ACHIEVEMENT_EVENT_HDTF_UNDER_TWO_HOURS_END"; }

	float m_flTotalGameTime;
};

DECLARE_ACHIEVEMENT(CAchievementHDTFBeatGameUnderTwoHours, HDTF_TWO_HOURS, "ACHIEVEMENT_HDTF_BEAT_UNDER_TWO_HOURS", 40);


int n_achfinished = 0;

class CAchievementHDTF100Percent : public CBaseAchievement
{

	DECLARE_CLASS(CAchievementHDTF100Percent, CBaseAchievement);

public:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(NumAchivementIDs);

		SetNextThink(1.0f);
		AchDone = 0;
	}

	virtual void Think()
	{
		BaseClass::Think();

		CAchievementMgr* pAchievementMgr = GetAchievementMgr();
		if (!pAchievementMgr)
			return;

		
		
		for (int i = START_ACH_ID + 1; i <= MAX_ACH_ID - 1; i++)
		{
			if (pAchievementMgr->HasAchievedID(i))
			{
				++AchDone;
			}

			n_achfinished = AchDone;

			if (i == MAX_ACH_ID - 1 && AchDone < NumAchivementIDs)
			{
				//if (cc_achievement_debug.GetBool())
				//{
				//	ConDColorMsg(Color(255, 215, 0, 255), "100%% achievement failed: Finished %i/%i\n", AchDone, NumAchivementIDs);
				//}
				SetNextThink(10.0f);
				SetCount(AchDone);
				AchDone = 0;
				if (IsAchieved())
				{
					pAchievementMgr->ResetAchievement(HDTF_100_PERCENT);
				}
				break;
			}
		}

		if (!IsAchieved() && AchDone >= NumAchivementIDs)
		{
			AwardAchievement();
		}
		

	}

protected:
	int AchDone;

};

CON_COMMAND(achievement_hdtf_100_status, "check 100% achievement status")
{
	if (n_achfinished != NumAchivementIDs)
	{
		ConColorMsg(Color(255, 215, 0, 255), "100%% achievement failed: Finished %i/%i\n", n_achfinished, NumAchivementIDs);
	}
	else
	{
		ConColorMsg(Color(255, 215, 0, 255), "100%% Achievement Finished:\nFinished %i/%i\n", n_achfinished, NumAchivementIDs);
	}
}

DECLARE_ACHIEVEMENT(CAchievementHDTF100Percent, HDTF_100_PERCENT, "ACHIEVEMENT_HDTF_100_PERCENT", 100);

class CAchievementHDTFCrashTheGame : public CFailableAchievement
{
	DECLARE_CLASS(CAchievementHDTFCrashTheGame, CFailableAchievement);

public:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);

		SetGoal(1);
	}

	virtual void OnEvaluationEvent()
	{
		Activate();

#ifdef PLATFORM_WINDOWS
		char steamPath[512] = { '\0' };
		if (GetSteamLocator()->LocateSteamDumpsDir(steamPath, sizeof(steamPath)))
		{
			filesystem->AddSearchPath(steamPath, "STEAM_DUMPS");

			FileFindHandle_t searchHandle;
			const char *fileName = filesystem->FindFirstEx("crash_hdtf.exe_*.dmp", "STEAM_DUMPS", &searchHandle);
			
			if (fileName == NULL)
			{
				SetFailed();
			}

			filesystem->RemoveSearchPath(steamPath, "STEAM_DUMPS");
		}
		else
		{
			// NOTE(wheatley): I'm not sure this will ever happen normally but we're not going to award this
			// achievement if can't even find where Steam is
			SetFailed();
		}
#endif

		BaseClass::OnEvaluationEvent();
	}

protected:
	// not used but required by the interface
	virtual const char *GetActivationEventName() { return "\0"; }
	virtual const char *GetEvaluationEventName() { return "\0"; }
};

DECLARE_ACHIEVEMENT(CAchievementHDTFCrashTheGame, HDTF_CRASH, "ACHIEVEMENT_EVENT_HDTF_CRASH", 20);

class CAchievementStartGame : public CFailableAchievement
{

	DECLARE_CLASS(CAchievementStartGame, CFailableAchievement);

public:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);

		SetGoal(1);

	}

	virtual void OnEvaluationEvent()
	{
		Activate();

		BaseClass::OnEvaluationEvent();
	}

protected:
	// not used but required by the interface
	virtual const char* GetActivationEventName() { return "\0"; }
	virtual const char* GetEvaluationEventName() { return "\0"; }
};
DECLARE_ACHIEVEMENT(CAchievementStartGame, HDTF_START_GAME, "ACHIEVEMENT_EVENT_HDTF_START_GAME", 20);


CON_COMMAND(achievement_hdtf_two_hours_state, "Prints out debug info for the speedrun achievement")
{
	IAchievementMgr *pAchievementMgr = engine->GetAchievementMgr();
	if (!pAchievementMgr)
		return;

	CAchievementHDTFBeatGameUnderTwoHours *ach = static_cast<CAchievementHDTFBeatGameUnderTwoHours*>(pAchievementMgr->GetAchievementByID(HDTF_TWO_HOURS));
	if (ach)
	{
		Msg("\n===================================================\n");
		Msg("Achievement: ACHIEVEMENT_HDTF_BEAT_UNDER_TWO_HOURS\n");
		Msg("In-game time recorded: %.2fs (%.2fs remaining)\nMaps visited: %i/%i\nIs active: %i\nIs achieved: %i\n",
			ach->GetElapsedTime(), HDTF_TWO_HOURS_IN_SECONDS - ach->GetElapsedTime(),
			ach->GetCount(), ach->GetGoal() - 1,
			ach->IsActive(),
			ach->IsAchieved());
		Msg("===================================================\n");
	}

	CAchievementHDTFPlayForTwoHours *ach2 = static_cast<CAchievementHDTFPlayForTwoHours *>(pAchievementMgr->GetAchievementByID(HDTF_PLAY_TWO_HOURS));
	if (ach2)
	{
		Msg("\n===================================================\n");
		Msg("Achievement: ACHIEVEMENT_HDTF_PLAY_TWO_HOURS\n");
		Msg("In-game time recorded: %.2fs (%.2fs remaining)\nIs achieved: %i\n",
			ach2->GetElapsedTime(), HDTF_TWO_HOURS_IN_SECONDS - ach2->GetElapsedTime(),
			ach2->IsAchieved());
		Msg("===================================================\n");
	}
}

//CON_COMMAND(crash)

#endif // GAME_DLL
