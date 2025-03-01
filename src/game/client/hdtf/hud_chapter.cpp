#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "hudelement.h"

#include <vgui_controls/Panel.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

class CHudChapter : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE(CHudChapter, vgui::Panel);

	CHudChapter(const char *pElementName);
	void Init();
	void Reset();
	bool ShouldDraw();

	void MsgFunc_ShowChapter(bf_read &msg);

protected:
	void Paint();

private:
	CPanelAnimationVar(Color, m_ActTextColor, "ActTextColor", "255 255 255 0");
	CPanelAnimationVar(Color, m_TitleTextColor, "TitleTextColor", "255 255 255 0");
	CPanelAnimationVar(vgui::HFont, m_hActFont, "ActFont", "Default");
	CPanelAnimationVar(vgui::HFont, m_hChapterFont, "ChapterFont", "Default");
	CPanelAnimationVar(vgui::HFont, m_hChapterTitleFont, "ChapterTitleFont", "Default");

	CPanelAnimationVarAliasType(float, m_flActLineX, "text_act_x", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flActLineY, "text_act_y", "0", "proportional_float");

	CPanelAnimationVarAliasType(float, m_flTitleLineX, "text_title_x", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flTitleLineY, "text_title_y", "10", "proportional_float");

	wchar_t m_ActText[16];
	wchar_t m_ChapterText[32];
	wchar_t m_ChapterTitle[64];

	bool m_bDisplayChapter;
};

DECLARE_HUDELEMENT(CHudChapter);
DECLARE_HUD_MESSAGE(CHudChapter, ShowChapter);

CHudChapter::CHudChapter(const char *pElementName) :
	CHudElement(pElementName), BaseClass(g_pClientMode->GetViewport(), "HudChapter")
{
	m_bDisplayChapter = true;
	SetAlpha(0);
}

void CHudChapter::Init()
{
	HOOK_HUD_MESSAGE(CHudChapter, ShowChapter);
}

void CHudChapter::Reset()
{
	SetAlpha(0);
}

bool CHudChapter::ShouldDraw()
{
	return ((GetAlpha() > 0) && CHudElement::ShouldDraw());
}

void CHudChapter::Paint()
{
	surface()->DrawSetTextFont(m_hActFont);
	surface()->DrawSetTextColor(m_ActTextColor);
	surface()->DrawSetTextPos(m_flActLineX, m_flActLineY);

	surface()->DrawPrintText(m_ActText, wcslen(m_ActText));

	if (m_bDisplayChapter)
	{
		int xoffset, actHeight;
		surface()->GetTextSize(m_hActFont, m_ActText, xoffset, actHeight);
		int chapterHeight, UNUSED = 0;
		surface()->GetTextSize(m_hChapterFont, m_ChapterText, UNUSED, chapterHeight);

		surface()->DrawSetTextPos(m_flActLineX + xoffset, m_flActLineY + (actHeight - chapterHeight));
		surface()->DrawSetTextFont(m_hChapterFont);
		surface()->DrawPrintText(m_ChapterText, wcslen(m_ChapterText));

		surface()->DrawSetTextColor(m_TitleTextColor);
		surface()->DrawSetTextPos(m_flTitleLineX, m_flTitleLineY);
		surface()->DrawSetTextFont(m_hChapterTitleFont);
		surface()->DrawPrintText(m_ChapterTitle, wcslen(m_ChapterTitle));
	}
}

void CHudChapter::MsgFunc_ShowChapter(bf_read &msg)
{
	int iAct = msg.ReadByte();
	int iChapter = msg.ReadByte();
	char m_DisplayChapter[32];
	msg.ReadString(m_DisplayChapter, 32, true);

	if (iAct == 0)
	{
		wchar_t *tempString = g_pVGuiLocalize->Find("#HDTF_Prologue_Title");
		memset(m_ActText, 0, sizeof(m_ActText));
		wcsncpy(m_ActText, tempString, sizeof(m_ActText) / sizeof(wchar_t));

		m_bDisplayChapter = false;
	}
	else
	{
		wchar_t actNum[8] = { L'\0' };
		wchar_t chapterNum[8] = { L'\0' };

		_snwprintf(actNum, ARRAYSIZE(actNum), L"%i", iAct);
		_snwprintf(chapterNum, ARRAYSIZE(chapterNum), L"%i", iChapter);

		g_pVGuiLocalize->ConstructString(m_ActText,
			sizeof(m_ActText),
			g_pVGuiLocalize->Find("#HDTF_Act_Title"), 
			1, 
			actNum);

		g_pVGuiLocalize->ConstructString(m_ChapterText,
			sizeof(m_ChapterText),
			g_pVGuiLocalize->Find("#HDTF_Chapter_Title"),
			1,
			chapterNum);

		m_bDisplayChapter = true;
	}

	wchar_t *chapterString = g_pVGuiLocalize->Find(m_DisplayChapter);
	memset(m_ChapterTitle, 0, sizeof(m_ChapterTitle));
	wcsncpy(m_ChapterTitle, chapterString, sizeof(m_ChapterTitle) / sizeof(wchar_t));

	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ShowChapter");
}
