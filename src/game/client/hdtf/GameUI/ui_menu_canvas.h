#pragma once

#include "vgui_controls/EditablePanel.h"

class HDTFMenuCanvas : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(HDTFMenuCanvas, vgui::EditablePanel);
public:
	HDTFMenuCanvas(vgui::Panel *parent, const char *name) : vgui::EditablePanel(parent, name)
	{
		SetParent(parent);
		m_bIsCanvasReady = true;
	}

	void OnCommand(const char* command)
	{
		// reroute such commands to parent!
		if (strlen(command) > 5 && !memcmp(command, "menu_", 5))
		{
			GetParent()->OnCommand(command);
			return;
		}

		BaseClass::OnCommand(command);
	}

	virtual void OnSwitchedTo() {}
	virtual void OnSwitchedFrom() {}
	virtual void OnBeforeSwitchTo() {}

	virtual bool	CanLeave() { return true; }
	virtual float	GetAlphaMultiplier() { return 1.f; }

	void	SetReady(bool ready) { m_bIsCanvasReady = ready; }
	bool	IsReady() { return m_bIsCanvasReady; }

private:
	bool	m_bIsCanvasReady;
};