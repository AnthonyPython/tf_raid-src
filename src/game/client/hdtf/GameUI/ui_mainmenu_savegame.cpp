#include "cbase.h"
#include "hdtf_ui.h"
#include "ui_mainmenu_loadgame.h"
#include "ui_mainmenu_savegame.h"

#include "vgui/ISurface.h"
#include "vgui/IScheme.h"

using namespace vgui;

MainMenu_SaveGame::MainMenu_SaveGame(vgui::Panel *parent, const char *name) : BaseClass(parent, name)
{ }

void MainMenu_SaveGame::AddToSaveList(int index)
{
	HDTFUI_SaveGameSlot *slot = new HDTFUI_SaveGameSlot(m_pScrollHolder, index);
	slot->SetSaveGame(m_SaveGames[index]);
	slot->AddActionSignalTarget(this);
	slot->SetCommand("SaveGame");
	m_pSaves.AddToTail(slot);
}

void MainMenu_SaveGame::LookForSaves()
{
	BaseClass::LookForSaves();

	HDTFUI_SaveGameSlot *slot = new HDTFUI_SaveGameSlot(m_pScrollHolder, -1);
	slot->SetupAsNewSave();
	slot->AddActionSignalTarget(this);
	m_pSaves.AddToHead(slot);
}
