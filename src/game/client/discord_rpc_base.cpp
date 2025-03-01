//=============================================================================
//
// Purpose: Discord Presence support.
//
//=============================================================================


#include "cbase.h"
#include "discord_rpc_base.h"
#include <inetchannelinfo.h>
#include "discord_rpc.h"
#include "discord_register.h"
#include <ctime>
#include "steam/isteammatchmaking.h"
#include "steam/isteamgameserver.h"
#include "steam/isteamfriends.h"
#include "steam/steam_api.h"

#include "tier0/icommandline.h"
#include <vgui/ILocalize.h>
#include <tier3/tier3.h>
#include "filesystem.h"
#include "c_playerresource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar cl_richpresence_printmsg("cl_richpresence_printmsg", "0", FCVAR_CLIENTDLL, "");
ConVar discord_rpc_disabled("discord_rpc_disabled", "0", FCVAR_ARCHIVE, "Disable Discord Rich Presence");

// update once every 10 seconds. discord has an internal rate limiter of 15 seconds as well
#define DISCORD_UPDATE_RATE 10.0f

const char* FinalCharLocalizedString;

CDiscordRPC *g_discordrpc = NULL;

//using this instead of the old method for getting a timestamp so that the timestamp starts at 0
//when we join a server
//-Nbc66
time_t timestamp = NULL;

CDiscordRPC::CDiscordRPC() : CAutoGameSystemPerFrame("CDiscordRPC")
{
	pszState = "";
	srctv_port = "";
	Q_memset(m_szLatchedMapname, 0, MAX_MAP_NAME);
	Q_memset(m_szLatchedMusicname, 0, DISCORD_FIELD_SIZE);
	m_bInitializeRequested = false;
	m_bInitialized = false;

	g_discordrpc = this;
}

CDiscordRPC::~CDiscordRPC()
{
	Discord_ClearPresence();
	Discord_Shutdown();
}

void CDiscordRPC::Shutdown()
{
	Discord_ClearPresence();
	Discord_Shutdown();
}

void CDiscordRPC::PostInit()
{
	if( discord_rpc_disabled.GetBool() )
	{
		Shutdown();
	}
	InitializeDiscord();
	m_bInitializeRequested = true;

	// make sure to call this after game system initialized
	ListenForGameEvent("server_spawn");

	//hltv ip
	//-Nbc66
	ListenForGameEvent("hltv_status");
}

void CDiscordRPC::Update(float frametime)
{
	if( discord_rpc_disabled.GetBool() )
	{
		Shutdown();
	}

	// NOTE: we want to run this even if they have use_discord off, so we can clear
	// any previous state that may have already been sent
	if( !NeedToUpdate() )
	{
		UpdateRichPresence();
	}
	Discord_RunCallbacks();

}

void CDiscordRPC::OnReady(const DiscordUser* user)
{
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Ready!\n");
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] User %s#%s - %s\n", user->username, user->discriminator, user->userId);

	g_discordrpc->Reset();
}

void CDiscordRPC::OnDiscordError(int errorCode, const char* szMessage)
{
	char buff[1024];
	Q_snprintf(buff, 1024, "[Rich Presence] Init failed. code %d - error: %s\n", errorCode, szMessage);
	Warning("%s", buff);

	// ignore 4000 until I figure out what exactly is causing it
	if (errorCode != 4000)
		g_discordrpc->m_bErrored = true;
}


void CDiscordRPC::OnJoinGame(const char* joinSecret)
{
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Join Game: %s\n", joinSecret);

	char szCommand[128];
	Q_snprintf(szCommand, sizeof(szCommand), "connect %s\n", joinSecret);
	engine->ExecuteClientCmd(szCommand);
}

//Spectating has been removed from discord, but ill still keep this here for anyone
//who needs to get the hltv ip address for whatever reason
//-Nbc66
void CDiscordRPC::OnSpectateGame(const char* spectateSecret)
{
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Spectate Game: %s\n", spectateSecret);

	char szCommand[128];
	Q_snprintf(szCommand, sizeof(szCommand), "connect %s\n", spectateSecret);
	engine->ExecuteClientCmd(szCommand);
}

