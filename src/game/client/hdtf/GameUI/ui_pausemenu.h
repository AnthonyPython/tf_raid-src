#pragma once

#include "hdtf_ui.h"

#include "vgui_controls/Panel.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/EditablePanel.h"

#include "video/ivideoservices.h"

class HDTFPauseMenu : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(HDTFPauseMenu, vgui::EditablePanel);
public:
	HDTFPauseMenu(vgui::Panel *parent, const char *name);

	HDTFMenuCanvas	*GetMainCanvas() { return canvas_Main; }

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	OnCommand(const char* command);
	void			PaintBackground();

	void			CreateCanvas();

private:
	// canvases
	HDTFMenuCanvas		*canvas_Main;
};