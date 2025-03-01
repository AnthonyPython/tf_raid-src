#include "cbase.h"
#include "hdtf_ui.h"
#include "ui_mainmenu_newgame.h"
#include "ui_menu_button.h"
#include "ui_loading.h"
#include "ui_menu_popup.h"

#include "vgui/ISurface.h"
#include "vgui/IVGui.h"

using namespace vgui;

// NOTE(wheatley): uncomment to hide all chapters but first chapters of each act
//#define WHACK_CHAPTER_LIST
#define DISPLAY_TUTORIAL

// ------------------------------------------------------------------------------------------
// CHAPTER ENTRY
// ------------------------------------------------------------------------------------------

MainMenu_ChapterEntry::MainMenu_ChapterEntry(vgui::Panel *parent, const char *name, chapterdescriptor_t chapter, int index) : Panel(parent, name)
{
	SetParent(parent);

	m_pChapterLabel = new Label(this, "chapterLabel", "");
	m_pChapterImage = new ImagePanel(this, "chapterImage");
	m_pChapterTitle = new Label(this, "chapterTitle", "");
	m_pChapterImage->SetShouldScaleImage(true);

	m_pChapterLabel->SetMouseInputEnabled(false);
	m_pChapterTitle->SetMouseInputEnabled(false);
	m_pChapterImage->SetMouseInputEnabled(false);

	actionMessage = NULL;

	SetChapter(chapter, index);

	m_iLineWidth = 0;

	m_bIsEnabled = true;
	m_bIsHovered = false;
}

MainMenu_ChapterEntry::~MainMenu_ChapterEntry()
{
	if (actionMessage != NULL)
	{
		actionMessage->deleteThis();
	}
}

void MainMenu_ChapterEntry::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pChapterLabel->SetFont(pScheme->GetFont("MainMenuChapterLabel", false));
	m_pChapterTitle->SetFont(pScheme->GetFont("MainMenuChapterTitle", false));
}

void MainMenu_ChapterEntry::PerformLayout()
{
	m_iLineWidth = 6;

	int ypos = -3;

	m_pChapterLabel->SetPos(m_iLineWidth * 2, ypos);
	m_pChapterLabel->SizeToContents();
	ypos += m_pChapterLabel->GetTall() - 5;

	if (m_pChapterTitle->IsVisible())
	{
		m_pChapterTitle->SetPos(m_iLineWidth * 2, ypos);
		m_pChapterTitle->SizeToContents();
		ypos += m_pChapterTitle->GetTall() - 2;
	}
	else
	{
		ypos += 2;
	}

	int w = 200;
	int h = 113;

	m_pChapterImage->SetPos(m_iLineWidth * 2, ypos);
	m_pChapterImage->SetSize(w, h);

	SetWide(max(m_pChapterLabel->GetWide(), max(m_pChapterTitle->GetWide(), m_pChapterImage->GetWide())) + m_iLineWidth * 2);
	SetTall(ypos + m_pChapterImage->GetTall());
}

void MainMenu_ChapterEntry::SetChapter(chapterdescriptor_t chapter, int index)
{
	if (chapter.m_iAct >= 0)
	{
		wchar_t label[32] = { L'\0' };
		if (chapter.m_iAct == 0)
		{
			wchar_t *temp = g_pVGuiLocalize->Find("#HDTF_Prologue_Title");
			if (temp)
				wcsncpy(label, temp, sizeof(label) / sizeof(wchar_t));
		}
		else if (chapter.m_iAct == HDTF_LAST_ACT)
		{
			wchar_t *temp = g_pVGuiLocalize->Find("#HDTF_Epilogue_Title");
			if (temp)
				wcsncpy(label, temp, sizeof(label) / sizeof(wchar_t));
		}
		else
		{
			wchar_t chapterNum[4] = { L'\0' };
			_snwprintf(chapterNum, ARRAYSIZE(chapterNum), L"%i", chapter.m_iChapterID);

			g_pVGuiLocalize->ConstructString(
				label,
				sizeof(label),
				g_pVGuiLocalize->Find("#HDTF_Chapter_Title"),
				1,
				chapterNum);
		}

		m_pChapterLabel->SetText(label);
		if (chapter.m_iAct == 0 || chapter.m_iAct == HDTF_LAST_ACT)
			m_pChapterTitle->SetVisible(false);
		else
			m_pChapterTitle->SetText(chapter.m_pChapterName);
		m_pChapterImage->SetImage(vgui::scheme()->GetImage(VarArgs("chapters/chapter%i", index), false));
	}
	else
	{
		m_pChapterTitle->SetVisible(false);
		m_pChapterLabel->SetText("#HDTF_Chapter_Tutorial");
		m_pChapterImage->SetImage(vgui::scheme()->GetImage(VarArgs("chapters/tutorial", index), false));
	}

	if (actionMessage != NULL)
	{
		actionMessage->deleteThis();
	}

	actionMessage = new KeyValues("StartNewGame", "chapter", index);
}

