#include "cbase.h"
#include "ui_pausemenu_main.h"

#include "vgui/ISurface.h"

using namespace vgui;

PauseMenu_Main::PauseMenu_Main(Panel *parent, const char *name) : HDTFMenuCanvas(parent, name)
{
	SetParent(parent);

	btn_ResumeGame = new HDTFMenuButton(this, "btnResumeGame", "#HDTF_Menu_ResumeGame");
	btn_SaveGame = new HDTFMenuButton(this, "btnLoadGame", "#HDTF_Menu_SaveGame");
	btn_LoadGame = new HDTFMenuButton(this, "btnLoadGame", "#HDTF_Menu_LoadGame");
	btn_Options = new HDTFMenuButton(this, "btnOptions", "#HDTF_Menu_Options");
	btn_Disconnect = new HDTFMenuButton(this, "btnReturn", "#HDTF_Menu_Return");
	btn_Quit = new HDTFMenuButton(this, "btnQuit", "#HDTF_Menu_Quit");

	btn_ResumeGame->SetCommand("menu_resume");
	btn_SaveGame->SetCommand("menu_savegame");
	btn_LoadGame->SetCommand("menu_loadgame");
	btn_Options->SetCommand("menu_options");
	btn_Disconnect->SetCommand("menu_return");
	btn_Quit->SetCommand("menu_quit");
}

void PauseMenu_Main::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void PauseMenu_Main::PerformLayout()
{
	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);

	int width = scheme()->GetProportionalScaledValue(430);

	int btnSpacing = 0; //scheme()->GetProportionalScaledValue(3);
	int btnOffsetX = scheme()->GetProportionalScaledValue(45);
	int btnStartY = 
		scheme()->GetProportionalScaledValue(150) + 
		scheme()->GetProportionalScaledValue(60) + 
		btnSpacing;
	int btnHeight = BUTTONS_HEIGHT;

	btn_ResumeGame->SetPos(btnOffsetX, btnStartY);
	btn_SaveGame->SetPos(btnOffsetX, btnStartY + (btnHeight + btnSpacing) * 1);
	btn_LoadGame->SetPos(btnOffsetX, btnStartY + (btnHeight + btnSpacing) * 2);
	btn_Options->SetPos(btnOffsetX, btnStartY + (btnHeight + btnSpacing) * 4);
	btn_Disconnect->SetPos(btnOffsetX, btnStartY + (btnHeight + btnSpacing) * 6);
	btn_Quit->SetPos(btnOffsetX, btnStartY + (btnHeight + btnSpacing) * 7);

	btn_ResumeGame->SetSize(width, btnHeight);
	btn_SaveGame->SetSize(width, btnHeight);
	btn_LoadGame->SetSize(width, btnHeight);
	btn_Options->SetSize(width, btnHeight);
	btn_Disconnect->SetSize(width, btnHeight);
	btn_Quit->SetSize(width, btnHeight);
}

PauseMenu_Main::~PauseMenu_Main()
{
}
