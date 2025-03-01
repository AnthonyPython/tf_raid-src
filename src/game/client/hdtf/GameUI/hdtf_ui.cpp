#include "cbase.h"
#include "hdtf_ui.h"
#include "hdtf_ui_interface.h"
#include "ui_loading.h"

#include "vgui/ILocalize.h"
#include "vgui/IPanel.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/IVGui.h"
#include "vgui/IInput.h"
#include "ienginevgui.h"
#include <engine/IEngineSound.h>
#include "filesystem.h"
#include "usermessages.h"
#include "hdtf_cutscene_ui.h"

#include "ui_mainmenu_options.h"
#include "ui_mainmenu_loadgame.h"
#include "ui_mainmenu_savegame.h"

using namespace vgui;

// See interface.h/.cpp for specifics:  basically this ensures that we actually Sys_UnloadModule the dll and that we don't call Sys_LoadModule 
//  over and over again.
static CDllDemandLoader g_GameUIDLL("GameUI");

HDTFUI_Root *guiroot = NULL;

void OverrideGameUI()
{
	if (!HDTFUI->GetPanel())
	{
		HDTFUI->Create(NULL);
	}
	if (guiroot->GetGameUI())
	{
		//guiroot->MakePopup();
		guiroot->GetGameUI()->SetMainMenuOverride(guiroot->GetVPanel());
		guiroot->GetGameUI()->SetLoadingBackgroundDialog(guiloading->GetVPanel());
		return;
	}
}

HDTFUI_Root::HDTFUI_Root(VPANEL parent) : Panel(this, "HDTFUIRoot")
{
	SetParent(parent);
	guiroot = this;

	m_bCopyFrameBuffer = false;
	gameui = NULL;

	LoadGameUI();

	m_ExitingFrameCount = 0;

	PrepareCustom();

	m_bIsInGame = false;

	// this is the main menu panel (what we see when launch the game)
	mainMenu = new HDTFMainMenu(this, "HDTFMainMenu");

	// this is the pause menu (what we see when hit 'ESC' in-game)
	pauseMenu = new HDTFPauseMenu(this, "HDTFPauseMenu");
	pauseMenu->SetVisible(false);

	usermessages->HookMessage("CutsceneStart", &HDTFUI_Root::StartCutscene);
}

IGameUI *HDTFUI_Root::GetGameUI()
{
	if (!gameui)
	{
		if (!LoadGameUI())
			return NULL;
	}

	return gameui;
}

bool HDTFUI_Root::LoadGameUI()
{
	if (!gameui)
	{
		CreateInterfaceFn gameUIFactory = g_GameUIDLL.GetFactory();
		if (gameUIFactory)
		{
			gameui = (IGameUI *)gameUIFactory(GAMEUI_INTERFACE_VERSION, NULL);
			if (!gameui)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}

HDTFUI_Root::~HDTFUI_Root()
{
	m_pSwitchHistory.Purge();

	gameui = NULL;
	g_GameUIDLL.Unload();
}

void HDTFUI_Root::PrepareCustom()
{
	// shared canvases (like options, load, etc)
	m_pCanvasOptions = new MainMenu_Options(this, "HDTFMainMenu_Options");
	m_pCanvasOptions->SetZPos(98);
	m_pCanvasOptions->SetVisible(false);

	m_pCanvasLoadGame = new MainMenu_LoadGame(this, "HDTFMainMenu_LoadGame");
	m_pCanvasLoadGame->SetZPos(98);
	m_pCanvasLoadGame->SetVisible(false);

	m_pCanvasSaveGame = new MainMenu_SaveGame(this, "HDTFMainMenu_SaveGame");
	m_pCanvasSaveGame->SetZPos(98);
	m_pCanvasSaveGame->SetVisible(false);

	// canvas transition vars
	m_bInCanvasTransition = false;
	m_flCanvasTransition = 0.f;
	m_pSwitchTarget = NULL;

	// back button
	m_bShowBackButton = false;
	m_pBackButton = new Button(this, "backButton", "#HDTF_MainMenu_Back");
	m_pBackButton->SetCommand("menu_back");
	m_pBackButton->AddActionSignalTarget(this);
	m_pBackButton->SetVisible(m_bShowBackButton);
	m_pBackButton->SetZPos(99);
}

void HDTFUI_Root::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pBackButton->SetFont(pScheme->GetFont("MainMenuButtons", false));
}

void HDTFUI_Root::PerformLayout()
{
	// Resize the panel to the screen size
	// Otherwise, it'll just be in a little corner
	int wide, tall;
	vgui::surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);

	m_pBackButton->SetPos(scheme()->GetProportionalScaledValue(40), tall - (tall / 9));
	m_pBackButton->SetWide(scheme()->GetProportionalScaledValue(100));
	m_pBackButton->SetFgColor(Color(255, 255, 255, 255));
	m_pBackButton->SetSelectedColor(Color(100, 5, 0, 255), Color(0, 0, 0, 0));
	m_pBackButton->SetArmedColor(Color(255, 25, 0, 255), Color(0, 0, 0, 0));
	m_pBackButton->SetPaintBorderEnabled(false);
}

