#pragma once

#include "vgui_controls/Button.h"

class HDTFMenuButton : public vgui::Button
{
	DECLARE_CLASS_SIMPLE(HDTFMenuButton, vgui::Button);
public:
	HDTFMenuButton(vgui::Panel *parent, const char *name, const char *text);

	void			SetEnabled(bool state);
	virtual void	FireActionSignal();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void Paint();
	virtual void PaintBackground();

	void OnCursorEntered()
	{
		if (!m_bIsHovered)
		{
			m_bIsHovered = true;
			OnHoverStateChanged(true);
		}
	}

	void OnCursorExited()
	{
		if (m_bIsHovered)
		{
			m_bIsHovered = false;
			OnHoverStateChanged(false);
		}
	}

	void OnHoverStateChanged(bool newState);

	bool IsHovered()
	{
		return m_bIsHovered;
	}

private:
	int			m_iGradient;

	bool		m_bIsHovered;
};