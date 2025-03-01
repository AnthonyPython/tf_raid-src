#include "cbase.h"

#ifdef GAME_DLL
class CDiscordRPEntity : public CLogicalEntity
{
public:
	DECLARE_CLASS(CDiscordRPEntity, CLogicalEntity);
	DECLARE_DATADESC();

	CDiscordRPEntity();

	void Spawn(void);

	void Precache(void);

	void InputUpdatePresence(inputdata_t &inputdata);

	int				m_iAct;
	string_t		m_sChapter;
	int				m_iChapter;

	int			m_iCustomString;
	string_t		m_sDetails;
	string_t		m_sState;
	string_t		m_sLargeImageKey;
	string_t		m_sLargeImageText;
};

// Start of our data description for the class
BEGIN_DATADESC(CDiscordRPEntity)

DEFINE_INPUTFUNC(FIELD_VOID, "UpdatePresence", InputUpdatePresence),

DEFINE_KEYFIELD(m_iAct, FIELD_INTEGER, "act"),
DEFINE_KEYFIELD(m_sChapter, FIELD_STRING, "chapterStr"),
DEFINE_KEYFIELD(m_iChapter, FIELD_INTEGER, "chapter"),
DEFINE_KEYFIELD(m_iCustomString, FIELD_INTEGER, "customString"),
DEFINE_KEYFIELD(m_sDetails, FIELD_STRING, "details"),
DEFINE_KEYFIELD(m_sState, FIELD_STRING, "state"),

DEFINE_KEYFIELD(m_sLargeImageKey, FIELD_STRING, "largeImageKey"),
DEFINE_KEYFIELD(m_sLargeImageText, FIELD_STRING, "largeImageText")

END_DATADESC()

LINK_ENTITY_TO_CLASS(env_discord, CDiscordRPEntity);
#endif

/*

Copy this to the FGD:

@PointClass base(Targetname) = env_discord : "Discord Rich Presence Manager. Call UpdatePresence on map & save load or whenever you feel like."
[
	act(integer) : "Act Index" : 1 : "The Act index."
	chapter(integer) : "Chapter Index" : 1 : "The Chapter index."
	chapterStr(string) : "Chapter Name" : "The Test Map" : "The Chapter name."
	
	largeImageKey(string) : "Large Image Key" : "modimagehere" : "The Large Image key. Points to an art asset defined in the Discord Developer Portal."
	largeImageText(string) : "Large Image Text" : "Someone fucked up Mitchell's face" : "Text that should be shown when mouse is hovered over the large image in Discord."
	
	customString(choices) : "Enable Custom String" : 0 : "Whenever Discord should show custom defined details and state." =
	[
		0 : "No"
		1 : "Yes"
	]
	
	details(string) : "Details" : "This is shown below the game title" : "The text shown below the game title when custom string is enabled."
	state(string) : "State" : "This is shown below details" : "The text shown below details when custom string is enabled."
	
	input UpdatePresence(void) : "Updates presence"
]

*/