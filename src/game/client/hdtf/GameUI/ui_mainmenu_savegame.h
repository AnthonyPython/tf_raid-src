#pragma once

#include "vgui_controls/Panel.h"

class MainMenu_SaveGame : public MainMenu_LoadGame
{
	DECLARE_CLASS_SIMPLE(MainMenu_SaveGame, MainMenu_LoadGame);
public:
	MainMenu_SaveGame(vgui::Panel *parent, const char *name);

protected:
	virtual void	AddToSaveList(int saveIndex);
	virtual void	AddNoSavesLabel() { /* do nothing */ }
	void			LookForSaves();
};