#pragma once

#include "vgui_controls/Panel.h"
#include "vgui_controls/Label.h"
#include "GameUI/IGameUI.h"
#include "GameUI/ui_mainmenu.h"

class HDTFUI_Loading : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(HDTFUI_Loading, vgui::Panel);
public:
	HDTFUI_Loading(vgui::VPANEL parent);

	void			PrepareForLoading(const int chapterIndex);
	void			PrepareForLoading(const char *mapName);

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	Paint();

			void	SolveEnginePanel();
			void	PickLoadingScreen();

private:
	IGameUI		*GetGameUI();

	vgui::Label	*m_pLoadingText;
	vgui::Label	*m_pChapterTitle;

	int			m_iBarX;
	int			m_iBarY;
	int			m_iBarW;
	int			m_iBarH;

	bool		m_bHasValidBGTexture;
	int			m_iLoadingBGTexture;

	int			m_iActualChapterIndex;
	int			m_iLastChapterIndex;
};

extern HDTFUI_Loading *guiloading;