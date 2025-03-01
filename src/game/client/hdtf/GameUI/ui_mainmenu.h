#pragma once

#include "hdtf_ui.h"

#include "vgui_controls/Panel.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/EditablePanel.h"

#include "video/ivideoservices.h"

class HDTFMainMenu : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(HDTFMainMenu, vgui::EditablePanel);
public:
	HDTFMainMenu(vgui::Panel *parent, const char *name);
	~HDTFMainMenu();

	HDTFMenuCanvas	*GetMainCanvas() { return canvas_Main; }
	void			RestartBGVideo();
	void			StopBackgroundVideo() { EndBackgroundVideo(); }

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	OnCommand(const char* command);
	virtual float	GetAlphaMultiplier();
			void	PaintBackground();

			void	StartBackgroundVideo(const char *video);
			void	EndBackgroundVideo();
			void	OnVideoFinished();

			void	CreateCanvas();

			char	*PickVideo();

			void	ActionContinueGame();

			void	OpenWebsiteOverlay(const char* msz_URL);
private:
	// canvases
	HDTFMenuCanvas		*canvas_Main;
	HDTFMenuCanvas		*canvas_NewGame;

	// variables
	int					m_iLoadingBGTexture;
	float				m_flLoadingBGAlpha;
	IVideoMaterial		*m_BGVideoMaterial;
	IMaterial			*m_pBGMaterial;
	float				m_flVideoU;
	float				m_flVideoV;
	char				*m_pVideoPath;
};