void CDiscordRPC::OnJoinRequest(const DiscordUser* joinRequest)
{
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Join Request: %s#%s\n", joinRequest->username, joinRequest->discriminator);
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Join Request Accepted\n");
	Discord_Respond(joinRequest->userId, DISCORD_REPLY_YES);
}

void CDiscordRPC::SetLogo(void)
{
	const char* pszLargeImage = "";
	const char* pszLargeImageText = "";

	const char* pszSmallImage = "";
	const char* pszSmallImageText = "";


	m_sDiscordRichPresence.largeImageKey = pszLargeImage;
	m_sDiscordRichPresence.largeImageText = pszLargeImageText;
	m_sDiscordRichPresence.smallImageKey = pszSmallImage;
	m_sDiscordRichPresence.smallImageText = pszSmallImageText;
}

//Fuck me this function sucks but if you got a better solution im all ears
//Techno you better make this look like a actually good working function which can get
//Unicode characters to be converted to a char now that would be fucking epic
//but also impossible, for the time being we are stuck with this sorry
//-Nbc66
const char* CDiscordRPC::LocalizeDiscordString(const char* LocalizedString)
{
	const wchar_t* WcharLocalizedString = g_pVGuiLocalize->Find(LocalizedString);
	//char array is set this way to account for ASCII's
	//characters which are generaly 256 charachetrs with Windows-1252 8-bit charachter encoding
	//just dont fuck with the array size or you are going to have a bad time man
	//-Nbc66
	char CharLocalizedArray[256];
	g_pVGuiLocalize->ConvertUnicodeToANSI(WcharLocalizedString, CharLocalizedArray, sizeof(CharLocalizedArray));
	FinalCharLocalizedString = V_strdup(CharLocalizedArray);

	return FinalCharLocalizedString;

}

void CDiscordRPC::InitializeDiscord()
{
	DiscordEventHandlers handlers;
	Q_memset( &handlers, 0, sizeof( handlers ) );
	handlers.ready = &CDiscordRPC::OnReady;
	handlers.errored = &CDiscordRPC::OnDiscordError;
	handlers.joinGame = &CDiscordRPC::OnJoinGame;

	//Spectating was removed in 2020 just keeping it here for reference
	//-Nbc66
	handlers.spectateGame = &CDiscordRPC::OnSpectateGame;

	handlers.joinRequest = &CDiscordRPC::OnJoinRequest;

	char command[512];
	V_snprintf( command, sizeof( command ), "%s -game \"%s\" -novid -steam\n", CommandLine()->GetParm( 0 ), CommandLine()->ParmValue( "-game" ) );
	Discord_Register( DiscordAppID(), command );
	Discord_Initialize( DiscordAppID(), &handlers, false, "" );
	Reset();
	m_bInitialized = true;
}

bool CDiscordRPC::NeedToUpdate()
{
	if ( // Music and map name need to be set in order for us to return False
		(m_szLatchedMapname[0] == '\0' && m_szLatchedMusicname[0] == '\0'))
		return false;

	return gpGlobals->realtime >= m_flLastUpdatedTime + DISCORD_UPDATE_RATE;
}

void CDiscordRPC::Reset()
{
	if ( discord_rpc_disabled.GetBool() )
	{
		Shutdown();
	}

	Q_memset(&m_sDiscordRichPresence, 0, sizeof(m_sDiscordRichPresence));
	m_sDiscordRichPresence.details = "Main Menu";

	m_sDiscordRichPresence.state = m_szLatchedMusicname;

	m_sDiscordRichPresence.endTimestamp;

	if (SteamFriends != NULL)
	{
		SteamFriends()->SetRichPresence( "status", "Main Menu" );
		SteamFriends()->SetRichPresence( "connect", NULL );
		SteamFriends()->SetRichPresence( "steam_display", "Main Menu" );
		SteamFriends()->SetRichPresence( "steam_player_group", NULL );
		SteamFriends()->SetRichPresence( "steam_player_group_size", NULL );
	}

	SetLogo();

	Discord_UpdatePresence(&m_sDiscordRichPresence);
	if (!engine->IsLevelMainMenuBackground())
	{
		delete[] FinalCharLocalizedString, srctv_port;
	}
}

