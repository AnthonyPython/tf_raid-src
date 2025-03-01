#pragma once

class IDiscordRPCGameSystem : public CAutoGameSystem
{
	virtual bool Init() = 0;
	virtual void PostInit() = 0;
	virtual void Shutdown() = 0;

	virtual void LevelInitPostEntity() = 0;
	virtual void LevelShutdownPreEntity() = 0;
private:
	bool m_bDiscordInited;
};

class CDiscordRPCGameSystem : public IDiscordRPCGameSystem
{
	bool Init();
	void PostInit();
	void Shutdown();

	void LevelInitPostEntity();
	void LevelShutdownPreEntity();
private:
	bool m_bDiscordInited;
};

extern IDiscordRPCGameSystem *g_DiscordRPC;

#define DISCORD_DYNAMIC_LIB

#pragma comment(lib, "discord-rpc.lib")

void __Discord_Update_InGame(char *details, char *state, char *largeImageKey, char *largeImageText);