void HDTFUI_Root::OnThink()
{
	BaseClass::OnThink();

	const bool inGame = (engine->IsInGame() || engine->IsConnected()) && !engine->IsLevelMainMenuBackground();

	if (m_bIsInGame != inGame)
	{
		m_bIsInGame = inGame;

		if (!m_bIsInGame)
			((HDTFMainMenu *)mainMenu)->RestartBGVideo();

		mainMenu->SetVisible(!m_bIsInGame);
		pauseMenu->SetVisible(m_bIsInGame);

		if (m_bIsInGame)
			m_pBackButton->SetText("#HDTF_MainMenu_BackNoHelp");
		else
			m_pBackButton->SetText("#HDTF_MainMenu_Back");

		if (m_pActiveCanvas)
		{
			m_pActiveCanvas->SetEnabled(false);
			m_pActiveCanvas->SetVisible(false);

			m_pActiveCanvas = m_bIsInGame ? ((HDTFPauseMenu*)pauseMenu)->GetMainCanvas() : ((HDTFMainMenu*)mainMenu)->GetMainCanvas();
			m_pActiveCanvas->SetEnabled(true);
			m_pActiveCanvas->SetVisible(true);
		}

		m_bInCanvasTransition = false;

		if (m_pSwitchTarget)
		{
			m_pSwitchTarget->SetEnabled(false);
			m_pSwitchTarget->SetVisible(false);
			m_pSwitchTarget = NULL;
		}

		m_pBackButton->SetVisible(false);

		m_pSwitchHistory.Purge();

		return;
	}

	if (m_bInCanvasTransition)
	{
		if (m_pSwitchTarget != NULL && m_flCanvasTransition == 1.f)
		{
			m_pActiveCanvas->SetVisible(false);

			m_pSwitchTarget->SetVisible(true);

			m_pSwitchTarget->OnBeforeSwitchTo();

			// waiting until this panel gets ready
			if (m_pSwitchTarget->IsReady())
			{
				m_pSwitchTarget->RequestFocus();
				m_pActiveCanvas->OnSwitchedFrom();
				m_pActiveCanvas = m_pSwitchTarget;

				m_pActiveCanvas->SetEnabled(true);
				m_pActiveCanvas->OnSwitchedTo();

				m_pActiveCanvas->MoveToFront();
				m_pActiveCanvas->RequestFocus();

				m_pBackButton->SetVisible(m_bShowBackButton);

				m_pSwitchTarget = NULL;
			}
		}
		else if (m_pSwitchTarget == NULL && m_flCanvasTransition == 0.f)
		{
			m_bInCanvasTransition = false;
		}
	}
	else
	{
		if (vgui::input()->IsKeyDown(KEY_ESCAPE) && !m_bBackButtonDown && !inGame)
		{
			m_bBackButtonDown = true;
			SwitchBack();
		}
	}

	m_bBackButtonDown = vgui::input()->IsKeyDown(KEY_ESCAPE);
}

void HDTFUI_Root::PaintTraverse(bool repaint, bool allowForce)
{
	const float alphaMultiplier = surface()->DrawGetAlphaMultiplier();

	if (m_bInCanvasTransition)
	{
		if (m_pSwitchTarget != NULL)
		{
			m_flCanvasTransition = Approach(1.f, m_flCanvasTransition, gpGlobals->absoluteframetime * 3.5f);
		}
		else
		{
			if (m_bInstantSwitching)
			{
				m_bInstantSwitching = false;
				m_bInCanvasTransition = false;
				m_flCanvasTransition = 0.f;
			}
			else
			{
				m_flCanvasTransition = Approach(0.f, m_flCanvasTransition, gpGlobals->absoluteframetime * 3.5f);
			}
		}

		surface()->DrawSetAlphaMultiplier(1.f - m_flCanvasTransition);
	}
	else
	{
		if (m_pActiveCanvas)
		{
			surface()->DrawSetAlphaMultiplier(m_pActiveCanvas->GetAlphaMultiplier());
		}
		else
		{
			surface()->DrawSetAlphaMultiplier(1.f);
		}
	}

	BaseClass::PaintTraverse(repaint, allowForce);

	surface()->DrawSetAlphaMultiplier(alphaMultiplier);
}

