#include "cbase.h"
#include "discord_rpc_shared.h"

#ifdef GAME_DLL

CDiscordRPEntity::CDiscordRPEntity()
{

}

void CDiscordRPEntity::Spawn(void)
{

}

void CDiscordRPEntity::Precache(void)
{

}

void CDiscordRPEntity::InputUpdatePresence(inputdata_t & inputdata)
{
	CRecipientFilter filter;
	filter.AddAllPlayers();
	UserMessageBegin(filter, "DiscordUpdate");
		WRITE_WORD(m_iAct);
		WRITE_WORD(m_iChapter);
		WRITE_STRING(m_sChapter.ToCStr());
		WRITE_WORD(m_iCustomString);
		WRITE_STRING(m_sDetails.ToCStr());
		WRITE_STRING(m_sState.ToCStr());
		WRITE_STRING(m_sLargeImageKey.ToCStr());
		WRITE_STRING(m_sLargeImageText.ToCStr());
	MessageEnd();
}

#endif