void CDiscordRPC::UpdatePlayerInfo()
{
	C_PlayerResource* pResource = g_PR;
	if (!pResource)
		return;

	int maxPlayers = gpGlobals->maxClients;
	int curPlayers = 0;

	const char* pzePlayerName = NULL;

	for (int i = 1; i < maxPlayers; i++)
	{
		if (pResource->IsConnected(i))
		{
			curPlayers++;
			if (pResource->IsLocalPlayer(i))
			{
				pzePlayerName = pResource->GetPlayerName(i);
			}
		}
	}

	if (m_szLatchedHostname[0] != '\0')
	{
		if (cl_richpresence_printmsg.GetBool())
		{
			ConColorMsg(Color(114, 137, 218, 255), "[Discord] sending details of\n '%s'\n", m_szServerInfo);
		}
		m_sDiscordRichPresence.partySize = curPlayers;
		m_sDiscordRichPresence.partyMax = maxPlayers;
		m_sDiscordRichPresence.state = m_szLatchedHostname;
		//m_sDiscordRichPresence.state = szStateBuffer;
	}
}

void CDiscordRPC::FireGameEvent(IGameEvent* event)
{
	const char* type = event->GetName();

	if (Q_strcmp(type, "server_spawn") == 0)
	{
		//setup the discord timestamp to be 0 when we join a server
		//-Nbc66
		timestamp = time(0);

		Q_strncpy(m_szLatchedHostname, event->GetString("hostname"), 255);

	}

	//Check the master keyvalue for the hltv servers ip
	//-Nbc66
	if (!Q_strcmp(type, "hltv_status"))
	{
		srctv_port = V_strdup(event->GetString("master"));
	}
	else
	{
		srctv_port = NULL;
	}

}

void CDiscordRPC::UpdateRichPresence()
{
	m_flLastUpdatedTime = gpGlobals->realtime;
	Discord_UpdatePresence(&m_sDiscordRichPresence);
}


void CDiscordRPC::UpdateNetworkInfo()
{
	INetChannelInfo* ni = engine->GetNetChannelInfo();
	if (ni)
	{
		//ConWarning( "tv_port = %s\n", srctv_port );

		// adding -party here because secrets cannot match the party id
		m_sDiscordRichPresence.partyId = VarArgs("%s-party", ni->GetAddress());
		m_sDiscordRichPresence.joinSecret = ni->GetAddress();
	}

	//doesn't work until i can figure out how to get the source tv ip
	//-Nbc66: somewhere around 2019 lol
	// 
	//works now lol
	//-Nbc66: 15.02.2022
	if ((srctv_port != NULL) && (srctv_port[0] == '\0'))
	{
		m_sDiscordRichPresence.spectateSecret = NULL;
	}
	else
	{
		m_sDiscordRichPresence.spectateSecret = srctv_port;
	}

}

void CDiscordRPC::LevelInitPreEntity()
{
	// we cant update our presence here, because if its the first map a client loaded,
	// discord api may not yet be loaded, so latch
	Q_strcpy(m_szLatchedMapname, MapName());
	//V_snprintf(szStateBuffer, sizeof(szStateBuffer), "MAP: %s", m_szLatchedMapname);
	// important, clear last update time as well
	m_flLastUpdatedTime = MAX(0, gpGlobals->realtime - DISCORD_UPDATE_RATE);

	Reset();
}

void CDiscordRPC::LevelShutdownPostEntity()
{
	m_flLastUpdatedTime = MAX(0, gpGlobals->realtime - DISCORD_UPDATE_RATE);
	Reset();
}