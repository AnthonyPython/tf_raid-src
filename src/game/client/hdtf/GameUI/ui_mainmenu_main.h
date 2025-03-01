#pragma once

#include "ui_mainmenu.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ImagePanel.h"
#include "ui_menu_button.h"

class HDTFImageButton;

class MainMenu_Main : public HDTFMenuCanvas
{
	DECLARE_CLASS_SIMPLE(MainMenu_Main, HDTFMenuCanvas);
public:
	MainMenu_Main(vgui::Panel *parent, const char *name);
	~MainMenu_Main();

	virtual void	OnBeforeSwitchTo();

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	PerformLayout();
	virtual void	Paint();

	void			CheckHasAnySaves();

private:
	int				logoX;
	int				logoY;
	int				logoW;
	int				logoH;

	bool			bHasAnySaves;

	void			DrawLogo();

	CMaterialReference	m_LogoMaterial;

	HDTFMenuButton		*btn_Continue;
	HDTFMenuButton		*btn_NewGame;
	HDTFMenuButton		*btn_LoadGame;
	HDTFMenuButton		*btn_Options;
	HDTFMenuButton		*btn_Quit;

	HDTFImageButton		*btn_SocialTwitter;
	HDTFImageButton		*btn_SocialDiscord;
	HDTFImageButton		*btn_SocialPatreon;
};
