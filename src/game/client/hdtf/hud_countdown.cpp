#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "hudelement.h"

#include <vgui_controls/Panel.h>

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static float g_DigitTime[10] = {
	600.f,
	60.f,
	0.f,		// ignored :
	10.f,
	1.f,
	0.f,		// ignored :
	1.0f,
	0.1f,
	0.001f,
	0.0001f
};

class CHudCountdown : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE(CHudCountdown, vgui::Panel);

	CHudCountdown(const char *pElementName);
	void Init();
	bool ShouldDraw();

	void MsgFunc_ShowCountdown(bf_read &msg);

	void FormatTime(float inTime, wchar_t *buff, const int buffSize);

protected:
	void Paint();
	void OnThink();

private:
	float		m_flAlpha;
	float		m_flStartTime;
	float		m_flEndTime;
	wchar_t		m_pCaption[128];
	wchar_t		m_pTime[11];

	CPanelAnimationVar(Color, m_CaptionColor, "CaptionColor", "255 255 255 255");
	CPanelAnimationVar(Color, m_DigitColorNormal, "DigitColorNormal", "255 255 255 255");
	CPanelAnimationVar(Color, m_DigitColorZero, "DigitColorZero", "255 0 0 255");

	CPanelAnimationVar(vgui::HFont, m_hCaptionFont, "CaptionFont", "Default");
	CPanelAnimationVar(vgui::HFont, m_hDigitFont, "DigitFont", "Default");
};

DECLARE_HUDELEMENT(CHudCountdown);
DECLARE_HUD_MESSAGE(CHudCountdown, ShowCountdown);

CHudCountdown::CHudCountdown(const char *pElementName) :
	CHudElement(pElementName), BaseClass(g_pClientMode->GetViewport(), "HudCountdown")
{
	m_flAlpha = 0.f;
	m_flStartTime = 0.f;
	m_flEndTime = 0.f;
}

void CHudCountdown::Init()
{
	HOOK_HUD_MESSAGE(CHudCountdown, ShowCountdown);
#ifdef DEBUG
	wchar_t *temp = L"caption";
	wcsnset(m_pCaption, 0, sizeof(m_pCaption));
	wcsncpy(m_pCaption, temp, sizeof(m_pCaption) / sizeof(wchar_t));
#endif
}

bool CHudCountdown::ShouldDraw()
{
	return CHudElement::ShouldDraw();
}

void CHudCountdown::FormatTime(float inTime, wchar_t *buff, const int buffSize)
{
	inTime = max(inTime, 0);
	const int miliseconds = (inTime - (int)inTime) * 10000;
	const int minutes = inTime / 60;
	const int seconds = (int)inTime % 60;

	V_snwprintf(buff, buffSize, L"%02i:%02i:%04i", minutes, seconds, miliseconds);
}

void CHudCountdown::OnThink()
{
	BaseClass::OnThink();

	if (gpGlobals->curtime < m_flStartTime)
	{
		SetPaintEnabled(false);
		m_flAlpha = 0.f;

		m_flStartTime = -1;
		m_flEndTime = -1;

		return;
	}

	const float m_flTimer = (m_flEndTime - gpGlobals->curtime);

	if (m_flTimer <= -1.f)
		m_flAlpha = max(m_flAlpha - gpGlobals->absoluteframetime, 0.f);
	else
		m_flAlpha = min(m_flAlpha + gpGlobals->absoluteframetime, 1.f);

	if (m_flAlpha == 0.f)
	{
		SetPaintEnabled(false);
	}
	else
	{
		SetPaintEnabled(true);
		FormatTime(m_flTimer, m_pTime, sizeof(m_pTime));
	}
}

void CHudCountdown::Paint()
{
	float alphaMultiplier = surface()->DrawGetAlphaMultiplier();

	surface()->DrawSetAlphaMultiplier(m_flAlpha);

	const float m_flTimer = (m_flEndTime - gpGlobals->curtime);

	const int w = GetWide();

	int captionWidth;
	int captionHeight;
	surface()->GetTextSize(m_hCaptionFont, m_pCaption, captionWidth, captionHeight);

	surface()->DrawSetTextFont(m_hCaptionFont);
	surface()->DrawSetTextColor(m_CaptionColor);
	surface()->DrawSetTextPos(w / 2 - captionWidth / 2, 0);

	surface()->DrawPrintText(m_pCaption, wcslen(m_pCaption));

	// timer

	int timeWidth;
	int timeHeight = 0;
	surface()->GetTextSize(m_hDigitFont, m_pTime, timeWidth, timeHeight);

	surface()->DrawSetTextFont(m_hDigitFont);
	surface()->DrawSetTextPos(w / 2 - timeWidth / 2, captionHeight + 2);

	for (int i = 0; i < 10; i++)
	{
		if(g_DigitTime[i] > m_flTimer)
			surface()->DrawSetTextColor(m_DigitColorZero);
		else
			surface()->DrawSetTextColor(m_DigitColorNormal);

		surface()->DrawUnicodeChar(m_pTime[i]);
	}

	surface()->DrawSetAlphaMultiplier(alphaMultiplier);
}

void CHudCountdown::MsgFunc_ShowCountdown(bf_read &msg)
{
	m_flStartTime = msg.ReadFloat();
	m_flEndTime = msg.ReadFloat();
	char m_CaptionToken[32];
	msg.ReadString(m_CaptionToken, 32, true);

	wchar_t *tempString = g_pVGuiLocalize->Find(m_CaptionToken);
	memset(m_pCaption, 0, sizeof(m_pCaption));
	wcsncpy(m_pCaption, tempString, sizeof(m_pCaption) / sizeof(wchar_t));
}