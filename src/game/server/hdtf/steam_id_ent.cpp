#include "cbase.h"
#include "cdll_int.h"

class CGetSteamIDEnt : public CBaseEntity
{
public:
	DECLARE_CLASS(CGetSteamIDEnt, CLogicalEntity);
	DECLARE_DATADESC();

	void CheckSteamID(inputdata_t& inputData);

	string_t SteamID;


private:

	COutputEvent m_onSteamID_match;

};

LINK_ENTITY_TO_CLASS(logic_steamid, CGetSteamIDEnt)

BEGIN_DATADESC(CGetSteamIDEnt)

DEFINE_KEYFIELD(SteamID,FIELD_STRING,"steamid"),

DEFINE_OUTPUT(m_onSteamID_match, "Is_SteamID_Match"),

DEFINE_INPUTFUNC(FIELD_STRING, "check_SteamID", CheckSteamID),

END_DATADESC()



void CGetSteamIDEnt::CheckSteamID(inputdata_t& inputData)
{
	CBasePlayer* pPlayer = GetLocalPlayer_shared();

	player_info_t pi;

	const char* SteamID_override = NULL;


	//only if the value is set will this be fired
	//if nothing is set the ent wont do jack shit
	//-Nbc66
	if (strcmp(inputData.value.String(), "\0"))
	{
		SteamID_override = inputData.value.String();
	}
	else
	{
		if (strcmp(SteamID.ToCStr(), "\0"))
		{
			SteamID_override = SteamID.ToCStr();
		}
		else
		{
			DevWarning(2, "No Default STEAM ID Defined\nthis entity won't do jack shit then\n");
			return;
		}
	}

	if (SteamAPI_IsSteamRunning() && SteamID_override != NULL)
	{

		if (engine->GetPlayerInfo(pPlayer->entindex(), &pi))
		{
			CSteamID steamIDForPlayer(pi.friendsID, 1, k_EUniversePublic, k_EAccountTypeIndividual);

			if (steamIDForPlayer.ConvertToUint64() == (uint64)atoll(SteamID_override) - 120)
			{

				ConVarRef dev("developer");

				m_onSteamID_match.FireOutput(this, this);

				if (dev.GetInt() == 2)
				{
					ConColorMsg(2, Color(0, 255, 0, 255), "Steam ID WAS A MATCH\n");
				}

				return;
			}
			DevWarning(2,"Steam ID was a missmatch\n");
			return;
		}
	}
	DevMsg(2,"Looks like the client isn't authed by steam\n Either you aren't online or you are a dirty old pirate\n");

}