void MainMenu_ChapterEntry::OnHoverStateChanged(bool state)
{
	if (state)
	{
		m_pChapterLabel->SetFgColor(Color(255, 25, 0, 255));
		m_pChapterTitle->SetFgColor(Color(255, 25, 0, 255));
	}
	else
	{
		m_pChapterLabel->SetFgColor(Color(255, 255, 255, 255));
		m_pChapterTitle->SetFgColor(Color(255, 255, 255, 255));
	}
}

void MainMenu_ChapterEntry::OnMousePressed(MouseCode code)
{
	if (m_bIsEnabled && code == MOUSE_LEFT)
	{
		PostActionSignal(actionMessage->MakeCopy());
	}
}

void MainMenu_ChapterEntry::Paint()
{
	if (IsHovered())
		surface()->DrawSetColor(255, 25, 0, 255);
	else
		surface()->DrawSetColor(255, 255, 255, 255);

	surface()->DrawFilledRect(0, 0, m_iLineWidth, GetTall());
}

// ------------------------------------------------------------------------------------------
// ACT ENTRY
// ------------------------------------------------------------------------------------------
MainMenu_ActEntry::MainMenu_ActEntry(vgui::Panel *parent, const char *name, int act) : Panel(parent, name)
{
	SetParent(parent);

	wchar_t label[32];

	wchar_t actNum[4] = { L'\0' };
	_snwprintf(actNum, ARRAYSIZE(actNum), L"%i", act);

	g_pVGuiLocalize->ConstructString(
		label,
		sizeof(label),
		g_pVGuiLocalize->Find("#HDTF_Act_Title"),
		1,
		actNum);

	m_pLabel = new Label(this, "actLabel", label);
}

void MainMenu_ActEntry::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetSize(212, 20);

	m_pLabel->SetFont(pScheme->GetFont("MainMenuChapterLabel", false));
}

void MainMenu_ActEntry::PerformLayout()
{
	m_pLabel->SizeToContents();

	int w = GetWide();
	int h = GetTall();

	int tw = m_pLabel->GetWide();
	int th = m_pLabel->GetTall();

	m_pLabel->SetPos(
		(w / 2) - (tw / 2),
		(h / 2) - (th / 2));
}

void MainMenu_ActEntry::Paint()
{
	int spacing = m_pLabel->GetTall();
	int thickness = 2;
	int midY = (GetTall() / 2) - (thickness / 2);
	int midX = GetWide() / 2;
	int tw = m_pLabel->GetWide() / 2;

	surface()->DrawSetColor(100, 100, 100, 255);
	surface()->DrawFilledRect(0, midY, (midX - tw) - spacing, midY + thickness);
	surface()->DrawFilledRect((midX + tw) + spacing, midY, GetWide(), midY + thickness);
}

// ------------------------------------------------------------------------------------------
// NEW GAME CANVAS PANEL
// ------------------------------------------------------------------------------------------
MainMenu_NewGame::MainMenu_NewGame(Panel *parent, const char *name) : HDTFMenuCanvas(parent, name)
{
	SetParent(parent);

	m_pChapterFrame = new Panel(this, "chapterHolderFrame");
	m_pChapterHolder = new Panel(m_pChapterFrame, "chapterHolder");

	m_pCommentaryButton = new Button(this, "commentaryButton", "#HDTF_NewGame_Commentary");
	m_pCommentaryButton->SetCommand("DeveloperCommentary");
	m_pCommentaryButton->AddActionSignalTarget(this);
	m_pCommentaryButton->SetVisible(true);
	m_pCommentaryButton->SetZPos(99);
	m_pCommentaryButton->SetPaintBorderEnabled(false);

	m_pCommentaryTitle = new Label(this, "commentaryTitle", "#HDTF_NewGame_CommentaryTitle");
	m_pCommentaryTitle->SetVisible(false);

	m_iScrollSize = 1;
	m_iMaxScroll = 0;
	m_iScrollOffset = 0;

	ConVarRef sv_unlockedchapters("sv_unlockedchapters");
	m_iChapterCount = sv_unlockedchapters.GetInt();

	ivgui()->AddTickSignal(GetVPanel(), 500);

	ReloadChapters();

	m_bIsInCommentaryMode = false;
	m_iSelectedCommentaryChapter = 0;
	m_flTransparency = 1.f;
	m_bTargetCommentaryMode = false;
}

