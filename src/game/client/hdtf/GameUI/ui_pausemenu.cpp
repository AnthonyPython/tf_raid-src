#include "cbase.h"
#include "ui_pausemenu.h"
#include "engine/IEngineSound.h"

#include "vgui/ISurface.h"
#include "vgui/IInput.h"

// main menu slides (canvases)
#include "ui_pausemenu_main.h"
#include "ui_mainmenu_options.h"

#include "ui_menu_popup.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

HDTFPauseMenu::HDTFPauseMenu(Panel *parent, const char *name) : EditablePanel(parent, name)
{
	SetParent(parent);

	CreateCanvas();
}

void HDTFPauseMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);
}

// -------------------------------------------------------
// Purpose: create all canvases and fill them up
// -------------------------------------------------------
void HDTFPauseMenu::CreateCanvas()
{
	canvas_Main = new PauseMenu_Main(this, "canvas_PauseMain");
}

// -------------------------------------------------------
// Purpose: background painting
// -------------------------------------------------------
void HDTFPauseMenu::PaintBackground()
{
	const int w = GetWide();
	const int h = GetTall();

	const float alphaMultiplier = surface()->DrawGetAlphaMultiplier();

	surface()->DrawSetAlphaMultiplier(1.f);

	// ---------------------------------
	// CINEMATIC OVERLAY
	// ---------------------------------
	const int cineOverlayHeight = h / 8;

	surface()->DrawSetColor(0, 0, 0, 255);
	surface()->DrawFilledRect(0, 0, w, cineOverlayHeight);
	surface()->DrawFilledRect(0, h - cineOverlayHeight, w, h);

	surface()->DrawSetColor(0, 0, 0, 200);
	surface()->DrawFilledRect(0, 0, w, h);

	surface()->DrawSetAlphaMultiplier(alphaMultiplier);
}

// -------------------------------------------------------
// Purpose: processes the commands
// -------------------------------------------------------
void HDTFPauseMenu::OnCommand(const char* command)
{
	// main tab switches
	if (FStrEq(command, "menu_resume"))
	{
		guiroot->GetGameUI()->SendMainMenuCommand("ResumeGame");
	}
	else if (FStrEq(command, "menu_savegame"))
	{
		guiroot->SwitchCanvas(guiroot->GetSaveGameCanvas());
	}
	else if (FStrEq(command, "menu_loadgame"))
	{
		guiroot->SwitchCanvas(guiroot->GetLoadGameCanvas());
	}
	else if (FStrEq(command, "menu_options"))
	{
		//guiroot->GetGameUI()->SendMainMenuCommand("OpenOptionsDialog");
		guiroot->SwitchCanvas(guiroot->GetOptionsCanvas());
	}
	else if (FStrEq(command, "menu_return"))
	{
		HDTFUI_Popup *popup = new HDTFUI_Popup(
			this,
			"quit_game_queue",
			"#HDTF_ReturnToMenu_Message",
			POPUP_PROMPT,
			"#HDTF_Menu_Yes",
			"#HDTF_Menu_No",
			"ConfirmReturn");

		popup->AddActionSignalTarget(this);

		popup->TakeControl();
	}
	else if (FStrEq(command, "ConfirmReturn"))
	{
		engine->ClientCmd_Unrestricted("disconnect");
	}
	else if (FStrEq(command, "menu_quit"))
	{
		HDTFUI_Popup *popup = new HDTFUI_Popup(
			this,
			"quit_game_queue",
			"#HDTF_Quit_InGame_Message",
			POPUP_PROMPT,
			"#HDTF_Menu_Yes",
			"#HDTF_Menu_No",
			"ConfirmExit");

		popup->AddActionSignalTarget(this);

		popup->TakeControl();
	}
	else if (FStrEq(command, "ConfirmExit"))
	{
		guiroot->GetGameUI()->SendMainMenuCommand("QuitNoConfirm");
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}
