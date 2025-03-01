#pragma once

#include "ui_mainmenu.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ImagePanel.h"
#include "ui_menu_button.h"

class PauseMenu_Main : public HDTFMenuCanvas
{
	DECLARE_CLASS_SIMPLE(PauseMenu_Main, HDTFMenuCanvas);
public:
	PauseMenu_Main(vgui::Panel *parent, const char *name);
	~PauseMenu_Main();

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	PerformLayout();

private:
	HDTFMenuButton		*btn_ResumeGame;
	HDTFMenuButton		*btn_SaveGame;
	HDTFMenuButton		*btn_LoadGame;
	HDTFMenuButton		*btn_Options;
	HDTFMenuButton		*btn_Disconnect;
	HDTFMenuButton		*btn_Quit;
};
