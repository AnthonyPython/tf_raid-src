#include "cbase.h"
#include "discord_rpc.h"
#include "discord-rpc.h"
#include "usermessages.h"

static CDiscordRPCGameSystem __g_discordrpc;
IDiscordRPCGameSystem *g_DiscordRPC = &__g_discordrpc; 

static DiscordEventHandlers __g_DiscordEventHandlers;

struct ChapterInfo_s {
	const char* ChapterName;
};

struct MapInfo_s {
	const char* MapName;
	int Chapter;
	const char* SubTitle;
	bool isMenuLevel;
};

static long startTimestamp = 0;

void __Discord_Ready()
{
	Msg("[Discord] Ready!\n");
}

void __Discord_Disconnected(int errorCode, const char* message)
{
	Error("[Discord] Error (%d): %s\n", errorCode, message);
}

void __Discord_Errored(int errorCode, const char* message)
{
	Error("[Discord] Error (%d): %s\n", errorCode, message);
}

void __Discord_Update_Menu()
{
	DiscordRichPresence dP;
	memset(&dP, 0, sizeof(dP));
	dP.state = "In Game";
	dP.largeImageKey = "modimagehere";
	Discord_UpdatePresence(&dP);
}

void UserMessage_Discord_Update(bf_read &msg)
{
	int act, chapter;
	char chapterName[256];

	bool useCustomString;
	char details[1024];
	char state[1024];

	char largeImageKey[64];
	char largeImageText[64];

	act = msg.ReadWord();
	chapter = msg.ReadWord();
	bool bStr = msg.ReadString(chapterName, 256);
	Assert(bStr);
	useCustomString = msg.ReadWord() > 0 ? true : false;
	bStr = msg.ReadString(details, 1024);
	Assert(bStr);
	bStr = msg.ReadString(state, 1024);
	Assert(bStr);

	bStr = msg.ReadString(largeImageKey, 64);
	bStr = msg.ReadString(largeImageText, 64);

	if (!useCustomString)
	{
		Q_snprintf(details, 1024, "Chapter %d: %s", chapter, chapterName);
		Q_snprintf(state, 1024, "Playing Act %d", act);
	}

	__Discord_Update_InGame(details, state, largeImageKey, largeImageText);
}

void __Discord_Update_InGame(char *details, char *state, char *largeImageKey, char *largeImageText)
{
	DiscordRichPresence dP;
	memset(&dP, 0, sizeof(dP));

	dP.largeImageKey = largeImageKey;
	dP.largeImageText = largeImageText;

	dP.startTimestamp = startTimestamp;

	dP.details = details;
	dP.state = state;


	Discord_UpdatePresence(&dP);
}

bool CDiscordRPCGameSystem::Init()
{
	m_bDiscordInited = true;
	g_pVCR->Hook_Time(&startTimestamp);
	memset(&__g_DiscordEventHandlers, 0, sizeof(__g_DiscordEventHandlers));
	__g_DiscordEventHandlers.ready = &__Discord_Ready;
	__g_DiscordEventHandlers.errored = &__Discord_Errored;
	__g_DiscordEventHandlers.disconnected = &__Discord_Disconnected;
	
	Discord_Initialize("501128820703887371", &__g_DiscordEventHandlers, 1, "723390");

	usermessages->HookMessage("DiscordUpdate", &UserMessage_Discord_Update);

	return true;
}

void CDiscordRPCGameSystem::PostInit()
{
	__Discord_Update_Menu();
}

void CDiscordRPCGameSystem::Shutdown()
{
	Discord_Shutdown();
}

void CDiscordRPCGameSystem::LevelInitPostEntity()
{
	if (!engine->IsLevelMainMenuBackground())
	{
		// Wait for a level ent to update our stuff
	}
	else {
		__Discord_Update_Menu();
	}
}

void CDiscordRPCGameSystem::LevelShutdownPreEntity()
{
	__Discord_Update_Menu();
}
