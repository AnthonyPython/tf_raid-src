#include "cbase.h"
#include "usermessages.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CChapterMessage : public CPointEntity
{
public:
	DECLARE_CLASS(CChapterMessage, CPointEntity);

	void	Spawn(void);

private:

	void InputDisplay(inputdata_t &inputdata);

	int m_iAct;
	int m_iChapter;
	string_t m_iszChapterTitle;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(env_chapter_message, CChapterMessage);

BEGIN_DATADESC(CChapterMessage)

DEFINE_KEYFIELD(m_iAct, FIELD_INTEGER, "actnumber"),
DEFINE_KEYFIELD(m_iChapter, FIELD_INTEGER, "chapternumber"),
DEFINE_KEYFIELD(m_iszChapterTitle, FIELD_STRING, "chaptertitle"),

DEFINE_INPUTFUNC(FIELD_VOID, "DisplayChapter", InputDisplay),

END_DATADESC()

void CChapterMessage::Spawn(void)
{
	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_NONE);
}

void CChapterMessage::InputDisplay(inputdata_t &inputdata)
{
	CBroadcastRecipientFilter filter;
	filter.AddAllPlayers();

	UserMessageBegin(filter, "ShowChapter");
	WRITE_BYTE(m_iAct);
	WRITE_BYTE(m_iChapter);
	WRITE_STRING(m_iszChapterTitle.ToCStr());
	MessageEnd();
}
