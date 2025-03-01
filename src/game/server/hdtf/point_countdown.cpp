#include "cbase.h"

class CPointCountdown : public CPointEntity
{
public:
	DECLARE_CLASS(CPointCountdown, CPointEntity);
	DECLARE_DATADESC();

	CPointCountdown();

	void		OnRestore();
	void		InputStartCountdown(inputdata_t &inputData);

protected:
	void		StartCountdown(bool bRestart = true);

private:
	float		m_flTimer;
	string_t	m_iszCaption;
	float		m_flStoredStartTime;
};

LINK_ENTITY_TO_CLASS(point_countdown, CPointCountdown);

BEGIN_DATADESC(CPointCountdown)

DEFINE_KEYFIELD(m_flTimer, FIELD_FLOAT, "TimeAmount"),
DEFINE_KEYFIELD(m_iszCaption, FIELD_STRING, "CaptionString"),

DEFINE_INPUTFUNC(FIELD_VOID, "StartCountdown", InputStartCountdown),

DEFINE_FIELD(m_flStoredStartTime, FIELD_TIME),

END_DATADESC()

CPointCountdown::CPointCountdown()
{
	m_flStoredStartTime = -1.f;
}

void CPointCountdown::OnRestore()
{
	BaseClass::OnRestore();

	if (m_flStoredStartTime != -1.f && m_flStoredStartTime + m_flTimer > gpGlobals->curtime)
	{
		StartCountdown(false);
	}
}

void CPointCountdown::StartCountdown(bool bRestart)
{
	if (bRestart)
	{
		m_flStoredStartTime = gpGlobals->curtime;
	}

	CRecipientFilter filter;
	filter.AddAllPlayers();
	UserMessageBegin(filter, "ShowCountdown");
		WRITE_FLOAT(m_flStoredStartTime);
		WRITE_FLOAT(m_flStoredStartTime + m_flTimer);
		WRITE_STRING(m_iszCaption.ToCStr());
	MessageEnd();
}

void CPointCountdown::InputStartCountdown(inputdata_t &inputData)
{
	StartCountdown();
}