// -------------------------------------------------------
// Purpose: processes commands from child elements
// -------------------------------------------------------
void HDTFUI_Root::OnCommand(const char* command)
{
	if (FStrEq(command, "menu_back"))
	{
		SwitchBack();
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

// -------------------------------------------------------
// Purpose: switch to another canvas
// -------------------------------------------------------
bool HDTFUI_Root::SwitchCanvas(HDTFMenuCanvas *target, bool writeHistory)
{
	// already in transition
	if (m_bInCanvasTransition)
		return false;

	if (!target)
		return false;

	if (m_pActiveCanvas == target)
		return false;

	// this panel's not allowing us to leave yet
	if (!m_pActiveCanvas->CanLeave())
		return false;

	m_bInCanvasTransition = true;
	m_pSwitchTarget = target;

	m_pActiveCanvas->SetEnabled(false);

	if (writeHistory)
	{
		m_pSwitchHistory.AddToTail(m_pActiveCanvas);
		m_bShowBackButton = true;
	}

	return true;
}

// -------------------------------------------------------
// Purpose: switch to another canvas instantly
// -------------------------------------------------------
bool HDTFUI_Root::SwitchCanvasInstant(HDTFMenuCanvas *target, bool writeHistory, bool resetHistory)
{
	// already in transition
	if (m_bInCanvasTransition)
		return false;

	if (!target)
		return false;

	if (m_pActiveCanvas == target)
		return false;

	// this panel's not allowing us to leave yet
	if (!m_pActiveCanvas->CanLeave())
		return false;

	m_pSwitchTarget = target;

	m_pActiveCanvas->SetEnabled(false);
	m_pActiveCanvas->SetVisible(false);

	m_pSwitchTarget->SetVisible(true);

	m_pSwitchTarget->RequestFocus();
	m_pActiveCanvas->OnSwitchedFrom();
	m_pActiveCanvas = m_pSwitchTarget;

	m_pActiveCanvas->SetEnabled(true);
	m_pActiveCanvas->OnSwitchedTo();

	m_pActiveCanvas->MoveToFront();
	m_pActiveCanvas->RequestFocus();

	m_pBackButton->SetVisible(m_bShowBackButton);

	m_pSwitchTarget = NULL;

	if (writeHistory)
	{
		m_pSwitchHistory.AddToTail(m_pActiveCanvas);
		m_bShowBackButton = true;
	}
	else if (resetHistory)
	{
		m_pSwitchHistory.Purge();
		m_bShowBackButton = false;
	}

	return true;
}

// -------------------------------------------------------
// Purpose: switch to previous canvas through history
// -------------------------------------------------------
bool HDTFUI_Root::SwitchBack()
{
	if (m_pSwitchHistory.Count() == 0)
		return false;

	if (SwitchCanvas(m_pSwitchHistory.Element(m_pSwitchHistory.Count() - 1), false))
	{
		m_pSwitchHistory.Remove(m_pSwitchHistory.Count() - 1);

		if (m_pSwitchHistory.Count() == 0)
			m_bShowBackButton = false;

		return true;
	}

	return false;
}

void HDTFUI_Root::StartCutscene(bf_read &msg)
{
	char filePath[MAX_PATH];
	char sceneName[MAX_PATH];

	msg.ReadString(sceneName, MAX_PATH);
	bool canSkip = msg.ReadByte();

	Q_strncpy(filePath, "media/", MAX_PATH);
	Q_strncat(filePath, sceneName, MAX_PATH);
	Q_strncat(filePath, ".bik", MAX_PATH);

	if (guiroot->GetGameUI()->IsMainMenuVisible())
	{
		engine->ClientCmd_Unrestricted("cancelselect");
	}

	HDTFCutscenePlayer *player = new HDTFCutscenePlayer();
	if (player->StartPlayback(filePath))
	{
		player->SetSkipAble(canSkip);
		player->TakeControl();
	}
	else
	{
		delete player;
	}
}
