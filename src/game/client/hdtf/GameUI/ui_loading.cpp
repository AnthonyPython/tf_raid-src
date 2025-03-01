#include "cbase.h"
#include "ui_loading.h"
#include "hdtf_ui.h"

#include "GameUI/ui_mainmenu_newgame.h"
#include "vgui/ISurface.h"
#include "vgui/IInput.h"
#include "vgui/IVGui.h"
#include "vgui_controls/Frame.h"
#include "vgui_controls/ProgressBar.h"
#include "filesystem.h"

using namespace vgui;

HDTFUI_Loading *guiloading = NULL;

HDTFUI_Loading::HDTFUI_Loading(VPANEL parent) : Panel(NULL, "HDTFUIRoot")
{
	SetParent(parent);
	guiloading = this;

	SetZPos(100);

	m_iLoadingBGTexture = surface()->CreateNewTextureID();
	m_bHasValidBGTexture = false;

	m_pLoadingText = new Label(this, "LoadingLabel", "#HDTF_Loading");
	m_pChapterTitle = new Label(this, "ChapterLabel", "");

	m_iActualChapterIndex = INT_MIN;
	m_iLastChapterIndex = -10;
}

void HDTFUI_Loading::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// Resize the panel to the screen size
	// Otherwise, it'll just be in a little corner
	int wide, tall;
	vgui::surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);

	m_pLoadingText->SetFont(pScheme->GetFont("LoadingScreenTitle"));
	m_pChapterTitle->SetFont(pScheme->GetFont("LoadingScreenChapter"));
}

IGameUI *HDTFUI_Loading::GetGameUI()
{
	return guiroot->GetGameUI();
}

void HDTFUI_Loading::SolveEnginePanel()
{
	int w = GetWide();
	int h = GetTall();

	if (vgui::input()->GetAppModalSurface())
	{
		VPANEL _panel = vgui::input()->GetAppModalSurface();
		VPANEL _progress = ipanel()->GetChild(_panel, 2);

		vgui::Frame *dialog = dynamic_cast<vgui::Frame *>(ipanel()->GetPanel(_panel, "GameUI"));
		vgui::ProgressBar *progress = dynamic_cast<vgui::ProgressBar *>(ipanel()->GetPanel(_progress, "GameUI"));

		if (dialog)
		{
			int lw, lh;
			dialog->GetSize(lw, lh);
			dialog->SetPos((w / 2) - (lw / 2), h - (h / 4) - (lh / 2));
			dialog->SetPaintBackgroundEnabled(false);
			dialog->SetCloseButtonVisible(false);
		}

		if (progress)
		{
			int px, py;
			progress->GetParent()->GetPos(px, py);
			progress->GetPos(m_iBarX, m_iBarY);
			m_iBarX += px;
			m_iBarY += py + 1;

			m_iBarW = progress->GetWide();
			m_iBarH = progress->GetTall() - 1;

			progress->SetBarInset(0);
			progress->SetMargin(0);
			progress->SetSegmentInfo(3, 6);
			progress->SetPaintBorderEnabled(false);
			progress->SetPaintBackgroundEnabled(false);
			progress->SetFgColor(Color(200, 20, 0, 255));

			// place our 'LOADING' message
			m_pLoadingText->SetSize(w, 30);
			m_pLoadingText->SetPos(0, m_iBarY - m_pLoadingText->GetTall());
			m_pLoadingText->SetContentAlignment(vgui::Label::a_center);
			m_pLoadingText->SetFgColor(Color(255, 25, 0, 255));

			// now the chapter title
			m_pChapterTitle->SetSize(w, 30);
			m_pChapterTitle->SetPos(0, m_iBarY + m_iBarH + 10);
			m_pChapterTitle->SetContentAlignment(vgui::Label::a_center);
			m_pChapterTitle->SetFgColor(Color(255, 25, 0, 255));
		}
	}
}

void HDTFUI_Loading::PrepareForLoading(const int chapterIndex)
{
	m_iActualChapterIndex = chapterIndex;

	if (m_iActualChapterIndex == -1)
		m_pChapterTitle->SetText("#HDTF_Chapter_Tutorial");
	else
		m_pChapterTitle->SetText(g_ChapterList[chapterIndex].m_pChapterName);
}

void HDTFUI_Loading::PrepareForLoading(const char *mapName)
{
	m_iActualChapterIndex = -1;

	int act = (int)mapName[1] - '0';
	int chapter = (int)mapName[3] - '0';

	for (int i = 0; i < ARRAYSIZE(g_ChapterList); i++)
	{
		if (g_ChapterList[i].m_iAct == act && g_ChapterList[i].m_iChapterID == chapter)
		{
			PrepareForLoading(i);
			break;
		}
	}
}

void HDTFUI_Loading::PickLoadingScreen()
{
	if (m_iActualChapterIndex == m_iLastChapterIndex)
		return;

	char *chapterBgFile;
	if (m_iActualChapterIndex == -1)
		chapterBgFile = "vgui/loadingscreens/screen_tutorial";
	else
	{
		char temp[256];
		Q_snprintf(temp, sizeof(temp), "vgui/loadingscreens/screen%i", m_iActualChapterIndex);
		chapterBgFile = temp;
	}

	m_bHasValidBGTexture = false;
	char fullFileName[512] = { 0 };
	Q_snprintf(fullFileName, sizeof(fullFileName), "materials/%s.vmt", chapterBgFile);
	if (filesystem->FileExists(fullFileName))
	{
		m_bHasValidBGTexture = true;
		surface()->DrawSetTextureFile(m_iLoadingBGTexture, chapterBgFile, true, true);
	}
	
	m_iLastChapterIndex = m_iActualChapterIndex;
}

void HDTFUI_Loading::Paint()
{
	SolveEnginePanel();
	PickLoadingScreen();

	int w = GetWide();
	int h = GetTall();

	surface()->DrawSetColor(0, 0, 0, 255);
	surface()->DrawFilledRect(0, 0, w, h);

	if (m_bHasValidBGTexture)
	{
		surface()->DrawSetColor(255, 255, 255, 255);
		surface()->DrawSetTexture(m_iLoadingBGTexture);
		surface()->DrawTexturedRect(0, 0, w, h);
	}
	
	surface()->DrawSetColor(255, 25, 0, 255);
	surface()->DrawLine(m_iBarX, m_iBarY, m_iBarX, m_iBarY + m_iBarH);
	surface()->DrawLine(m_iBarX + m_iBarW, m_iBarY, m_iBarX + m_iBarW, m_iBarY + m_iBarH);
}