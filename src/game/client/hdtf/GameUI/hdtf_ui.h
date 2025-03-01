#pragma once

#include "vgui_controls/Panel.h"
#include "GameUI/IGameUI.h"
#include "ui_menu_canvas.h"
#include "GameUI/ui_mainmenu.h"
#include "GameUI/ui_pausemenu.h"

#define BUTTONS_HEIGHT 22

class HDTFUI_Root : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(HDTFUI_Root, vgui::Panel);
public:
	HDTFUI_Root(vgui::VPANEL parent);
	virtual ~HDTFUI_Root();

	IGameUI*		GetGameUI();

	vgui::Panel		*GetMainCanvas() { return mainMenu; }
	HDTFMenuCanvas	*GetOptionsCanvas() { return m_pCanvasOptions; }
	HDTFMenuCanvas	*GetLoadGameCanvas() { return m_pCanvasLoadGame; }
	HDTFMenuCanvas	*GetSaveGameCanvas() { return m_pCanvasSaveGame; }

	vgui::Panel		*GetPauseMenu() { return pauseMenu; }

	bool			IsSwitching() { return m_bInCanvasTransition; }
	float			GetTransitionValue() { return m_flCanvasTransition; }
	bool			SwitchCanvas(HDTFMenuCanvas *target, bool writeHistory = true);
	bool			SwitchCanvasInstant(HDTFMenuCanvas *target, bool writeHistory = true, bool resetHistory = false);
	bool			SwitchBack();

	// whacks current active canvas and replaces it with given one
	// THIS IS USED ONLY ONCE AT THE GAME START UP - USE SwitchCanvas INSTEAD
	void			ForceActiveCanvas(HDTFMenuCanvas *newActive) { m_pActiveCanvas = newActive; }

	static void		StartCutscene(bf_read &msg);

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	PerformLayout();
	virtual void	OnCommand(const char* command);
	virtual void	OnThink();
			void	PaintTraverse(bool repaint, bool allowForce);

			void	PrepareCustom();

private:
	bool			LoadGameUI();

	int				m_ExitingFrameCount;
	bool			m_bCopyFrameBuffer;

	IGameUI*		gameui;

	bool			m_bIsInGame;
	vgui::Panel		*mainMenu;
	vgui::Panel		*pauseMenu;

	// shared canvases
	HDTFMenuCanvas	*m_pCanvasOptions;
	HDTFMenuCanvas	*m_pCanvasLoadGame;
	HDTFMenuCanvas	*m_pCanvasSaveGame;

	// canvas switching
	bool							m_bInstantSwitching;
	bool							m_bBackButtonDown;
	bool							m_bInCanvasTransition;
	float							m_flCanvasTransition;
	HDTFMenuCanvas					*m_pSwitchTarget;
	HDTFMenuCanvas					*m_pActiveCanvas;
	CUtlVector<HDTFMenuCanvas *>	m_pSwitchHistory;

	// back button
	vgui::Button		*m_pBackButton;
	bool				m_bShowBackButton;
};

extern HDTFUI_Root *guiroot;