void MainMenu_NewGame::ReloadChapters()
{
	for (int i = 0; i < m_pChapterList.Count(); i++)
	{
		m_pChapterList[i]->MarkForDeletion();
	}
	m_pChapterList.Purge();

	SetReady(false);

#if defined(DISPLAY_TUTORIAL)
	chapterdescriptor_t tutorial;
	tutorial.m_iAct = -1;
	MainMenu_ChapterEntry *pTutorial = new MainMenu_ChapterEntry(m_pChapterHolder, "chapterEntry", tutorial, -1);
	pTutorial->AddActionSignalTarget(this);
	m_pChapterList.AddToTail(pTutorial);
#endif

	int thisAct = 0;
	for (int i = 0; i < min(HDTF_CHAPTER_COUNT, m_iChapterCount); i++)
	{
		// this hack locks all chapters but first chapter of each act
#ifdef WHACK_CHAPTER_LIST
		if (g_ChapterList[i].m_iChapterID > 1)
			continue;
#endif

		if (g_ChapterList[i].m_iAct != HDTF_LAST_ACT && g_ChapterList[i].m_iAct != thisAct)
		{
			thisAct = g_ChapterList[i].m_iAct;
			MainMenu_ActEntry *pAct = new MainMenu_ActEntry(m_pChapterHolder, "actEntry", g_ChapterList[i].m_iAct);
			m_pChapterList.AddToTail(pAct);
		}

		MainMenu_ChapterEntry *pEntry = new MainMenu_ChapterEntry(m_pChapterHolder, "chapterEntry", g_ChapterList[i], i);
		pEntry->AddActionSignalTarget(this);
		m_pChapterList.AddToTail(pEntry);
	}

	InvalidateLayout();
}

void MainMenu_NewGame::ToggleCommentaryMode()
{
	if (m_bTargetCommentaryMode != m_bIsInCommentaryMode)
		return;

	m_bIsInCommentaryMode = !m_bIsInCommentaryMode;
}

void MainMenu_NewGame::OnSwitchedTo()
{
	m_bIsInCommentaryMode = false;
	m_bTargetCommentaryMode = false;
	m_flTransparency = 1.f;
}

void MainMenu_NewGame::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pCommentaryButton->SetFont(pScheme->GetFont("MainMenuButtons"));
	m_pCommentaryTitle->SetFont(pScheme->GetFont("MainMenuPanelName"));
}

void MainMenu_NewGame::PerformLayout()
{
	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);

	int spacing = scheme()->GetProportionalScaledValue(10);
	int xpos = scheme()->GetProportionalScaledValue(45);
	int borderHeight = tall / 8;

	int yOffset = 0;
	if (m_bIsInCommentaryMode)
	{
		yOffset = m_pCommentaryTitle->GetTall() + spacing;
	}

	m_pChapterFrame->SetPos(xpos, borderHeight + spacing + yOffset);
	m_pChapterFrame->SetSize(wide - xpos, tall - (borderHeight + spacing) * 2 - yOffset);

	m_pChapterHolder->SetWide(m_pChapterFrame->GetWide());

	int ypos = 0;
	for (int i = 0; i < m_pChapterList.Count(); i++)
	{
		Panel *pEntry = m_pChapterList.Element(i);

		pEntry->PerformLayout();
		pEntry->SetPos(0, ypos);

		m_iScrollSize = pEntry->GetTall() / 3;

		ypos += pEntry->GetTall() + spacing;
	}

	m_pChapterHolder->SetTall(ypos);

	m_iMaxScroll = m_pChapterHolder->GetTall() - m_pChapterFrame->GetTall();

	m_pCommentaryButton->SetPos(scheme()->GetProportionalScaledValue(180), tall - (tall / 9));
	m_pCommentaryButton->SetWide(scheme()->GetProportionalScaledValue(120));
	m_pCommentaryButton->SetFgColor(Color(255, 255, 255, 255));
	m_pCommentaryButton->SetSelectedColor(Color(100, 5, 0, 255), Color(0, 0, 0, 0));
	m_pCommentaryButton->SetArmedColor(Color(255, 25, 0, 255), Color(0, 0, 0, 0));
	m_pCommentaryButton->SetPaintBorderEnabled(false);

	m_pCommentaryTitle->SetVisible(m_bIsInCommentaryMode);
	m_pCommentaryTitle->SetPos(xpos, borderHeight + spacing);
	m_pCommentaryTitle->SizeToContents();

	if (m_bIsInCommentaryMode)
		m_pCommentaryButton->SetText("#HDTF_NewGame_CommentaryDisable");
	else
		m_pCommentaryButton->SetText("#HDTF_NewGame_Commentary");

	SetReady(true);
}

MainMenu_NewGame::~MainMenu_NewGame()
{
	m_pChapterList.PurgeAndDeleteElements();
}

void MainMenu_NewGame::OnMouseWheeled(int delta)
{
	m_iScrollOffset = max(0, min(m_iMaxScroll, m_iScrollOffset - m_iScrollSize * delta));
	m_pChapterHolder->SetPos(0, -m_iScrollOffset);
}

void MainMenu_NewGame::OnStartNewGame(KeyValues *data)
{
	int selectedChapter = data->GetInt("chapter");

	if (m_bIsInCommentaryMode)
	{
		m_iSelectedCommentaryChapter = selectedChapter;

		HDTFUI_Popup *popup = new HDTFUI_CommentaryPopup(this, "CommitCommentaryGameStart");

		popup->AddActionSignalTarget(this);
		popup->TakeControl();

		return;
	}

	StartGame(selectedChapter);
}

void MainMenu_NewGame::StartGame(int fromChapter, bool inCommentaryMode)
{
	char mapcommand[256];
	mapcommand[0] = 0;

	ConVarRef commentary("commentary");
	commentary.SetValue(inCommentaryMode ? 1 : 0);

	if (fromChapter >= 0)
	{
		if(inCommentaryMode)
			Q_snprintf(mapcommand, sizeof(mapcommand), "disconnect\nprogress_enable\nmap_commentary %s\n", g_ChapterList[fromChapter].m_pMapName);
		else	
			Q_snprintf(mapcommand, sizeof(mapcommand), "disconnect\nprogress_enable\nmap %s\n", g_ChapterList[fromChapter].m_pMapName);
	}
	else
	{
		if (inCommentaryMode)
			Q_snprintf(mapcommand, sizeof(mapcommand), "disconnect\nprogress_enable\nmap_commentary a0c0p0_bootcamp\n");
		else
			Q_snprintf(mapcommand, sizeof(mapcommand), "disconnect\nprogress_enable\nmap a0c0p0_bootcamp\n");
	}

	// we should stop the background video before load
	// or it might glitch when loading started
	HDTFMainMenu *pMenu = dynamic_cast<HDTFMainMenu *>(guiroot->GetMainCanvas());
	if (pMenu)
		pMenu->StopBackgroundVideo();

	guiloading->PrepareForLoading(fromChapter);

	engine->ClientCmd(mapcommand);
}

void MainMenu_NewGame::OnTick()
{
	ConVarRef sv_unlockedchapters("sv_unlockedchapters");
	if (m_iChapterCount != sv_unlockedchapters.GetInt())
	{
		m_iChapterCount = sv_unlockedchapters.GetInt();
		ReloadChapters();
	}
}

void MainMenu_NewGame::Paint()
{
	BaseClass::Paint();

	if (m_bTargetCommentaryMode != m_bIsInCommentaryMode)
	{
		if (m_flTransparency != 0.f)
		{
			m_flTransparency = Approach(0.f, m_flTransparency, gpGlobals->absoluteframetime * 3.5f);
		}
		else
		{
			m_bTargetCommentaryMode = m_bIsInCommentaryMode;
			InvalidateLayout(true);
		}
	}
	else
	{
		if (m_flTransparency != 1.f)
		{
			m_flTransparency = Approach(1.f, m_flTransparency, gpGlobals->absoluteframetime * 3.5f);
		}
	}
}

void MainMenu_NewGame::OnCommand(const char *command)
{
	if (FStrEq(command, "DeveloperCommentary"))
	{
		ToggleCommentaryMode();
	}
	else if (FStrEq(command, "CommitCommentaryGameStart"))
	{
		StartGame(m_iSelectedCommentaryChapter, true);
	}
}
